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
//       2021/02/04
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
//   Min Vel      11.529e0*  0.9970e3 [* Earth estimated]
//  Mean Vel      11.850e0*  1.0220e3 [* Earth estimated]
//   Max Vel      12.145e0*  1.0820e3 [* Earth estimated]
//
//       Period               27.3217 days
//       Period             2,360,600 seconds
//
// Implementation notes-
//       Earth's default starting velocity and position were adjusted to
//       reduce earth/moon barycenter movement.
//
//       With delta_t == 1.0, the moon's orbit min/max position increases by
//       about 5km per orbit. The orbit's duration also increases by about 60
//       seconds per orbit. This is an algorithmic error which can be halved
//       each time delta_t is halved, up to some undetermined limit.
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
#include <gui/Keysym.h>             // For X11 keysymdef.h macros
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
,  USE_EARTH_POS= true              // Use Earth min/max positions?
,  USE_MOON_POS= !USE_EARTH_POS     // Use Moon min/max positions?
}; // Compilation controls

enum { DIM= 5'300 };                // Array size (Enough for 8+ E/M orbits)

//-------------------------------------------------------------------------
// Compile-time controls
//-------------------------------------------------------------------------
enum
{ COM_NONE= 0                       // Do not adjust for COM
, COM_ORB= 1                        // Adjust every orbit
, COM_HOUR= 2                       // Adjust every hour
, COM_INT= 3                        // Adjust every interval
};
static int             USE_COM= COM_HOUR; // When to adjust mass center
static double          USE_CAF= 1.500;    // Mass center adjustment factor

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static double          RUN_TIME= 3'000'000'000; // Run time, simulated seconds
static double          delta_t= 0.25; // Time interval in seconds
static double          G= 6.6743015E-11;  // Gravitational constant

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static sim::Com        root("ROOT"); // The simulation root center of mass
static sim::Orb        earth("Earth", &root); // The Earth Orbital
static sim::Orb        moon("Moon", &root);  // The Moon Orbital

static std::array<sim::Pos,DIM> E_pos; // Earth position array
static std::array<sim::Pos,DIM> M_pos; // Moon position array
static uint32_t        pos_ix= 0;    // Current position index
static uint32_t        pos_used= 0;  // Number of positions used

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
//       d2p
//
// Purpose-
//       Distance to pixel offset
//
//----------------------------------------------------------------------------
static inline int                   // Pixel offset
   d2p(                             // Distance to pixel offset
     double            dist)        // Distance (meters)
{  return int(dist / 1000000.0); }  // Today's heuristic

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
{  printf(" %s[%10.3e,%10.3e,%10.3e]", name, xyz.x, xyz.y, xyz.z); }

static inline void
   debug(
     const char*       name,
     xcb_image_t&      image)
{
   printf("%s(%p) [%u,%u]\n", name, &image, image.width, image.height);
   printf("..format(%d) pad(%d) depth(%d) bpp(%d) unit(%d)\n", image.format
         , image.scanline_pad, image.depth, image.bpp, image.unit);
   printf("..plane_mask(%d) byte_order(%d) bit_order(%d) stride(%d)\n"
         , image.plane_mask, image.byte_order, image.bit_order, image.stride);
   printf("..size(%u) base(%p) data(%p)\n"
         , image.size, image.base, image.data);
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
     sim::Orb&         lhs,         // This orb and
     sim::Orb&         rhs)         // This orb
{
   double D= lhs.pos.mag(rhs.pos);  // Distance between the Orbs
   if( D < 1.0)                     // Avoid divide by zero
     D= 1.0;
   double F= (G * lhs.mass * rhs.mass)/(D * D); // Total force

   return { (rhs.pos.x-lhs.pos.x)*F/D
          , (rhs.pos.y-lhs.pos.y)*F/D
          , (rhs.pos.z-lhs.pos.z)*F/D};
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
   Window::Window(                  // Constructor
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

   center_x= rect.width / 2;        // Center of screen
   center_y= rect.height / 2;

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
// Implementation note-
//       Only earth/moon coding.
//
//----------------------------------------------------------------------------
void
   Window::image_draw( void )       // Update the image
{
   // Only coded for two-body
   memset(image.base, 0, image.size); // Initialize to black

   for(unsigned i= pos_ix+1; i<pos_used; i++) {
     Pos& E= E_pos[i];
     XY orb= {center_x + d2p(E.x), center_y - d2p(E.y)};
     if( opt_hcdm )
       printf("[%4d] E[%10.1e,%10.1e,%4d,%4d]  ", i, E.x, E.y, orb.x, orb.y);
     if( orb.x >= 0 && orb.y >=0
         && orb.x < rect.width && orb.y < rect.height ) {
       xcb_image_put_pixel(&image, orb.x, orb.y, earth.color);
     }

     Pos& M= M_pos[i];
     orb= {center_x + d2p(M.x), center_y - d2p(M.y)};
     if( opt_hcdm )
       printf("[%4d] M[%10.1e,%10.1e,%4d,%4d]\n", i
             , M.x, M.y, orb.x, orb.y);
     if( orb.x >= 0 && orb.y >=0
         && orb.x < rect.width && orb.y < rect.height ) {
       xcb_image_put_pixel(&image, orb.x, orb.y, moon.color);
     }
   }

   for(unsigned i= 0; i<pos_ix; i++) {
     Pos& E= E_pos[i];
     XY orb= {center_x + d2p(E.x), center_y - d2p(E.y)};
     if( opt_hcdm )
       printf("[%4d] E[%10.1e,%10.1e,%4d,%4d]  ", i, E.x, E.y, orb.x, orb.y);
     if( orb.x >= 0 && orb.y >=0
         && orb.x < rect.width && orb.y < rect.height ) {
       xcb_image_put_pixel(&image, orb.x, orb.y, earth.color);
     }

     Pos& M= M_pos[i];
     orb= {center_x + d2p(M.x), center_y - d2p(M.y)};
     if( opt_hcdm )
       printf("M[%10.1e,%10.1e,%4d,%4d]\n"
             , M.x, M.y, orb.x, orb.y);
     if( orb.x >= 0 && orb.y >=0
         && orb.x < rect.width && orb.y < rect.height ) {
       xcb_image_put_pixel(&image, orb.x, orb.y, moon.color);
     }
   }

   {
     Pos& E= earth.pos;
     XY orb= {center_x + d2p(E.x), center_y - d2p(E.y)};
     if( opt_hcdm )
       printf("E[%10.1e,%10.1e,%4d,%4d]  ", E.x, E.y, orb.x, orb.y);
     if( orb.x >= 0 && orb.y >=0
         && orb.x < rect.width && orb.y < rect.height ) {
       xcb_image_put_pixel(&image, orb.x, orb.y, earth.color);
     }

     Pos& M= moon.pos;
     orb= {center_x + d2p(M.x), center_y - d2p(M.y)};
     if( opt_hcdm )
       printf("M[%10.1e,%10.1e,%4d,%4d]\n", M.x, M.y, orb.x, orb.y);
     if( orb.x >= 0 && orb.y >=0
         && orb.x < rect.width && orb.y < rect.height ) {
       xcb_image_put_pixel(&image, orb.x, orb.y, moon.color);
     }
   }

   xcb_image_put_pixel(&image, center_x, center_y, root.color);
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

   // Initialize the pixel buffer
   pixel= image;                    // Copy the image buffer

   // One pixel image buffer
   pixel= image;                    // Start with image
   pixel.width= 1;
   pixel.height= 1;
   pixel.stride= 4;
   pixel.size= 4;
   pixel.base= &pdata;
   pixel.data= (uint8_t*)pixel.base;
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
//
// Method-
//       sim::Window::put_xy
//
// Purpose-
//       Draw one pixel on screen
//
//----------------------------------------------------------------------------
void
   Window::put_xy(                  // Draw Pixel at location
     int               x,           // X (Width) index  (from left)
     int               y,           // Y (Height) index (from top)
     uint32_t          p)           // The Pixel to draw
{
   if( key_debug['x'] )
     debugh("sim::Window(%p)::put_xy(%4d,%4d,%.6x)\n", this, x, y, p);

   if( x >= 0 && y >=0 && x < rect.width && y < rect.height ) {
     pdata= p;                      // Set the Pixel data
     ENQUEUE("xcb_image_put", xcb_image_put
            (c, widget_id, drawGC, &pixel, x, y, 0));
   }
}

void
   Window::put_xy(                  // Draw Pixel at location
     int               x,           // X (Width) index  (from left)
     int               y)           // Y (Height) index (from top)
{
   if( x >= 0 && y >=0 && x < rect.width && y < rect.height ) {
     pdata= xcb_image_get_pixel(&image, x, y);
     if( key_debug['H'] )
       printf("sim::Window(%p)::put_xy(%4d,%4d) %.6x\n", this, x, y, pdata);

     ENQUEUE("xcb_image_put", xcb_image_put
            (c, widget_id, drawGC, &pixel, x, y, 0));
   } else if( key_debug['H'] )
     printf("sim::Window(%p)::put_xy(%d,%d) RANGE\n", this, x, y);

#if 0 // Single-shot debugging info
static int once= true;
if( once ) {
  once= false;
  ::debug("image", image); printf("\n");
  ::debug("pixel", pixel); printf("..pdata(%p) %.6x\n", &pdata, pdata);
}
#endif
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

   if( key == XK_Shift_L || key == XK_Shift_R )
     return;                        // Ignore shift keys

   if( key > 0 &&  key < 128 ) {
     key_debug[key]= !key_debug[key];
     if( key != ' ' )               // Space key is silent
       printf("key_debug[%c] %s\n", key, key_debug[key] ? "ON" : "OFF");
     opt_hcdm= key_debug['H'];
     return;
   }

   if( key == XK_Return )
     device->operational= false;    // Return key terminates
}

//----------------------------------------------------------------------------
//
// Method-
//       sim::Xyz::fr_string
//
// Purpose-
//       Update values from string
//
//----------------------------------------------------------------------------
int
   Xyz::fr_string(                  // Update values from
     const char*       text)        // This text string
{
   if( opt_hcdm )
     debugh("sim::Xyz(%p)::fr_string(%s)\n", this, text);

   while( *text == ' ' )            // Skip leading space
     text++;

   for(int i= 0; i<3; i++) {
     if( *text == '\0' )
       return 0;

     if( *text == ',' ) {
       text++;
       continue;
     }

     char* last= nullptr;
     double V= strtod(text, &last); // Convert value
     if( last == text )             // If invalid
       return 1;
     if( *last != ' ' && *last != ',' && *last != '\0' )
       return 1;

     text= last;
     if( i == 0 )
       x= V;
     else if( i == 1 )
       y= V;
     else {
       z= V;
       if( *text != '\0' )
         return 1;
       break;
     }

     while( *text == ' ' )          // Skip trailing space
       text++;
     if( *text == ',' )             // Skip comma
       text++;
   }

   return 0;
}
}  // namespace sim (Simulation object namespace)

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
//   Min Dist      4.414e6   0.3633e9
//  Mean Dist      4.671e6   0.3844e9
//   Max Dist      4.927e6   0.4055e9
//
//   Min Vel      11.529e0*  0.997e3   [* Earth estimated]
//  Mean Vel      11.850e0*  1.022e3   [* Earth estimated average]
//   Max Vel      12.145e0*  1.082e3   [* Earth estimated]
//
//       Period             27.3217e1 days
//       Period              2.3606e6 seconds 2,360,594.88
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   init(                            // Initialize
     int               argc,        // Argument count (Unused)
     char*             argv[])      // Argument array (Unused)
{
   gui::opt_hcdm= opt_hcdm;         // gui: Hard Core Debug Mode?
   gui::opt_verbose= opt_verbose;   // gui: debugging verbosity

   // ROOT -------------------------------------------------------------------
   root.orb_list.fifo(&earth);
   root.orb_list.fifo(&moon);
   root.color=  0x00FF0000;         // Red center of mass
   earth.color= 0x000000FF;         // Blue earth
   moon.color=  0x00E0E0E0;         // Grey moon

   // COM  -------------------------------------------------------------------
// enum {COM_NONE= 0, COM_ORB= 1, COM_HOUR= 2, COM_INT= 3};

   // MOON -------------------------------------------------------------------
   moon.mass=   0.07346E24;         // Moon mass, kilograms
   moon.pos.x=   .3633E9;           // Moon position, m      [minimum]
   moon.vel.y= -0.997E3;            // Moon velocity, m/sec  [minimum]

   moon.pos.x=   .3844E9;           // Moon position, m      [mean]
   moon.vel.y= -1.022E3;            // Moon velocity, m/sec  [mean]

   moon.pos.x=   .4055E9;           // Moon position, m      [maximum]
   moon.vel.y= -1.082E3;            // Moon velocity, m/sec  [maximum]

   // EARTH ------------------------------------------------------------------
   earth.mass=   5.9724E24;         // Earth mass, kilograms
   earth.pos.x= -4.414E6;           // Earth position, m     [minimum]
   earth.vel.y= 11.529;             // Earth velocity, m/sec [minimum]

   earth.pos.x= -4.671E6;           // Earth position, m     [mean]
   earth.vel.y= 11.850;             // Earth velocity, m/sec [average]

   earth.pos.x= -4.927E6;           // Earth position, m     [maximum]
   earth.vel.y= 12.145;             // Earth velocity, m/sec [maximum]

   // MOON/EARTH (default) ---------------------------------------------------
   moon.pos.x=   .3844E9;           // Moon position, m      [mean]
   moon.vel.y= -1.022E3;            // Moon velocity, m/sec  [mean]

   // Earth: Experimental most consistent (so far)
   earth.pos.x= -4.728086531E6;     // Earth position, m     [~(C.O.M.)]
   earth.vel.y= 12.5705;            // Earth velocity, m/sec [experimental]

   int rc= 0;                       // Return code, default OK
   for(int argx= optind; argx<argc; argx++) {
     const char* text= argv[argx];
     sim::Xyz* xyz= nullptr;        // Default, not found
     if( memcmp(text, "ep:", 3) == 0 )
       xyz= &earth.pos;
     else if( memcmp(text, "ev:", 3) == 0 )
       xyz= &earth.vel;
     else if( memcmp(text, "mp:", 3) == 0 )
       xyz= &moon.pos;
     else if( memcmp(text, "mv:", 3) == 0 )
       xyz= &moon.vel;

     if( xyz ) {
       if( xyz->fr_string(text+3) ) {
         rc= 1;
         fprintf(stderr, "Invalid value '%s'\n", text);
       }
     } else {
       char* last= nullptr;
       if( memcmp(text, "g:", 2) == 0 ) {
         G= strtod(text+2, &last);
       } else if( memcmp(text, "c:", 2) == 0 ) {
         USE_COM= (int)strtol(text+2, &last, 0);
       }

       if( last == nullptr ) {
         rc= 1;
         fprintf(stderr, "Object not found '%s'\n", text);
       } else if( *last != '\0' ) {
         rc= 1;
         fprintf(stderr, "Invalid value '%s'\n", text);
       }
     }
   }

   // Initial center of mass -------------------------------------------------
   root.init();                     // Initialize (mass)
   root.pos= root.com();            // Compute center of mass

   debugf("F: %d USE_COM\n", USE_COM);
   debugf("F: %.8e USE_CAF\n", USE_CAF);
   debugf("T: %.8e\n", delta_t);
   debugf("G: %.8e\n", G);
   debugf("V: %.8e\n", earth.vel.mag());
   debugf("C: pos: [%.6e,%.6e,%.6e] m\n"
         , root.pos.x, root.pos.y, root.pos.z);

   sim::Pos pos= earth.pos;
   sim::Vel vel= earth.vel;
   debugf("E: pos: [%8.1f,%8.1f,%8.4f] km, vel: [%8.4f,%8.4f,%8.4f] m/s\n"
         , pos.x/1000.0, pos.y/1000.0, pos.z/1000.0, vel.x, vel.y, vel.z);
   pos= moon.pos;
   vel= moon.vel;
   debugf("M: pos: [%8.1f,%8.1f,%8.4f] km, vel: [%8.4f,%8.4f,%8.4f] km/s\n"
         , pos.x/1000.0, pos.y/1000.0, pos.z/1000.0
         , vel.x/1000.0, vel.y/1000.0, vel.z/1000.0);

   return rc;
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
   // Mainline code: Run the simulation
   //-------------------------------------------------------------------------
   try {
     gui::Device device;            // The base Device
     sim::Window window(&device);   // The Window

     device.configure();
     window.image_init();
     window.image_draw();
     window.clear();
     window.show();
     window.flush();

     // Run the simulation
     sim::Pos com= root.pos;        // Default, use root center of mass
     sim::Pos cMaxH= {};            // COM position at hMaxC
     sim::Pos cMaxO= {};            // COM position at oMaxC
     sim::Pos eMaxP= {-1e100, -1e100, -1e100}; // Earth max distance from COM
     sim::Pos eMinP= {+1e100, +1e100, +1e100}; // Earth min distance from COM
     sim::Pos mMaxP= {-1e100, -1e100, -1e100}; // Moon max distance from COM
     sim::Pos mMinP= {+1e100, +1e100, +1e100}; // Moon min distance from COM
     double hMaxC= 0.0;             // Maximum COM drift (hourly)
     double oMaxC= 0.0;             // Maximum COM drift (orbital)
     double vMaxE= 0.0;             // Maximum Earth velocity
     double vMinE= 1e10;            // Minimum Earth velocity
     double vMaxM= 0.0;             // Maximum Moon velocity
     double vMinM= 1e10;            // Minimum Moon velocity
     window.key_debug['d']= true;   // Default, delay on
     window.key_debug['o']= true;   // Default, orbital on
     double hour_interval= 0.0;     // The current hourly interval
     double moon_interval= 0.0;     // The current rotation interval
     sim::Orb& E= earth;            // Shorthand for earth orbital
     sim::Orb& M= moon;             // Shorthand for moon orbital
     for(double time= 0; time<RUN_TIME; time += delta_t) {
       hour_interval += delta_t;
       moon_interval += delta_t;

       // key_debug['p'] (pause) =============================================
       if( window.key_debug['p'] ) {
         xcb_generic_event_t* e= xcb_wait_for_event(device.c);
         if( e ) {
           device.handle_event(e);
           free(e);
         }
         if( !device.operational )
           break;
       }

       //=====================================================================
       // Calculate new positions and velocities
       sim::Xyz F= force(E, M);
       sim::Xyz A= {F.x/E.mass, F.y/E.mass, F.z/E.mass};
       sim::Pos oldE= E.pos;        // Used to calculate earth velocity
       double T= delta_t;           // (Shorthand)
       E.pos.x += (E.vel.x * T) + 0.5*A.x*T*T;
       E.pos.y += (E.vel.y * T) + 0.5*A.y*T*T;
       E.pos.z += (E.vel.z * T) + 0.5*A.z*T*T;
       double eDel= oldE.mag(E.pos);
       E.circ += eDel;

       E.vel.x += A.x * delta_t;
       E.vel.y += A.y * delta_t;
       E.vel.z += A.z * delta_t;

       if( USE_EARTH_POS ) {
         if( eDel > vMaxE ) {
           vMaxE= eDel;
           eMaxP= E.pos;
           mMaxP= M.pos;
         }
         if( eDel < vMinE ) {
           vMinE= eDel;
           eMinP= E.pos;
           mMinP= M.pos;
         }
       }

       A= {F.x/M.mass, F.y/M.mass, F.z/M.mass};
       sim::Pos oldM= M.pos;        // Used to calculate moon circumference
       M.pos.x += (M.vel.x * T) - 0.5*A.x*T*T;
       M.pos.y += (M.vel.y * T) - 0.5*A.y*T*T;
       M.pos.z += (M.vel.z * T) - 0.5*A.z*T*T;
       double mDel= oldM.mag(M.pos);
       M.circ += mDel;

       M.vel.x -= A.x * delta_t;
       M.vel.y -= A.y * delta_t;
       M.vel.z -= A.z * delta_t;

       if( USE_MOON_POS ) {
         if( mDel > vMaxM ) {
           vMaxM= mDel;
           eMaxP= E.pos;
           mMaxP= M.pos;
         }
         if( mDel < vMinM ) {
           vMinM= mDel;
           eMinP= E.pos;
           mMinP= M.pos;
         }
       }

       if( false ) {                // Save min/max without regard to velocity?
         eMaxP.max(E.pos);
         mMaxP.max(M.pos);
         eMinP.min(E.pos);
         mMinP.min(M.pos);
       }

       com= root.com();
       double D= com.mag();
       if( D > hMaxC ) {            // Hourly and orbital maximums independent
         hMaxC= D;
         cMaxH= com;
       }
       if( D > oMaxC ) {
         oMaxC= D;
         cMaxO= com;
       }

       //=====================================================================
       // Hourly graphic display
       if( hour_interval >= 3600.0 ) { // Every hour
         hour_interval= 0.0;

         // Draw current position
         window.image_draw();

         // Display changed changed pixels (only)
         if( pos_used >= DIM ) {    // If there are old pixels
           sim::Pos P= E_pos[pos_ix];
           int x= window.center_x + d2p(P.x);
           int y= window.center_y - d2p(P.y);
           window.put_xy(x, y);

           P= M_pos[pos_ix];
           x= window.center_x + d2p(P.x);
           y= window.center_y - d2p(P.y);
           window.put_xy(x, y);
         }

         sim::Pos P= E.pos;
         int x= window.center_x + d2p(P.x);
         int y= window.center_y - d2p(P.y);
         window.put_xy(x, y);

         P= M.pos;
         x= window.center_x + d2p(P.x);
         y= window.center_y - d2p(P.y);
         window.put_xy(x, y);
         window.flush();

         // key_debug['d'] (delay) ===========================================
         if( window.key_debug['d'] ) // Delay between window updates?
            usleep(1024);           // (Delay in microseconds)

         // Update position array
         if( pos_used < DIM )
           ++pos_used;
         E_pos[pos_ix]= E.pos;
         M_pos[pos_ix]= M.pos;
         ++pos_ix;
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

         // Keep center of mass centered (hourly)
         if( USE_COM == COM_HOUR ) {
           cMaxH *= USE_CAF;
           E.pos += cMaxH;
           M.pos += cMaxH;
           hMaxC= 0.0;
           cMaxH= {};
         }
       }

       //=====================================================================
       // key_debug['o'] (orbital information display)========================
//     if( ((oldM.y-com.y) > 0.0 && (M.pos.y-com.y) <= 0.0)
//         || moon_interval >= 3'000'000.0 ) {
       if( (oldM.y > 0.0 && M.pos.y <= 0.0 && moon_interval > 300'000)
           || moon_interval >= 3'000'000.0 ) {
         if( window.key_debug['o'] || (time + 3'000'000) >= RUN_TIME ) {
           if( USE_EARTH_POS ) {
             debugf("\n%10.0f %8.0f,"
                   " E[min,avg,max]V: [%8.4f,%8.4f,%8.4f] EC: %6.01f\n"
                   , time, moon_interval
                   , vMinE/delta_t, E.circ/moon_interval, vMaxE/delta_t
                   , E.circ/1000.0);
             debugf(".. E: @EminV: [%10.1f,%10.1f], @EmaxV: [%10.1f,%10.1f]\n"
                   , eMinP.x/1000.0, eMinP.y/1000.0
                   , eMaxP.x/1000.0, eMaxP.y/1000.0);
             debugf(".. M: @EminV: [%10.1f,%10.1f], @EmaxV: [%10.1f,%10.1f]\n"
                   , mMinP.x/1000.0, mMinP.y/1000.0
                   , mMaxP.x/1000.0, mMaxP.y/1000.0);
           } else {
             vMaxM /= 1000.0;       // (Use km/second)
             vMinM /= 1000.0;
             M.circ /= 1000.0;      // (Use km)
             debugf("\n%10.0f %8.0f,"
                   " M[min,avg,max]V: [%8.4f,%8.4f,%8.4f] MC: %6.01f\n"
                   , time, moon_interval
                   , vMinM/delta_t, M.circ/moon_interval, vMaxM/delta_t
                   , E.circ/1000.0);
             debugf(".. E: @MminV: [%10.1f,%10.1f], @MmaxV: [%10.1f,%10.1f]\n"
                   , eMinP.x/1000.0, eMinP.y/1000.0
                   , eMaxP.x/1000.0, eMaxP.y/1000.0);
             debugf(".. M: @MminV: [%10.1f,%10.1f], @MmaxV: [%10.1f,%10.1f]\n"
                   , mMinP.x/1000.0, mMinP.y/1000.0
                   , mMaxP.x/1000.0, mMaxP.y/1000.0);
           }

           // Current center of mass, maximum distance
           if( USE_COM == COM_HOUR || USE_COM == COM_ORB ) {
             debugf(".. C: @(MAX): [%10.3e,%10.3e], %.6e\n"
                     , cMaxO.x, cMaxO.y, oMaxC);
           } else {
             debugf(".. C: @(NOW): [%10.3e,%10.3e], %.6e\n"
                   , com.x, com.y, oMaxC); // (Still use orbital maximum)
           }
         }

         // Try to keep center of mass centered (orbitally)
         if( USE_COM == COM_ORB ) {
           cMaxO *= USE_CAF;
           E.pos += cMaxO;
           M.pos += cMaxO;
         }

         // Reset orbital statistics
         moon_interval= 0.0;
         E.circ= 0.0;
         M.circ= 0.0;
         oMaxC= 0.0;
         vMaxE= 0.0;
         vMinE= 1e10;
         vMaxM= 0.0;
         vMinM= 1e10;
         cMaxO= {};
         eMaxP= {-1e100, -1e100, -1e100}; // Max distance from COM
         eMinP= {+1e100, +1e100, +1e100}; // Min distance from COM
         mMaxP= {-1e100, -1e100, -1e100}; // Max distance from COM
         mMinP= {+1e100, +1e100, +1e100}; // Min distance from COM
       }
       // (orbital) ==========================================================

       if( USE_COM == COM_INT ) {   // If adjusting COM at every interval
         com *= USE_CAF;
         E.pos += com;
         M.pos += com;
       }
     }

     while(device.operational) {
       printf("Hit return to exit\n");
       xcb_generic_event_t* e= xcb_wait_for_event(device.c);
       if( e ) {
         device.handle_event(e);
         free(e);
       }
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
