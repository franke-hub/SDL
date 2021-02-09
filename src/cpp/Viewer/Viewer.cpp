//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Viewer.cpp
//
// Purpose-
//       Viewer master control.
//
// Last change date-
//       2021/02/09
//
//----------------------------------------------------------------------------
#include <cstdio>
#include <stdlib.h>
#include <string.h>                 // For memset
#include <unistd.h>
#include <xcb/xcb.h>                // For xcb definitions
#include <xcb/xproto.h>             // For xcb prototypes

#include <gui/Device.h>             // For gui::Device
#include <gui/Global.h>             // For gui::get_image_order
#include <gui/Window.h>             // For gui::Window
#include <pub/Debug.h>              // For Debug, namespace pub::debugging

#include "JpegDecoder.h"            // For JpegDecoder
#include "Viewer.h"                 // For Viewer (implemented here)

using namespace pub::debugging;     // For debugging subroutines

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const int opt_hcdm= false;   // Option: Hard Core Debug Mode

//----------------------------------------------------------------------------
//
// Class-
//       Viewer
//
// Purpose-
//       Implementation
//
//----------------------------------------------------------------------------
   Viewer::~Viewer()
{
   if( opt_hcdm )
     debugh("Tester(%p)::~Tester\n", this);

   // Delete the graphics context
   if( drawGC ) {
     ENQUEUE("xcb_free_gc", xcb_free_gc(c, drawGC) );
     drawGC= 0;
   }

   // Delete the Image data
   reset();

   // Complete any pending operations
   flush();
}

   Viewer::Viewer(
     Widget*           widget,      // Our parent Widget
     const char*       name)        // The Window's name
:  gui::Window(widget, name)
{
   if( opt_hcdm )
     debugh("Tester(%p)::Tester\n", this);

   load(name);                      // Load the file

   use_size.width=  decoder.width;
   use_size.height= decoder.height;
   min_size= use_size;
}

void
   Viewer::configure( void )        // Configure the Window
{
   if( opt_hcdm )
     debugh("Tester(%p)::configure Named(%s)\n", this, get_name().c_str());

   // Create the Window
   gui::Pixel_t bg= 0x00FFFFFF;
   gui::Pixel_t fg= 0x00FF0000;

   emask |= XCB_EVENT_MASK_KEY_PRESS;
   emask |= XCB_EVENT_MASK_BUTTON_PRESS;
   emask |= XCB_EVENT_MASK_EXPOSURE;
   emask |= XCB_EVENT_MASK_STRUCTURE_NOTIFY;

   Window::configure();
   flush();

   // Create a Graphic Context, not used for much
   drawGC= xcb_generate_id(c);
   uint32_t mask= XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
   uint32_t parm[2];
   parm[0]= fg;
   parm[1]= bg;
   ENQUEUE("xcb_create_gc", xcb_create_gc(c, drawGC, widget_id, mask, parm) );

   flush();
}

void
   Viewer::draw( void )             // Draw the Window
{
   if( opt_hcdm )
     debugh("Tester(%p)::draw(%s)\n", this, get_name().c_str());

   ENQUEUE("xcb_image_put", xcb_image_put
          (c, widget_id, drawGC, &image, 0, 0, 0) );

   flush();
}

int                                 // Return code (0 OK)
   Viewer::load(                    // Load a JPEG file
     const char*       name)        // File name
{
   reset();                         // Reset the Viewer
   int rc= decoder.decode(name);    // Decode the JPEG file
   if( rc )
     return rc;                     // If error, return

   // Initialize the XCB image
   image.width= decoder.width;      // Width, in pixels
   image.height= decoder.height;    // Height, in pixels
   image.format= XCB_IMAGE_FORMAT_Z_PIXMAP; // Format type
   image.scanline_pad= 32;          // Scanline pad (bits)
   image.depth= 24;                 // Depth (bits)
   image.bpp= 32;                   // Storage per pixel (bits)
   image.unit= 32;                  // Scanline unit (bits)
   image.plane_mask= 0;             // (Unused here)
   image.byte_order= gui::get_image_order(); // Byte order
   image.bit_order=  XCB_IMAGE_ORDER_MSB_FIRST; // Bit order
   image.stride= decoder.width * 4; // Bytes per image row
   image.size= decoder.width * decoder.height * 4; // Size of image (bytes)
   image.base= malloc(image.size);  // Allocated storage
   image.data= (uint8_t*)decoder.buffer; // The actual image
   memset(image.base, 0, image.size); // Initialize the allocated storage

   for(unsigned h= 0; h<decoder.height; h++) {
     uint32_t* pixel= decoder.buffer + h * decoder.width;
     for(unsigned w= 0; w<decoder.width; w++) {
       xcb_image_put_pixel(&image, w, h, pixel[w]);
     }
   }

   return 0;                        // Alles in ordnung
}

void
   Viewer::reset( void )            // Reset the Viewer
{
   // Free the image buffer
   if( image.base ) {
     free(image.base);
     image.base= nullptr;
   }
}

void
   Viewer::expose(const xcb_rectangle_t) // (Partially) (re)draw this Window
{  // We redraw the entire window
   draw();
}

void
   Viewer::key_input(               // Handle this
     xcb_keysym_t      key,         // Key input event
     int               state)       // Alt/Ctl/Shift state mask
{  (void)key; (void)state;          // Parameters currently unused
   device->operational= false;      // Any key terminates
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
   (void)argc;                      // Unused parameter
   unsigned int        errorCount= 0;
   gui::Device         device;
   Viewer              window(&device, argv[1]);

   if( window.decoder.buffer )      // If decoder successful
   {
     device.configure();
     device.draw();
     window.show();
     window.flush();
     device.start();
     device.join();
   }

   if( errorCount != 0 )
     errorCount= 1;

   return errorCount;
}

