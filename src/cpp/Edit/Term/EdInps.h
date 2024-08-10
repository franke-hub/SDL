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
//       2024/07/27
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
#include "EdOpts.h"                 // For EdOpts (local data area)
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
enum CURSOR_STATE                   // Cursor state
{  CS_HIDDEN                        // Hidden
,  CS_VISIBLE                       // Visible
}; // enum CURSOR_STATE

struct Cursor {                     // Cursor controls
int                    state;       // CURSOR_STATE
int                    x;           // Last X position
int                    y;           // Last Y position
}; // struct Cursor

enum                                // Generic enum
{  HIST_MESS_ROW= 1                 // History/Message line row
}; // Generic enum

//----------------------------------------------------------------------------
// EdInps::Attributes
//----------------------------------------------------------------------------
WINDOW*                win= nullptr; // The NCURSES window (stdscr)

Cursor                 mouse_cursor= {CS_VISIBLE, 0, 0}; // Mouse cursor

// TOP area background colors
GC_t                   bg_chg=  0;  // GC: TOP: BG: File changed
GC_t                   bg_sts=  0;  // GC: TOP: BG: File unchanged

// Graphic contexts (Color pairs)
const GC_t             gc_font= 1;  // Graphic Context: Text line (default)
const GC_t             gc_flip= 2;  // GC: Cursor character
const GC_t             gc_mark= 3;  // GC: Marked line or block
const GC_t             gc_chg=  4;  // GC: TOP: File changed
const GC_t             gc_msg=  5;  // GC: TOP: Message line
const GC_t             gc_sts=  6;  // GC: TOP: File unchanged

// Operational controls
int                    operational= false; // TRUE while operational
int                    poll_char= 0; // Method poll(), read-ahead character

// Option controls
EdOpts                 opts;        // Local data area, used by EdOpts.i

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
