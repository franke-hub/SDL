//----------------------------------------------------------------------------
//
//       Copyright (c) 2011-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Raw2Map.cpp
//
// Purpose-
//       Graphical raw floating point file mapping.
//
// Last change date-
//       2021/07/13
//
// Input-
//       stdin: A raw floating point file, 256x256.
//         (Terminal input not supported.)
//
// Output-
//       Graphic map of values.
//
//----------------------------------------------------------------------------
#include <stdarg.h>                 // For va_list
#include <math.h>                   // For sqrt
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <sys/stat.h>               // For struct stat
#include <xcb/xcb_image.h>          // For xcb_image_t, associated functions
#include <xcb/xproto.h>             // For xcb prototypes

#include <gui/Buffer.h>             // For gui::Buffer
#include <gui/Device.h>             // For gui::Device
#include <gui/Types.h>              // For gui types
#include <gui/Window.h>             // For gui::Window
using namespace gui;

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
typedef gui::Pixel_t   Color_t;     // Pixel color

static float           height[256][256]; // The input data array
static float           MAX_HEIGHT;  // Maximum height
static float           MIN_HEIGHT;  // Minimum height

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
class Image : public gui::Window {  // Our Image Window
public:
typedef gui::Window    Super;       // Our super-class

Buffer                 buffer;      // The Window Buffer
xcb_gcontext_t         drawGC= 0;   // The default graphics context

virtual
   ~Image( void )
{
   if( drawGC ) {                   // Delete graphics context, if any
     ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, drawGC) );
     drawGC= 0;
   }

   flush();
}

   Image(
     Widget*           device)      // The Device
:  Super(device, "Image"), buffer(256,256)
{
   use_size.width= 256;
   use_size.height= 256;
   min_size= use_size;
}

virtual void
   configure( void )                // Configure the Image Window
{
   bg= 0x00000000;
   fg= 0x00ffffff;

   emask |= XCB_EVENT_MASK_KEY_PRESS;
   emask |= XCB_EVENT_MASK_EXPOSURE;

   Super::configure();

   // Create the graphics context
   drawGC= xcb_generate_id(c);
   uint32_t mask= XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
   uint32_t parm[]= {fg, bg};
   ENQUEUE("xcb_create_gc", xcb_create_gc(c, drawGC, widget_id, mask, parm) );
   flush();
}

virtual void
   draw( void )                     // Draw the Image Window
{
   xcb_expose_event_t e;
   e.x= e.y= 0;
   e.width= e.height= 256;
   buffer.expose(this, drawGC, &e);
}

virtual void
   expose(xcb_expose_event_t*)     // Handle expose event
{  draw(); }
}; // class Image

