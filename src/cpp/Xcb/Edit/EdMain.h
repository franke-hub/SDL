//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdMain.h
//
// Purpose-
//       Editor: Main Window
//
// Last change date-
//       2020/10/02
//
// Implementation note-
//       Used to test utility of a built-in DeviceWindow.
//
//----------------------------------------------------------------------------
#ifndef EDMAIN_H_INCLUDED
#define EDMAIN_H_INCLUDED

#include "Bringup.h"                // TODO: REMOVE

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

using namespace xcb;                // For xcb objects

//----------------------------------------------------------------------------
//
// Class-
//       EdMain
//
// Purpose-
//       Editor Main Window (PLACEHOLDER)
//
//----------------------------------------------------------------------------
class EdMain : public xcb::TextWindow { // Editor Main Window
//----------------------------------------------------------------------------
// EdMain::Attributes
public: // NONE DEFINED
enum // Compile-time constants
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= true                // Use bringup diagnostics?
}; // Compile-time constants

//----------------------------------------------------------------------------
// EdMain::Constructor
//----------------------------------------------------------------------------
public:
   EdMain( void )                   // Constructor
:  TextWindow(nullptr, "EdMain")
{
   if( opt_hcdm )
     debugh("EdMain(%p)::EdMain\n", this);
}

//----------------------------------------------------------------------------
// EdMain::Destructor
//----------------------------------------------------------------------------
virtual
   ~EdMain( void )                  // Destructor
{
   if( opt_hcdm )
     debugh("EdMain(%p)::~EdMain\n", this);
}

//============================================================================
// xcb::EdMain: Overridden Window classes
//----------------------------------------------------------------------------
public:
virtual void
   configure_notify(                // Handle this
     xcb_configure_notify_event_t* event) // Configure notify event
{
   if( opt_hcdm )
     debugh("EdMain(%p)::configure_notify(%d,%d)\n", this
           , event->width, event->height);

   resize(event->width, event->height);
}

virtual void
   expose(                          // Handle this
     const
     xcb_rectangle_t&  rect)        // Expose event
{
   if( opt_hcdm )
     debugh("EdMain(%p)::expose([%d,%d,%d,%d])\n", this
             , rect.x, rect.y, rect.width, rect.height);

   draw();
}
//============================================================================

//----------------------------------------------------------------------------
//
// Method-
//       EdMain::draw
//
// Purpose-
//       Redraw the window
//
//----------------------------------------------------------------------------
virtual void
   draw( void )                     // Redraw the Window
{
   if( opt_hcdm )
     debugh("EdMain(%p)::draw()\n", this);

   // Clear the window. // TODO: optimize (if necessary)
   if( true ) {
     WH_size_t size= get_size(__LINE__); // ** EXPERIMENTAL **
     rect.width=  size.width;
     rect.height= size.height;
   }

   NOQUEUE("xcb_clear_area", xcb_clear_area
          ( c, 0, widget_id, 0, 0, rect.width, rect.height) );

   if( USE_BRINGUP && false ) {
     // BRINGUP: Draw diagonal line (to see where boundaries are)
     if( opt_hcdm && opt_verbose > 2 ) {
//     debug(pub::utility::to_string("%4d EdMain diagonal", __LINE__).c_str());
       xcb_point_t points[2]= { {0,                0}
                              , {PT_t(rect.width), PT_t(rect.height)}
                              };
       NOQUEUE("xcb_poly_line", xcb_poly_line(c
              , XCB_COORD_MODE_ORIGIN, widget_id, font.fontGC, 2, points));
       if( opt_verbose > 2 )
         debugf("%4d POLY {0,{%d,%d}}\n", __LINE__, rect.width, rect.height);
     }
   }

   // Redraw complete
   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMain::resize
//
// Purpose-
//       Resize the window
//
//----------------------------------------------------------------------------
void
   resize(                          // Resize the Window
     int               x,           // New width
     int               y)           // New height
{
   if( opt_hcdm )
     debugh("EdMain(%p)::resize(%d,%d)\n", this, x, y);

   if( x < 128 ) x= 128;
   if( y < 128 ) y= 128;
   if( true  ) {                    // This is required. ??? WHY ???
     x += 7; x &= 0xfffffff8;
     y += 7; y &= 0xfffffff8;
   }

   // If size unchanged, do nothing
   WH_size_t size= get_size(__LINE__);
   if( size.width == x && size.height == y ) // If unchanged
     return;                        // Nothing to do

   // Reconfigure the window
   set_size(x, y, __LINE__);

   // Diagnostics
   if( opt_hcdm ) {
     WH_size_t size= get_size();
     debugf("%4d [%d x %d]= chg_size <= [%d x %d]\n",  __LINE__
           , size.width, size.height, rect.width, rect.height);
     rect.width=  size.width;
     rect.height= size.height;
   }

// draw(); // Not required: Expose events generated
}
}; // class EdMain
#endif // EDMAIN_H_INCLUDED
