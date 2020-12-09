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
//       EdFull.h
//
// Purpose-
//       Editor: Full Window
//
// Last change date-
//       2020/12/08
//
// Implementation note-
//       Used to test utility of a built-in DeviceWindow.
//
//----------------------------------------------------------------------------
#ifndef EDFULL_H_INCLUDED
#define EDFULL_H_INCLUDED

#include <pub/utility.h>            // For pub::to_string

#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include "Xcb/Font.h"               // For Font
#include "Xcb/Device.h"             // For Device
#include "Xcb/Global.h"             // For ENQUEUE macro
#include "Xcb/Keysym.h"             // For X11/keysymdef.h
#include "Xcb/Types.h"              // For Types
#include "Xcb/TextWindow.h"         // For TextWindow base class

#include "Xcb/Global.h"             // For xcb::opt_* controls, xcb::trace
#include "Xcb/Widget.h"             // For xcb::Widget

#include <Editor.h>                 // For namespace editor::debug

//----------------------------------------------------------------------------
//
// Class-
//       EdFull
//
// Purpose-
//       Editor Full Window (PLACEHOLDER)
//
//----------------------------------------------------------------------------
class EdFull : public xcb::TextWindow { // Editor Full Window
//----------------------------------------------------------------------------
// EdFull::Attributes
public: // NONE DEFINED
enum // Compile-time constants
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= true                // Use bringup diagnostics?
}; // Compile-time constants

//----------------------------------------------------------------------------
// EdFull::Constructor
//----------------------------------------------------------------------------
public:
   EdFull( void )                   // Constructor
:  xcb::TextWindow(nullptr, "EdFull")
{
   if( editor::debug::opt_hcdm )
     editor::debug::debugh("EdFull(%p)::EdFull\n", this);
}

//----------------------------------------------------------------------------
// EdFull::Destructor
//----------------------------------------------------------------------------
virtual
   ~EdFull( void )                  // Destructor
{
   if( editor::debug::opt_hcdm )
     editor::debug::debugh("EdFull(%p)::~EdFull\n", this);
}

//============================================================================
// EdFull: Overridden Window classes
//----------------------------------------------------------------------------
public:
virtual void
   configure_notify(                // Handle this
     xcb_configure_notify_event_t* event) // Configure notify event
{
   if( editor::debug::opt_hcdm )
     editor::debug::debugh("EdFull(%p)::configure_notify(%d,%d)\n", this
           , event->width, event->height);

   resize(event->width, event->height);
}

virtual void
   expose(                          // Handle this
     const
     xcb_rectangle_t&  rect)        // Expose event
{
   if( editor::debug::opt_hcdm )
     editor::debug::debugh("EdFull(%p)::expose([%d,%d,%d,%d])\n", this
             , rect.x, rect.y, rect.width, rect.height);

   draw();
}
//============================================================================

//----------------------------------------------------------------------------
//
// Method-
//       EdFull::draw
//
// Purpose-
//       Redraw the window
//
//----------------------------------------------------------------------------
virtual void
   draw( void )                     // Redraw the Window
{
   if( editor::debug::opt_hcdm )
     editor::debug::debugh("EdFull(%p)::draw()\n", this);

   // Clear the window. // TODO: optimize (if necessary)
   if( true ) {
     xcb::WH_size_t size= get_size(__LINE__); // ** EXPERIMENTAL **
     rect.width=  size.width;
     rect.height= size.height;
   }

   NOQUEUE("xcb_clear_area", xcb_clear_area
          ( c, 0, widget_id, 0, 0, rect.width, rect.height) );

   if( USE_BRINGUP ) {
     // BRINGUP: Draw diagonal line (to see where boundaries are)
     if( editor::debug::opt_hcdm && editor::debug::opt_verbose > 2 ) {
//     debug(pub::utility::to_string("%4d EdFull diagonal", __LINE__).c_str());
       xcb_point_t points[2]= { {0,                0}
                              , {xcb::PT_t(rect.width), xcb::PT_t(rect.height)}
                              };
       NOQUEUE("xcb_poly_line", xcb_poly_line(c
              , XCB_COORD_MODE_ORIGIN, widget_id, font.fontGC, 2, points));
       if( editor::debug::opt_verbose > 2 )
         editor::debug::debugf("%4d POLY {0,{%d,%d}}\n", __LINE__, rect.width, rect.height);
     }
   }

   // Redraw complete
   flush();
}
}; // class EdFull
#endif // EDFULL_H_INCLUDED
