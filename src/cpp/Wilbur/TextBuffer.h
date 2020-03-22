//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TextBuffer.h
//
// Purpose-
//       A text buffer, used to accumulate text of any length.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef TEXTBUFFER_H_INCLUDED
#define TEXTBUFFER_H_INCLUDED

#include <com/istring.h>

//----------------------------------------------------------------------------
//
// Class-
//       TextBuffer
//
// Purpose-
//       Text buffer.
//
//----------------------------------------------------------------------------
class TextBuffer {                  // Text buffer
//----------------------------------------------------------------------------
// TextBuffer::Attributes
//----------------------------------------------------------------------------
protected:
int                    inp;         // Number of bytes read, -1 iff appendable
int                    out;         // Number of bytes written
int                    textSize;    // Number of allocated bytes
char*                  textBuff;    // Pointer to text buffer
char                   buff[512];   // Internal buffer

//----------------------------------------------------------------------------
// TextBuffer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~TextBuffer( void );             // Destructor
   TextBuffer( void );              // Default constructor

public:                             // Allowed:
   TextBuffer(                      // Copy constructor
     const TextBuffer& source);     // Source TextBuffer

TextBuffer& operator=(              // Assignment operator
     const TextBuffer& source);     // Source TextBuffer

//----------------------------------------------------------------------------
// TextBuffer::Operators
//----------------------------------------------------------------------------
public:
int                                 // The character (EOF iff invalid index)
   operator[](                      // Get TextBuffer character
     int               X) const;    // At this position

//----------------------------------------------------------------------------
// TextBuffer::Methods
//----------------------------------------------------------------------------
public:
int                                 // The next character (EOF at end)
   get( void );                     // Get next TextBuffer character

void
   put(                             // Put character into TextBuffer
     int               C);          // The character

void
   put(                             // Put string into TextBuffer
     const char*       string);     // The string

void
   put(                             // Put text into TextBuffer
     const char*       text,        // The text
     unsigned          size);       // The text length

void
   put(                             // Put string into TextBuffer
     const std::string&string);     // The string

void
   put(                             // Put TextBuffer into TextBuffer
     const TextBuffer& source);     // Source TextBuffer

void
   reset( void );                   // Reset the TextBuffer

int                                 // The TextBuffer length
   size( void ) const;              // Get TextBuffer length

int                                 // The last skipped character
   skip(                            // Skip charaters
     int               count);      // Number of characters to skip

char*                               // The text
   toChar( void );                  // Convert to char*

std::string                         // Resultant string
   toString( void ) const;          // Convert to string

protected:
void
   expand(                          // Expand the text
     int               size);       // To this size
}; // class TextBuffer

#endif // TEXTBUFFER_H_INCLUDED
