//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdView.cpp
//
// Purpose-
//       Editor: Implement EdView.h
//
// Last change date-
//       2021/01/24
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <sys/types.h>              // For system types
#include <pub/Debug.h>              // For namespace pub::debugging

#include "Active.h"                 // For Active
#include "Config.h"                 // For namespace config
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile, EdLine
#include "EdMark.h"                 // For EdMark
#include "EdText.h"                 // For EdText
#include "EdView.h"                 // For EdView (Implementation class)

using namespace config;             // For opt_* variables
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= true                // Extra bringup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
//
// Method-
//       EdView::EdView
//       EdView::~EdView
//
// Purpose-
//       Construtor.
//       Destrutor.
//
//----------------------------------------------------------------------------
   EdView::EdView( void )           // Constructor
:  active()
{
   if( opt_hcdm )
     debugh("EdView(%p)::EdView\n", this);
}

   EdView::~EdView( void )          // Destructor
{
   if( opt_hcdm )
     debugh("EdView(%p)::~EdView\n", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   EdView::debug(                   // Debugging display
     const char*       info) const  // Associated info
{
   if( info ) debugf("EdView(%p)::debug(%s)\n", this, info);
   debugf("..col_zero(%zd) col(%u) row_zero(%zd) row(%u)\n"
         , col_zero, col, row_zero, row);
   debugf("..gc_font(%u) gc_flip(%u) gc_mark(%u)\n"
         , gc_font, gc_flip, gc_mark);
   if( cursor ) {
     debugf("..cursor: "); cursor->debug(); // (Multi-statement line)
   } else
     debugf("..cursor(%p)\n", cursor);
   active.debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::activate
//
// Purpose-
//       Activate thie EdView
//
//----------------------------------------------------------------------------
void
   EdView::activate( void )         // Activate this EdView
{
   editor::view= this;
   editor::text->draw_info();       // (History or Status line)
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::get_gc
//
// Purpose-
//       Get the current graphic context
//
//----------------------------------------------------------------------------
xcb_gcontext_t                       // The current graphic context
   EdView::get_gc( void )            // Get current graphic context
{
   xcb_gcontext_t gc= gc_font;
   if( cursor->flags & EdLine::F_MARK ) {
     ssize_t column= ssize_t(col_zero + col);
     EdMark& mark= *editor::mark;
     if( mark.mark_col < 0
         || ( column >= mark.mark_lh && column <= mark.mark_rh ) )
       gc= gc_mark;
   }

   return gc;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::commit
//
// Purpose-
//       Commit the Active data line
//
// Implementation notes-
//       This is the EdText commit, overridden in EdHist (to do nothing)
//
//----------------------------------------------------------------------------
void
   EdView::commit( void )           // Commit the Active line
{
   const char* buffer= active.get_changed();
   if( opt_hcdm )
     debugh("EdView(%p)::commit buffer(%s)\n", this , buffer);

   if( buffer ) {                   // If actually changed
     // Create a new REDO line, duplicating the current line chain
     EdLine* line= new EdLine();
     *line= *cursor;                // (Duplicates links and delimiters)

     // Replace the text in the REDO line
     line->text= editor::allocate(buffer);
     active.reset(line->text);      // (Prevents duplicate commit)

     // The new line replaces the cursor line in the file_list
     editor::file->remove(cursor, cursor); // (Does not change links)
     editor::file->insert(cursor->get_prev(), line);

     // Create and initialize a REDO object
     EdRedo* redo= new EdRedo();
     redo->head_remove= redo->tail_remove= cursor;
     redo->head_insert= redo->tail_insert= line;
     editor::file->redo_insert(redo); // (Sets file->changed)

     Config::trace(".CSR", "Vcmt", line, cursor); // (New, old)
     cursor= line;                  // Replace the cursor
   } else if( USE_BRINGUP )         // TODO: REMOVE or change to if( HCDM )
     Config::trace(".CSR", "Vnop", cursor, cursor);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::enter_key
//
// Purpose-
//       Handle enter keypress
//
//----------------------------------------------------------------------------
void
   EdView::enter_key( void )        // Handle enter keypress
{  move_cursor_V(+1); }

//----------------------------------------------------------------------------
//
// Method-
//       EdView::move_cursor_V
//
// Purpose-
//       Move cursor vertically
//
//----------------------------------------------------------------------------
void
   EdView::move_cursor_V(           // Move cursor vertically
     int             n)             // The relative row (Down positive)
{
   EdText* text= editor::text;

   text->undo_cursor();
   if( (cursor->flags & EdLine::F_PROT) == 0 ) // If not protected
     commit();                      // Commit the active line

   int rc= 1;                       // Default, no draw
   if( n > 0 ) {                    // Move down
     while( n-- ) {
       if( text->row_used > row )
         row++;
       else {
         EdLine* line= (EdLine*)text->head->get_next();
         if( line ) {
           text->head= line;
           text->row_used--;
           row_zero++;
           rc= 0;
         } else {
           break;
         }
         if( text->tail->get_next() == nullptr )
           row--;
       }
     }
   } else if( n < 0 ) {             // Move up
     while( n++ ) {
       if( row > text->USER_TOP )
         row--;
       else {
         EdLine* line= (EdLine*)text->head->get_prev();
         if( line ) {
           text->head= line;
           row_zero--;
           rc= 0;
         } else {
           break;
         }
       }
     }
   }

   text->synch_active();
   if( rc == 0 )
     text->draw();
   else
     text->draw_info();
}
