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
//       EdHist.cpp
//
// Purpose-
//       Editor: Implement EdHist.h
//
// Last change date-
//       2021/01/10
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <unistd.h>                 // For close, ftruncate
#include <sys/stat.h>               // For stat
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/UTF8.h>               // For pub::UTF8

#include "Xcb/Global.h"             // For Xcb globals
#include "Xcb/Types.h"              // For Xcb::Line

#include "Config.h"                 // For namespace config
#include "Editor.h"                 // For namespace editor
#include "EdHist.h"                 // For EdHist
#include "EdText.h"                 // For EdText

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
// Class-
//       EdHist::EdHist
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   EdHist::EdHist( void )           // Constructor
:  EdView(), hist_list()
{  if( HCDM || opt_hcdm ) traceh("EdHist(%p)::EdHist\n", this);
   hist_list.fifo(new EdLine());    // Initial line
}

//----------------------------------------------------------------------------
//
// Class-
//       EdHist::~EdHist
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   EdHist::~EdHist( void )          // Destructor
{  if( HCDM || opt_hcdm ) traceh("EdHist(%p)::~EdHist\n", this);

   for(EdLine* line= hist_list.remq(); line; line= hist_list.remq())
     delete line;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   EdHist::debug(                   // Debugging display
     const char*       info) const  // Associated info
{  debugf("EdHist(%p)::debug(%s)\n", this, info ? info : "");

   debugf("..[%p,%p] %p '%s'\n", hist_list.get_head(), hist_list.get_tail()
         , cursor, cursor ? cursor->text : "");
   unsigned n= 0;
   for(EdLine* line= hist_list.get_head(); line; line= line->get_next() )
     debugf("[%2d] %p '%s'\n", n++, line, line->text);

   EdView::debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::activate
//
// Purpose-
//       Activate the history view
//
//----------------------------------------------------------------------------
void
   EdHist::activate( void )         // Activate the history view
{  if( HCDM || opt_hcdm ) traceh("EdHist(%p)::actiate\n", this);

   col_zero= col= 0;                // Start in column 0
   cursor= nullptr;
   active.reset("");
   EdView::activate();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::enter_key
//
// Purpose-
//       Handle enter keypress
//
// Implementation note-
//       The active buffer is used as a work area.
//
//----------------------------------------------------------------------------
void
   EdHist::enter_key( void )        // Handle enter keypress
{
   char* buffer= const_cast<char*>(active.truncate());
   if( HCDM || opt_hcdm )
     traceh("EdHist(%p)::commit buffer(%s)\n", this, buffer);

   while( *buffer == ' ' )          // Ignore leading blanks
     buffer++;

   cursor= nullptr;                 // No current history line
   col_zero= col= 0;                // Starting in column 0

   int C= *((const unsigned char*)buffer);
   if( C == '\0' ) {                // Empty line: ignore
//   editor::data->activate();      // (Use ESC to exit history view)
//   editor::text->draw_info();
     return;
   }

   // Search for duplicate history line
   for(EdLine* line= hist_list.get_tail(); line; line= line->get_prev()) {
     if( strcmp(buffer, line->text) == 0 ) { // If duplicate line
       cursor= line;
       break;
     }
   }

   // Update history list, moving to last if old or inserting a new line
   if( cursor )
     hist_list.remove(cursor, cursor);
   else
     cursor= new EdLine( editor::allocate(buffer) );
   hist_list.fifo(cursor);

   // Process the command (Using mutable buffer)
   const char* text= cursor->text;
   const char* error= editor::command(buffer);
   if( error )                      // If error encountered
     editor::put_message(error);
   else
     text= "";
   active.reset(text);              // Reset the (mutated) buffer

   editor::text->draw_info();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::get_buffer
//
// Purpose-
//       Get the active buffer (with blank fill)
//
//----------------------------------------------------------------------------
const char*                         // Get the active buffer
   EdHist::get_buffer( void )       // Get the active buffer
{
   active.fetch(col_zero + 256);    // Blank fill
   const char* text= active.get_buffer();
   text += pub::UTF8::index(text, col_zero);
   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::move_cursor_V
//
// Purpose-
//       Move cursor vertically
//
//----------------------------------------------------------------------------
void
   EdHist::move_cursor_V(           // Move cursor vertically
     int             n)             // The relative row (Down positive)
{
   if( n > 0 ) {                    // Move down
     if( cursor == nullptr ) {
       cursor= hist_list.get_head();
       n--;
     }
     while( n-- ) {
       if( cursor->get_next() == nullptr )
         break;
       cursor= cursor->get_next();
     }
   } else if( n < 0 ) {             // Move up
     if( cursor == nullptr ) {
       cursor= hist_list.get_tail();
       n++;
     }
     while( n++ ) {
       if( cursor->get_prev() == nullptr )
         break;
       cursor= cursor->get_prev();
     }
   }

   col_zero= col= 0;
   active.reset(cursor->text);
   editor::text->draw_info();
}
