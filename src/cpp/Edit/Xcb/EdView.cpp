//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2022 Frank Eskesen.
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
//       2022/12/30
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <sys/types.h>              // For system types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Trace.h>              // For pub::Trace

#include "Active.h"                 // For Active
#include "Config.h"                 // For namespace config
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile, EdLine
#include "EdMark.h"                 // For EdMark
#include "EdTerm.h"                 // For EdTerm
#include "EdView.h"                 // For EdView (Implementation class)

using namespace config;             // For opt_* variables
using namespace pub::debugging;     // For debugging
using pub::Trace;                   // For pub::Trace

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
   debugf("..cursor(%p) col_zero(%zd) col(%u) row_zero(%zd) row(%u)\n"
         , cursor, col_zero, col, row_zero, row);
   debugf("..gc_font(%u) gc_flip(%u) gc_mark(%u)\n"
         , gc_font, gc_flip, gc_mark);
   if( cursor )
     cursor->debug();
   active.debug(info);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::draw_active
//
// Purpose-
//       Redraw the active (data) line
//
//----------------------------------------------------------------------------
void
   EdView::draw_active( void )      // Redraw the active line
{
   EdTerm* term= editor::term;
   active.index(col_zero+term->col_size); // Blank fill
   EdLine line= *cursor;            // (Copy the cursor flags)
   line.flags |= EdLine::F_AUTO;    // (Do not trace ~EdLine)
   line.text= active.get_buffer();
   term->draw_line(row, &line);
   if( editor::view == this )
     term->draw_cursor();
   term->flush();
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
//       EdView::activate
//
// Purpose-
//       Activate this EdView
//
//----------------------------------------------------------------------------
void
   EdView::activate( void )         // Activate this EdView
{
   editor::view= this;              // (EdHist or EdView)
   editor::term->draw_top();        // (History or Status line)
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
//       This is the EdTerm commit, overridden in EdHist (to do nothing)
//
//----------------------------------------------------------------------------
void
   EdView::commit( void )           // Commit the Active line
{
   const char* buffer= active.get_changed();
   if( opt_hcdm )
     debugh("EdView(%p)::commit buffer(%s)\n", this , buffer);

   if( buffer                       // If actually changed
       && (cursor->flags & EdLine::F_PROT) == 0 ) { // and not protected
     // Create a new REDO line, duplicating the current line chain
     EdLine* line= new EdLine();
     *line= *cursor;                // (Duplicates links, flags, and delimiters)

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
     editor::mark->handle_redo(editor::file, redo);

     Trace::trace(".CSR", "Vcmt", cursor, line); // (Old, new)
     cursor= line;                  // Replace the cursor
   } else if( USE_BRINGUP ) {
     Trace::trace(".CSR", "Vnop", cursor, cursor); // (Old, old)
   }
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
   EdTerm* term= editor::term;

   term->undo_cursor();
   commit();                        // Commit the active line

   int rc= 1;                       // Default, no draw
   if( n > 0 ) {                    // Move down
     while( n-- ) {
       if( term->row_used > row )
         row++;
       else {
         EdLine* line= (EdLine*)term->head->get_next();
         if( line ) {
           term->head= line;
           term->row_used--;
           row_zero++;
           rc= 0;
         } else {
           break;
         }
         if( term->tail->get_next() == nullptr )
           row--;
       }
     }
   } else if( n < 0 ) {             // Move up
     while( n++ ) {
       if( row > term->USER_TOP )
         row--;
       else {
         EdLine* line= (EdLine*)term->head->get_prev();
         if( line ) {
           term->head= line;
           row_zero--;
           rc= 0;
         } else {
           break;
         }
       }
     }
   }

   term->synch_active();
   if( rc == 0 )
     term->draw();
   else
     term->draw_status();
}
