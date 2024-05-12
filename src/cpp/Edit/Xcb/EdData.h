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
//       EdData.h
//
// Purpose-
//       Editor: Editor data view
//
// Last change date-
//       2024/05/09
//
//----------------------------------------------------------------------------
#ifndef EDDATA_H_INCLUDED
#define EDDATA_H_INCLUDED

#include <string>                   // For std::string
#include <sys/types.h>              // For system types

#include "Active.h"                 // For Active
#include "EdLine.h"                 // For EdLine
#include "EdUnit.h"                 // For EdUnit::GC_t
#include "EdView.h"                 // For EdView (Base class)

//----------------------------------------------------------------------------
//
// Class-
//       EdData
//
// Purpose-
//       Editor data view.
//
//----------------------------------------------------------------------------
class EdData : public EdView {      // Editor data view
//----------------------------------------------------------------------------
// EdData::Attributes
//----------------------------------------------------------------------------
public:
// (Copied) graphic contexts (Cursor position)
GC_t                   gc_font= 0;  // Graphic context: default character
GC_t                   gc_flip= 0;  // Graphic context: cursor character
GC_t                   gc_mark= 0;  // Graphic context: marked character

//----------------------------------------------------------------------------
// EdData::Constructor/Destructor
//----------------------------------------------------------------------------
   EdData( void );                  // Constructor

virtual
   ~EdData( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       EdData::debug
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
//       EdData::get_gc
//
// Purpose-
//       Get the current graphic context
//
//----------------------------------------------------------------------------
virtual GC_t                        // The current graphic context
   get_gc( void );                  // Get current graphic context

//----------------------------------------------------------------------------
//
// Method-
//       EdData::activate
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
//       EdData::commit
//
// Purpose-
//       Commit the Active data line
//
//----------------------------------------------------------------------------
virtual void
   commit( void );                  // Commit the Active line

//----------------------------------------------------------------------------
//
// Method-
//       EdData::draw_active
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
//       EdData::enter_key
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
//       EdData::move_cursor_V
//
// Purpose-
//       Move cursor vertically
//
//----------------------------------------------------------------------------
virtual void
   move_cursor_V(                   // Move cursor vertically
     int             n= 1);         // The relative row (Down positive)
}; // class EdData
#endif // EDDATA_H_INCLUDED
