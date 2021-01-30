//----------------------------------------------------------------------------
//
//       Copyright (C) 2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Gravity.cpp
//
// Purpose-
//       Gravitational simulator.
//
// Last change date-
//       2021/01/29
//
// Data points-
//       https://nssdc.gsfc.nasa.gov/planetary/factsheet/moonfact.html
//       Orb       Earth     Moon
//  Mean Radius    6.732e6   1.7374e6
//       Mass      5.9724e24 0.07346e24
//
//   Min Dist      4.4140e6  0.3633e9
//  Mean Dist      4.6710e6  0.3844e9
//   Max Dist      4.9270e6  0.4055e9
//
//   Min Vel      12.1311e1* 0.9970e3 [* Earth estimated]
//  Mean Vel      12.4328e1  1.0220e3
//   Max Vel      13.1627e1* 1.0820e3 [* Earth estimated]
//
//       Period             27.3217e1 days
//       Period              2.3606e6 seconds 2,360,594.88
//
// Implementation notes-
//       BUG: Orbits drift in +Y direction.
//       BUG: Moon starts at maximum distance, but at 180 distance greater.
//
//----------------------------------------------------------------------------
#include <array>                    // For std::array
#include <exception>                // For std::exception
#include <memory>                   // For std::shared_ptr, std::unique_ptr
#include <string>                   // For std::string

#include <ctype.h>                  // For isprint, toupper
#include <errno.h>                  // For errno
#include <fcntl.h>                  // For O_* constants
#include <getopt.h>                 // For getopt_long
#include <limits.h>                 // For INT_MAX, INT_MIN
#include <math.h>                   // For sqrt
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <stdarg.h>                 // For va_list
#include <unistd.h>                 // For close, ftruncate
#include <sys/stat.h>               // For stat
#include <sys/types.h>              // For type definitions
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types
#include <xcb/xcb_bitops.h>         // For xcb_host_byte_order

#include <gui/Device.h>             // For gui::Device
#include <gui/Global.h>             // For gui::opt_* controls
#include <gui/Window.h>             // For gui::Window
#include <pub/Debug.h>              // For Debug object

#include "Config.h"                 // For namespace config
#include "Gravity.h"                // For Gravity objects

using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging
using namespace config;             // For opt_* controls

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra bringup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static double          G= 6.67E-11;  // Gravitational constant
static double          delta_t= 1.0; // Time interval in seconds

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static sim::Orbital    root("ROOT"); // The simulation root center of mass
static sim::Orbital    earth_orb("Earth", &root); // The Earth Orbital
static sim::Orbital    moon_orb("Moon", &root);  // The Moon Orbital

static uint32_t        center_color= 0x00FF0000; // Red center of mass
static uint32_t        earth_color= 0x000000FF; // Blue earth
static uint32_t        moon_color=  0x00F0F0F0; // Grey moon
enum { DIM= 5'200 };                 // Array size (Enough for two moon orbits)
static std::array<sim::Pos,DIM> earth_pos; // Earth position array
static std::array<sim::Pos,DIM> moon_pos;  // Moon position array
static uint32_t        pos_ix= 0;    // Current position index
static uint32_t        pos_used= 0;  // Number of positions used
static char            opt_debug[26]= {}; // Switchable debug flag

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
// Struct-
//       XY
//
// Purpose-
//       XY integer container.
//
//----------------------------------------------------------------------------
struct XY {                         // XY integer container
int                    x;           // X value
int                    y;           // Y value
}; // struct XY

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
     sim::Xyz          xyz)
{
   printf(" %s[%.1e,%.1e,%.1e]", name, xyz.x, xyz.y, xyz.z);
}

static inline void
   debug(
     const char*       name,
     sim::Pos          pos)
{  sim::Xyz xyz= {pos.x, pos.y, pos.z}; debug(name, xyz); }

