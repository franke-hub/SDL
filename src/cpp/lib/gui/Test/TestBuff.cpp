//----------------------------------------------------------------------------
//
//       Copyright (C) 2021-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestBuff.cpp
//
// Purpose-
//       Testcase: Test ~/src/cpp/inc/gui/Buffer.h
//
// Last change date-
//       2023/05/01
//
//----------------------------------------------------------------------------
#include <exception>                // For std::exception
#include <string>                   // For std::string

#include <ctype.h>                  // For isprint, toupper
#include <errno.h>                  // For errno
#include <fcntl.h>                  // For O_* constants
#include <getopt.h>                 // For getopt_long
#include <limits.h>                 // For INT_MAX, INT_MIN
#include <semaphore.h>              // For sem_open, sem_close
#include <stdarg.h>                 // For va_list
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <unistd.h>                 // For close, ftruncate
#include <sys/mman.h>               // For mmap, shm_open, ...
#include <sys/stat.h>               // For S_* constants
#include <sys/signal.h>             // For signal, SIGINT, SIGSEGV
#include <sys/types.h>              // For type definitions
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types
#include <xcb/xcb_image.h>          // For xcb_image_t, associated functions

#include <gui/Device.h>             // For gui::Device
#include <gui/Keysym.h>             // For X11 keysymdef.h macros
#include <gui/Window.h>             // For gui::Window
#include <pub/Debug.h>              // For Debug object
#include <pub/Exception.h>          // For pub::Exception
#include <pub/utility.h>            // For utility::dump

#include <Config.h>                 // For namespace config
#include <Tester.h>                 // For Tester (window)

#include "gui/Buffer.h"             // For gui::Buffer (Test class)

using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging
using namespace config;             // For opt_* controls

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= true                // Extra bringup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
static int             opt_index;   // Option index

static const char*     OSTR= ":";   // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",    no_argument,       &opt_help,    true} // --help
,  {"hcdm",    no_argument,       &opt_hcdm,    true} // --hcdm

,  {"test",    required_argument, nullptr,      0} // --test {required}
,  {"verbose", optional_argument, &opt_verbose, 0} // --verbose {optional}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_HELP
,  OPT_HCDM

