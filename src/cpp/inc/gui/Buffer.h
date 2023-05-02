//----------------------------------------------------------------------------
//
//       Copyright (C) 2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Buffer.h
//
// Purpose-
//       Container for pixel data.
//
// Last change date-
//       2021/02/01
//
//----------------------------------------------------------------------------
#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#include <xcb/xproto.h>             // For xcb_expose_event_t, ...
#include <xcb/xcb_image.h>          // For xcb_image_t, associated functions

#include <gui/Types.h>              // For gui::Pixel
#include <gui/Pixmap.h>             // For gui::Pixmap

namespace gui {
//----------------------------------------------------------------------------
//
// Class-
//       gui::Buffer
//
// Purpose-
//       Pixel container
//
//----------------------------------------------------------------------------
class Buffer {                      // Pixel container
//----------------------------------------------------------------------------
// gui::Buffer::Attributes
//----------------------------------------------------------------------------
public:
Pixel_t*               buffer= nullptr; // The pixel buffer
unsigned               width= 0;    // Width  (X) size
unsigned               height= 0;   // Height (Y) length

xcb_image_t            image= {};   // XCB image manipulator

//----------------------------------------------------------------------------
// gui::Buffer::Constructor/Destructor/Operators
//----------------------------------------------------------------------------
public:
   Buffer(                          // Constructor
     unsigned          width= 0,    // Width  (X Pixel count)
     unsigned          height= 0,   // Height (Y Pixel count)
     Pixel_t           p= 0);       // (Initial) background color

   ~Buffer( void );                 // Destructor

   Buffer(const Buffer&) = delete;  // NO copy constructor
Buffer& operator=(const Buffer&) = delete; // NO assignment operator

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
   clear(                           // Clear the Buffer
     Pixel_t           p);          // Setting all Pixels to this

//----------------------------------------------------------------------------
//
// Method-
//       gui::Buffer::expose
//
// Purpose-
//       Expose the Buffer, drawing it on a Pixmap
//
//----------------------------------------------------------------------------
void
   expose(                          // Handle expose event
     Pixmap*           pixmap,      // Pixmap (or Window)
     xcb_gcontext_t    gc,          // The graphic context
     xcb_expose_event_t*
                       event);      // The expose event

//----------------------------------------------------------------------------
//
// Method-
//       gui::Buffer::get_xy
//       gui::Buffer::put_xy
//
// Purpose-
//       Get Pixel at location      (unchecked)
//       Set Pixel at location      (unchecked)
//
//----------------------------------------------------------------------------
Pixel_t                             // The Pixel
   get_xy(                          // Get Pixel at location
     unsigned          x,           // X (Width) index  (from left)
     unsigned          y)           // Y (Height) index (from top)
{  return buffer[y*height + x]; }

void
   put_xy(                          // Set Pixel at location
     unsigned          x,           // X (Width) index  (from left)
     unsigned          y,           // Y (Height) index (from top)
     Pixel_t           p)           // The Pixel to set
{  buffer[y*height + x]= p; }

//----------------------------------------------------------------------------
//
// Method-
//       gui::Buffer::resize
//
// Purpose-
//       Resize the buffer, initialize new pixels to background pixel
//
//----------------------------------------------------------------------------
void
   resize(                          // Get Pixel at location
     unsigned          x,           // X (Width)
     unsigned          y,           // Y (Height)
     Pixel_t           p= 0);       // Background Pixel
}; // class gui::Buffer
}  // namespace gui
#endif // BUFFER_H_INCLUDED
