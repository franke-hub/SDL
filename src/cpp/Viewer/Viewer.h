//----------------------------------------------------------------------------
//
//       Copyright (c) 2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Viewer.h
//
// Purpose-
//       Viewer classes.
//
// Last change date-
//       2021/01/28
//
//----------------------------------------------------------------------------
#ifndef VIEWER_H_INCLUDED
#define VIEWER_H_INCLUDED

#include <xcb/xproto.h>              // For xcb prototypes, etc.
#include <xcb/xcb_image.h>           // For struct xcb_image
#include <gui/Window.h>              // Viewer base class

#include "JpegDecoder.h"             // Jpeg decoder

//----------------------------------------------------------------------------
//
// Class-
//       Viewer
//
// Purpose-
//       Viewer Window
//
//----------------------------------------------------------------------------
class Viewer : public gui::Window { // Viewer Window
//----------------------------------------------------------------------------
// Viewer::Attributes
//----------------------------------------------------------------------------
public:
JpegDecoder            decoder;     // Image decoder

// XCB fields
xcb_gcontext_t         drawGC= 0;   // The default graphic context
xcb_image_t            image= {};   // The xcb_image_t

//----------------------------------------------------------------------------
// Viewer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Viewer( void );                 // Destructor
   Viewer(                          // Constructor
     Widget*           widget= nullptr, // Our parent Widget
     const char*       name= nullptr); // The Window's name

private:                            // Bitwise copy is prohibited
   Viewer(const Viewer&) = delete;  // Disallowed copy constructor
Viewer& operator=(const Viewer&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Viewer::Methods
//----------------------------------------------------------------------------
public:
virtual void
   configure( void ) override;      // Create the Window (Layout complete)

virtual void
   draw( void ) override;           // Draw the Window

int                                 // Return code (0 OK)
   load(                            // Load a JPEG file
     const char*       name);       // File name

void
   reset( void );                   // Reset the Viewer

//----------------------------------------------------------------------------
// Viewer::Event handlers
//----------------------------------------------------------------------------
virtual void
   expose(                          // (Partially) (re)draw this Window
     const
     xcb_rectangle_t   rect) override; // This (relative) Window range

virtual void
   key_input(                       // Handle this
     xcb_keysym_t      key,         // Key input event
     int               state) override; // Alt/Ctl/Shift state mask
}; // class Viewer
#endif // VIEWER_H_INCLUDED
