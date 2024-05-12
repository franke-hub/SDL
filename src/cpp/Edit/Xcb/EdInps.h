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
//       Editor: Terminal input services.
//
// Last change date-
//       2024/05/06
//
// Implementation notes-
//       See EdOuts.h for terminal output services.
//
//----------------------------------------------------------------------------
#ifndef EDINPS_H_INCLUDED
#define EDINPS_H_INCLUDED

#include <xcb/xproto.h>             // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes extension

#include <gui/Device.h>             // For gui::Device
#include <gui/Font.h>               // For gui::Font
#include <gui/Window.h>             // For gui::Window (Base class)
#include <pub/List.h>               // For pub::List

#include "Active.h"                 // For Active
#include "EdUnit.h"                 // For EdUnit (Base class)
#include "EdType.h"                 // For Editor types

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
class EdInps : public EdUnit, public gui::Window { // Editor input services
//----------------------------------------------------------------------------
// EdInps::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum CURSOR_STATE                   // Mouse cursor state
{  CS_RESET= 0                      // Reset (initial state, visible)
,  CS_HIDDEN                        // Hidden
,  CS_VISIBLE                       // Visible
}; // enum CURSOR_STATE

struct Motion {                     // System motion controls
int                    state;       // System mouse CURSOR_STATE
xcb_timestamp_t        time;        // Last movement timestamp
int                    x;           // Last X position
int                    y;           // Last Y position
}; // struct Motion

//----------------------------------------------------------------------------
// EdInps::Attributes
//----------------------------------------------------------------------------
gui::Device*           device= nullptr; // Our Device
gui::Font*             font= nullptr; // Our Font

Motion                 motion= {CS_VISIBLE, 0, 0, 0}; // System motion controls

// Graphic contexts
GC_t                   gc_font= 0;  // Graphic Context: Standard line
GC_t                   gc_flip= 0;  // GC: Cursor character
GC_t                   gc_mark= 0;  // GC: Marked line or block
GC_t                   bg_chg= 0;   // GC: TOP: BG: File changed
GC_t                   bg_sts= 0;   // GC: TOP: BG: File unchanged
GC_t                   gc_chg= 0;   // GC: TOP: File changed
GC_t                   gc_msg= 0;   // GC: TOP: Message line
GC_t                   gc_sts= 0;   // GC: TOP: File unchanged

// NOT IMPLEMENTED
GC_t                   protGC= 0;   // GC: Protected line
GC_t                   pcsrGC= 0;   // GC: Protected cursor character

// XCB atoms
xcb_atom_t             protocol= 0; // WM_PROTOCOLS atom
xcb_atom_t             wm_close= 0; // WM_CLOSE atom

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::EdInps
//       EdInps::~EdInps
//
// Purpose-
//       (DEFAULT) constructor
//       Destructor
//
//----------------------------------------------------------------------------
   EdInps(                          // DEFAULT constructor
     Widget*           parent= nullptr, // Parent Widget
     const char*       name= nullptr);

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
{  return x/font->length.width; }

int                                 // The row
   get_row(                         // Get row
     int               y) const     // For this y pixel position
{  return y/font->length.height; }

int                                 // The offset in Pixels
   get_x(                           // Get offset in Pixels
     int               col) const   // For this column
{  return col * font->length.width + 1; }

int                                 // The offset in Pixels
   get_y(                           // Get offset in Pixels
     int               row) const   // For this row
{  return row * font->length.height + 1; }

xcb_point_t                         // The offset in Pixels
   get_xy(                          // Get offset in Pixels
     int               col,         // And this column
     int               row) const   // For this row
{  return {gui::PT_t(get_x(col)), gui::PT_t(get_y(row))}; }

//----------------------------------------------------------------------------
//
// Screen control methods-
//       EdUnit::draw               Redraw everything
//       EdUnit::draw_line          Draw a screen line
//       EdUnit::draw_history       Draw the history line
//       EdUnit::draw_message       Draw the message line
//       EdUnit::draw_status        Draw the status line
//       EdUnit::draw_top           Draw the top lines
//       EdUnit::draw_text          Draw a screen line
//
//       EdUnit::hide_cursor        Hide the cursor
//       EdUnit::show_cursor        Show the cursor
//
//       EdUnit::hide_mouse         Hide the mouse
//       EdUnit::show_mouse         Show the mouse
//
//       EdUnit::move_cursor_H      Move the cursor horizontally
//       EdUnit::move_screen_V      Move the screen vertically
//       EdUnit::move_window        Move the Window
//
//----------------------------------------------------------------------------
virtual void
   draw( void ) = 0;                // Redraw the Window

virtual void
   draw_line(                       // Draw a screen line
     uint32_t          row,         // The row number (absolute)
     const EdLine*     line) = 0;   // The line to draw

virtual void
   draw_history( void ) = 0;        // Redraw the history line

virtual bool                        // Return code, TRUE if message drawn
   draw_message( void ) = 0;        // Redraw the message line if present

virtual void
   draw_status( void ) = 0;         // Redraw the status line

virtual void
   draw_top( void ) = 0;            // Redraw the top (heading) lines

virtual void
   draw_text(                       // Draw a screen line
     GC_t              gc,          // Using this graphic context
     uint32_t          row,         // The row number (absolute)
     const char*       text) = 0;   // The text to draw

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual void
   hide_cursor( void ) = 0;         // Hide the character cursor

virtual void
   show_cursor( void ) = 0;         // Show the character cursor

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual void
   hide_mouse( void ) = 0;          // Hide the mouse cursor

virtual void
   show_mouse( void ) = 0;          // Show the mouse cursor

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual int                         // Return code, 0 if draw performed
   move_cursor_H(                   // Move cursor horizontally
     size_t            column) = 0; // The (absolute) column number

virtual void
   move_screen_V(                   // Move file lines vertically
     int32_t           rows) = 0;   // The row count (down is positive)

// (The window does not need to be positioned inside the display)
virtual void
   move_window(                     // Move the editor window
     int32_t           x,           // (Absolute) X position (in Pixels)
     int32_t           y) = 0;      // (Absolute) Y potision (in Pixels)

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::flush
//
// Purpose-
//       Complete enqueued I/O operations
//
//----------------------------------------------------------------------------
virtual void
   flush( void )                    // Complete enqueued I/O operations
{  Pixmap::flush(); }

//----------------------------------------------------------------------------
//
// Pseudo-thread methods-
//       EdInps::start
//       EdInps::stop
//       EdInps::join
//
// Purpose-
//       Pseudo-thread implementation.
//
//----------------------------------------------------------------------------
virtual void
   start( void );                   // Start "Thread"

virtual void
   stop( void );                    // Stop "Thread"

virtual void
   join( void );                    // Wait for "Thread"

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
