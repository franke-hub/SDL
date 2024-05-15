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
//       Editor: Terminal output services.
//
// Last change date-
//       2024/05/15
//
// Implementation notes-
//       Attributes are defined in EdInps.h and EdUnit.h
//
//----------------------------------------------------------------------------
#ifndef EDOUTS_H_INCLUDED
#define EDOUTS_H_INCLUDED

#include <string>                   // For std::string
#include <sys/types.h>              // For

#include "Active.h"                 // For Active
#include "EdInps.h"                 // For EdInps (base class)

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class EdFile;
class EdLine;
class EdView;

//----------------------------------------------------------------------------
//
// Class-
//       EdOuts
//
// Purpose-
//       TextWindow keyboard, mouse, and screen controller.
//
//----------------------------------------------------------------------------
class EdOuts : public EdInps {      // Editor text Window viewport
//----------------------------------------------------------------------------
// EdOuts::Constructor/destructor
//----------------------------------------------------------------------------
public:
   EdOuts( void );                  // Constructor

virtual
   ~EdOuts( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Public method-
//       EdOuts::activate(EdFile*)
//       EdOuts::activate(EdLine*)
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
//       EdOuts::draw               Redraw everything
//       EdOuts::draw_line          Draw a screen line
//       EdOuts::draw_history       Draw the history line
//       EdOuts::draw_message       Draw the message line
//       EdOuts::draw_status        Draw the status line
//       EdOuts::draw_top           Draw the top lines
//       EdOuts::draw_text          Draw a screen line
//
//       EdOuts::move_cursor_H      Move the cursor horizontally
//       EdOuts::move_screen_V      Move the screen vertically
//
//----------------------------------------------------------------------------
virtual void
   draw( void );                    // Redraw the Window

virtual void
   draw_line(                       // Draw one data line
     unsigned          row,         // The (absolute) row number
     const EdLine*     line);       // The line to draw

virtual void
   draw_history( void );            // Redraw the history line

virtual bool                        // Return code, TRUE if handled
   draw_message( void );            // Redraw the message line if present

virtual void
   draw_status( void );             // Redraw the status line

virtual void
   draw_top( void );                // Redraw the top (heading) lines

virtual void
   draw_text(                       // Draw text line
     GC_t              GC,          // The target graphic context
     uint32_t          row,         // The (absolute) row number
     const char*       text);       // Using this text

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual int                         // Return code, 0 if draw performed
   move_cursor_H(                   // Move cursor horizontally
     size_t            column);     // The (absolute) column number

virtual void
   move_screen_V(                   // Move file lines vertically
     int32_t           rows);       // The row count (down is positive)

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::flush
//
// Purpose-
//       Complete enqueued I/O operations
//
//----------------------------------------------------------------------------
virtual void
   flush( void );                   // Complete enqueued I/O operations

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
// Screen output methods-
//       EdOuts::hide_cursor
//       EdOuts::show_cursor
//
// Purpose-
//       Hide the screen cursor
//       Show the screen cursor
//
//----------------------------------------------------------------------------
virtual void
   hide_cursor( void );             // Hide the cursor

virtual void
   show_cursor( void );             // Show the cursor

//----------------------------------------------------------------------------
//
// Screen output methods-
//       EdOuts::putch
//       EdOuts::putcr
//
// Purpose-
//       Draw char at [col,row] position
//       Draw text at [col,row] position
//
//----------------------------------------------------------------------------
virtual void
   putch(                           // Draw character
     GC_t              gc,          // The graphic context
     unsigned          col,         // The column
     unsigned          row,         // The row
     int               text);       // The character

virtual void
   putcr(                           // Draw text
     GC_t              gc,          // The graphic context
     unsigned          col,         // The column
     unsigned          row,         // The row
     const char*       text);       // The text

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
     uint32_t          width,       // New width  (May be columns or pixels)
     uint32_t          height);     // New height (May be columns or pixels)
}; // class EdOuts
#endif // EDOUTS_H_INCLUDED
