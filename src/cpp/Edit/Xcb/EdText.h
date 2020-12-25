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
//       EdText.h
//
// Purpose-
//       Editor: TextWindow screen
//
// Last change date-
//       2020/12/23
//
//----------------------------------------------------------------------------
#ifndef EDTEXT_H_INCLUDED
#define EDTEXT_H_INCLUDED

#include <string>                   // For std::string
#include <sys/types.h>              // For
#include <pub/List.h>               // For pub::List

#include <xcb/xproto.h>             // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes extension
#include "Xcb/Types.h"              // For xcb::Line

#include "Xcb/TextWindow.h"         // For xcb::TextWindow (Base class)

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
class EdText : public xcb::TextWindow { // Editor TextWindow viewport
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
int                    state;       // System cursor state
xcb_timestamp_t        time;        // Last movement timestamp
int                    x;           // Last X position
int                    y;           // Last Y position
}; // struct Motion

//----------------------------------------------------------------------------
// EdText::Attributes
//----------------------------------------------------------------------------
xcb_gcontext_t         gc_chg= 0;   // Graphic context: status, changed file
xcb_gcontext_t         gc_cmd= 0;   // Graphic context: history line
xcb_gcontext_t         gc_msg= 0;   // Graphic context: message line
xcb_gcontext_t         gc_sts= 0;   // Graphic context: status, default
Motion                 motion= {CS_VISIBLE, 0, 0, 0}; // System motion controls

// XCB atoms
xcb_atom_t             protocol= 0; // WM_PROTOCOLS atom
xcb_atom_t             wm_close= 0; // WM_CLOSE atom

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
   configure( void );               // Configure the Window

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
//       xcb::EdText::get_text
//
// Purpose-
//       Get the text (which may be in flux.)
//
//----------------------------------------------------------------------------
virtual const char*                 // The associated text
   get_text(                        // Get text
     xcb::Line*        line);       // For this Line

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
