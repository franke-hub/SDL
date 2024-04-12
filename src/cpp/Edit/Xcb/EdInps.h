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
//       EdInps.h
//
// Purpose-
//       Editor: Input/output server (See EdOuts.h)
//
// Last change date-
//       2024/04/11
//
//----------------------------------------------------------------------------
#ifndef EDINPS_H_INCLUDED
#define EDINPS_H_INCLUDED

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
class EdLine;

//----------------------------------------------------------------------------
//
// Class-
//       EdInps
//
// Purpose-
//       TextWindow keyboard, mouse, and screen controller.
//
//----------------------------------------------------------------------------
class EdInps : public gui::Window { // Editor text Window viewport
//----------------------------------------------------------------------------
// EdInps::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef xcb_gcontext_t GC_t;        // Editor: Graphic Context type

enum CURSOR_STATE                   // Mouse cursor state
{  CS_RESET= 0                      // Reset (initial state, visible)
,  CS_HIDDEN                        // Hidden
,  CS_VISIBLE                       // Visible
}; // enum CURSOR_STATE

enum KEYBOARD_STATE                 // Keyboard state (Low order 16 bits zero)
{  KS_RESERVED_XCB= 0x0000ffff      // XCB reserved, i.e. XCB_KEY_BUT_MASK_*
,  KS_INS=          0x00010000      // Insert state
,  KS_ESC=          0x00020000      // Escape state
}; // enum KEYBOARD_STATE

struct Motion {                     // System motion controls
int                    state;       // System mouse CURSOR_STATE
xcb_timestamp_t        time;        // Last movement timestamp
int                    x;           // Last X position
int                    y;           // Last Y position
}; // struct Motion

enum STATUS_FLAGS                   // (Bit flags)
{  SF_RESET=           0x0000       // Reset, no flags set
,  SF_FOCUS=           0x0001       // SET when we have focus
,  SF_MESSAGE=         0x0002       // SET when *any* message
,  SF_NFC_MESSAGE=     0x0004       // SET when "No Files Changed" message
}; // enum KEYBOARD_STATE

//----------------------------------------------------------------------------
// EdInps::Attributes
//----------------------------------------------------------------------------
Active                 active;      // Active object
gui::Font&             font;        // Font reference (*editor::font)
EdLine*                head= nullptr; // Current first data screen line
EdLine*                tail= nullptr; // Current last  data screen line

unsigned               col_size= 0; // The current screen column count
unsigned               row_size= 0; // The current screen row count
unsigned               row_used= 0; // The last used screen row

int                    status= SF_RESET; // STATUS_FLAG bits
Motion                 motion= {CS_VISIBLE, 0, 0, 0}; // System motion controls
uint32_t               keystate= KS_INS; // Keyboard state

// Graphic contexts
GC_t                   fontGC= 0;   // The standard graphic context
GC_t                   flipGC= 0;   // The inverted graphic context
GC_t                   markGC= 0;   // The selected graphic context
GC_t                   bg_chg= 0;   // Graphic background: status, changed file
GC_t                   bg_sts= 0;   // Graphic background: status, default
GC_t                   gc_chg= 0;   // Graphic context: status, changed file
GC_t                   gc_msg= 0;   // Graphic context: message line
GC_t                   gc_sts= 0;   // Graphic context: status, default

// XCB atoms
xcb_atom_t             protocol= 0; // WM_PROTOCOLS atom
xcb_atom_t             wm_close= 0; // WM_CLOSE atom

// Configuration controls
unsigned               MINI_C=40;   // Minimum columns (Width)
unsigned               MINI_R=10;   // Minimum rows    (Height)
unsigned               USER_TOP= 2; // Number of reserved TOP lines
unsigned               USER_BOT= 0; // Number of reserved BOTTOM lines

//----------------------------------------------------------------------------
// EdInps::Constructor
//----------------------------------------------------------------------------
public:
   EdInps(                          // Constructor
     Widget*           parent= nullptr, // Parent Widget
     const char*       name= nullptr);

//----------------------------------------------------------------------------
// EdInps::Destructor
//----------------------------------------------------------------------------
virtual
   ~EdInps( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::debug
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
// Accessor methods-
//       EdInps::get_col
//       EdInps::get_row
//       EdInps::get_x
//       EdInps::get_y
//       EdInps::get_xy
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
     int               x) const     // For this x pixel position
{  return x/font.length.width; }

int                                 // The row
   get_row(                         // Get row
     int               y) const     // For this y pixel position
{  return y/font.length.height; }

int                                 // The offset in Pixels
   get_x(                           // Get offset in Pixels
     int               col) const   // For this column
{  return col * font.length.width + 1; }

int                                 // The offset in Pixels
   get_y(                           // Get offset in Pixels
     int               row) const   // For this row
{  return row * font.length.height + 1; }

xcb_point_t                         // The offset in Pixels
   get_xy(                          // Get offset in Pixels
     int               col,         // And this column
     int               row) const   // For this row
{  return {gui::PT_t(get_x(col)), gui::PT_t(get_y(row))}; }

//----------------------------------------------------------------------------
//
// Keypress extension methodss-
//       EdInps::key_alt
//       EdInps::key_ctl
//
// Purpose-
//       Handle alt-key event
//       Handle ctl-key event
//
// Implementation notes-
//       These methods use EdInps methods, so they can't be subroutines.
//
//----------------------------------------------------------------------------
void
   key_alt(                         // Handle this
     xcb_keysym_t      key);        // Alt_Key input event

void
   key_ctl(                         // Handle this
     xcb_keysym_t      key);        // Ctrl_Key input event

//----------------------------------------------------------------------------
//
// Screen control methods-
//       EdInps::putxy
//       EdInps::putcr
//
// Purpose-
//       Draw text at [x,y] pixel position
//       Draw text at [col,row] position
//
//----------------------------------------------------------------------------
void
   putxy(                           // Draw text
     GC_t              fontGC,      // Using this graphic context
     unsigned          left,        // At this left (X) offset in Pixels
     unsigned          top,         // At this top  (Y) offset in Pixels
     const char*       text);       // Using this text

void
   putxy(                           // Draw text (using default draw and GC)
     unsigned          left,        // Left (X) offset in Pixels
     unsigned          top,         // Top  (Y) offset in Pixels
     const char*       text)        // Using this text
{  putxy(fontGC, left, top, text); }

void
   putcr(                           // Draw text
     GC_t              fontGC,      // Using this graphic context
     unsigned          col,         // At this left (column) offset
     unsigned          row,         // At this top  (row) offset
     const char*       text)        // Using this text
{  putxy(fontGC, get_x(col), get_y(row), text); }

//----------------------------------------------------------------------------
//
// Pure virtual methods, implemented in EdOuts.cpp-
//       EdInps::activate(EdFile*)  Activate a file
//       EdInps::resized            Window resized event
//
//       EdInps::draw               Redraw everything
//       EdInps::draw_cursor        Draw the cursor
//       EdInps::draw_line          Draw a line
//       EdInps::draw_top           Draw the top lines
//       EdInps::draw_history       Draw the history line
//       EdInps::draw_message       Draw the message line
//       EdInps::draw_status        Draw the status line
//       EdInps::move_cursor_H      Move the cursor horizontally
//       EdInps::move_screen_V      Move the screen vertically
//       EdInps::undo_cursor        Clear the cursor
//
//       EdInps::hide_mouse
//       EdInps::show_mouse
//
//----------------------------------------------------------------------------
virtual void
   activate(                        // Acativate
     EdFile*           file)= 0;    // This file

virtual void
   resized(                         // Resize the Window
     unsigned          x,           // New width
     unsigned          y)= 0;       // New height

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual void
   draw( void )= 0;                 // Redraw the Window

virtual void
   draw_cursor(bool set= true)= 0;  // Set the character cursor

virtual void
   draw_line(                       // Draw one data line
     unsigned          row,         // The row number (absolute)
     const EdLine*     line)= 0;    // The line to draw

virtual void
   draw_top( void )= 0;             // Redraw the top (heading) lines

virtual void
   draw_history( void )= 0;         // Redraw the history line

virtual bool                        // Return code, TRUE if handled
   draw_message( void )= 0;         // Redraw the message line if present

virtual void
   draw_status( void )= 0;          // Redraw the status line

virtual int                         // Return code, 0 if draw performed
   move_cursor_H(                   // Move cursor horizontally
     size_t            column)= 0;  // The (absolute) column number

virtual void
   move_screen_V(                   // Move screen vertically
     int               rows)= 0;    // The row count (down is positive)

inline void
   undo_cursor( void )              // Clear the character cursor
{  draw_cursor(false); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual void
   hide_mouse( void )= 0;           // Hide the mouse cursor

virtual void
   show_mouse( void )= 0;           // Show the mouse cursor

//----------------------------------------------------------------------------
//
// Methods-
//       Window event handler methods
//
// Purpose-
//       Window *_event_t handlers
//
//----------------------------------------------------------------------------
virtual void
   button_press(                    // Handle this
     xcb_button_press_event_t* event); // Button press event

virtual void
   client_message(                  // Handle this
     xcb_client_message_event_t* E); // Client message event

virtual void
   configure_notify(                // Handle this
     xcb_configure_notify_event_t* E); // Configure notify event

virtual void
   expose(                          // Handle this
     xcb_expose_event_t* E);        // Expose event

virtual void
   focus_in(                        // Handle this
     xcb_focus_in_event_t* E);      // Focus-in event

virtual void
   focus_out(                       // Handle this
     xcb_focus_out_event_t* E);     // Focus-out event

virtual void
   key_input(                       // Handle this
     xcb_keysym_t      key,         // Key input event
     int               state);      // Alt/Ctl/Shift state mask

virtual void
   motion_notify(                   // Handle this
     xcb_motion_notify_event_t* E); // Motion notify event

virtual void
   property_notify(                 // Handle this
     xcb_property_notify_event_t* E); // Property notify event
}; // class EdInps
#endif // EDINPS_H_INCLUDED
