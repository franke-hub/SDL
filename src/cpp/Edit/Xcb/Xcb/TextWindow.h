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
//       2020/12/23
//
// Implementation TODO-
//       The TextWindow uses a one pixel [left,top,bottom,right] draw margin.
//
//       TODO: ??Optimize?? So far not needed.
//
//----------------------------------------------------------------------------
#ifndef TEXTWINDOW_H_INCLUDED
#define TEXTWINDOW_H_INCLUDED

#include <string>                   // For std::string
#include <pub/utility.h>            // TODO: REMOVE. for to_string

#include <xcb/xcb.h>                // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes
#include <xcb/xproto.h>             // For XCB interfaces

#include "Xcb/Active.h"             // For Active
#include "Xcb/Font.h"               // For Font
#include "Xcb/Device.h"             // For Device
#include "Xcb/Global.h"             // For ENQUEUE macro
#include "Xcb/Keysym.h"             // For X11/keysymdef.h
#include "Xcb/Types.h"              // For Types
#include "Xcb/Window.h"             // For Window base class

namespace xcb {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class TextWindow;

//----------------------------------------------------------------------------
//
// Class-
//       xcb::TextView
//
// Purpose-
//       TextWindow view.
//
//----------------------------------------------------------------------------
class TextView {                    // TextWindow view
//----------------------------------------------------------------------------
// xcb::TextView::Attributes
//----------------------------------------------------------------------------
public:
TextWindow*            text= nullptr; // Associated TextWindow

// TextWindow offsets. (Negative values are from bottom row)
int                    USER_TOP= 0; // Physical top row
int                    USER_BOT= 0; // Physical bottom row

// Dynamic screen position
size_t                 col_zero= 0; // Current column[0]
size_t                 row_zero= 0; // Current row[0]
unsigned               col= 0;      // Current physical cursor column
unsigned               row= 0;      // Current physical cursor row

Line*                  line= nullptr; // Current cursor line
Line*                  head= nullptr; // Head display line
Line*                  tail= nullptr; // Tail display line

size_t                 mark_lh= 0;  // Left mark column
size_t                 mark_rh= 0;  // Right mark column

// Graphic context object identifiers are copies, neither allocated nor deleted
xcb_gcontext_t         gc_flip= 0;  // Graphic context: cursor character
xcb_gcontext_t         gc_font= 0;  // Graphic context: normal line
xcb_gcontext_t         gc_mark= 0;  // Graphic context: marked character

//----------------------------------------------------------------------------
// xcb::TextView::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   TextView(                        // Constructor
     TextWindow*       _text)       // Associated TextWindow
:  text(_text) {}

virtual
   ~TextView( void ) = default;     // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextView::debug
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
//       xcb::TextView::get_text
//
// Purpose-
//       Get the text (which may be in flux.)
//
//----------------------------------------------------------------------------
virtual const char*                 // The associated text
   get_text(                        // Get text
     Line*             line)        // For this Line
{  return line->text; }             // (OVERRIDE this method, if needed)

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextView::draw
//
// Purpose-
//       Draw the TextView
//
//----------------------------------------------------------------------------
virtual void
   draw(                            // Draw the TextView
     size_t            col_zero= 0) // Starting from this column
{  (void)col_zero; }                // (OVERRIDE this method)
}; // class TextView

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
Active                 active;      // Active line
Font                   font;        // Current Font
std::string            font_name;   // The font name

Line*                  line= nullptr; // Current first line
Line*                  last= nullptr; // Current last line displayed

xcb_gcontext_t         fontGC= 0;   // The standard graphic context
xcb_gcontext_t         flipGC= 0;   // The inverted graphic context
xcb_gcontext_t         markGC= 0;   // The selected graphic context
unsigned               col_size= 0; // The current screen column count
unsigned               row_size= 0; // The current screen row count
unsigned               row_used= 0; // The last used screen row

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
     const char*       info= nullptr) const; // Associated info

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
   draw(                            // Redraw the Window
     size_t            col_zero);   // Starting from this column

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
     unsigned          x);          // For this x pixel position

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
     unsigned          y);          // For this y pixel position

//----------------------------------------------------------------------------
//
// Method-
//       xcb::TextWindow::get_text
//
// Purpose-
//       Get the text (which may be in flux.)
//
//----------------------------------------------------------------------------
virtual const char*                 // The associated text
   get_text(                        // Get text
     Line*             line)        // For this Line
{  return line->text; }             // This does nothing extra

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
     unsigned          x,           // New width
     unsigned          y);          // New height

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
}; // class xcb::TextWindow
}  // namespace xcb
#undef  USE_BRINGUP                 // Macro cleanup
#endif // TEXTWINDOW_H_INCLUDED
