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
//       2020/12/08
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <sys/types.h>              // For system types

#include "Xcb/Global.h"             // For xcb::opt_hcdm, xcb::debugh

#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile, EdLine
#include "EdText.h"                 // For EdText
#include "EdView.h"                 // For EdView (Implementation class)

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra brinbup diagnostics?
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
   if( xcb::opt_hcdm )
     xcb::debugh("EdView(%p)::EdView\n", this);
}

   EdView::~EdView( void )          // Destructor
{
   if( xcb::opt_hcdm )
     xcb::debugh("EdView(%p)::~EdView\n", this);
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
     const char*       text) const  // Associated text
{
   xcb::debugf("EdView(%p)::debug(%s)\n", this, text ? text : "");
   xcb::debugf("..col_zero(%zd) col(%u) row_zero(%zd) row(%u)\n"
              , col_zero, col, row_zero, row);
   if( xcb::opt_verbose >= 0 )
     xcb::debugf("..gc_font(%u) gc_flip(%u)\n", gc_font, gc_flip);
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
   using namespace editor;

   const char* buffer= active.get_changed();
   if( xcb::opt_hcdm )
     xcb::debugh("EdView(%p)::commit buffer(%s)\n", this , buffer);

   if( buffer ) {                   // If actually changed
     // Create and initialize a REDO object
     EdRedo* redo= new EdRedo();
     EdLine* line= new EdLine();
     *line= *((EdLine*)text->cursor);
     line->get_prev()->set_next(line);
     line->get_next()->set_prev(line);
     redo->head_remove= redo->tail_remove= (EdLine*)text->cursor;
     redo->head_insert= redo->tail_insert= line;
     text->file->insert_undo(redo); // (Sets text->file->changed)

     // Change the newly created line
     size_t length= active.get_used();
     if( length == 0 )
       line->text= editor::NO_STRING;
     else {
       char* revise= editor::get_text(length + 1);
       strcpy(revise, buffer);
       line->text= revise;
     }
     active.reset(line->text);      // (Prevent duplicate commit)
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
   if( (((EdLine*)text->cursor)->flags & EdLine::F_PROT) == 0 ) // If not protected
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
           xcb::trace(".BOT", 0);
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
           xcb::trace(".TOP", 0);
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