//----------------------------------------------------------------------------
//
// Subroutine-
//       error
//
// Purpose-
//       Write a message onto stderr
//
//----------------------------------------------------------------------------
static void
   error(                           // Write error message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter fault exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter fault exit
{
   error("Raw2Map <filename\n\n"
         "Converts a raw input file into a graphic map.\n"
         );
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   int                 ERROR;       // Error encountered indicator
   int                 HELPI;       // Help encountered indicator
   int                 verify;      // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   ERROR= false;                    // Set defaults
   HELPI= false;
   verify= false;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       if( strcmp("-help", argp) == 0 )  // If help request
         HELPI= true;

       else if( strcmp("-verify", argp) == 0 )
         verify= true;

       else                         // If invalid switch
       {
         ERROR= true;
         error("Invalid parameter '%s'\n", argv[argi]);
       }
     }
     else                           // If filename parameter
     {
       ERROR= true;
       error("Unexpected file name '%s'\n", argv[argi]);
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( !HELPI )
   {
     struct stat s;
     int fn= fileno(stdin);
     int rc= fstat(fn, &s);
     if( rc != 0 )
     {
       ERROR= true;
       error("ERROR: %d= stat(STDIN), ", rc);
       perror("stat(STDIN)");
     }
   }

   if( HELPI || ERROR )
   {
     if( ERROR )
       error("\n");

     info();
   }

   if( verify )
   {
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       load
//
// Purpose-
//       Load the raw input file
//
//----------------------------------------------------------------------------
static void
   load( void )                     // Load the raw input file
{
   int inpSize= 256;
   float* line= (float*)malloc(inpSize * sizeof(float));
   for(int i= 0; i<256; i++)
   {
     int L= fread(line, sizeof(float), inpSize, stdin);
     if( L <= 0 )
       break;

     if( L != inpSize )
     {
       error("Error: File(STDIN), read error(%d) %d\n", L, inpSize);
       break;
     }

     for(int j= 0; j<256; j++)
       height[j][i]= line[j];
   }

   MAX_HEIGHT= height[0][0];
   MIN_HEIGHT= height[0][0];
   for(int i= 0; i<256; i++)
   {
     for(int j= 0; j<256; j++)
     {
       if( height[i][j] > MAX_HEIGHT )
         MAX_HEIGHT= height[i][j];

       if( height[i][j] < MIN_HEIGHT )
         MIN_HEIGHT= height[i][j];
     }
   }

   free(line);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       h2c
//
// Purpose-
//       Convert height to Color_t
//
//----------------------------------------------------------------------------
static Color_t                      // Resultant color
   h2c(                             // Convert height to color
     float             h)           // Height
{
   Color_t result= 0x0000000;

   if( h > 16.0 )                   // Below this is black
   {
     if( h < 20.0 )                 // Black to light blue
     {
       float f= (h - 16.0) / 4.0;
       f *= 256.0;
       int c= (int)f;

       result= c;
     }
     else if( h < 30.0 )            // Light green to red
     {
       float f= (h - 20.0) / 10.0;
       f *= 256.0;
       int c= (int)f;
       int i= 255 - c;

       result= (c<<16) | (i<<8);
     }
     else if( h < 40.0 )            // Red to yellow
     {
       float f= (h - 30.0) / 10.0;
       f *= 256.0;
       int c= (int)f;

       result= 0x00ff0000 | (c<<8);
     }
     else if( h < 140.0 )           // Yellow to white
     {
       float f= (h - 40.0) / 100.0;
       f *= 256.0;
       int c= (int)f;

       result= 0x00ffff00 | c;
     }
     else                           // White
       result= 0x00ffffff;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       show
//
// Purpose-
//       Display the input data
//
//----------------------------------------------------------------------------
static void
   show( void )                     // Display the raw data
{
   Device device;                   // The Device
   Image  window(&device);          // The Image Window
   device.configure();              // Initialize
   device.draw();

   Buffer& buffer= window.buffer;   // Use Image Window Buffer
   for(unsigned y= 0; y<256; y++)
   {
     for(unsigned x= 0; x<256; x++)
     {
       buffer.put_xy(x, y, h2c(height[x][y]));
     }
   }

   if( false )
   {
     for(int i= 0; i<256; i++)
     {
       buffer.put_xy(0,i,0x00ff0000);
       buffer.put_xy(i,0,0x00ff0000);
       buffer.put_xy(255,i,0x00ff0000);
       buffer.put_xy(i,255,0x00ff0000);
     }
   }

   if( false )
   {
     for(int i= 0; i<256; i++)
     {
       for(int j= 0; j<256; j++)
       {
         if( j != 0 )
           printf(" ");
         printf("%5.1f", height[i][j]);
       }

       printf("\n");
     }
   }

   if( false )
   {
     for(float x= 16; x<64.0; x += 4)
       printf("%3.0f 0x%.6x\n", x, h2c(x));
   }

   window.draw();
   window.show();
   window.flush();
   device.run();
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
   parm(argc, argv);                // Parameter analysis
   load();                          // Load the data
   show();                          // Show the data

   return EXIT_SUCCESS;
}

