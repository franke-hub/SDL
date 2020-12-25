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
//       EdMisc.cpp
//
// Purpose-
//       Editor: Implement EdMisc.h
//
// Last change date-
//       2020/12/23
//
// Implementation note-
//       Used to avoid circular references.
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
#include "Editor.h"                 // For namespace editor
#include "EdMisc.h"                 // For EdHist

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
//       EdMisc::EdMisc
//       EdMisc::~EdMisc
//
// Purpose-
//       Constructor/Destructor
//
//----------------------------------------------------------------------------
   EdMisc::EdMisc(                  // Constructor
     Widget*           parent,      // The parent Widget
     const char*       name,        // The Widget name
     unsigned          width,       // (X) size width
     unsigned          height)      // (Y) size height
:  Window(parent, name ? name : "EdMisc")
{
   if( opt_hcdm )
     debugh("EdMisc(%p)::EdMisc(%u,%u)\n", this, width, height);

   if( width  < 14 ) width=  14;    // Must be large enough for text
   if( height < 14 ) height= 14;    // Must be large enough for text
   use_size.width=  xcb::WH_t(width);
   use_size.height= xcb::WH_t(height);
   min_size= use_size;
}

   EdMisc::~EdMisc( void )                  // Destructor
{
   if( opt_hcdm )
     debugh("EdMisc(%s)::~EdMisc\n", this->get_name().c_str());

   if( drawGC ) {
     ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, drawGC) );
     drawGC= 0;
   }

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMisc::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
void
   EdMisc::configure( void )        // Configure the Window
{
   if( opt_hcdm )
     debugh("EdMisc(%p)::configure Named(%s)\n", this, get_name().c_str());

   // Create the Window
   Window::configure();

   // Create the Graphic Context
   xcb_connection_t* const conn= window->c;
   xcb_drawable_t    const draw= window->widget_id;
   xcb::Pixel_t bg= 0x00FFFFFF;
   xcb::Pixel_t fg= 0x00FF0000;

   drawGC= xcb_generate_id(conn);
   uint32_t mask= XCB_GC_FOREGROUND | XCB_GC_BACKGROUND;
   uint32_t parm[2];
   parm[0]= fg;
   parm[1]= bg;
   window->ENQUEUE("xcb_create_gc",
                   xcb_create_gc(conn, drawGC, draw, mask, parm) );

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMisc::draw
//
// Purpose-
//       Draw the Window
//
// Implementation note-
//       ANOMOLY: The draw ONLY visible when the debugging display occurs.
//                (Looks like a timing problem.)
//       PROBLEM: USER ERROR: Expose events ignored. (Now fixed.)
//
//----------------------------------------------------------------------------
void
   EdMisc::draw( void )             // Draw the Window
{
   if( opt_hcdm )
     debugh("EdMisc(%p)::draw Named(%s)\n", this, get_name().c_str());

   xcb::PT_t X= xcb::PT_t(rect.width - 1);
   xcb::PT_t Y= xcb::PT_t(rect.height - 1);
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
   if( opt_hcdm || false ) {        // ???WHY IS THIS NEEDED???
     debugf("EdMisc::draw %u:[%d,%d]\n", drawGC, X, Y);
     for(int i= 0; i<6; i++)
       debugf("[%2d]: [%2d,%2d]\n", i, points[i].x, points[i].y);
   }

// (Attempts to fix problem without expose handler.)
// ::pub::Thread::sleep(0.001);     // Does this fix the problem?  NO!
// ::pub::Thread::sleep(0.010);     // Does this fix the problem? (sometimes)
// ::pub::Thread::sleep(0.020);     // Does this fix the problem? YES!

   flush();
}

//----------------------------------------------------------------------------
// EdMisc::Event handlers
//----------------------------------------------------------------------------
void
   EdMisc::expose(                  // Handle this
     xcb_expose_event_t* event)     // Expose event
{
   if( opt_hcdm )
     debugh("EdMisc(%p)::expose(%u) %d [%d,%d,%d,%d]\n", this, event->window
           , event->count, event->x, event->y, event->width, event->height);

   draw();
}
