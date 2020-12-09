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
//       2020/12/08
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <unistd.h>                 // For close, ftruncate
#include <sys/stat.h>               // For stat
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include "Xcb/Global.h"             // For Xcb globals
#include "Xcb/Types.h"              // For Xcb::Line

#include "Editor.h"                 // For namespace editor::debug
#include "EdHist.h"                 // For EdHist
#include "EdText.h"                 // For EdText

using namespace editor::debug;
#define debugf editor::debug::debugf // Avoid ADL lookup

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra brinbup diagnostics?
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
{  if( opt_hcdm )
     debugh("EdHist(%p)::EdHist\n", this);

   hist_list.fifo(new HistLine());  // Add empty text line (uncounted)
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
{  if( opt_hcdm ) debugh("EdHist(%p)::~EdHist\n", this);

   HistLine* line= hist_list.remq();
   while ( line ) {
     delete line;
     line= hist_list.remq();
   }
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
     const char*       text) const  // Associated text
{
   debugf("EdHist(%p)::debug(%s)\n", this, text ? text : "");

   debugf("..hist_line(%p) '%s'\n", hist_line
         , hist_line ? hist_line->text : "");
   unsigned n= 0;
   for(HistLine* line= hist_list.get_tail(); line; line= (HistLine*)line->get_prev() ) {
     debugf("[%2d] %p '%s'\n", n++, line, line->text);
   }

// EdView::debug(text);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::activate
//
// Purpose-
//       Activate the history line
//
//----------------------------------------------------------------------------
void
   EdHist::activate( void )         // Activate the history line
{
   if( hist_line == nullptr ) {     // If no history line active
     col_zero= col= 0;              // Start in column 0
     hist_line= hist_list.get_tail(); // Use TOP empty line
     active.reset(hist_line->text);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::commit
//
// Purpose-
//       Commit the Active line
//
// Implementation note-
//       The active buffer is used as a work area.
//
//----------------------------------------------------------------------------
void
   EdHist::commit( void )           // Commit the Active line
{
   char* buffer= const_cast<char*>(active.truncate());
   if( opt_hcdm )
     debugh("EdHist(%p)::commit buffer(%s)\n", this, buffer);

   while( *buffer == ' ' )          // Ignore leading blanks
     buffer++;

   hist_line= nullptr;              // No current history line
   col_zero= col= 0;                // Starting in column 0

   int C= *((const unsigned char*)buffer);
   if( C == '\0' || C == '#' ) {    // Ignore comment
     hist_line= nullptr;
     editor::text->view= editor::text->data;
     editor::text->draw_info();
     return;
   }

   // Search for duplicate history line
   for(HistLine* line= hist_list.get_head(); line; line= (HistLine*)line->get_next()) {
     if( strcmp(buffer, line->text) == 0 ) { // If duplicate line
       hist_line= line;
       break;
     }
   }

   // If not a duplicate, add line to history
   if( hist_line == nullptr ) {
     if( hist_rows < MAX_ROWS ) {
       hist_line= new HistLine(buffer);
       hist_rows++;
     } else {
       hist_line= (HistLine*)hist_list.remq();
       hist_line->reset(buffer);
     }
     hist_list.insert(hist_list.get_tail()->get_prev(), hist_line, hist_line);
     active.reset(hist_line->text);
     active.get_buffer();           // (Used as a work area)
   }

   // Process the command
   editor::command(buffer);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::get_active
//
// Purpose-
//       Get the active  history line
//
//----------------------------------------------------------------------------
const char*                         // Get the active history line
   EdHist::get_active( void )       // Get the active history line
{
   active.fetch(col_zero + 256);    // Blank fill
   const char* text= active.get_buffer();
   text += pub::UTF8::index((const unsigned char*)text, col_zero);
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
     while( n-- ) {
       if( hist_line->get_next() == nullptr )
         break;
       hist_line= (HistLine*)hist_line->get_next();
     }
   } else if( n < 0 ) {             // Move up
     while( n++ ) {
       if( hist_line->get_prev() == nullptr )
         break;
       hist_line= (HistLine*)hist_line->get_prev();
     }
   }

   col_zero= col= 0;
   active.reset(hist_line->text);
   editor::text->draw_info();
}
