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
//       EdText.h
//
// Purpose-
//       Editor: TextWindow screen
//
// Last change date-
//       2021/01/24
//
//----------------------------------------------------------------------------
#ifndef EDTEXT_H_INCLUDED
#define EDTEXT_H_INCLUDED

#include <string>                   // For std::string
#include <sys/types.h>              // For
#include <xcb/xproto.h>             // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes extension

#include <gui/Font.h>               // For gui::Font
#include <gui/Window.h>             // For gui::Window (Base class)
#include <pub/List.h>               // For pub::List

#include "Active.h"                 // For Active

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
//       EdText
//
// Purpose-
//       TextWindow keyboard, mouse, and screen controller.
//
//----------------------------------------------------------------------------
class EdText : public gui::Window { // Editor text Window viewport
//----------------------------------------------------------------------------
// EdText::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum                                // System mouse cursor state
{  CS_RESET= 0                      // Reset (initial state, visible)
,  CS_HIDDEN                        // Hidden
,  CS_VISIBLE                       // Visible
};

struct Motion {                     // System motion controls
int                    state;       // System mouse cursor state
xcb_timestamp_t        time;        // Last movement timestamp
int                    x;           // Last X position
int                    y;           // Last Y position
}; // struct Motion

//----------------------------------------------------------------------------
// EdText::Attributes
//----------------------------------------------------------------------------
Active&                active;      // Active reference (*config::active)
gui::Font&             font;        // Font reference (*config::font)
EdLine*                head= nullptr; // Current first data screen line
EdLine*                tail= nullptr; // Current last  data screen line

unsigned               col_size= 0; // The current screen column count
unsigned               row_size= 0; // The current screen row count
unsigned               row_used= 0; // The last used screen row

Motion                 motion= {CS_VISIBLE, 0, 0, 0}; // System motion controls

// Graphic contexts
xcb_gcontext_t         fontGC= 0;   // The standard graphic context
xcb_gcontext_t         flipGC= 0;   // The inverted graphic context
xcb_gcontext_t         markGC= 0;   // The selected graphic context
xcb_gcontext_t         gc_chg= 0;   // Graphic context: status, changed file
xcb_gcontext_t         gc_cmd= 0;   // Graphic context: history line
xcb_gcontext_t         gc_msg= 0;   // Graphic context: message line
xcb_gcontext_t         gc_sts= 0;   // Graphic context: status, default

// XCB atoms
xcb_atom_t             protocol= 0; // WM_PROTOCOLS atom
xcb_atom_t             wm_close= 0; // WM_CLOSE atom

// Configuration controls
unsigned               MINI_C=40;   // Minimum columns (Width)
unsigned               MINI_R=10;   // Minimum rows    (Height)
unsigned               USER_TOP= 1; // Number of reserved TOP lines
unsigned               USER_BOT= 0; // Number of reserved BOTTOM lines

//----------------------------------------------------------------------------
// EdText::Constructor
//----------------------------------------------------------------------------
public:
   EdText(                          // Constructor
     Widget*           parent= nullptr, // Parent Widget
     const char*       name= nullptr);

//----------------------------------------------------------------------------
// EdText::Destructor
//----------------------------------------------------------------------------
virtual
   ~EdText( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       EdText::debug
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
// Public method-
//       EdText::activate
//
// Purpose-
//       Set the current file
//       Set the current line
//
//----------------------------------------------------------------------------
void
   activate(                        // Activate
     EdFile*           file);       // This file

void
   activate(                        // Activate
     EdLine*           _line);      // This line

//----------------------------------------------------------------------------
//
// Method-
//       EdText::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
virtual void
   configure( void );               // Create the Window (Layout complete)

//----------------------------------------------------------------------------
//
// Methods-
//       EdText::draw_cursor
//       EdText::undo_cursor
//
// Purpose-
//       Draw/clear the screen cursor character
//
//----------------------------------------------------------------------------
void
   draw_cursor(bool set= true);     // Set the character cursor

void
   undo_cursor( void )              // Clear the character cursor
{  draw_cursor(false); }

//----------------------------------------------------------------------------
//
// Public method-
//       EdText::draw
//       EdText::draw_info
//       EdText::draw_message
//       EdText::draw_history
//       EdText::draw_status
//
// Purpose-
//       Draw the entire screen, data and info
//       Draw the information line: draw_message, draw_history, or draw_status
//         Draw the message line
//         Draw the history line
//         Draw the status line
//
//----------------------------------------------------------------------------
virtual void
   draw( void );                    // Redraw the Window

void
   draw_info( void );               // Redraw the information line

bool                                // Return code, TRUE if handled
   draw_history( void );            // Redraw the history line

bool                                // Return code, TRUE if handled
   draw_message( void );            // Message line

void
   draw_status( void );             // Redraw the status line

//----------------------------------------------------------------------------
//
// Method-
//       EdText::get_col
//       EdText::get_row
//       EdText::get_x
//       EdText::get_y
//       EdText::get_xy
//
// Purpose-
//       Convert pixel x position to (screen) column
//       Convert pixel y position to (screen) row
//       Get pixel position for column.
//       Get pixel position for row.
//       Get [col,row] pixel position.
//
//----------------------------------------------------------------------------
int                                 // The column
   get_col(                         // Get column
     int               x);          // For this x pixel position

int                                 // The row
   get_row(                         // Get row
     int               y);          // For this y pixel position

int                                 // The offset in Pixels
   get_x(                           // Get offset in Pixels
     int               col);        // For this column

int                                 // The offset in Pixels
   get_y(                           // Get offset in Pixels
     int               row);        // For this row

xcb_point_t                         // The offset in Pixels
   get_xy(                          // Get offset in Pixels
     int               col,         // And this column
     int               row);        // For this row

//----------------------------------------------------------------------------
//
// Method-
//       EdText::get_text
//
// Purpose-
//       Get the text (which may be in flux.)
//
//----------------------------------------------------------------------------
virtual const char*                 // The associated text
   get_text(                        // Get text
     EdLine*           line);       // For this EdLine

//----------------------------------------------------------------------------
//
// Method-
//       EdText::grab_mouse
//       EdText::hide_mouse
//       EdText::show_mouse
//
// Purpose-
//       Grab the mouse cursor
//       Hide the mouse cursor
//       Show the mouse cursor
//
//----------------------------------------------------------------------------
void
   grab_mouse( void );              // Grab the mouse cursor

void
   hide_mouse( void );              // Hide the mouse cursor

void
   show_mouse( void );              // Show the mouse cursor

//----------------------------------------------------------------------------
//
// Method-
//       EdText::move_cursor_H
//
// Purpose-
//       Move cursor horizontally
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 if draw performed
   move_cursor_H(                   // Move cursor horizontally
     size_t            column);     // The (absolute) column number

//----------------------------------------------------------------------------
//
// Method-
//       EdText::putxy
//
// Purpose-
//       Draw text at position
//
//----------------------------------------------------------------------------
void
   putxy(                           // Draw text
     xcb_gcontext_t    fontGC,      // Using this graphic context
     unsigned          left,        // At this left (X) offset
     unsigned          top,         // At this top  (Y) offset
     const char*       text);       // Using this text

void
   putxy(                           // Draw text (using default draw and GC)
     unsigned          left,        // Left (X) offset
     unsigned          top,         // Top  (Y) offset
     const char*       text)        // Using this text
{  putxy(fontGC, left, top, text); }

void
   putxy(                           // Draw text
     xcb_point_t       xy,          // At this offset
     const char*       text)        // Using this text
{  putxy(unsigned(xy.x), unsigned(xy.y), text); }

void
   putxy(                           // Draw text
     xcb_gcontext_t    fontGC,      // Using this graphic context
     xcb_point_t       xy,          // At this offset
     const char*       text)        // Using this text
{  putxy(fontGC, unsigned(xy.x), unsigned(xy.y), text); }

//----------------------------------------------------------------------------
//
// Method-
//       EdText::resize
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
//       EdText::synch_active
//
// Purpose-
//       Set the Active (cursor) line to the current row.
//
// Inputs-
//       this->line= top screen line
//       data->row   screen row
//
//----------------------------------------------------------------------------
void
   synch_active( void );            // Set the Active (cursor) line

//----------------------------------------------------------------------------
//
// Public method-
//       EdText::key_input
//
// Purpose-
//       Handle keypress event
//
//----------------------------------------------------------------------------
void
   key_alt(                         // Handle this
     xcb_keysym_t      key);        // Alt_Key input event

void
   key_ctl(                         // Handle this
     xcb_keysym_t      key);        // Ctrl_Key input event

int                                 // Return code, TRUE if error message
   key_protected(                   // Handle this protected line
     xcb_keysym_t      key,         // Input key
     int               state);      // Alt/Ctl/Shift state mask

virtual void
   key_input(                       // Handle this
     xcb_keysym_t      key,         // Key input event
     int               state);      // Alt/Ctl/Shift state mask

//============================================================================
// EdText::Event handlers
//============================================================================
public:
virtual void
   button_press(                    // Handle this
     xcb_button_press_event_t* event); // Button press event

virtual void
   client_message(                  // Handle this
     xcb_client_message_event_t* E); // Client message event

virtual void
   configure_notify(                // Handle this
     xcb_configure_notify_event_t* E); // Configure notify event

void
   expose(                          // Handle this
     xcb_expose_event_t* E);        // Expose event

virtual void
   motion_notify(                   // Handle this
     xcb_motion_notify_event_t* E); // Motion notify event
}; // class EdText
#endif // EDTEXT_H_INCLUDED
