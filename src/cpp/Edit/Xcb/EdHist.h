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
//       EdHist.h
//
// Purpose-
//       Editor: History EdView
//
// Last change date-
//       2021/01/24
//
//----------------------------------------------------------------------------
#ifndef EDHIST_H_INCLUDED
#define EDHIST_H_INCLUDED

#include <string>                   // For std::string
#include <sys/types.h>              // For system types

#include <pub/List.h>               // For pub::List

#include "EdFile.h"                 // For EdLine (base class)
#include "EdView.h"                 // For EdView (base class)

//----------------------------------------------------------------------------
//
// Class-
//       EdHist
//
// Purpose-
//       Editor command controller
//
//----------------------------------------------------------------------------
class EdHist : public EdView {      // Editor command controller
//----------------------------------------------------------------------------
// EdHist::Attributes
//----------------------------------------------------------------------------
public:
pub::List<EdLine>      hist_list;   // History line list

//----------------------------------------------------------------------------
// EdHist::Destructor/Constructor
//----------------------------------------------------------------------------
virtual
   ~EdHist( void );                 // Destructor
   EdHist( void );                  // Default constructor

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::debug
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
//       EdHist::get_gc
//
// Purpose-
//       Get the current graphic context
//
//----------------------------------------------------------------------------
virtual xcb_gcontext_t               // The current graphic context
   get_gc( void )                    // Get current graphic context
{  return gc_font; }

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::activate
//
// Purpose-
//       Activate the history view
//
//----------------------------------------------------------------------------
virtual void
   activate( void );                // Activate the history view

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::commit
//
// Purpose-
//       Commit the Active line     (Does nothing)
//
//----------------------------------------------------------------------------
virtual void
   commit( void ) {}                // Commit the Active line

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::enter_key
//
// Purpose-
//       Handle enter keypress
//
//----------------------------------------------------------------------------
virtual void
   enter_key( void );               // Handle enter keypress

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::get_buffer
//
// Purpose-
//       Get the active buffer (With blank fill for draw)
//
//----------------------------------------------------------------------------
virtual const char*                 // Get the active buffer
   get_buffer( void );              // Get the active buffer

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::move_cursor_V
//
// Purpose-
//       Move cursor vertically
//
//----------------------------------------------------------------------------
virtual void
   move_cursor_V(                   // Move cursor vertically
     int             n= 1);         // The relative row (Down positive)
}; // class EdHist
#endif // EDHIST_H_INCLUDED