,  OPT_TEST
,  OPT_VERBOSE
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug
//
// Purpose-
//       (Overloaded) debugging display
//
//----------------------------------------------------------------------------
static inline void
   debug(
     const char*       name,
     xcb_image_t&      image)
{
   debugf("%s(%p) [%u,%u]\n", name, &image, image.width, image.height);
   debugf("..format(%d) pad(%d) depth(%d) bpp(%d) unit(%d)\n", image.format
         , image.scanline_pad, image.depth, image.bpp, image.unit);
   debugf("..plane_mask(%d) byte_order(%d) bit_order(%d) stride(%d)\n"
         , image.plane_mask, image.byte_order, image.bit_order, image.stride);
   debugf("..size(%u) base(%p) data(%p)\n"
         , image.size, image.base, image.data);
// pub::utility::dump(image.base, image.size);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   init(int, char**)                // Initialize
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{  return 0; }                      // Placeholder

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       terminate
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{  /* placeholder */ }

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter description.
//
//----------------------------------------------------------------------------
static int                          // Return code (Always 1)
   info( void)                      // Parameter description
{
   fprintf(stderr, "%s <options> ...\n"
                   "Test Window\n\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --hcdm\tHard Core Debug Mode\n"

                   "  --test=T\tSelect test T\n" // (For expansion)
                   "  --verbose\t{=n} Verbosity, default 0\n"
                   , __FILE__
          );

   return 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       to_integer
//
// Purpose-
//       Convert string to integer, handling error cases
//
// Implementation note-
//       Leading or trailing blanks are NOT allowed.
//
//----------------------------------------------------------------------------
static int                          // The integer value
   to_integer(                      // Extract and verify integer value
     const char*       inp)         // From this string
{
   errno= 0;
   char* strend;                    // Ending character
   long value= strtol(inp, &strend, 0);
   if( strend == inp || *inp == ' ' || *strend != '\0' )
     errno= EINVAL;
   else if( value < INT_MIN || value > INT_MAX )
     errno= ERANGE;

   return int(value);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm_int
//
// Purpose-
//       Convert parameter to integer, handling error cases
//
// Implementation note-
//       optarg: The argument string
//       opt_index: The argument index
//
//----------------------------------------------------------------------------
static int                          // The integer value
   parm_int( void )                 // Extract and verify integer value
{
   int value= to_integer(optarg);
   if( errno ) {
     opt_help= true;
     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n", OPTS[opt_index].name, optarg);
     else if( *optarg == '\0' )
       fprintf(stderr, "--%s, no value specified\n", OPTS[opt_index].name);
     else
       fprintf(stderr, "--%s, format error: '%s'\n", OPTS[opt_index].name, optarg);
   }

   return value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static int                          // Return code (0 if OK)
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   int C;                           // The option character
   while( (C= getopt_long(argc, argv, OSTR, OPTS, &opt_index)) != -1 )
   {
     switch( C )
     {
       case 0:
       {{{{
         switch( opt_index )
         {
           case OPT_HELP:           // These options handled by getopt
           case OPT_HCDM:
             break;

           case OPT_TEST:
             opt_test= optarg;
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= parm_int();
             break;

           default:
             fprintf(stderr, "%4d Unexpected opt_index(%d)\n", __LINE__,
                             opt_index);
             break;
         }
         break;
       }}}}

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Option requires an argument '%s'.\n", __LINE__,
                           argv[optind-1]);
         else
           fprintf(stderr, "%4d Option requires an argument '-%c'.\n", __LINE__,
                           optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Unknown option '%s'.\n", __LINE__,
                           argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "%4d Unknown option '-%c'.\n", __LINE__, optopt);
         else
           fprintf(stderr, "%4d Unknown option character '0x%x'.\n", __LINE__,
                           (optopt & 0x00ff));
         break;

       default:
         fprintf(stderr, "%4d ShouldNotOccur ('%c',0x%x).\n", __LINE__,
                         C, (C & 0x00ff));
         break;
     }
   }

   // Return sequence
   int rc= 0;
   if( opt_help )
     rc= info();
   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       wait
//
// Purpose-
//       Wait for screen keypress
//
//----------------------------------------------------------------------------
static void
   wait(                            // Wait for screen keypress
     Tester&           window)      // On this Window
{
   usleep(125000);                  // Delay
   while(window.device->operational) {
     xcb_generic_event_t* e= xcb_poll_for_event(window.c);
     if( e == nullptr )
       break;

     window.device->handle_event(e);
     free(e);
   }

   int was= window.key_debug[' '];
   printf("Type screen space key to continue\n");
   while(window.device->operational) {
     xcb_generic_event_t* e= xcb_wait_for_event(window.c);
     if( e ) {
       window.device->handle_event(e);
       free(e);
     }

     if( window.key_debug[' '] != was )
       return;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   int rc= parm(argc, argv);        // Argument analysis
   if( rc ) return rc;              // Return if invalid

   rc= init(argc, argv);            // Initialize
   if( rc ) return rc;              // Return if invalid

   //-------------------------------------------------------------------------
   // Mainline code: Load files
   //-------------------------------------------------------------------------
   try {
     gui::Buffer buffer;            // The test Buffer
     gui::Device device;            // The base Device
     Tester window(&device);        // The Window
     unsigned size= 800;
     window.use_size.width=  size;
     window.use_size.height= size;
     window.min_size= window.use_size;
     buffer.resize(size,size);

     // Initial buffer
     buffer.clear(0x00ffffE0);
     for(unsigned y= 0; y<buffer.height; y++) {
       buffer.put_xy(y, y, 0x007fbfff);
       for(unsigned x= 0; x<buffer.width; x++) {
         buffer.put_xy(x, size-1, 0x007fbfff);
         buffer.put_xy(size-1, y, 0x007fbfff);
         buffer.put_xy(x,      0, 0x007fbfff);
         buffer.put_xy(0,      y, 0x007fbfff);
       }
     }

     // Initial window
     device.configure();
     device.draw();
     window.show();
     window.flush();
     wait(window);

     // Buffer window
//   debug("Image", buffer.image);
     xcb_expose_event_t event= {};
     event.x= 0;
     event.y= 0;
     event.width= size;
     event.height= size;
     buffer.expose(&window, window.drawGC, &event);
//   wait(window);

     // Black box across image (V1)
     buffer.expose(&window, window.drawGC, &event);
     gui::Buffer box(10, 10, 0);    // A black box
     for(unsigned x= 0; x<(size-10); x+=20) {
       xcb_image_put(window.c, window.widget_id, window.drawGC
                    , &box.image, x, 40, 0);
       window.flush();
       usleep(500000);
     }
//   wait(window);

     // Black box across image (V2)
     buffer.expose(&window, window.drawGC, &event);
     window.flush();
     event.y= 60;
     event.width= 10;
     event.height= 10;
     for(unsigned x= 0; x<(size-10); x+=20) {
       event.x= x;
       for(unsigned xx= x; xx<x+10; xx++) {
         for(unsigned yy= event.y; int(yy)<event.y+10; yy++) {
           buffer.put_xy(xx, yy, 0x00FF0000);
         }
       }
       buffer.expose(&window, window.drawGC, &event);
       window.flush();
       usleep(500000);
     }
     wait(window);
   } catch(pub::Exception& X) {
     printf("%s\n", std::string(X).c_str());
   } catch(std::exception& X) {
     printf("std::exception.what(%s))\n", X.what());
   } catch(const char* X) {
     printf("catch(const char* '%s')\n", X);
   } catch(...) {
     printf("catch(...)\n");
   }

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   term();                          // Termination cleanup
   printf("Completed\n");
   return 0;
}
