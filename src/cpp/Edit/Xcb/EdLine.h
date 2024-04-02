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
//       EdLine.h
//
// Purpose-
//       Editor: Line descriptor
//
// Last change date-
//       2024/03/30
//
// Implementation notes-
//       Implemented in EdFile.cpp
//
//----------------------------------------------------------------------------
#ifndef EDLINE_H_INCLUDED
#define EDLINE_H_INCLUDED

#include <pub/List.h>               // For pub::List

#include "Editor.h"                 // For Editor

//----------------------------------------------------------------------------
//
// Class-
//       EdLine
//
// Purpose-
//       Editor line descriptor
//
// Implementation note-
//       Lines are allocated and deleted, but text is never deleted
//
//----------------------------------------------------------------------------
class EdLine : public pub::List<EdLine>::Link { // Editor Line descriptor
//----------------------------------------------------------------------------
// EdLine::Attributes
public:
const char*            text;        // Text, never nullptr

uint16_t               flags= 0;    // Control flags
enum FLAGS                          // Control flags
{  F_NONE= 0x0000                   // No flags
,  F_MARK= 0x0001                   // Line is marked (selected)
,  F_PROT= 0x0002                   // Line is read/only
,  F_HIDE= 0x0004                   // Line is hidden
,  F_AUTO= 0x0100                   // Line is in automatic (stack) storage
};

unsigned char          delim[2]= {'\0', 0}; // Delimiter (NONE default)
//   For [0]= '\n', [1]= either '\r' or '\0' for DOS or Unix format.
//   For [0]= '\0', [1]= repetition count. {'\0',0}= NO delimiter

//----------------------------------------------------------------------------
// EdLine::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdLine(                          // Constructor
     const char*       text= nullptr); // (Immutable) text

   ~EdLine( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::debug
//
// Purpose-
//       (Minimal) debugging display
//
//----------------------------------------------------------------------------
void
   debug( void ) const;             // Debugging display

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::is_within
//
// Purpose-
//       Is this line within range head..tail (inclusive)?
//
//----------------------------------------------------------------------------
bool
   is_within(                       // Is this line within range head..tail?
     const EdLine*     head,        // First line in range
     const EdLine*     tail) const; // Final line in range
}; // class EdLine
#endif // EDLINE_H_INCLUDED
