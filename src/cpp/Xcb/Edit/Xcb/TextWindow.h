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
//       Xcb/TextWindow.h
//
// Purpose-
//       XCB based text Window
//
// Last change date-
//       2020/10/16
//
// Implementation note-
//                    ******** DO NOT SIMULTANEOUSLY USE ********
//       XCB_EVENT_MASK_RESIZE_REDIRECT and XCB_EVENT_MASK_STRUCTURE_NOTIFY
//       When used together, actually changing the window size is problematic.
//       TODO: Understand why this happens.
//
//       The TextWindow uses a one pixel [left,top] draw margin. There is no
//       [bottom, right] margin.
//
// Implementation TODO-
//       TODO: Test screen motion without any optimization.
//       TODO: Understand draw() "resize ??? WHY ???"
//       TODO: Separate code from header.
//
//----------------------------------------------------------------------------
#ifndef TEXTWINDOW_H_INCLUDED
#define TEXTWINDOW_H_INCLUDED

#include "Editor.h"                 // TODO: REMOVE. For include file debugging

#include <string>                   // For std::string
#include <pub/utility.h>            // TODO: REMOVE. for to_string

#include <xcb/xcb.h>                // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes
#include <xcb/xproto.h>             // For XCB interfaces

#include "Xcb/Font.h"               // For Font
#include "Xcb/Device.h"             // For Device
#include "Xcb/Global.h"             // For ENQUEUE macro
#include "Xcb/Keysym.h"             // For X11/keysymdef.h
#include "Xcb/Types.h"              // For Types
#include "Xcb/Window.h"             // For Window base class

