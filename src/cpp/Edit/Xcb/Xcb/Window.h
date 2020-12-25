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
//       Xcb/Window.h
//
// Purpose-
//       XCB based Window
//
// Last change date-
//       2020/12/23
//
// Implementation notes-
//       The window field addresses the PARENT window. (Widget already has a
//       parent field. The parent Widget is not necessarily the parent Window.)
//
// Implementation note-
//       WARNING: ******** DO NOT SIMULTANEOUSLY USE ********
//       XCB_EVENT_MASK_RESIZE_REDIRECT and XCB_EVENT_MASK_STRUCTURE_NOTIFY
//       When used together, actually changing the window size is problematic.
//       TODO: Understand why this happens.
//
//----------------------------------------------------------------------------
#ifndef XCB_WINDOW_H_INCLUDED
#define XCB_WINDOW_H_INCLUDED

#include <string>                   // For std::string

#include <X11/Xlib.h>               // For X11 types (Display)
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include "Xcb/Global.h"             // For ENQUEUE/NOQUEUE macros
#include "Xcb/Types.h"              // For namespace xcb types
#include "Xcb/Pixmap.h"             // For xcb::Pixmap (base class)

namespace xcb {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Device;

//----------------------------------------------------------------------------
//
// Class-
//       xcb::Window
//
// Purpose-
//       Window object.
//
//----------------------------------------------------------------------------
class Window : public Pixmap {      // The Window object
//----------------------------------------------------------------------------
//
// Struct-
//       xcb::Window::State
//
// Purpose-
//       Window state controls
//
//----------------------------------------------------------------------------
public:
enum                                // Window state mask
{  WS_VISIBLE= 0x00000001           // Window is visible
};

//----------------------------------------------------------------------------
// xcb::Window::Attributes
//----------------------------------------------------------------------------
public:
uint32_t               emask= 0;    // Window event mask (XCB_EVENT_MASK_*)
uint32_t               state= 0;    // Window state flags (WS_*)

//----------------------------------------------------------------------------
// xcb::Window::Constructors/Destructors/Operators
//----------------------------------------------------------------------------
protected:
   Window(                          // Constructor
     Widget*           widget= nullptr, // Our parent Widget
     const char*       name= nullptr); // The Window's name

public:
virtual
   ~Window( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Window::configure
//
// Purpose-
//       Configure the Window
//
// Implementation note-
//       Invoke Pixmap::configure when overriding this method to properly
//       prepare for window creation. (Pixmap::configure sets parent_id.)
//
//----------------------------------------------------------------------------
public:
virtual void                        // (Optionally) override to
   configure( void );               // Create the Window (Layout complete)

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Window::debug
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
//       xcb::Window::draw
//
// Purpose-
//       (Re)draw this Window.
//
//----------------------------------------------------------------------------
virtual void
   draw( void ) {};                 // (Re)draw this window

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Window::expose
//       xcb::Window::key_input
//
// Purpose-
//       (Translated) expose event.
//       (Translated) key_press event.
//
//----------------------------------------------------------------------------
virtual void
   expose(                          // (Partially) (re)draw this Window
     const
     xcb_rectangle_t   rect)        // This (relative) Window range
{  (void)rect; }

virtual void
   key_input(                       // Handle this
     xcb_keysym_t      key,         // Key input event
     int               state)       // Alt/Ctl/Shift state mask
{  (void)key; (void)state; }

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Window::get_size
//
// Purpose-
//       Get current width and height
//
//----------------------------------------------------------------------------
WH_size_t                           // The current window size
   get_size(                        // Get current window size
     int               line= 0);    // Caller's line number

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Window::set_icon_name
//
// Purpose-
//       Set the icon window name
//
//----------------------------------------------------------------------------
void
   set_icon_name(                   // Set icon window name
     const char*       text)        // Using this text
{  set_property(XCB_ATOM_WM_ICON_NAME, text); }

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Window::set_main_name
//
// Purpose-
//       Set the main window name
//
//----------------------------------------------------------------------------
void
   set_main_name(                   // Set main window name
     const char*       text)        // Using this text
{  set_property(XCB_ATOM_WM_NAME, text); }

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Window::set_property
//
// Purpose-
//       Set the main window name
//
//----------------------------------------------------------------------------
void
   set_property(                    // Set a window manager property
     xcb_atom_t        atom,        // The property to change
     const char*       text)        // Using this text
{
   NOQUEUE("xcb_change_property", xcb_change_property
          ( c, XCB_PROP_MODE_REPLACE, widget_id
          , atom, XCB_ATOM_STRING, 8, uint32_t(strlen(text)), text) );
}

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Window::set_size
//
// Purpose-
//       Update width and height
//
//----------------------------------------------------------------------------
void
   set_size(                        // Set window size
     int               x,           // New width
     int               y,           // New height
     int               line= 0);    // Caller's line number

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Window::atom_to_name
//       xcb::Window::name_to_atom
//
// Purpose-
//       Extract name from xcb_atom_t
//       Extract xcb_atom_t from name
//
//----------------------------------------------------------------------------
std::string                         // The associated name
   atom_to_name(                    // Get associated name
     xcb_atom_t        atom);       // For this atom

xcb_atom_t                          // The associated xcb_atom_t
   name_to_atom(                    // Get xcb_atom_t
     const char*       name,        // For this name
     int               only= false); // (Do not create atom indicator)

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Window::hide
//       xcb::Window::show
//
// Purpose-
//       Hide or show the window
//
//----------------------------------------------------------------------------
void
   hide( void );                    // Hide the Window

void
   show( void );                    // Show the Window

//----------------------------------------------------------------------------
// xcb::Window::Event handlers, overridden in implementation class.
//----------------------------------------------------------------------------
public:
virtual void
   button_press(                    // Handle this
     xcb_button_press_event_t*) {}  // Button press event

virtual void
   button_release(                  // Handle this
     xcb_button_release_event_t*) {} // Button release event

virtual void
   circulate_notify(                // Handle this
     xcb_circulate_notify_event_t*) {} // Circulate notify event

virtual void
   circulate_request(               // Handle this
     xcb_circulate_request_event_t*) {} // Circulate request event

virtual void
   client_message(                  // Handle this
     xcb_client_message_event_t*) {} // Client message event

virtual void
   colormap_notify(                 // Handle this
     xcb_colormap_notify_event_t*) {} // Colormap notify event

virtual void
   configure_notify(                // Handle this
     xcb_configure_notify_event_t*) {} // Configure notify event

virtual void
   configure_request(               // Handle this
     xcb_configure_request_event_t*) {} // Configure request event

virtual void
   create_notify(                   // Handle this
     xcb_create_notify_event_t*) {} // Create notify event

virtual void
   destroy_notify(                  // Handle this
     xcb_destroy_notify_event_t*) {} // Destroy notify event

virtual void
   enter_notify(                    // Handle this
     xcb_enter_notify_event_t*) {}  // Enter notify event

virtual void
   expose(                          // Handle this
     xcb_expose_event_t* e)         // Expose event
{  xcb_rectangle_t r= {PT_t(e->x), PT_t(e->y), e->width, e->height};
   expose(r);
}

virtual void
   focus_in(                        // Handle this
     xcb_focus_in_event_t*) {}      // Focus in event

virtual void
   focus_out(                       // Handle this
     xcb_focus_out_event_t*) {}     // Focus out event

virtual void
   graphics_exposure(               // Handle this
     xcb_graphics_exposure_event_t*) {} // Graphics exposure event

virtual void
   gravity_notify(                  // Handle this
     xcb_gravity_notify_event_t*) {} // Gravity notify event

virtual void
   key_press(                       // Handle this
     xcb_key_press_event_t* event); // Key press event
// (Calls key_input with translated key code.)

virtual void
   key_release(                     // Handle this
     xcb_key_release_event_t*) {}   // Key release event

virtual void
   leave_notify(                    // Handle this
     xcb_leave_notify_event_t*) {}  // Leave notify event

virtual void
   map_notify(                      // Handle this
     xcb_map_notify_event_t*) {}    // Map notify event

virtual void
   map_request(                     // Handle this
     xcb_map_request_event_t*) {}   // Map request event

virtual void
   motion_notify(                   // Handle this
     xcb_motion_notify_event_t*) {} // Motion notify event

virtual void
   no_exposure(                     // Handle this
     xcb_no_exposure_event_t*) {}   // No exposure event

virtual void
   property_notify(                 // Handle this
     xcb_property_notify_event_t*) {} // Property notify event

virtual void
   reparent_notify(                 // Handle this
     xcb_reparent_notify_event_t*) {} // Reparent notify event

virtual void
   resize_request(                  // Handle this
     xcb_resize_request_event_t*) {} // Resize request event

virtual void
   selection_clear(                 // Handle this
     xcb_selection_clear_event_t*) {} // Selection clear event

virtual void
   selection_notify(               // Handle this
     xcb_selection_notify_event_t*) {} // Selection notify event

virtual void
   selection_request(               // Handle this
     xcb_selection_request_event_t*) {} // Selection request event

virtual void
   unmap_notify(                    // Handle this
     xcb_unmap_notify_event_t*) {}  // Unmap notify event

virtual void
   visibility_notify(               // Handle this
     xcb_visibility_notify_event_t*) {} // Visibility notify event
}; // class Window
}  // namespace xcb
#endif // XCB_WINDOW_H_INCLUDED
