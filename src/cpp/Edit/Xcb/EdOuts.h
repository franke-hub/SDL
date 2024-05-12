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
//       2024/05/07
//
// Implementation notes-
//       Attributes are defined in EdInps.h and EdUnit.h
//
//----------------------------------------------------------------------------
#ifndef EDOUTS_H_INCLUDED
#define EDOUTS_H_INCLUDED

#include <string>                   // For std::string
#include <sys/types.h>              // For

#include <gui/Font.h>               // For gui::Font
#include <gui/Window.h>             // For gui::Window (Base class)
#include <pub/List.h>               // For pub::List

#include "Active.h"                 // For Active
#include "EdInps.h"                 // For EdInps (base class)

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class EdFile;
class EdLine;

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
   EdOuts(                          // DEFAULT constructor
     Widget*           parent= nullptr, // Parent Widget
     const char*       name= nullptr);

//----------------------------------------------------------------------------
// EdOuts::Destructor
//----------------------------------------------------------------------------
virtual
   ~EdOuts( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::configure
//
// Purpose-
//       Configure the Window, invoked during Window creation.
//
//----------------------------------------------------------------------------
virtual void
   configure( void );               // Configure the Window

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
//       EdUnit::draw               Redraw everything
//       EdUnit::draw_line          Draw a screen line
//       EdUnit::draw_history       Draw the history line
//       EdUnit::draw_message       Draw the message line
//       EdUnit::draw_status        Draw the status line
//       EdUnit::draw_top           Draw the top lines
//       EdUnit::draw_text          Draw a screen line
//
//       EdOuts::hide_cursor        Hide the cursor
//       EdOuts::show_cursor        Show the cursor
//
//       EdOuts::grab_mouse         Grab the mouse
//       EdOuts::hide_mouse         Hide the mouse
//       EdOuts::show_mouse         Show the mouse
//
//       EdUnit::move_cursor_H      Move the cursor horizontally
//       EdUnit::move_screen_V      Move the screen vertically
//       EdUnit::move_window        Move the Window
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
     unsigned          row,         // The (absolute) row number
     const char*       text);       // Using this text

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual void
   hide_cursor( void );             // Hide the character cursor

virtual void
   show_cursor( void );             // Show the character cursor

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual void
   grab_mouse( void );              // Grab the mouse cursor

virtual void
   hide_mouse( void );              // Hide the mouse cursor

virtual void
   show_mouse( void );              // Show the mouse cursor

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual int                         // Return code, 0 if draw performed
   move_cursor_H(                   // Move cursor horizontally
     size_t            column);     // The (absolute) column number

virtual void
   move_screen_V(                   // Move file lines vertically
     int32_t           rows);       // The row count (down is positive)

// (The window does not need to be positioned inside the display)
virtual void
   move_window(                     // Move the editor window
     int32_t           x,           // (Absolute) X position (in Pixels)
     int32_t           y);          // (Absolute) Y potision (in Pixels)

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
// Screen control methods-
//       EdOuts::putxy
//       EdOuts::putcr
//
// Purpose-
//       Draw text at [left,top] pixel position
//       Draw text at [col,row] character position
//
//----------------------------------------------------------------------------
virtual void
   putxy(                           // Draw text
     GC_t              gc,          // Using this graphic context
     uint32_t          left,        // At this left (X) offset in Pixels
     uint32_t          top,         // At this top  (Y) offset in Pixels
     const char*       text);       // Using this text

virtual void
   putxy(                           // Draw text (with default graphic context)
     uint32_t          left,        // Left (X) offset in Pixels
     uint32_t          top,         // Top  (Y) offset in Pixels
     const char*       text)        // Using this text
{  putxy(gc_font, left, top, text); }

virtual void
   putcr(                           // Draw text
     GC_t              gc,          // Using this graphic context
     uint32_t          col,         // At this left (column) offset
     uint32_t          row,         // At this top  (row) offset
     const char*       text)        // Using this text
{  putxy(gc, get_x(col), get_y(row), text); }

virtual void
   putcr(                           // Draw text (with default graphic context)
     uint32_t          col,         // At this left (column) offset
     uint32_t          row,         // At this top  (row) offset
     const char*       text)        // Using this text
{  putcr(gc_font, col, row, text); }

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

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::set_font
//
// Purpose-
//       Set the Font
//
//----------------------------------------------------------------------------
virtual int
   set_font(                        // Set the Font
     const char*       name=nullptr); // To this Font name

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::set_geom
//
// Purpose-
//       Set the geometry
//
//----------------------------------------------------------------------------
virtual void
   set_geom(const geometry_t&);     // Set the Geometry
}; // class EdOuts
#endif // EDOUTS_H_INCLUDED
