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
//       EdView.cpp
//
// Purpose-
//       Editor: Implement EdView.h
//
// Last change date-
//       2020/12/27
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <sys/types.h>              // For system types
#include <pub/Debug.h>              // For namespace pub::debugging

#include "Config.h"                 // For namespace config
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile, EdLine
#include "EdText.h"                 // For EdText
#include "EdView.h"                 // For EdView (Implementation class)

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
{  editor::view= this; }

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
   if( cursor->flags & EdLine::F_MARK )
     gc= gc_mark;

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
//       This is the EdText commit, overridden in EdHist
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
     size_t length= active.get_used();
     if( length == 0 )
       line->text= "";
     else {
       char* revise= editor::allocate(length + 1);
       strcpy(revise, buffer);
       line->text= revise;
     }
     active.reset(line->text);      // (Prevents duplicate commit)

     // The new line replaces the cursor line in the file_list
     editor::file->remove(cursor, cursor); // (Does not change links)
     editor::file->insert(cursor->get_prev(), line);

     // Create and initialize a REDO object
     EdRedo* redo= new EdRedo();
     redo->head_remove= redo->tail_remove= cursor;
     redo->head_insert= redo->tail_insert= line;
     editor::file->insert_undo(redo); // (Sets file->changed)

     cursor= line;                  // Replace the cursor
   }
}

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
         EdLine* line= (EdLine*)text->line->get_next();
         if( line ) {
           text->line= line;
           text->row_used--;
           row_zero++;
           rc= 0;
         } else {
           break;
         }
         if( text->last->get_next() == nullptr )
           row--;
       }
     }
   } else if( n < 0 ) {             // Move up
     while( n++ ) {
       if( row > text->USER_TOP )
         row--;
       else {
         EdLine* line= (EdLine*)text->line->get_prev();
         if( line ) {
           text->line= line;
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
