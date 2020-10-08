//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
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
//       Editor: Line.
//
// Last change date-
//       2020/10/03 (Version 2, Release 1) - Extra compiler warnings
//
//----------------------------------------------------------------------------
#ifndef EDLINE_H_INCLUDED
#define EDLINE_H_INCLUDED

#ifndef EDITOR_H_INCLUDED
#include "Editor.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       EdLine
//
// Purpose-
//       Define editor line.
//
//----------------------------------------------------------------------------
class EdLine : public List<EdLine>::Link { // Editor line
//----------------------------------------------------------------------------
// EdLine::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum Delimiter                      // Delimiters
{  DT_NONE                          // "" (Last line in file)
,  DT_LF                            // "\n"
,  DT_CR                            // "\r"
,  DT_CRLF                          // "\r\n"
,  DT_CRCRLF                        // "\r\r\n"
,  DT_CRCRCRLF                      // "\r\r\r\n"
,  DT_NULL                          // "\0"
,  DT_NUL2                          // "\0\0"
,  DT_NUL3                          // "\0\0\0"
,  DT_NUL4                          // "\0\0\0\0"
,  DT_COUNT                         // Number of delimiter types
}; // enum Delimiter

//----------------------------------------------------------------------------
// EdLine::Constructors
//----------------------------------------------------------------------------
public:
   ~EdLine( void );                 // Destructor
   EdLine( void );                  // Constructor

//----------------------------------------------------------------------------
// EdLine::Accessor methods
//----------------------------------------------------------------------------
public:
inline unsigned                     // Length of text string
   getSize( void ) const;           // Get text string length

inline const char*                  // Associated text string
   getText( void ) const;           // Get text string

inline void
   setText(                         // Set text string
     char*             text);       // New text string

//----------------------------------------------------------------------------
// EdLine::Methods
//----------------------------------------------------------------------------
public:
int                                 // TRUE if within range
   between(                         // Is line within range?
     const EdLine*     head,        // First range line
     const EdLine*     tail) const; // Final range line

//----------------------------------------------------------------------------
// EdLine::Debugging methods
//----------------------------------------------------------------------------
public:
void
   check( void ) const;             // Debugging check

void
   debug(                           // Debugging display
     const char*       message= "") const; // Display message

//----------------------------------------------------------------------------
// EdLine::Attributes
//----------------------------------------------------------------------------
public:
   char*             text;          // -> Text string
   struct                           // Line controls
   {
     unsigned        _0       : 8;  // Reserved for expansion
     unsigned        _1       : 8;  // Reserved for expansion
     unsigned        readonly : 1;  // This line is a protected, system line
     unsigned        marked   : 1;  // This line is marked
     unsigned        _2       : 5;  // Reserved for expansion
     unsigned        hidden   : 1;  // This is a hidden line

     unsigned        delim    : 8;  // Line delimiter type
   } ctrl;                          // Line controls
}; // class EdLine

#include "EdLine.i"

#endif // EDLINE_H_INCLUDED