static inline void
   debug(
     const char*       name,
     sim::Vel          vel)
{  sim::Xyz xyz= {vel.x, vel.y, vel.z}; debug(name, xyz); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       d2p
//
// Purpose-
//       Distance to pixel offset
//
//----------------------------------------------------------------------------
static inline int                   // Pixel offset
   d2p(                             // Distance to pixel offset
     double            dist)        // Distance
{  return int(dist / 1000000.0); }  // Today's heuristic

//----------------------------------------------------------------------------
//
// Subroutine-
//       distance
//
// Purpose-
//       Orb to Orb distance
//
//----------------------------------------------------------------------------
static inline double                // Distance (kilometers)
   distance(                        // Distance between
     sim::Orbital&     lhs,         // This orb and
     sim::Orbital&     rhs)         // This orb
{
   double D= (rhs.pos.x-lhs.pos.x) * (rhs.pos.x-lhs.pos.x)
           + (rhs.pos.y-lhs.pos.y) * (rhs.pos.y-lhs.pos.y)
           + (rhs.pos.z-lhs.pos.z) * (rhs.pos.z-lhs.pos.z);
   return sqrt(D);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       force
//
// Purpose-
//       Orb to Orb force
//
//----------------------------------------------------------------------------
static inline sim::Xyz              // Force vector
   force(                           // Force vector between
     sim::Orbital&     lhs,         // This orb and
     sim::Orbital&     rhs)         // This orb
{
   double D= distance(lhs, rhs);    // Distance between the Orbs
   if( D == 0.0)                    // Avoid divide by zero
     D= 1.0;
   double F= (G * lhs.mass * rhs.mass)/(D * D); // Total force

   return { (lhs.pos.x-rhs.pos.x)*F/D
          , (lhs.pos.y-rhs.pos.y)*F/D
          , (lhs.pos.z-rhs.pos.z)*F/D};
}

namespace sim {                     // Simulation object namespace
//----------------------------------------------------------------------------
//
// Method-
//       sim::Window::Window
//       sim::Window::~Window
//
// Purpose-
//       Constructor/Destructor
//
//----------------------------------------------------------------------------
   sim::Window::Window(             // Constructor
     Widget*           parent,      // The parent Widget
     const char*       name,        // The Widget name
     unsigned          width,       // (X) size width
     unsigned          height)      // (Y) size height
:  super(parent, name)
{
   if( opt_hcdm )
     debugh("sim::Window(%p)::Window(%u,%u)\n", this, width, height);

   if( width  < 100 ) width=  100;
   if( height < 100 ) height= 100;
   use_size.width=  gui::WH_t(width);
   use_size.height= gui::WH_t(height);
   min_size= use_size;
}

   sim::Window::~Window( void )     // Destructor
{
   if( opt_hcdm )
     debugh("sim::Window(%s)::~Window\n", this->get_name().c_str());

   // Delete the graphics context
   if( drawGC ) {
     ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, drawGC) );
     drawGC= 0;
   }

   // Delete the Image data
   image_term();

   // Complete any pending operations
   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       sim::Window::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
void
   sim::Window::configure( void )   // Configure the Window
{
   if( opt_hcdm )
     debugh("sim::Window(%p)::configure Named(%s)\n", this, get_name().c_str());

   // Create the Window
   bg= 0x00000000;                  // Black background
   fg= 0x00FF0000;                  // Red foreground

   emask |= XCB_EVENT_MASK_KEY_PRESS;
   emask |= XCB_EVENT_MASK_BUTTON_PRESS;
   emask |= XCB_EVENT_MASK_EXPOSURE;
   emask |= XCB_EVENT_MASK_STRUCTURE_NOTIFY;

   super::configure();
   flush();

   // Create the Graphic Context
   drawGC= xcb_generate_id(c);
   uint32_t mask= XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
   uint32_t parm[2];
   parm[0]= fg;
   parm[1]= bg;
   ENQUEUE("xcb_create_gc", xcb_create_gc(c, drawGC, widget_id, mask, parm) );

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       sim::Window::draw
//
// Purpose-
//       Draw the Window
//
//----------------------------------------------------------------------------
void
   sim::Window::draw( void )        // Draw the Window
{
   if( opt_hcdm )
     debugh("sim::Window(%p)::draw Named(%s)\n", this, get_name().c_str());

   ENQUEUE("xcb_image_put", xcb_image_put
          (c, widget_id, drawGC, &image, 0, 0, 0) );

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       sim::Window::image_draw
//       sim::Window::image_init
//       sim::Window::image_term
//
// Purpose-
//       Draw the image
//       Create the image
//       Delete the image
//
//----------------------------------------------------------------------------
void
   Window::image_draw( void )       // Update the image
{
   XY center= {rect.width / 2, rect.height / 2}; // Screen center pixel

   // Only coded for two-body
   memset(image.base, 0, image.size); // Initialize the allocated storage
// for(unsigned h= 0; h<rect.height; h++) {
//   for(unsigned w= 0; w<rect.width; w++) {
//     xcb_image_put_pixel(&image, w, h, 0);
//   }
// }

   if( pos_used == DIM ) {          // If wrapped
     for(unsigned i= pos_ix; i<DIM; i++) {
       XY
       orb= {center.x + d2p(earth_pos[i].x), center.y + d2p(earth_pos[i].y)};
       if( opt_hcdm )
         printf("[%4d] E[%10.1e,%10.1e,%4d,%4d]  ", i
               , earth_pos[i].x, earth_pos[i].y, orb.x, orb.y);
       if( orb.x >= 0 && orb.y >=0
           && orb.x < rect.width && orb.y < rect.height ) {
         xcb_image_put_pixel(&image, orb.x, orb.y, earth_color);
       }

       orb= {center.x + d2p(moon_pos[i].x), center.y + d2p(moon_pos[i].y)};
       if( opt_hcdm )
         printf("[%4d] M[%10.1e,%10.1e,%4d,%4d]\n", i
               , moon_pos[i].x, moon_pos[i].y, orb.x, orb.y);
       if( orb.x >= 0 && orb.y >=0
           && orb.x < rect.width && orb.y < rect.height ) {
         xcb_image_put_pixel(&image, orb.x, orb.y, moon_color);
       }
     }
   }

   for(unsigned i= 0; i<pos_used; i++) {
     XY
     orb= {center.x + d2p(earth_pos[i].x), center.y + d2p(earth_pos[i].y)};
     if( opt_hcdm )
       printf("[%4d] E[%10.1e,%10.1e,%4d,%4d]  ", i
             , earth_pos[i].x, earth_pos[i].y, orb.x, orb.y);
     if( orb.x >= 0 && orb.y >=0
         && orb.x < rect.width && orb.y < rect.height ) {
       xcb_image_put_pixel(&image, orb.x, orb.y, earth_color);
     }

     orb= {center.x + d2p(moon_pos[i].x), center.y + d2p(moon_pos[i].y)};
     if( opt_hcdm )
       printf("M[%10.1e,%10.1e,%4d,%4d]\n"
             , moon_pos[i].x, moon_pos[i].y, orb.x, orb.y);
     if( orb.x >= 0 && orb.y >=0
         && orb.x < rect.width && orb.y < rect.height ) {
       xcb_image_put_pixel(&image, orb.x, orb.y, moon_color);
     }
   }

   xcb_image_put_pixel(&image, center.x, center.y, center_color);
}

void
   Window::image_init( void )       // Create and initialze the image
{
   // Clean up any prior image
   image_term();

   // Initialize the image buffer
   image.width= rect.width;         // Width, in pixels
   image.height= rect.height;       // Height, in pixels
   image.format= XCB_IMAGE_FORMAT_Z_PIXMAP; // Format type
   image.scanline_pad= 32;          // Scanline pad (bits)
   image.depth= 24;                 // Depth (bits)
   image.bpp= 32;                   // Storage per pixel (bits)
   image.unit= 32;                  // Scanline unit (bits)
   image.plane_mask= 0;             // (Unused here)
   image.byte_order= xcb_host_byte_order(); // Byte order
   image.bit_order=  XCB_IMAGE_ORDER_MSB_FIRST; // Bit order
   image.stride= rect.width * 4;    // Bytes per image row
   image.size= rect.width * rect.height * 4; // Size of image (bytes)
   image.base= malloc(image.size);  // Allocated storage
   image.data= (uint8_t*)image.base;  // (Unused here)
   memset(image.base, 0, image.size); // Initialize the allocated storage
}

void
   Window::image_term( void )       // Clean up the image
{
   if( image.base ) {
     free(image.base);
     image.base= nullptr;
   }
}

//----------------------------------------------------------------------------
// sim::Window::Event handlers
//----------------------------------------------------------------------------
void
   Window::configure_notify(        // Handle this
     xcb_configure_notify_event_t* E) // Configure notify event
{
   if( opt_hcdm )
     debugh("sim::Window(%p)::configure_notify(%d,%d)\n", this
           , E->width, E->height);

   unsigned x= E->width;
   unsigned y= E->height;

   // If size unchanged, do nothing
   if( rect.width == x && rect.height == y ) // If unchanged
     return;                        // Nothing to do

   // Reconfigure and draw the window
   set_size(x, y);
   rect.width=  x;
   rect.height= y;
   draw();
}

void
   Window::expose(                  // Handle this
     xcb_expose_event_t* event)     // Expose event
{
   if( opt_hcdm )
     debugh("sim::Window(%p)::expose %d [%d,%d,%d,%d]\n", this
           , event->count, event->x, event->y, event->width, event->height);

   draw();
}

void
   Window::key_input(               // Handle this
     xcb_keysym_t      key,         // Key input event
     int               state)       // Alt/Ctl/Shift state mask
{  (void)key; (void)state;          // Parameters currently unused

   if( key >= 'a' && key <= 'z' ) {
     key -= 'a';
     opt_debug[key]= !opt_debug[key];
     return;
   }

   if( key >= 'A' && key <= 'Z' ) {
     key -= 'A';
     opt_debug[key]= !opt_debug[key];
     return;
   }

   device->operational= false;      // Any other key terminates
}
}  // namespace sim (Simlation object namespace)

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
// Data points-
//       https://nssdc.gsfc.nasa.gov/planetary/factsheet/moonfact.html
//       Orb       Earth     Moon
//  Mean Radius    6.732e6   1.7374e6
//       Mass      5.9724e24 0.07346e24
//
//   Min Dist      4.4140e6  0.3633e9
//  Mean Dist      4.6710e6  0.3844e9
//   Max Dist      4.9270e6  0.4055e9
//
//   Min Vel      12.1311e1* 0.9970e3 [* Earth estimated]
//  Mean Vel      12.4328e1  1.0220e3
//   Max Vel      13.1627e1* 1.0820e3 [* Earth estimated]
//
//       Period             27.3217e1 days
//       Period              2.3606e6 seconds 2,360,594.88
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   init(int, char**)                // Initialize
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   root.orb_list.fifo(&earth_orb);
   root.orb_list.fifo(&moon_orb);

   moon_orb.mass= 0.07346E24;       // Moon mass, kilograms
   moon_orb.vel.y= -0.9970E3;       // Moon velocity, m/sec [minimum]
   moon_orb.pos.x= .4055E9;         // Moon position, m     [maximum]

   earth_orb.mass= 5.9724E24;       // Earth mass, kilograms
   earth_orb.vel.y= 13.1627;        // Earth velocity, m/sec [maximum]
   earth_orb.pos.x= -4.414E6;       // Earth position, m     [minimum]

   earth_orb.vel.y= 12.1311;        // Earth velocity, m/sec [minimum]
   earth_orb.pos.x= -4.927E6;       // Earth position, m     [maximum]

   gui::opt_hcdm= opt_hcdm;         // gui: Hard Core Debug Mode?
   gui::opt_verbose= opt_verbose;   // gui: debugging verbosity

   return 0;                        // No worries, mate.
}

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
     gui::Device device;            // The base Device
     sim::Window window(&device);   // The Window

     device.configure();
     window.image_init();
     device.draw();
     window.show();
     window.flush();

     double ratio= earth_orb.mass/(earth_orb.mass + moon_orb.mass);

     // Run the simulation
//   double time= 0.0;              // The current time
     for(size_t interval= 0; interval<3'000'000'000; interval++) {
       if( uint32_t(interval % 3'600) == 0 ) { // Every hour
         sim::Orbital& E= earth_orb;
         sim::Orbital& M= moon_orb;

#if 0    // Center of Mass adjustment (Doesn't change anything)
         sim::Pos com= {M.pos.x-E.pos.x, M.pos.y-E.pos.y, M.pos.z-E.pos.z};
         double D= sqrt(com.x*com.x + com.y*com.y + com.z*com.z);
         com= {com.x*ratio/D, com.y*ratio/D, com.z*ratio/D};
         E.pos.x -= com.x;
         E.pos.y -= com.y;
         E.pos.z -= com.z;
         M.pos.x -= com.x;
         M.pos.y -= com.y;
         M.pos.z -= com.z;
         if( opt_debug['d' - 'a'] ) {
           debug("COM", com);
           debug("E", E.pos);
           debug("M", M.pos);
           printf("\n");
         }
#else
(void)ratio;
#endif

         if( opt_debug['p' - 'a'] )
           printf("%10zd D(%.1f) E[%.1f,%.1f,%.1f,%.1f] M[%.1f,%.1f,%.1f,%.1f]\n"
                 , interval, distance(E, M)
                 , E.pos.x, E.pos.y, E.vel.x, E.vel.y
                 , M.pos.x, M.pos.y, M.vel.x, M.vel.y);

         // Draw current position
         if( pos_used < DIM )
           ++pos_used;
         earth_pos[pos_ix]= E.pos;
         moon_pos[pos_ix]= M.pos;
         ++pos_ix;

static int once= true;
if( once && pos_used >= DIM ) {
  once= false;
  printf("DIM REACHED\n");
  fflush(stdout);
}

         window.image_draw();
         window.draw();
         window.flush();

         if( pos_ix >= DIM )
           pos_ix= 0;

         for(xcb_generic_event_t* e= device.poll(); e; e= device.poll()) {
           if( opt_hcdm )
             printf("Event(%p)\n", e);
           device.handle_event(e);
           free(e);
         }
         if( !device.operational )
           break;
       }
//     printf("%8zd\n", interval);

       // Calculate new positions and velocities
       sim::Orbital& E= earth_orb;
       sim::Orbital& M= moon_orb;
       double T= delta_t;

       sim::Xyz F= force(M, E);
       sim::Xyz A= {F.x/E.mass, F.y/E.mass, F.z/E.mass};
       E.pos.x += (E.vel.x * T) + 0.5*A.x*T*T;
       E.pos.y += (E.vel.y * T) + 0.5*A.y*T*T;
       E.pos.z += (E.vel.z * T) + 0.5*A.z*T*T;

       E.vel.x += A.x * delta_t;
       E.vel.y += A.y * delta_t;
       E.vel.z += A.z * delta_t;

       A= {F.x/M.mass, F.y/M.mass, F.z/M.mass};
       M.pos.x += (M.vel.x * T) - 0.5*A.x*T*T;
       M.pos.y += (M.vel.y * T) - 0.5*A.y*T*T;
       M.pos.z += (M.vel.z * T) - 0.5*A.z*T*T;

       M.vel.x -= A.x * delta_t;
       M.vel.y -= A.y * delta_t;
       M.vel.z -= A.z * delta_t;

//     if( (interval % 3600) == 0 ) {
//       printf("%6.2f ", double(interval) / 86400.0);
//       debug("F", F); debug("A", A); printf("\n");
//       printf("%6.2f ", double(interval) / 86400.0);
//       debug("E.pos", E.pos); debug("E.vel", E.vel); printf("\n");
//       printf("%6.2f ", double(interval) / 86400.0);
//       debug("M.pos", M.pos); debug("M.vel", M.vel); printf("\n");
//     }
     }
   } catch(pub::Exception& X) {
     debugf("%s\n", std::string(X).c_str());
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
