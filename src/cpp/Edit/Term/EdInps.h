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
//       2024/05/10
//
// Implementation notes-
//       See EdOuts.h for terminal output services.
//
//----------------------------------------------------------------------------
#ifndef EDINPS_H_INCLUDED
#define EDINPS_H_INCLUDED

#include <string>                   // For std::string
#include <sys/types.h>              // For system types
#include <ncurses.h>                // For ncurses library

#include <pub/List.h>               // For pub::List

#include "Active.h"                 // For Active
#include "EdType.h"                 // For Editor types
#include "EdUnit.h"                 // For EdUnit, base class
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
//       Terminal: keyboard and mouse handlers.
//
//----------------------------------------------------------------------------
class EdInps : public EdUnit {      // Editor Input services
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
int                    x;           // Last X position
int                    y;           // Last Y position
}; // struct Motion

//----------------------------------------------------------------------------
// EdInps::Attributes
//----------------------------------------------------------------------------
WINDOW*                win= nullptr; // The NCURSES window (stdscr)

Motion                 motion= {CS_VISIBLE, 0, 0}; // System motion controls

// Background colors
const GC_t             bg_chg=  16; // GC: TOP: BG: File changed
const GC_t             bg_sts=  17; // GC: TOP: BG: File unchanged

// Graphic contexts (Color pairs)
const GC_t             gc_flip= 18; // Graphic Context: Cursor character
const GC_t             gc_font= 20; // GC: Standard line
const GC_t             gc_mark= 22; // GC: Marked line or block
const GC_t             gc_chg=  24; // GC: TOP: File changed
const GC_t             gc_msg=  26; // GC: TOP: Message line
const GC_t             gc_sts=  28; // GC: TOP: File unchanged

// Operational controls
int                    operational= false; // TRUE while operational
int                    poll_char= 0; // Method poll(), read-ahead character

//----------------------------------------------------------------------------
// EdInps::Constructor/destructor
//----------------------------------------------------------------------------
   EdInps( void );                  // Constructor

virtual
   ~EdInps( void );                 // Destructor

void
   init( void );                    // Initialize

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
// Method-
//       EdUnit::flush
//
// Purpose-
//       Complete enqueued I/O operations
//
//----------------------------------------------------------------------------
virtual void
   flush( void );                   // Complete enqueued I/O operations

//----------------------------------------------------------------------------
//
// Keypress methods-
//       EdInps::key_alt
//       EdInps::key_ctl
//       EdInps::key_input
//
// Purpose-
//       Handle alt-key event
//       Handle ctl-key event
//       Handle input key event
//
//----------------------------------------------------------------------------
void
   key_alt(                         // Handle this
     uint32_t          key);        // Alt-Key input key

void
   key_ctl(                         // Handle this
     uint32_t          key);        // Ctrl-Key input key

void
   key_input(                       // Handle this
     uint32_t          key,         // Input key
     uint32_t          state);      // Alt/Ctl/Shift state mask

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::poll
//       EdInps::read
//
// Purpose-
//       Is character available?
//       read next character (waiting until it's available)
//
//----------------------------------------------------------------------------
bool                                // TRUE if a  character is available
   poll(                            // Is a character available?
     int               delay= 0);   // Delay in milliseconds

uint32_t                            // The next character
   read( void );                    // Get the next character

//----------------------------------------------------------------------------
//
// Screen output methods-
//       EdInps::putch
//       EdInps::putcr
//
// Purpose-
//       Draw char at [col,row] position
//       Draw text at [col,row] position
//
//----------------------------------------------------------------------------
void
   putch(                           // Draw character
     GC_t              gc,          // The graphic context
     unsigned          col,         // The column
     unsigned          row,         // The row
     int               text);       // The character

void
   putcr(                           // Draw text
     GC_t              gc,          // The graphic context
     unsigned          col,         // The column
     unsigned          row,         // The row
     const char*       text);       // The text

//----------------------------------------------------------------------------
//
// Pseudo-thread methods-
//       EdInps::start
//       EdInps:stop
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
}; // class EdInps
#endif // EDINPS_H_INCLUDED
