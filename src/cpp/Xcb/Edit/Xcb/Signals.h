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
//       Xcb/Signals.h
//
// Purpose-
//       Signal Event descriptor.
//
// Last change date-
//       2020/10/25
//
//----------------------------------------------------------------------------
#ifndef XCB_SIGNALS_H_INCLUDED
#define XCB_SIGNALS_H_INCLUDED

#include <sys/types.h>              // For uint8_t
#include <xcb/xproto.h>             // For xcb_point_t
#include "pub/Signals.h"            // For namespace pub::signals

#include <Xcb/Widget.h>             // For xcb::Widget

namespace xcb {
//----------------------------------------------------------------------------
//
// Struct-
//       xcb::Event
//
// Purpose-
//       Event descriptor
//
// Implementation note-
//       The Signal interface does NOT require Events to be derived from this.
//
//----------------------------------------------------------------------------
struct Event {                      // Event descriptor
// xcb::Event::Attributes
uint8_t                type;        // Event (sub)type
uint8_t                detail[3];   // Event detail
xcb_point_t            offset;      // XY offset (May be Pixel or Column)

Widget*                widget;      // The Widget originating this Event

// xcb::Event::Constructors
   Event( void )                    // Default constructor
:  type(0), detail{0,0,0}, offset({0,0}), widget(nullptr) {}

   Event(                           // Constructor
     Widget*           _widget)     // The source Widget
:  type(0), detail{0,0,0}, offset({0,0}), widget(_widget) {}

   Event(                           // Constructor
     Widget*           _widget,     // The source Widget
     uint8_t           _type)       // The Event type
:  type(_type), detail{0,0,0}, offset({0,0}), widget(_widget) {}

// xcb::Event::Destructor
virtual
   ~Event( void ) = default;        // Destructor
}; // struct Event
}  // namespace xcb
#endif // XCB_SIGNALS_H_INCLUDED
