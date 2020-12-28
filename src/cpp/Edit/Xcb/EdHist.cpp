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
//       2020/12/25
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
// Subroutine: consistency_check, HistLine consistency check
//----------------------------------------------------------------------------
static inline void consistency_check(int line, const HistLine* hist)
{  if( hist->text != hist->line.c_str() )
     Config::alertf("%4d EdHist line: [%p!=%p] [%s','%s']\n", line, hist->text
                   , hist->line.c_str(), hist->text, hist->line.c_str());
}

//----------------------------------------------------------------------------
//
// Class-
//       HistLine::HistLine
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   HistLine::HistLine(              // Default/text constructor
     const char*       _text)       // Associated text
:  EdLine()
{  if( HCDM || opt_hcdm )
     traceh("HistLine(%p)::HistLine(%p)\n", this, _text);

   reset(_text);
}

//----------------------------------------------------------------------------
//
// Class-
//       HistLine::~HistLine
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   HistLine::~HistLine( void )      // Default destructor
{  if( HCDM || opt_hcdm )
     traceh("HistLine(%p)::~HistLine\n", this);

   if( HCDM ) consistency_check(__LINE__, this);
}

//----------------------------------------------------------------------------
//
// Class-
//       HistLine::get_text
//
// Purpose-
//       (Checked) access the text
//
//----------------------------------------------------------------------------
const char*                         // The text
   HistLine::get_text( void ) const // Get text
{  if( HCDM ||  opt_hcdm )
     traceh("HistLine(%p)::get_text %p '%s'\n", this, text, text);

   if( HCDM ) consistency_check(__LINE__, this);
   return text;
}

//----------------------------------------------------------------------------
//
// Class-
//       HistLine::reset
//
// Purpose-
//       Reset the text
//
//----------------------------------------------------------------------------
void
   HistLine::reset(                 // Reset the text
     const char*       _text)       // To this string
{  if( HCDM || opt_hcdm )
     traceh("HistLine(%p)::reset(%p) '%s'\n", this, text, text);

   const char* from= line.c_str();
   if( _text == nullptr )           // If text omitted
     _text= "";                     // Use empty string
   line= _text;                     // Set dynamic string
   text= line.c_str();              // Use dynamic string's text
   if( HCDM )
     traceh("%4d EdHist text(%p<=%p) '%s'<='%s'\n", __LINE__
           , from, text, from, text);
}

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

   hist_list.fifo(new HistLine());  // Initial line
   hist_rows++;                     // (Counted)
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

   for(HistLine* line= hist_list.remq(); line; line= hist_list.remq()) {
     delete line;
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
     const char*       info) const  // Associated info
{  debugf("EdHist(%p)::debug(%s)\n", this, info ? info : "");

   debugf("..[%p,%p] %p '%s'\n", hist_list.get_head(), hist_list.get_tail()
         , hist_line, hist_line ? hist_line->get_text() : "");
   unsigned n= 0;
   for(HistLine* line= hist_list.get_head(); line; line= line->get_next() ) {
     debugf("[%2d] %p '%s'\n", n++, line, line->get_text());
   }
   debugf("[%2d] hist_rows\n", hist_rows);

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
   hist_line= nullptr;
   active.reset("");
   EdView::activate();
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
   if( HCDM || opt_hcdm )
     traceh("EdHist(%p)::commit buffer(%s)\n", this, buffer);

   while( *buffer == ' ' )          // Ignore leading blanks
     buffer++;

   hist_line= nullptr;              // No current history line
   col_zero= col= 0;                // Starting in column 0

   int C= *((const unsigned char*)buffer);
   if( C == '\0' ) {                // Empty line: ignore
//   editor::data->activate();
//   editor::text->draw_info();
     return;
   }

   // Search for duplicate history line
   for(HistLine* line= hist_list.get_tail(); line; line= line->get_prev()) {
     if( strcmp(buffer, line->get_text()) == 0 ) { // If duplicate line
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
       hist_line= hist_list.remq();
       hist_line->reset(buffer);
     }
     hist_list.fifo(hist_line);
   }

   // Process the command
   const char* error= editor::command(buffer);
   active.reset(hist_line->get_text());
   if( error )                      // If error encountered
     editor::put_message(error);

   editor::text->draw_info();
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
     if( hist_line == nullptr ) {
       hist_line= hist_list.get_head();
       n--;
     }
     while( n-- ) {
       if( hist_line->get_next() == nullptr )
         break;
       hist_line= hist_line->get_next();
     }
   } else if( n < 0 ) {             // Move up
     if( hist_line == nullptr ) {
       hist_line= hist_list.get_tail();
       n++;
     }
     while( n++ ) {
       if( hist_line->get_prev() == nullptr )
         break;
       hist_line= hist_line->get_prev();
     }
   }

   col_zero= col= 0;
   active.reset(hist_line->get_text());
   editor::text->draw_info();
}