namespace xcb {
//----------------------------------------------------------------------------
//
// Class-
//       xcb::TextWindow
//
// Purpose-
//       xcb::Window containing text.
//
//----------------------------------------------------------------------------
class TextWindow : public Window {  // Text Window
//----------------------------------------------------------------------------
// TextWindow::Typedefs and attributes // TODO: Move to .cpp file
//----------------------------------------------------------------------------
enum // Compile-time constants
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= true                // Use bringup diagnostics?
}; // Compile-time constants

//----------------------------------------------------------------------------
// xcb::TextWindow: Attributes
//----------------------------------------------------------------------------
public:
Font                   font;        // Current Font
std::string            font_name;   // The font name

Line*                  cursor= nullptr; // The current cursor line
Line*                  line= nullptr; // Current first line
Line*                  last= nullptr; // Current last line displayed
size_t                 col_zero= 0; // The current column[0]
size_t                 row_zero= 0; // The current row[0]

xcb_gcontext_t         fontGC= 0;   // The standard graphic context
xcb_gcontext_t         flipGC= 0;   // The inverted graphic context
unsigned               col_size= 0; // The current screen column count
unsigned               row_size= 0; // The current screen row count
unsigned               row_used= 0; // The number of used rows
unsigned               col= 0;      // Cursor column (X) (offset from col_zero)
unsigned               row= 0;      // Cursor row (Y) (offset from row_zero)

// Configuration controls
unsigned               COLS_W=80;   // Nominal columns
unsigned               ROWS_H=50;   // Nominal rows
unsigned               MINI_C=40;   // Minimum columns (Width)
unsigned               MINI_R=10;   // Minimum rows    (Height)
unsigned               USER_TOP= 0; // Number of reserved TOP lines
unsigned               USER_BOT= 0; // Number of reserved BOTTOM lines

//----------------------------------------------------------------------------
// xcb::TextWindow: Constructor
//----------------------------------------------------------------------------
public:
   TextWindow(                      // Constructor
     Widget*           parent= nullptr, // Our parent Widget
     const char*       name= nullptr);  // This Window's name

//----------------------------------------------------------------------------
// xcb::TextWindow: Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~TextWindow( void );             // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
public:
virtual void
   configure(                       // Initialize the Layout
     Layout::config_t& config);     // Using the Layout configurator

virtual void
   configure( void );               // Create the Window (Layout complete)

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
virtual void
   debug(                           // Debugging display
     const char*       text= nullptr) const; // Associated text

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::cursor_text
//
// Purpose-
//       Handle the cursor line, which may be in flux.
//
// Implementation note-
//       This method can be used when subclass uses Active lines.
//
//----------------------------------------------------------------------------
virtual const char*                 // The cursor line text
   cursor_text(                     // Get cursor line text
     const Line*       line) const  // For this (cursor) line
{  return line->text; }             // This does nothing extra

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::draw
//
// Purpose-
//       Redraw the window
//
//----------------------------------------------------------------------------
virtual void
   draw( void );                    // Redraw the Window

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::get_col
//
// Purpose-
//       Convert pixel x position to (screen) column
//
//----------------------------------------------------------------------------
unsigned                            // The column
   get_col(                         // Get column
     int               x);          // For this x pixel position

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::get_row
//
// Purpose-
//       Convert pixel y position to (screen) row
//
//----------------------------------------------------------------------------
unsigned                            // The row
   get_row(                         // Get row
     int               y);          // For this y pixel position

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::get_x
//
// Purpose-
//       Get pixel position for column.
//
//----------------------------------------------------------------------------
unsigned                            // The offset in Pixels
   get_x(                           // Get offset in Pixels
     unsigned          col);        // For this column

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::get_y
//
// Purpose-
//       Get pixel position for row.
//
//----------------------------------------------------------------------------
unsigned                            // The offset in Pixels
   get_y(                           // Get offset in Pixels
     unsigned          row);        // For this row

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::get_xy
//
// Purpose-
//       Get [col,row] pixel position.
//
//----------------------------------------------------------------------------
xcb_point_t                         // The offset in Pixels
   get_xy(                          // Get offset in Pixels
     unsigned          col,         // And this column
     unsigned          row);        // For this row

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::putxy
//
// Purpose-
//       Draw text at position
//
//----------------------------------------------------------------------------
void
   putxy(                           // Draw text
     unsigned          left,        // At this left (X) offset
     unsigned          top,         // At this top  (Y) offset
     const char*       text)        // Using this text
{  font.putxy(left, top, text); }

void
   putxy(                           // Draw text
     xcb_point_t       xy,          // At this offset
     const char*       text)        // Using this text
{  font.putxy(unsigned(xy.x), unsigned(xy.y), text); }

void
   putxy(                           // Draw text
     xcb_gcontext_t    fontGC,      // Using this graphic context
     unsigned          left,        // At this left (X) offset
     unsigned          top,         // At this top  (Y) offset
     const char*       text)        // Using this text
{  font.putxy(fontGC, left, top, text); }

void
   putxy(                           // Draw text
     xcb_gcontext_t    fontGC,      // Using this graphic context
     xcb_point_t       xy,          // At this offset
     const char*       text)        // Using this text
{  font.putxy(fontGC, unsigned(xy.x), unsigned(xy.y), text); }

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::resize
//
// Purpose-
//       Resize the window
//
//----------------------------------------------------------------------------
void
   resize(                          // Resize the Window
     int               x,           // New width
     int               y);          // New height

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::set_font
//
// Purpose-
//       Set the font.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   set_font(                        // Set the Font
     const char*       name= nullptr); // To this name

//----------------------------------------------------------------------------
// xcb::TextWindow: Overridden Window event handlers
//----------------------------------------------------------------------------
public:
virtual void
   configure_notify(                // Handle this
     xcb_configure_notify_event_t* event); // Configure notify event

virtual void
   expose(                          // Handle this
     xcb_expose_event_t* event);    // Expose event

virtual void
   resize_request(                  // Handle this
     xcb_resize_request_event_t*);  // Resize request event
}; // class xcb::TextWindow
}  // namespace xcb
#undef  USE_BRINGUP                 // Macro cleanup
#endif // TEXTWINDOW_H_INCLUDED
