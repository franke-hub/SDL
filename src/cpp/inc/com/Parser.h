//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Parser.h
//
// Purpose-
//       String Parser.
//
// Last change date-
//       2012/01/01
//
// Usage notes-
//       The Parser string is accessed by reference. It is not modified, but
//       must remain viable throughout the life of the Parser object.
//
//----------------------------------------------------------------------------
#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include <stdint.h>

//----------------------------------------------------------------------------
//
// Class-
//       Parser
//
// Purpose-
//       String Parser.
//
//----------------------------------------------------------------------------
class Parser {                      // String Parser
//----------------------------------------------------------------------------
// Parser::Attributes
//----------------------------------------------------------------------------
protected:
const char*            string;      // The current string
unsigned               offset;      // The current string offset

//----------------------------------------------------------------------------
// Parser::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Parser( void );                 // Destructor
   Parser(                          // Constructor
     const char*       string = NULL); // Initial string

//----------------------------------------------------------------------------
// Parser::Methods
//----------------------------------------------------------------------------
public:
virtual const char*                 // -> The string remainder
   getString( void ) const;         // Extract the string remainder

virtual const char*                 // -> The string
   setString(                       // Set the string
     const char*       string);     // The new string

inline int                          // The current character
   current( void ) const;           // Get the current characer

const char*                         // -> The remaining string
   findSpace( void );               // Skip to the next whitespace char

inline int                          // The next character
   next( void );                    // Get the next characer

const char*                         // -> The remaining string
   skipSpace( void );               // Find the next non-whitespace char

int32_t                             // The decimal resultant
   toDec32( void );                 // Extract decimal value

int64_t                             // The decimal resultant
   toDec64( void );                 // Extract decimal value

inline long                         // The decimal resultant
   toDec( void )                    // Extract decimal value
{
   return toDec32();
}

double                              // The double resultant
   toDouble( void );                // Extract double value

int32_t                             // The hexidecimal resultant
   toHex32( void );                 // Extract hexidecimal value

int64_t                             // The hexidecimal resultant
   toHex64( void );                 // Extract hexidecimal value

inline long                         // The hexidecimal resultant
   toHex( void )                    // Extract hexidecimal value
{
   return toHex32();
}
}; // class Parser

#include "Parser.i"

#endif  // PARSER_H_INCLUDED
