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
//       Tester.cpp
//
// Purpose-
//       Implement Tester.h, bringup test Window
//
// Last change date-
//       2021/01/26
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <unistd.h>                 // For close, ftruncate
#include <sys/stat.h>               // For stat
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include <pub/Debug.h>              // For namespace pub::debugging

#include "Config.h"                 // For namespace config
#include "Tester.h"                 // For Tester

using namespace config;             // For opt_* variables
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra bringup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
//
// Method-
//       Tester::Tester
//       Tester::~Tester
//
// Purpose-
//       Constructor/Destructor
//
//----------------------------------------------------------------------------
   Tester::Tester(                  // Constructor
     Widget*           parent,      // The parent Widget
     const char*       name,        // The Widget name
     unsigned          width,       // (X) size width
     unsigned          height)      // (Y) size height
:  gui::Window(parent, name ? name : "Tester")
{
   if( opt_hcdm )
     debugh("Tester(%p)::Tester(%u,%u)\n", this, width, height);

   if( width  < 100 ) width=  100;
   if( height < 100 ) height= 100;
   use_size.width=  gui::WH_t(width);
   use_size.height= gui::WH_t(height);
   min_size= use_size;
}

   Tester::~Tester( void )                  // Destructor
{
   if( opt_hcdm )
     debugh("Tester(%s)::~Tester\n", this->get_name().c_str());

   if( drawGC ) {
     ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, drawGC) );
     drawGC= 0;
   }

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       Tester::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
void
   Tester::configure( void )        // Configure the Window
{
   if( opt_hcdm )
     debugh("Tester(%p)::configure Named(%s)\n", this, get_name().c_str());

   // Create the Window
   emask |= XCB_EVENT_MASK_BUTTON_PRESS
          | XCB_EVENT_MASK_EXPOSURE
          | XCB_EVENT_MASK_STRUCTURE_NOTIFY
          ;

   Window::configure();

   // Create the Graphic Context
   gui::Pixel_t bg= 0x00FFFFFF;
   gui::Pixel_t fg= 0x00FF0000;

   drawGC= xcb_generate_id(c);
   uint32_t mask= XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
   uint32_t parm[2];
   parm[0]= fg;
   parm[1]= bg;
   ENQUEUE("xcb_create_gc", xcb_create_gc(c, drawGC, widget_id, mask, parm) );

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       Tester::draw
//
// Purpose-
//       Draw the Window
//
//----------------------------------------------------------------------------
void
   Tester::draw( void )             // Draw the Window
{
   if( opt_hcdm )
     debugh("Tester(%p)::draw Named(%s)\n", this, get_name().c_str());

   clear();
   gui::PT_t X= gui::PT_t(rect.width - 1);
   gui::PT_t Y= gui::PT_t(rect.height - 1);
   xcb_point_t points[]=
       { {0, 0}
       , {0, Y}
       , {X, Y}
       , {X, 0}
       , {0, 0}
       , {X, Y}
       };

   ENQUEUE("xcb_poly_line", xcb_poly_line_checked(c
          , XCB_COORD_MODE_ORIGIN, widget_id, drawGC, 6, points));
   if( opt_hcdm || false ) {
     debugf("Tester::draw %u:[%d,%d]\n", drawGC, X, Y);
     for(int i= 0; i<6; i++)
       debugf("[%2d]: [%2d,%2d]\n", i, points[i].x, points[i].y);
   }

   flush();
}

//----------------------------------------------------------------------------
// Tester::Event handlers
//----------------------------------------------------------------------------
void
   Tester::configure_notify(        // Handle this
     xcb_configure_notify_event_t* E) // Configure notify event
{
   if( opt_hcdm )
     debugh("Tester(%p)::configure_notify(%d,%d)\n", this
           , E->width, E->height);

   unsigned x= E->width;
   unsigned y= E->height;

   // If size unchanged, do nothing
   if( rect.width == x && rect.height == y ) // If unchanged
     return;                        // Nothing to do

   // Reconfigure and draw the window
   set_size(x, y);
   rect.width=  x;
   rect.height= y;
   draw();
}

void
   Tester::expose(                  // Handle this
     xcb_expose_event_t* event)     // Expose event
{
   if( opt_hcdm )
     debugh("Tester(%p)::expose(%u) %d [%d,%d,%d,%d]\n", this, event->window
           , event->count, event->x, event->y, event->width, event->height);

   draw();
}

