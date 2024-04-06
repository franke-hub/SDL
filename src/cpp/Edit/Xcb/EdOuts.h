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
//       EdOuts.h
//
// Purpose-
//       Editor: Input/output server (See EdInps.h)
//
// Last change date-
//       2024/04/06
//
// Implementation notes-
//       All attributes are defined in EdInps.h
//
//----------------------------------------------------------------------------
#ifndef EDOUTS_H_INCLUDED
#define EDOUTS_H_INCLUDED

#include <string>                   // For std::string
#include <sys/types.h>              // For
#include <xcb/xproto.h>             // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes extension

#include <gui/Font.h>               // For gui::Font
#include <gui/Window.h>             // For gui::Window (Base class)
#include <pub/List.h>               // For pub::List

#include "Active.h"                 // For Active
#include "EdInps.h"                 // For EdInps (base class)

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class EdFile;
class EdHist;
class EdLine;
class EdView;

//----------------------------------------------------------------------------
//
// Class-
//       EdOuts
//
// Purpose-
//       Input/output server
//
//----------------------------------------------------------------------------
class EdOuts : public EdInps {      // Editor: Input/output server
public:
//----------------------------------------------------------------------------
// EdOuts::Constructor
//----------------------------------------------------------------------------
   EdOuts(                          // Constructor
     Widget*           parent= nullptr, // Parent Widget
     const char*       name= nullptr);

//----------------------------------------------------------------------------
// EdOuts::Destructor
//----------------------------------------------------------------------------
virtual
   ~EdOuts( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Public method-
//       EdOuts::activate
//
// Purpose-
//       Set the current file
//       Set the current line
//
//----------------------------------------------------------------------------
virtual void
   activate(                        // Activate
     EdFile*           file);       // This file

virtual void
   activate(                        // Activate
     EdLine*           line);       // This line

//----------------------------------------------------------------------------
//
// Public method-
//       EdOuts::draw
//       EdOuts::draw_cursor
//       EdOuts::draw_line
//       EdOuts::draw_history
//       EdOuts::draw_message
//       EdOuts::draw_status
//       EdOuts::draw_top
//
// Purpose-
//       Draw the entire screen, data and info
//       Draw/clear the screen cursor character
//       Draw one data line
//       Draw the history line
//       Draw the message line
//       Draw the status line
//       Draw the top lines
//
//----------------------------------------------------------------------------
virtual void
   draw( void );                    // Redraw the Window

virtual void
   draw_cursor(bool set= true);     // Set the character cursor

virtual void
   draw_line(                       // Draw one data line
     unsigned          row,         // The (absolute) row number
     const EdLine*     line);       // The line to draw

virtual void
   draw_top( void );                // Redraw the top (heading) lines

virtual void
   draw_history( void );            // Redraw the history line

virtual bool                        // Return code, TRUE if handled
   draw_message( void );            // Redraw the message line if present

virtual void
   draw_status( void );             // Redraw the status line

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::get_text
//
// Purpose-
//       Get the text (which may be in flux.)
//
//----------------------------------------------------------------------------
virtual const char*                 // The associated text
   get_text(                        // Get text
     const EdLine*     line) const; // For this EdLine

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::move_cursor_H
//
// Purpose-
//       Move cursor horizontally
//
//----------------------------------------------------------------------------
virtual int                         // Return code, 0 if draw performed
   move_cursor_H(                   // Move cursor horizontally
     size_t            column);     // The (absolute) column number

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::move_screen_V
//
// Purpose-
//       Move screen vertically
//
//----------------------------------------------------------------------------
virtual void
   move_screen_V(                   // Move screen vertically
     int               rows);       // The row count (down is positive)

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::resized
//
// Purpose-
//       Handle Window resized event
//
//----------------------------------------------------------------------------
virtual void
   resized(                         // Handle Window resized event
     unsigned          x,           // New width
     unsigned          y);          // New height

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::synch_active
//
// Purpose-
//       Set the Active (cursor) line to the current row.
//
// Inputs-
//       this->line= top screen line
//       data->row   screen row
//
//----------------------------------------------------------------------------
virtual void
   synch_active( void );            // Set the Active (cursor) line

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::synch_file
//
// Purpose-
//       Save the current state in the active file
//
//----------------------------------------------------------------------------
virtual void
   synch_file(                      // Synchronize the active file
     EdFile*           file) const; // The active file, which is updated
}; // class EdOuts
#endif // EDOUTS_H_INCLUDED
