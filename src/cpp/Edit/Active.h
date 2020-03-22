//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2016 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Active.h
//
// Purpose-
//       Editor: Active Line.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#ifndef ACTIVE_H_INCLUDED
#define ACTIVE_H_INCLUDED

#ifndef EDITOR_H_INCLUDED
#include "Editor.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Active
//
// Purpose-
//       Active editor line.
//
//----------------------------------------------------------------------------
class Active {                      // Active editor line
//----------------------------------------------------------------------------
// Active::Enumerations
//----------------------------------------------------------------------------
public:
enum State                          // States
{  FSM_RESET= 0                     // Reset, unchanged
,  FSM_CHANGE                       // Active, changed
,  FSM_ERROR                        // Active, storage error
}; // enum State

//----------------------------------------------------------------------------
// Active::Constructors
//----------------------------------------------------------------------------
public:
   ~Active( void );                 // Destructor

   Active(                          // Constructor
     unsigned          size);       // Working line size

//----------------------------------------------------------------------------
// Active::Accessor methods
//----------------------------------------------------------------------------
public:
inline EdLine*                      // The current Line
   getLine( void ) const;           // Get current Line

inline const char*                  // Return message (NULL OK)
   setLine(                         // Set current Line w/o other changes
     EdRing*           edRing,      // Using this ring
     EdLine*           edLine);     // Using this line

inline EdRing*                      // The current Ring
   getRing( void ) const;           // Get current Ring

inline State                        // The current State
   getState( void ) const;          // Get the current State

const char*                         // Resultant (NULL OK)
   setState(                        // Set the current State
     State             state);      // To this State

inline const char*                  // The current text
   getText( void ) const;           // Get current text

inline unsigned                     // The number of bytes used
   getUsed( void ) const;           // Get number of bytes used

//----------------------------------------------------------------------------
// Active::Methods
//----------------------------------------------------------------------------
public:
const char*                         // Return message (NULL OK)
   appendString(                    // Concatenate string
     const char*       string,      // The join string
     unsigned          length);     // The join string length

const char*                         // Return message (NULL OK)
   appendString(                    // Concatenate string
     const char*       string);     // The join string

const char*                         // Return message (NULL OK)
   clear(                           // Clear to end of line
     unsigned          column);     // From this column (0 origin)

const char*                         // Return message (NULL OK)
   expand(                          // Expand the active line
     unsigned          column);     // To this size

const char*                         // Return message (NULL OK)
   fetch(                           // Fetch a line, making it active
     EdRing*           edRing,      // -> ring to activate
     EdLine*           edLine);     // -> line to activate

const char*                         // Return message (NULL OK)
   fetch(                           // Fetch a line, making it active
     EdLine*           edLine);     // -> line to activate

const char*                         // Return message (NULL OK)
   insertChar(                      // Insert character
     unsigned          column,      // The insert column
     int               code);       // The insert character

const char*                         // Return message (NULL OK)
   removeChar(                      // Remove character
     unsigned          column);     // The remove column

const char*                         // Return message (NULL OK)
   replaceChar(                     // Replace a character
     unsigned          column,      // The replacement column
     int               code);       // The replacement character

const char*                         // Return message (NULL OK)
   replaceLine(                     // Replace the entire line
     const char*       text);       // The replacement text

const char*                         // Return message (NULL OK)
   replaceString(                   // Replace a string
     unsigned          column,      // The replacement column
     unsigned          length,      // The replacement (delete) length
     const char*       string);     // The replacement (insert) string

const char*                         // Return message (NULL OK)
   reset( void );                   // Discard the active line

const char*                         // Return message (NULL OK)
   shrink( void );                  // Strip trailing blanks

const char*                         // Return message (NULL OK)
   store( void );                   // Store (replace) the active line

const char*                         // Return message (NULL OK)
   strip( void );                   // Shrink leading and trailing blanks

const char*                         // Return message (NULL OK)
   undo( void );                    // Undo any action on active line

//----------------------------------------------------------------------------
// Active::Debugging methods
//----------------------------------------------------------------------------
public:
void
   check( void ) const;             // Debugging check

void
   debug(                           // Debugging display
     const char*       message= "") const; // Display message

//----------------------------------------------------------------------------
// Active::Attributes
//----------------------------------------------------------------------------
protected:
   int                 state;       // Current State
   char*               base;        // Active static line
   unsigned            baseSize;    // Length(base)
   char*               text;        // Active line
   unsigned            textSize;    // Length(text)
   unsigned            textUsed;    // Strlen(text)

   EdRing*             ring;        // -> Associated active ring
   EdLine*             line;        // -> Associated active line
}; // class Active

#include "Active.i"

#endif // ACTIVE_H_INCLUDED
