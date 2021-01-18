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
//       Xcb/Active.h
//
// Purpose-
//       XCB Active Line descriptor.
//
// Last change date-
//       2021/01/17
//
// Implementation note-
//       Changed Lines also automatically remove any trailing blanks.
//
//       All length parameters are byte counts; column parameters are logical
//       and are automatically converted internally to byte offsets.
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
// xcb::Active::Typedefs and enumerations
//----------------------------------------------------------------------------
public:                             // UTF8 size_t aliases
typedef size_t         Ccount;      // A column count
typedef size_t         Column;      // A column number
typedef size_t         Length;      // A length in bytes
typedef size_t         Offset;      // A column number offset in bytes

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
Length                 buffer_size; // The buffer size, always > buffer_used
Length                 buffer_used; // The number of bytes used

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
void
   debug(                           // Debugging display
     const char*       info= nullptr) const; // Associated info

//----------------------------------------------------------------------------
// xcb::Active::Protected methods
//----------------------------------------------------------------------------
protected:
void
   expand(                           // Expand buffer_size (Does not fetch)
     Length            length);      // To this length (+1)

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::get_buffer
//       xcb::Active::get_changed
//
// Purpose-
//       (Unconditionally) access the buffer, leaving trailing blanks
//       (Conditionally) access the buffer, removing trailing blanks
//
//----------------------------------------------------------------------------
public:
const char*                         // The current buffer
   get_buffer(                      // Get ('\0' delimited) buffer
     Column            column= 0);  // Starting at this column

const char*                         // The changed text, nullptr if unchanged
   get_changed( void );             // Get ('\0' delimited) changed buffer

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::get_cols
//
// Purpose-
//       Return the buffer UTF8 Column count (trailing blanks removed)
//
//----------------------------------------------------------------------------
Ccount                              // The current UTF8 Column count
   get_cols( void );                // Get current UTF8 Column count

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::get_used
//
// Purpose-
//       Return the buffer length (including trailing blanks, if present)
//
//----------------------------------------------------------------------------
Length                              // The current buffer used Length
   get_used( void );                // Get current buffer used Length

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::index
//
// Purpose-
//       Convert column to offset.
//
//----------------------------------------------------------------------------
Offset                              // The character buffer Offset
   index(                           // Get character buffer Offset for
     Column            column);     // This Column

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::append_text
//
// Purpose-
//       Concatenate text substring.
//
//----------------------------------------------------------------------------
void
   append_text(                     // Concatenate text
     const char*       join,        // The join substring
     Length            size);       // The substring size

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
     Length            length= 0);   // With blank fill to this length

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
     Column            column,      // The current Column
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
     Column            column,      // The insert Column
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
     Column            column);     // At this Column

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
     Column            column,      // At this Column
     int               code);       // With this character

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::replace_text
//
// Purpose-
//       Replace (or insert) a ('\0' delimited) text string.
//
//----------------------------------------------------------------------------
void
   replace_text(                    // Replace (or insert) text
     Column            column,      // The replacement Column
     Ccount            ccount,      // The Column count of deleted columns
     const char*       text,        // The replacement (insert) text
     Length            size);       // The replacement (insert) text Length

void
   replace_text(                    // Replace (or insert) text
     Column            column,      // The replacement Column
     Ccount            ccount,      // The Column count of deleted columns
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
     const char*       text= nullptr); // To this (immutable) text

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::truncate
//
// Purpose-
//       Remove trailing blanks
//
//----------------------------------------------------------------------------
const char*                         // The truncated buffer
   truncate( void );                // Remove trailing blanks

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Active::undo
//
// Purpose-
//       Undo any changes.
//
//----------------------------------------------------------------------------
bool                                // Return code: TRUE if state changed
   undo( void );                    // Undo any changes
}; // class Active
}  // namespace xcb
#endif // XCB_ACTIVE_H_INCLUDED
