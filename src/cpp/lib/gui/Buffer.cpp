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
//       Buffer.cpp
//
// Purpose-
//       Implement Buffer.h
//
// Last change date-
//       2021/02/01
//
//----------------------------------------------------------------------------
#include <exception>                // For std::exception
#include <new>                      // For std::bad_alloc
#include <stdexcept>                // For std::range_error
#include <xcb/xcb.h>                // For xcb basic types
#include <xcb/xproto.h>             // For xcb types and prototypes
#include <xcb/xcb_bitops.h>         // For xcb_host_byte_order
#include <xcb/xcb_image.h>          // For xcb_image_t, associated functions

#include <pub/Debug.h>              // For namespace pub::debugging
#include <gui/Pixmap.h>             // For gui::Pixmap

#include "gui/Buffer.h"             // Implementation class

using namespace pub::debugging;     // For debugging methods

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug
//
// Purpose-
//       (Overloaded) debugging display
//
//----------------------------------------------------------------------------
#if 0  // Unused
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
}
#endif // Unused

namespace gui {
//----------------------------------------------------------------------------
//
// Method-
//       gui::Buffer::Buffer
//       gui::Buffer::~Buffer
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   Buffer::Buffer(                  // Constructor
     unsigned          width,       // Width  (X Pixel count)
     unsigned          height,      // Height (Y Pixel count)
     Pixel_t           p)           // (Initial) background color
{  resize(width, height, p); }

   Buffer::~Buffer( void )          // Destructor
{
   if( buffer ) {
     free(buffer);
     buffer= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       gui::Buffer::clear
//
// Purpose-
//       Clear the Buffer, setting all Pixels to the specified value
//
//----------------------------------------------------------------------------
void
   Buffer::clear(                   // Clear the Buffer
     Pixel_t           p)           // Setting all Pixels to this
{
   if( buffer ) {
     for(unsigned h= 0; h<height; h++) {
//     unsigned X0= width * h;
       for(unsigned w= 0; w<width; w++) {
//       buffer[X0 + w]= p;
         put_xy(w, h, p);
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       gui::Buffer::expose
//
// Purpose-
//       Expose the Buffer, drawing it on a Window
//
//----------------------------------------------------------------------------
void
   Buffer::expose(                  // Handle window expose event
     Pixmap*           pixmap,      // Pixmap (or Window)
     xcb_gcontext_t    gc,          // The graphic context
     xcb_expose_event_t*
                       event)       // The expose event
{
   if( buffer == nullptr )
     throw std::range_error("Not initialized");

   // Case 1: Full exposure
   if( event->x == 0 && event->y == 0
       && event->width == width && event->height == height ) {
     pixmap->ENQUEUE("xcb_image_put", xcb_image_put
                    ( pixmap->c, pixmap->widget_id, gc, &image, 0, 0, 0) );
     return;
   }

   // Case 2: NO exposure
   if( event->x >= width || event->y >= height )
     return;

   // Case 3: Partial exposure
   if( opt_hcdm && opt_verbose > 1 )
     printf("%4d HCDM %p [%u,%u] [%u,%u,%u,%u]\n", __LINE__, buffer
           , width, height, event->x, event->y, event->width, event->height);

   xcb_image_t exposure= image;     // Use the base image
   exposure.width= event->width;    // Set exposure width
   if( width - event->x < exposure.width )
     exposure.width= width - event->x;
   exposure.stride= exposure.width * 4;
   exposure.size= exposure.stride;

   exposure.height= 1;              // Draw one pixel line at a time
   unsigned maxY= event->y + event->height; // Set line limit
   if( height < maxY )
     maxY= height;

   for(unsigned y= event->y; y<maxY; y++) {
     exposure.base= buffer + (y * width) + event->x;
     exposure.data= (uint8_t*)exposure.base;
     if( opt_hcdm && opt_verbose > 1 )
       printf("%4d HCDM %p [%u,%u].%u\n", __LINE__, exposure.base
             , event->x, y, exposure.width);
     xcb_image_put(pixmap->c, pixmap->widget_id, gc, &exposure, event->x, y, 0);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       gui::Buffer::get_xy
//       gui::Buffer::put_xy
//
// Purpose-
//       Get Pixel at location
//       Set Pixel at location
//
//----------------------------------------------------------------------------
#if 0  // Inline in Buffer.h
Pixel_t                             // The Pixel
   Buffer::get_xy(                  // Get Pixel at location
     unsigned          x,           // X (Width) index  (from left)
     unsigned          y)           // Y (Height) index (from top)
{
   if( x > width || y > height )
     throw std::range_error("Buffer::get_xy");

   return buffer[y*height + x];
}

void
   Buffer::put_xy(                  // Set Pixel at location
     unsigned          x,           // X (Width) index  (from left)
     unsigned          y,           // Y (Height) index (from top)
     Pixel_t           p)           // The Pixel to set
{
   if( x > width || y > height )
     throw std::range_error("Buffer::put_xy");

   buffer[y*height + x]= p;
}
#endif // Inline in Buffer.h

//----------------------------------------------------------------------------
//
// Method-
//       gui::Buffer::resize
//
// Purpose-
//       Resize the buffer
//
//----------------------------------------------------------------------------
void
   Buffer::resize(                  // Get Pixel at location
     unsigned          x,           // X (Width)
     unsigned          y,           // Y (Height)
     Pixel_t           p)           // Background Pixel
{
   // Initialize the image buffer
   image.width= x;                  // Width, in pixels
   image.height= y;                 // Height, in pixels
   image.format= XCB_IMAGE_FORMAT_Z_PIXMAP; // Format type
   image.scanline_pad= 32;          // Scanline pad (bits)
   image.depth= 24;                 // Depth (bits)
   image.bpp= 32;                   // Storage per pixel (bits)
   image.unit= 32;                  // Scanline unit (bits)
   image.plane_mask= 0;             // (Unused here)
   image.byte_order= xcb_host_byte_order(); // Byte order
   image.bit_order=  XCB_IMAGE_ORDER_MSB_FIRST; // Bit order
   image.stride= image.width * 4;   // Bytes per image row
   image.size= image.width * image.height * 4; // Size of image (bytes)
   image.base= malloc(image.size);  // Allocated storage
   image.data= (uint8_t*)image.base;  // (Unused here)
   memset(image.base, 0, image.size); // Initialize the allocated storage

   // Copy the current image buffer to the new buffer
   if( image.base ) {
     unsigned wmax= x;
     if( width < x )
       wmax= width;
 
     unsigned hmax= y;
     if( height < y )
       hmax= height;
 
     Pixel_t* pixel= (Pixel_t*)image.base; // The new buffer
     for(unsigned h= 0; h<hmax; h++) {
       unsigned P0= h * y;
       unsigned B0= h * height;
       for(unsigned w= 0; w<wmax; w++) {
         pixel[P0 + w]= buffer[B0 + w];
       }
       for(unsigned w= wmax; w<y; w++) {
         pixel[P0 + w]= p;
       }
     }
 
     for(unsigned h= hmax; h<y; h++) {
       unsigned P0= h * y;
       for(unsigned w= 0; w<y; w++) {
         pixel[P0 + w]= p;
       }
     }
 
     // Update the image buffer
     if( buffer )
       free(buffer);
     buffer= (Pixel_t*)image.base;
     width= x;
     height= y;
   } else {                         // If no new image
     if( image.size )               // If storage allocation failed
       throw std::bad_alloc();

     if( buffer ) {
       free(buffer);
       buffer= nullptr;
       width= height= 0;
     }
   }
}
}  // namespace gui
