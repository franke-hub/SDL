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
//       EdMisc.h
//
// Purpose-
//       Editor: (Dummy window placeholder.)
//
// Last change date-
//       2020/12/12
//
//----------------------------------------------------------------------------
#ifndef EDMISC_H_INCLUDED
#define EDMISC_H_INCLUDED

#include "Xcb/Window.h"             // For xcb::Window
#include "Xcb/Types.h"              // For xcb types

//----------------------------------------------------------------------------
//
// Class-
//       EdMisc
//
// Purpose-
//       Dummy window, placeholder base
//
//----------------------------------------------------------------------------
class EdMisc : public xcb::Window { // Editor Window (placeholder)
//----------------------------------------------------------------------------
// EdMisc::Attributes
//----------------------------------------------------------------------------
public:
xcb_gcontext_t         drawGC= 0;   // The default graphic context

//----------------------------------------------------------------------------
// EdMisc::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdMisc(                          // Constructor
     Widget*           parent= nullptr, // The parent Widget
     const char*       name= nullptr, // The Widget name
     unsigned          width= 0,    // (X) size width
     unsigned          height= 0);  // (Y) size height

virtual
   ~EdMisc( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       EdMisc::configure
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
//       EdMisc::draw
//
// Purpose-
//       Draw the Window
//
//----------------------------------------------------------------------------
virtual void
   draw( void );                    // Draw the Window

//----------------------------------------------------------------------------
// EdMisc::Event handlers
//----------------------------------------------------------------------------
void
   expose(                          // Handle this
     xcb_expose_event_t* event);    // Expose event
}; // class EdMisc
#endif // EDMISC_H_INCLUDED
