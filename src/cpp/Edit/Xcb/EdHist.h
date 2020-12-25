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
//       EdHist.h
//
// Purpose-
//       Editor: History EdView
//
// Last change date-
//       2020/12/23
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
//       HistLine
//
// Purpose-
//       History line.
//
//----------------------------------------------------------------------------
class HistLine : public EdLine {    // Editor history line
public:
std::string            line;        // Saved line text

// HistLine::Constructor/Destructor ==========================================
   HistLine(                        // Default/text constructor
     const char*       _text= nullptr); // Associated text

   ~HistLine( void );               // Default destructor

// HistLine::Accessor methods ================================================
public:
inline HistLine*
   get_next( void ) const
{  return (HistLine*)EdLine::get_next(); }

inline HistLine*
   get_prev( void ) const
{  return (HistLine*)EdLine::get_prev(); }

const char*
   get_text( void ) const;          // Get the text

// HistLine::Methods =========================================================
void
   reset(                           // Reset the text
     const char*       _text= nullptr); // To this string
}; // class HistLine

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
// EdHist::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum { MAX_ROWS= 16 };              // Maximum history list size
typedef pub::List<HistLine>
                       HistList;    // History line list

//----------------------------------------------------------------------------
// EdHist::Attributes
//----------------------------------------------------------------------------
HistList               hist_list;   // History line list
HistLine*              hist_line= nullptr; // The current history line, if any
unsigned               hist_rows= 0; // Number of History rows

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
//       Activate the history line
//
//----------------------------------------------------------------------------
virtual void
   activate( void );                // Activate the history line

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
virtual void
   commit( void );                  // Commit the Active line

//----------------------------------------------------------------------------
//
// Method-
//       EdHist::get_active
//
// Purpose-
//       Get the active  history line
//
//----------------------------------------------------------------------------
virtual const char*                 // Get the active history line
   get_active( void );              // Get the active history line

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
