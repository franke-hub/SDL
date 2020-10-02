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
//       Xcb/Device.h
//
// Purpose-
//       XCB device driver
//
// Last change date-
//       2020/09/30
//
//----------------------------------------------------------------------------
#ifndef XCB_DEVICE_H_INCLUDED
#define XCB_DEVICE_H_INCLUDED

#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include "Xcb/Signal.h"             // For Signal
#include "Xcb/Widget.h"             // For Widget
#include "Xcb/Window.h"             // Our base class

namespace xcb {
//----------------------------------------------------------------------------
//
// Struct-
//       xcb::DeviceEvent
//
// Purpose-
//       Generic Device Event
//
//----------------------------------------------------------------------------
struct DeviceEvent : public Event { // Device Event descriptor
using Event::Event;                 // Use Event constructors

enum TYPE                           // Event Subtype
{  TYPE_ERROR                       // (Invalid type)
,  TYPE_CLOSE                       // Close Device event
}; // enum TYPE
}; // struct DeviceEvent

//----------------------------------------------------------------------------
//
// Class-
//       xcb::Device
//
// Purpose-
//       xcb::Device object, the root xcb::Window.
//
//----------------------------------------------------------------------------
class Device : public Window {      // XCB device driver
//----------------------------------------------------------------------------
// xcb::Device::Attributes
//----------------------------------------------------------------------------
public:
Signal<DeviceEvent>    signal;      // The DeviceEvent signal
Display*               display= nullptr; // X11 Display
bool                   operational= true; // TRUE while operational

// Defaults
xcb_rectangle_t        geom= {0,0,0,0}; // Device geometry

// XCB atoms
xcb_atom_t             protocol= 0; // WM_PROTOCOLS atom
xcb_atom_t             wm_close= 0; // WM_CLOSE atom

//----------------------------------------------------------------------------
// xcb::Device::Constructors/Destructors/Operators
//----------------------------------------------------------------------------
public:
   Device( void );                  // Constructor

virtual
   ~Device( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Device::atom_to_name
//       xcb::Device::name_to_atom
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
// xcb::Device::Methods
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Implementation note-
//   For all other Widgets, configure() is the *last* configure method.
//   Device::configure() *begins* the configuation process
virtual void
   configure( void );               // Configure everything

virtual void
   debug_tree(                      // Debug the Widget tree
     const char*       info= nullptr); // Caller's information

virtual void
   draw( void );                    // Draw everything

static Window*                      // The Window, if located
   locate(                          // Recursively locate
     xcb_window_t      target,      // This XCB window
     Widget*           widget);     // Starting here (not checked)

xcb_keysym_t                        // The associated symbol
   to_keysym(                       // Convert to Keysym
     const xcb_key_press_event_t*
                       event) const; // This key press (or release) event

//----------------------------------------------------------------------------
// xcb::Device::Thread simulation
//----------------------------------------------------------------------------
public:
void
   join( void );                    // Wait for Thread completion

virtual void
   run( void );                     // Event processor

void
   start( void );                   // Start the Thread

//----------------------------------------------------------------------------
// xcb::Device::Event handlers (Not associated with a Window or Drawable)
//----------------------------------------------------------------------------
virtual void
   ge_generic(                      // Handle this
     xcb_ge_generic_event_t*) {}    // GE generic event

virtual void
   keymap_notify(                   // Handle this
     xcb_keymap_notify_event_t*) {} // Keymap notify event

virtual void
   mapping_notify(                  // Handle this
     xcb_mapping_notify_event_t*) {} // Mapping notify event
}; // class xcb::Device
}  // namespace xcb
#endif // XCB_DEVICE_H_INCLUDED
