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
//       SafeParser.h
//
// Purpose-
//       Safe String Parser.
//
// Last change date-
//       2012/01/01
//
// Usage notes-
//       The SafeParser object differs from the Parser object only in that
//       the input string is copied, removing the requirement that the
//       input string remain constant throughout the life of the object.
//
//----------------------------------------------------------------------------
#ifndef SAFEPARSER_H_INCLUDED
#define SAFEPARSER_H_INCLUDED

#ifndef PARSER_H_INCLUDED
#include "Parser.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       SafeParser
//
// Purpose-
//       Safe String Parser.
//
//----------------------------------------------------------------------------
class SafeParser : public Parser {  // Safe String Parser
//----------------------------------------------------------------------------
// SafeParser::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~SafeParser( void );             // Destructor (Deletes copy)
   SafeParser(                      // Constructor (Creates copy)
     const char*       string = NULL); // Initial string

//----------------------------------------------------------------------------
// SafeParser::Methods
//----------------------------------------------------------------------------
public:
virtual const char*                 // -> The string
   setString(                       // Set the new string
     const char*       string);     // The new string

const char*                         // The resultant string
   trim( void );                    // Remove leading and trailing blanks
}; // class SafeParser

#endif  // SAFEPARSER_H_INCLUDED
