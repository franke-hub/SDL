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
//       EdView.h
//
// Purpose-
//       Editor: TextWindow view
//
// Last change date-
//       2020/12/04
//
//----------------------------------------------------------------------------
#ifndef EDVIEW_H_INCLUDED
#define EDVIEW_H_INCLUDED

#include <string>                   // For std::string
#include <sys/types.h>              // For system types

#include "Xcb/Global.h"             // For xcb::opt_hcdm, xcb::debugh

#include "Editor.h"                 // For Editor
#include "EdFile.h"                 // For EdFile, EdLine
#include "EdText.h"                 // For EdText

//----------------------------------------------------------------------------
//
// Class-
//       EdView
//
// Purpose-
//       TextWindow view.
//
//----------------------------------------------------------------------------
class EdView {                      // Editor TextWindow view
//----------------------------------------------------------------------------
// EdView::Attributes
//----------------------------------------------------------------------------
public:
xcb::Active            active;      // The Active text buffer

size_t                 col_zero= 0; // Current column[0]
size_t                 row_zero= 0; // Current row[0]
unsigned               col= 0;      // Current screen column
unsigned               row= 0;      // Current screen row

// Graphic context object identifiers are copies, neither allocated nor deleted
xcb_gcontext_t         gc_flip= 0;  // Graphic context: cursor character
xcb_gcontext_t         gc_font= 0;  // Graphic context: normal line

//----------------------------------------------------------------------------
// EdView::Constructor
//----------------------------------------------------------------------------
public:
   EdView( void )                   // Constructor
:  active()
{
   if( xcb::opt_hcdm )
     xcb::debugh("EdView(%p)::EdView\n", this);
}

//----------------------------------------------------------------------------
// EdView::Destructor
//----------------------------------------------------------------------------
virtual
   ~EdView( void )                  // Destructor
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
virtual void
   debug(                           // Debugging display
     const char*       text= nullptr) const // Associated text
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
//       TODO: Add REDO/UNDO logic
//
//----------------------------------------------------------------------------
virtual void
   commit( void )                   // Commit the Active line
{
   Editor* edit= Editor::editor;
   EdText* text= edit->text;

   const char* buffer= active.get_changed();
   if( xcb::opt_hcdm )
     xcb::debugh("EdView(%p)::commit buffer(%s)\n", this , buffer);

   if( buffer ) {                   // If actually changed (and changeable)
     text->file->changed= true;     // The file has changed

     size_t length= active.get_used();
     if( length == 0 )
       text->cursor->text= Editor::NO_STRING;
     else {
       char* revise= edit->get_text(length + 1);
       strcpy(revise, buffer);
       text->cursor->text= revise;
     }
     active.reset(text->cursor->text); // (Prevent duplicate commit) // TODO: VERIFY NEEDED
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
virtual void
   move_cursor_V(                   // Move cursor vertically
     int             n= 1)          // The relative row (Down positive)
{
   Editor* edit= Editor::editor;
   EdText* text= edit->text;

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
}; // class EdView
#endif // EDVIEW_H_INCLUDED
