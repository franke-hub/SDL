//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Xcb/Pixmap.h
//
// Purpose-
//       XCB based Pixmap
//
// Last change date-
//       2020/12/23
//
//----------------------------------------------------------------------------
#ifndef XCB_PIXMAP_H_INCLUDED
#define XCB_PIXMAP_H_INCLUDED

#include <string>                   // For std::string

#include <X11/Xlib.h>               // For X11 types (Display)
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include "Xcb/Global.h"             // For ENQUEUE/NOQUEUE macros
#include "Xcb/Types.h"              // For namespace xcb types
#include "Xcb/Layout.h"             // For Layout (base class)

namespace xcb {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Device;

//----------------------------------------------------------------------------
//
// Class-
//       xcb::Pixmap
//
// Purpose-
//       Pixmap object.
//
//----------------------------------------------------------------------------
class Pixmap : public Layout {      // The Pixmap object
public:
//----------------------------------------------------------------------------
//
// Struct-
//       xcb::Pixmap::Pending
//
// Purpose-
//       Pending XCB request table entry.
//
//----------------------------------------------------------------------------
struct Pending {                    // Pending XCB request table
const char*            opname;      // The operation name
int                    opline;      // The associated line number
xcb_void_cookie_t      op;          // The Cookie
}; // struct Pending

//----------------------------------------------------------------------------
// xcb::Pixmap::Attributes
//----------------------------------------------------------------------------
protected:
enum { DIM_PENDING= 16 };           // Number of available queued operations
Pending                pending[DIM_PENDING]; // The pending operation queue
unsigned               penduse= 0;  // Number of operations pending

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
// xcb::Pixmap::Constructors/Destructors/Operators
//----------------------------------------------------------------------------
protected:
   Pixmap(                          // Constructor
     Widget*           widget= nullptr, // Our parent Widget
     const char*       name= nullptr); // The Pixmap's name

public:
virtual
   ~Pixmap( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Pixmap::configure
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
//       xcb::Pixmap::debug
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
//       xcb::Pixmap::clear
//
// Purpose-
//       Clear the window, setting it to the background pixel.
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
//       xcb::Pixmap::draw
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
//       xcb::Pixmap::get_size
//
// Purpose-
//       Get current width and height
//
//----------------------------------------------------------------------------
WH_size_t                           // The current Pixmap size
   get_size(                        // Get current Pixmap size
     int               line= 0);    // Caller's line number

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Pixmap::set_size
//
// Purpose-
//       Set current width and height
//
//----------------------------------------------------------------------------
void
   set_size(                        // Set Pixmap size
     int               x,           // New width
     int               y,           // New height
     int               line= 0);    // Caller's line number

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Pixmap::enqueue
//       xcb::Pixmap::noqueue
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
     const char*       name,        // Operation name
     xcb_void_cookie_t op);         // Operation cookie

void                                // Response handled in reply loop
   noqueue(                         // Drive operation
     int               line,        // Source line number
     const char*       name,        // Operation name
     xcb_void_cookie_t op);         // Operation cookie

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Pixmap::flush
//
// Purpose-
//       Complete all pending enqueued operations.
//
// Implementation note-
//       This also invokes xcb_flush.
//
//----------------------------------------------------------------------------
void
   flush( void );                   // Complete all pending operations

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Pixmap::synchronously
//
// Purpose-
//       Synchronous operation, or operation completion
//
//----------------------------------------------------------------------------
void
   synchronously(                   // Synchronous XCB operation
     int               line,        // Source line number
     const char*       name,        // The operation name
     xcb_void_cookie_t op);         // The synchronous operation (cookie)

void
   synchronously(                   // Synchronous XCB operation
     xcb_void_cookie_t op);         // The synchronous operation (cookie)

//----------------------------------------------------------------------------
// xcb::Pixmap::Event handlers, overridden in implementation class.
//----------------------------------------------------------------------------
public:
virtual void
   graphics_exposure(               // Handle this
     xcb_graphics_exposure_event_t*) {} // Graphics exposure event

virtual void
   no_exposure(                     // Handle this
     xcb_no_exposure_event_t*) {}   // No exposure event
}; // class Pixmap
}  // namespace xcb
#endif // XCB_PIXMAP_H_INCLUDED
