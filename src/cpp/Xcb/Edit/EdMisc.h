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
//       EdMisc.h
//
// Purpose-
//       Editor: (Dummy window placeholder.)
//
// Last change date-
//       2020/09/06
//
//----------------------------------------------------------------------------
#ifndef EDMISC_H_INCLUDED
#define EDMISC_H_INCLUDED

#include "Editor.h"                 // TODO: REMOVE. For include file debugging

#include "Xcb/Global.h"             // For xcb::opt_* controls, xcb::trace
#include "Xcb/Window.h"             // For xcb::Window
#include "Xcb/Types.h"              // For xcb_point_t

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
     unsigned          width,       // (X) size width
     unsigned          height,      // (Y) size height
     const char*       name= nullptr) // Widget name
:  Window(nullptr, name ? name : "EdMisc")
{
   if( opt_hcdm )
     debugh("EdMisc(%p)::EdMisc(%u,%u)\n", this, width, height);

   if( height < 14 ) height= 14;    // Must be large enough for text
   use_size.width=  xcb::WH_t(width);
   use_size.height= xcb::WH_t(height);
   min_size= use_size;
}

//----------------------------------------------------------------------------
virtual
   ~EdMisc( void )                  // Destructor
{
   if( opt_hcdm )
     debugh("EdMisc(%s)::~EdMisc\n", this->get_name().c_str());

   if( drawGC ) {
     ENQUEUE("xcb_free_gc", xcb_free_gc_checked(connection, drawGC) );
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
virtual void
   configure( void )                // Configure the Window
{
   if( opt_hcdm )
     debugh("EdMisc(%p)::configure Named(%s)\n", this, get_name().c_str());

   // Create the Window
   Window::configure();

   // Create the Graphic Context
   xcb_connection_t* const conn= window->connection;
   xcb_drawable_t    const draw= window->window_id;
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
//----------------------------------------------------------------------------
virtual void
   draw( void );                    // Draw the Window

//----------------------------------------------------------------------------
// EdMisc: Event handlers
//----------------------------------------------------------------------------
void
   expose(                          // Handle this
     xcb_expose_event_t* event)     // Expose event
{
   if( opt_hcdm )
     debugh("EdMisc(%p)::expose(%d) %d [%d,%d,%d,%d]\n", this
             , event->window, event->count
             , event->x, event->y, event->width, event->height);

   draw();
}
}; // class EdMisc
#endif // EDMISC_H_INCLUDED
