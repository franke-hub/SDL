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
//       2024/08/30
//
//----------------------------------------------------------------------------
#ifndef EDVIEW_H_INCLUDED
#define EDVIEW_H_INCLUDED

#include <string>                   // For std::string
#include <sys/types.h>              // For system types

#include "Active.h"                 // For Active
#include "EdLine.h"                 // For EdLine
#include "EdType.h"                 // For GC_t

//----------------------------------------------------------------------------
//
// Class-
//       EdView
//
// Purpose-
//       Editor data view.
//
// Implementation notes-
//       Implemented in EdData.cpp
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

//----------------------------------------------------------------------------
// EdView::Constructor/Destructor
//----------------------------------------------------------------------------
   EdView( void );                  // Constructor

virtual
   ~EdView( void ) = default;       // Destructor

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
//       Get the current LINE's column index
//
//----------------------------------------------------------------------------
virtual size_t                      // The current column index
   get_column( void )               // Get current column index
{  return col_zero + col; }         // The current column index

//----------------------------------------------------------------------------
//
// Method-
//       EdView::get_gc
//
// Purpose-
//       Get the current graphic context
//
//----------------------------------------------------------------------------
virtual GC_t                        // The current graphic context
   get_gc( void ) = 0;              // Get current graphic context

//----------------------------------------------------------------------------
//
// Method-
//       EdView::get_row
//
// Purpose-
//       Get the current FILE row
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
   activate( void ) = 0;            // Activate the view

//----------------------------------------------------------------------------
//
// Method-
//       EdView::commit
//
// Purpose-
//       Commit the Active data line
//
// Implementation notes-
//       EdData::commit updates a modified active line with UNDO.
//       EdHist::commit does nothing.
//
//----------------------------------------------------------------------------
virtual void
   commit( void ) = 0;              // Commit the Active line

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
   draw_active( void ) = 0;         // Redraw the active line

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
   enter_key( void ) = 0;           // Handle enter keypress

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
     int             n= 1) = 0;     // The relative row (Down positive)
}; // class EdView
#endif // EDVIEW_H_INCLUDED
