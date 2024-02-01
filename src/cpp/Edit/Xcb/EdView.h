//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
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
//       Editor: Editor data view
//
// Last change date-
//       2024/01/25
//
//----------------------------------------------------------------------------
#ifndef EDVIEW_H_INCLUDED
#define EDVIEW_H_INCLUDED

#include <string>                   // For std::string
#include <sys/types.h>              // For system types
#include <xcb/xproto.h>             // For xcb_gcontext_t, ...

#include "Active.h"                 // For Active
#include "Editor.h"                 // For Editor
#include "EdFile.h"                 // For EdFile, EdLine
#include "EdTerm.h"                 // For EdTerm

//----------------------------------------------------------------------------
//
// Class-
//       EdView
//
// Purpose-
//       Editor data view.
//
//----------------------------------------------------------------------------
class EdView {                      // Editor data view
//----------------------------------------------------------------------------
// EdView::Attributes
//----------------------------------------------------------------------------
public:
Active                 active;      // The Active text buffer
EdLine*                cursor= nullptr; // The Active cursor line

size_t                 col_zero= 0; // Current column[0]
size_t                 row_zero= 0; // Current row[0]
unsigned               col= 0;      // Current screen column
unsigned               row= 0;      // Current screen row

// Graphic context object identifiers are copies, neither allocated nor deleted
xcb_gcontext_t         gc_flip= 0;  // Graphic context: cursor character
xcb_gcontext_t         gc_font= 0;  // Graphic context: normal line
xcb_gcontext_t         gc_mark= 0;  // Graphic context: marked character

//----------------------------------------------------------------------------
// EdView::Destructor/Constructor
//----------------------------------------------------------------------------
public:
virtual
   ~EdView( void );                 // Destructor

   EdView( void );                  // Constructor

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
     const char*       info= nullptr) const; // Associated info

//----------------------------------------------------------------------------
//
// Method-
//       EdView::get_column
//
// Purpose-
//       Get the current column
//
//----------------------------------------------------------------------------
virtual size_t                      // The current column number
   get_column( void )               // Get current column number
{  return col_zero + col; }         // The current column number

//----------------------------------------------------------------------------
//
// Method-
//       EdView::get_gc
//
// Purpose-
//       Get the current graphic context
//
//----------------------------------------------------------------------------
virtual xcb_gcontext_t              // The current graphic context
   get_gc( void );                  // Get current graphic context

//----------------------------------------------------------------------------
//
// Method-
//       EdView::get_row
//
// Purpose-
//       Get the current row
//
//----------------------------------------------------------------------------
virtual size_t                      // The current row number
   get_row( void )                  // Get current row number
{  return row_zero + row; }         // The current row number

//----------------------------------------------------------------------------
//
// Method-
//       EdView::activate
//
// Purpose-
//       Activate the view
//
//----------------------------------------------------------------------------
virtual void
   activate( void );                // Activate the view

//----------------------------------------------------------------------------
//
// Method-
//       EdView::commit
//
// Purpose-
//       Commit the Active data line
//
// Implementation notes-
//       This is the data view commit. It's overridden in EdHist
//
//----------------------------------------------------------------------------
virtual void
   commit( void );                  // Commit the Active line

//----------------------------------------------------------------------------
//
// Method-
//       EdView::draw_active
//
// Purpose-
//       Redraw the active (data) line
//
//----------------------------------------------------------------------------
virtual void
   draw_active( void );             // Redraw the active line

//----------------------------------------------------------------------------
//
// Method-
//       EdView::enter_key
//
// Purpose-
//       Handle enter keypress.
//
//----------------------------------------------------------------------------
virtual void
   enter_key( void );               // Handle enter keypress

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
     int             n= 1);         // The relative row (Down positive)
}; // class EdView
#endif // EDVIEW_H_INCLUDED
