//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Xcb/Active.h
//
// Purpose-
//       XCB Active Line descriptor.
//
// Last change date-
//       2020/09/06
//
// Implementation note-
//       Any Line changes also remove any trailing blanks.
//
//----------------------------------------------------------------------------
#ifndef XCB_ACTIVE_H_INCLUDED
#define XCB_ACTIVE_H_INCLUDED

#include <sys/types.h>              // For size_t

namespace xcb {
//----------------------------------------------------------------------------
//
// Class-
//       xcb::Active
//
// Purpose-
//       Active (modifiable) text.
//
//----------------------------------------------------------------------------
class Active {                      // Active editor line
//----------------------------------------------------------------------------
// xcb::Active::Enumerations
//----------------------------------------------------------------------------
public:
enum FSM                            // Finite State Machine states
{  FSM_RESET= 0                     // Unchanged, Reset
,  FSM_FETCHED                      // Unchanged, Fetched
,  FSM_CHANGED                      // Modified
}; // enum FSM

//----------------------------------------------------------------------------
// xcb::Active::Attributes
//----------------------------------------------------------------------------
protected:
const char*            source;      // The source text
mutable char*          buffer= nullptr; // The working buffer (Never shrinks)
size_t                 buffer_size; // The buffer size, always > buffer_used
size_t                 buffer_used; // The number of bytes used

FSM                    fsm= FSM_RESET; // Finite State Machine (state)

//----------------------------------------------------------------------------
// xcb::Active::Constructors
//----------------------------------------------------------------------------
public:
   ~Active( void );                 // Destructor
   Active( void );                  // Constructor

//----------------------------------------------------------------------------
// xcb::Active::debug, Debugging display
//----------------------------------------------------------------------------
virtual void
   debug(                           // Debugging display
     const char*       info= nullptr) const; // Associated text

//----------------------------------------------------------------------------
// xcb::Active::Protected methods
//----------------------------------------------------------------------------
protected:
void
   expand(                           // Expand buffer_size (Does not fetch)
     size_t            column);      // To this length (+1)

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::append_text
//
// Purpose-
//       Concatenate text substring.
//
//----------------------------------------------------------------------------
public:
void
   append_text(                     // Concatenate text
     const char*       join,        // The join substring
     size_t            size);       // The substring size

void
   append_text(                     // Concatenate text
     const char*       join);       // The join string

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::fetch
//
// Purpose-
//       Fetch the source line, optionally expanding it
//
//----------------------------------------------------------------------------
void
   fetch(                            // Fetch the source line
     size_t            column= 0);   // With blank fill to this length

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::get_buffer
//
// Purpose-
//       (Unconditionally) access the buffer
//
//----------------------------------------------------------------------------
const char*                         // The current buffer
   get_buffer( void ) const;        // Get '\0' delimited buffer

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::get_changed
//
// Purpose-
//       (Conditionally) access the buffer, returning nullptr if unchanged.
//
// Implementation note-
//       If changed, trailing blanks are removed.
//
//----------------------------------------------------------------------------
const char*                         // The changed text, nullptr if unchanged
   get_changed( void ) const;       // Get changed text string

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::get_used
//
// Purpose-
//       Return the buffer length (including trailing blanks, if present)
//
//----------------------------------------------------------------------------
size_t                              // The current buffer used length
   get_used( void );                // Get current buffer used length

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::insert_char
//
// Purpose-
//       Insert a character.
//
//----------------------------------------------------------------------------
void
   insert_char(                     // Insert character
     size_t            column,      // The current column
     int               code);       // The insert character

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::insert_text
//
// Purpose-
//       Insert a text string.
//
//----------------------------------------------------------------------------
void
   insert_text(                     // Insert text
     size_t            column,      // The insert column
     const char*       text);       // The insert text

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::remove_char
//
// Purpose-
//       Remove a character.
//
//----------------------------------------------------------------------------
void
   remove_char(                     // Remove the character
     size_t            column);     // At this column

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::replace_char
//
// Purpose-
//       Replace a character.
//
//----------------------------------------------------------------------------
void
   replace_char(                    // Replace the character
     size_t            column,      // At this column
     int               code);       // With this character

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::replace_text
//
// Purpose-
//       Replace (or insert) a text string.
//
//----------------------------------------------------------------------------
void
   replace_text(                    // Replace (or insert) text
     size_t            column,      // The replacement column
     size_t            length,      // The replacement (delete) length
     const char*       text);       // The replacement (insert) text

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::reset
//
// Purpose-
//       Reset the Active text string (setting fsm= FSM_RESET)
//
//----------------------------------------------------------------------------
void
   reset(                           // Set source text
     const char*       text);       // To this (immutable) text

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::undo
//
// Purpose-
//       Undo any changes.
//
//----------------------------------------------------------------------------
int                                 // Return code: 0 if state changed
   undo( void );                    // Undo any changes
}; // class Active
}  // namespace xcb
#endif // XCB_ACTIVE_H_INCLUDED
