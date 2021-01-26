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
//       Tester.h
//
// Purpose-
//       Bringup test Window.
//
// Last change date-
//       2021/01/26
//
//----------------------------------------------------------------------------
#ifndef TESTER_H_INCLUDED
#define TESTER_H_INCLUDED

#include <gui/Types.h>              // For GUI types
#include <gui/Window.h>             // For gui::Window

//----------------------------------------------------------------------------
//
// Class-
//       Tester
//
// Purpose-
//       Bringup test Window
//
//----------------------------------------------------------------------------
class Tester : public gui::Window { // Bringup Window
//----------------------------------------------------------------------------
// Tester::Attributes
//----------------------------------------------------------------------------
public:
xcb_gcontext_t         drawGC= 0;   // The default graphic context

//----------------------------------------------------------------------------
// Tester::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   Tester(                          // Constructor
     Widget*           parent= nullptr, // The parent Widget
     const char*       name= nullptr, // The Widget name
     unsigned          width= 0,    // (X) size width
     unsigned          height= 0);  // (Y) size height

virtual
   ~Tester( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       Tester::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
virtual void
   configure( void );               // Configure the Window

//----------------------------------------------------------------------------
//
// Method-
//       Tester::draw
//
// Purpose-
//       Draw the Window
//
//----------------------------------------------------------------------------
virtual void
   draw( void );                    // Draw the Window

//----------------------------------------------------------------------------
// Tester::Event handlers
//----------------------------------------------------------------------------
void
   configure_notify(                // Handle this
     xcb_configure_notify_event_t* E); // Configure notify event

void
   expose(                          // Handle this
     xcb_expose_event_t* event);    // Expose event
}; // class Tester
#endif // TESTER_H_INCLUDED
