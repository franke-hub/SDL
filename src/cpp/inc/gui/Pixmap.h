//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       gui/Pixmap.h
//
// Purpose-
//       XCB based Pixmap
//
// Last change date-
//       2024/06/07
//
//----------------------------------------------------------------------------
#ifndef GUI_PIXMAP_H_INCLUDED
#define GUI_PIXMAP_H_INCLUDED

#include <string>                   // For std::string

#include <X11/Xlib.h>               // For X11 types (Display)
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include "gui/Global.h"             // For ENQUEUE/NOQUEUE macros
#include "gui/Types.h"              // For namespace gui types
#include "gui/Layout.h"             // For Layout (base class)

namespace gui {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Device;

//----------------------------------------------------------------------------
//
// Class-
//       gui::Pixmap
//
// Purpose-
//       Pixmap object.
//
//----------------------------------------------------------------------------
class Pixmap : public Layout {      // The Pixmap object
public:
using Layout::configure;

//----------------------------------------------------------------------------
// gui::Pixmap::Typedefs and enumerations
//----------------------------------------------------------------------------
enum { DIM_PENDING= 16 };           // The pending operation queue size

//----------------------------------------------------------------------------
// struct gui::Pixmap::Pending: A pending operation queue entry
//----------------------------------------------------------------------------
struct Pending {                    // Pending XCB request table
const char*            opname;      // The operation name
const char*            opfile;      // The operation file
int                    opline;      // The associated line number
xcb_void_cookie_t      op;          // The Cookie
}; // struct Pending

//----------------------------------------------------------------------------
// gui::Pixmap::Attributes
//----------------------------------------------------------------------------
protected:
Pending                pending[DIM_PENDING]; // The pending operation queue
unsigned               penduse= 0;  // The number of pending operations

public:
Device*                device= nullptr; // Our parent Device
Window*                window= nullptr; // The PARENT Window
xcb_connection_t*      c= nullptr;  // XCB connection*
xcb_screen_t*          s= nullptr;  // XCB screen*
xcb_window_t           parent_id= 0; // XCB parent window id
xcb_pixmap_t           widget_id= 0; // (This) XCB pixmap/window id
Pixel_t                fg= 0x00000000; // Foreground, default BLACK
Pixel_t                bg= 0x00FFFFFF; // Background, default WHITE

//----------------------------------------------------------------------------
// gui::Pixmap::Constructors/Destructors/Operators
//----------------------------------------------------------------------------
public:
   Pixmap(                          // Constructor
     Widget*           widget= nullptr, // Our parent Widget
     const char*       name= nullptr); // The Pixmap's name

virtual
   ~Pixmap( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       gui::Pixmap::configure
//
// Purpose-
//       Configure the Pixmap
//
//----------------------------------------------------------------------------
public:
virtual void                        // (Not normally overridden)
   configure(                       // Initialize using
     Device*           device,      // This parent device and
     Window*           window);     // This parent window

virtual void                        // (Optionally) override to
   configure( void );               // Create the Pixmap (Layout complete)

//----------------------------------------------------------------------------
//
// Method-
//       gui::Pixmap::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
virtual void
   debug(                           // Debugging display
     const char*       info= nullptr) const; // Associated info

//----------------------------------------------------------------------------
//
// Method-
//       gui::Pixmap::get_pending
//
// Purpose-
//       Get the number of pending operations.
//
//----------------------------------------------------------------------------
unsigned                            // The number of pending operations
   get_pending( void ) const        // Get number of pending operations
{  return penduse; }

//----------------------------------------------------------------------------
//
// Method-
//       gui::Pixmap::clear
//
// Purpose-
//       Clear the pixmap, setting it to the background pixel.
//
// Implementation note-
//       flush() required.
//
//----------------------------------------------------------------------------
virtual void
   clear( void )                    // Clear this Pixmap/Window
{  xcb_clear_area(c, 0, widget_id, 0, 0, rect.width, rect.height); }

//----------------------------------------------------------------------------
//
// Method-
//       gui::Pixmap::draw
//
// Purpose-
//       (Re)draw this Pixmap.
//
//----------------------------------------------------------------------------
virtual void
   draw( void ) {};                 // (Re)draw this Pixmap

//----------------------------------------------------------------------------
//
// Method-
//       gui::Pixmap::get_size
//       gui::Pixmap::set_size
//
// Purpose-
//       Get current width and height
//       Set current width and height
//
//----------------------------------------------------------------------------
WH_size_t                           // The current Pixmap/Window size
   get_size( void );                // Get current Pixmap/Window size

void
   set_size(                        // Set Pixmap size
     int               x,           // New width
     int               y);          // New height

//----------------------------------------------------------------------------
//
// Method-
//       gui::Pixmap::enqueue
//       gui::Pixmap::noqueue
//
// Purpose-
//       Add operation to pending queue
//
// Implementation note-
//       The noqueue method does nothing. It can be used to switch back and
//       forth when deciding between an xcb_* or a xcb_*_checked interface.
//       (Use enqueue when using the xcb_*_checked interface.)
//
//----------------------------------------------------------------------------
void
   enqueue(                         // Add operation to pending queue
     int               line,        // Source line number
     const char*       file,        // Source file name
     const char*       name,        // Operation name
     xcb_void_cookie_t op);         // Operation cookie

void                                // Response handled in reply loop
   noqueue(                         // Drive operation
     int               line,        // Source line number
     const char*       file,        // Source file name
     const char*       name,        // Operation name
     xcb_void_cookie_t op);         // Operation cookie

//----------------------------------------------------------------------------
//
// Method-
//       gui::Pixmap::flush
//
// Purpose-
//       Complete all pending enqueued operations.
//
// Implementation note-
//       This also invokes xcb_flush.
//
//----------------------------------------------------------------------------
virtual void
   flush( void );                   // Complete all pending operations

//----------------------------------------------------------------------------
//
// Method-
//       gui::Pixmap::synchronously
//
// Purpose-
//       Synchronous operation, or operation completion
//
//----------------------------------------------------------------------------
void
   synchronously(                   // Synchronous XCB operation
     int               line,        // Source line number
     const char*       file,        // Source file name
     const char*       name,        // The operation name
     xcb_void_cookie_t op);         // The synchronous operation (cookie)

void
   synchronously(                   // Synchronous XCB operation
     xcb_void_cookie_t op);         // The synchronous operation (cookie)

//----------------------------------------------------------------------------
// gui::Pixmap::Event handlers, overridden in implementation class.
//----------------------------------------------------------------------------
public:
virtual void
   graphics_exposure(               // Handle this
     xcb_graphics_exposure_event_t*) {} // Graphics exposure event

virtual void
   no_exposure(                     // Handle this
     xcb_no_exposure_event_t*) {}   // No exposure event
}; // class Pixmap
}  // namespace gui
#endif // GUI_PIXMAP_H_INCLUDED
