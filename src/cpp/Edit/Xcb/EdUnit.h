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
//       EdUnit.h
//
// Purpose-
//       Editor: Input/output unit interface
//
// Last change date-
//       2024/05/09
//
//----------------------------------------------------------------------------
#ifndef EDUNIT_H_INCLUDED
#define EDUNIT_H_INCLUDED

#include <pub/List.h>               // For pub::List

#include "Active.h"                 // For Active
#include "EdType.h"                 // For Editor types

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class EdFile;
class EdLine;

//----------------------------------------------------------------------------
//
// Class-
//       EdUnit
//
// Purpose-
//       Keyboard, mouse, and screen controller.
//
// Implementation notes-
//       Input keys and modifiers are usually passed separately, but are
//       combined (using or) in trace records.
//
//----------------------------------------------------------------------------
class EdUnit {                      // Editor text Window viewport
//----------------------------------------------------------------------------
// EdUnit::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum KEY_STATE                      // Input data and logic control (key_state)
{  KS_RESET=           0x00000000   // RESET state
,  KS_UTF_8=           0x0001FFFF   // UTF-8/extended key data mask
,  KS_INPUT=           0x00FFFFFF   // Input: data and key modifiers
,  KS_LOGIC=           0xFF000000   // Reserved for persistent controls
,  KS_RESERVED=        0xF0F00000   // (8 bits) Unused/reserved

// Input key modifiers
,  KS_ALT=             0x00020000   // ALT   key modifier
,  KS_CTL=             0x00040000   // CTrL  key modifier
,  KS_SHI=             0x00080000   // SHIft key modifier (UNUSED)

// Logical states
,  KS_INS=             0x01000000   // Insert state
,  KS_ESC=             0x02000000   // Escape state (next key escaped)

// Message states
,  KS_MSG=             0x04000000   // Message active
,  KS_NFC=             0x08000000   // "No Files Changed" message active
}; // enum KEY_STATE

// Configuration controls
static constexpr int   MINI_C=40;   // Minimum columns (Width)
static constexpr int   MINI_R=10;   // Minimum rows    (Height)
static constexpr int   USER_TOP= 2; // Number of reserved TOP lines
static constexpr int   USER_BOT= 0; // Number of reserved BOTTOM lines

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// EdUnit::Init (initialization/termination)
struct Init {                       // EdUnit initialization/termination
static EdUnit*                      // The EdUnit
   initialize( void );              // Initialize

static void
   terminate(EdUnit*);              // Terminate (the EdUnit)

static void
   at_exit( void );                 // Idempotent termination handler
}; // struct Init

//----------------------------------------------------------------------------
// EdUnit::Attributes
//----------------------------------------------------------------------------
Active                 active;      // Active object
EdLine*                head= nullptr; // Current first data screen line
EdLine*                tail= nullptr; // Current last  data screen line

uint32_t               col_size= 0; // The current screen column count
uint32_t               row_size= 0; // The current screen row count
uint32_t               row_used= 0; // The last used screen row
uint32_t               key_state= KS_INS; // Keyboard logic control state

//----------------------------------------------------------------------------
// EdUnit::Static attributes
//----------------------------------------------------------------------------
static const char*     EDITOR;      // The editor's name
static const char*     DEFAULT_CONFIG; // The default configuration file

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::EdUnit
//       EdUnit::~EdUnit
//
// Purpose-
//       (DEFAULT) constructor
//       Destructor
//
//----------------------------------------------------------------------------
   EdUnit( void ) = default;        // DEFAULT constructor

virtual
   ~EdUnit( void ) = default;       // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
virtual void
   debug(                           // Debugging display
     const char*       info= nullptr) const = 0; // Associated info

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::activate(EdFile*)
//       EdUnit::activate(EdLine*)
//
// Purpose-
//       Activate a file.
//       Activate a file.
//
//----------------------------------------------------------------------------
virtual void
   activate(                        // Activate
     EdFile*           file) = 0;   // This file

virtual void
   activate(                        // Activate
     EdLine*           line) = 0;   // This line

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
//       EdUnit::grab_mouse         Grab the mouse
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
   hide_cursor( void )              // Hide the character cursor
{  }                                // (Default does nothing)

virtual void
   show_cursor( void )              // Show the character cursor
{  }                                // (Default does nothing)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual void
   grab_mouse( void )               // Grab the mouse cursor
{  }                                // (Default does nothing)

virtual void
   hide_mouse( void )               // Hide the mouse cursor
{  }                                // (Default does nothing)

virtual void
   show_mouse( void )               // Show the mouse cursor
{  }                                // (Default does nothing)

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual int                         // Return code, 0 if draw performed
   move_cursor_H(                   // Move cursor horizontally
     size_t            column) = 0; // The (absolute) column number

virtual void
   move_screen_V(                   // Move file lines vertically
     int32_t           rows) = 0;   // The row count (down is positive)

// (The window does not need to be positioned inside the display)
virtual void
   move_window(int32_t, int32_t)    // Move the editor window (X,Y)
{  }                                // (Default does nothing)

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::flush
//
// Purpose-
//       Complete enqueued I/O operations
//
//----------------------------------------------------------------------------
virtual void
   flush( void )                    // Complete enqueued I/O operations
{  }                                // (Default does nothing)

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::resized
//
// Purpose-
//       Handle resized event.
//
//----------------------------------------------------------------------------
virtual void
   resized(                         // Resize the Window
     uint32_t          width,       // New width  (May be columns or pixels)
     uint32_t          height) = 0; // New height (May be columns or pixels)

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::set_font
//
// Purpose-
//       Set the font
//
// Implementation notes-
//       The default font will return 0 (or throw a terminating exception)
//
//----------------------------------------------------------------------------
virtual int                         // Return code, 0 OK
   set_font(const char* name= nullptr) // Set the Font
{  (void)name; return 0; }          // (Default does nothing)

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::set_geom
//
// Purpose-
//       Set the geometry
//
//----------------------------------------------------------------------------
virtual void
   set_geom(const geometry_t&)      // Set the geometry
{  }                                // (Default does nothing)

//----------------------------------------------------------------------------
//
// Pseudo-thread methods-
//       EdUnit::start
//       EdUnit::stop
//       EdUnit::join
//
// Purpose-
//       Pseudo-thread implementation.
//
//----------------------------------------------------------------------------
virtual void
   start( void ) = 0;               // Start "Thread"

virtual void
   stop( void ) = 0;                // Stop "Thread"

virtual void
   join( void ) = 0;                // Wait for "Thread"

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::synch_active
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
//       EdUnit::synch_file
//
// Purpose-
//       Save the current state in the active file
//
//----------------------------------------------------------------------------
virtual void
   synch_file( void ) const;        // Synchronize the active file

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::op_*
//
// Purpose-
//       Implement unit operations
//
// Implementation notes-
//       Use return after calling op_goto_changed, not break.
//       (The KS_NFC state must persist until the NEXT keystroke completes.)
//
//----------------------------------------------------------------------------
void
   op_debug( void );                // Enter/exit debug mode

void
   op_copy_cursor_to_hist( void );  // Copy cursor line to history

void
   op_copy_file_name_to_hist( void ); // Copy file name to history

void
   op_copy_hist_to_file( void );    // Insert history line into file

void
   op_exit_safely( void );          // Exit unless changed file exists

void
   op_goto_changed( void );         // Activate next changed file

void
   op_goto_next_file( void );       // Activate  to next file

void
   op_goto_prev_file( void );       // Activate  to prior file

void
   op_help( void );                 // Display help information

void
   op_insert_line( void );          // Insert a new, empty line

void
   op_join_line( void );            // Join cursor line with next line

void
   op_key_arrow_down( void );       // Handle down arrow key

void
   op_key_arrow_left( void );       // Handle left arrow key

void
   op_key_arrow_right( void );      // Handle right arrow key

void
   op_key_arrow_up( void );         // Handle up arrow key

void
   op_key_backspace( void );        // Handle backspace key

void
   op_key_dead( void );             // Handle dead key

void
   op_key_delete( void );           // Handle delete key

void
   op_key_end( void );              // Handle end key

void
   op_key_enter( void );            // Handle enter key

void
   op_key_home( void );             // Handle home key

void
   op_key_idle( void );             // Handle NOP key

void
   op_key_insert( void );           // Handle insert key

void
   op_key_page_down( void );        // Handle page down key

void
   op_key_page_up( void );          // Handle page up key

void
   op_key_tab_forward( void );      // Handle forward tab operation

void
   op_key_tab_reverse( void );      // Handle reverse tab operation

void
   op_line_to_bot( void );           // Move cursor line to end of screen

void
   op_line_to_top( void );           // Move cursor line to top of screen

void
   op_mark_block( void );           // Create/modify a block mark

void
   op_mark_copy( void );            // Copy marked lines

void
   op_mark_cut( void );             // Cut the mark

void
   op_mark_delete( void );          // Delete marked lines

void
   op_mark_format( void );          // Format a mark using word tokens

void
   op_mark_line( void );            // Create/modify a line mark

void
   op_mark_move( void );            // Move marked lines

void
   op_mark_paste( void );           // Paste the current stash

void
   op_mark_stash( void );           // Stash the current mark

void
   op_mark_undo( void );            // Undo the mark

void
   op_quit( void );                 // Unconditionally quit current file

void
   op_redo( void );                 // Redo the previous (file) undo

void
   op_repeat_change( void );        // Repeat the prior change operation

void
   op_repeat_locate( void );        // Repeat the prior locate operation

void
   op_safe_exit( void );            // Exit if no file changed

void
   op_safe_quit( void );            // Quit current file if unchanged

void
   op_save( void );                 // Save the current file

void
   op_split_line( void );           // Split the cursor line into two lines

void
   op_swap_view( void );            // Handle swap view

void
   op_undo( void );                 // Undo the previous (file) redo
}; // class EdUnit
#endif // EDUNIT_H_INCLUDED
