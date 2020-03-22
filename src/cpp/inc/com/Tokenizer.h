//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Tokenizer.h
//
// Purpose-
//       Tokenizer object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef TOKENIZER_H_INCLUDED
#define TOKENIZER_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Tokenizer
//
// Purpose-
//       Tokenize an input string into output tokens.
//       The default token delimiter is white space.
//
//----------------------------------------------------------------------------
class Tokenizer {                   // Tokenizer object
//----------------------------------------------------------------------------
// Tokenizer::Attributes
//----------------------------------------------------------------------------
private:
char*                  delim;       // The Tokenizer delimiter
unsigned               length;      // The original string length
unsigned               offset;      // The current offset
char*                  string;      // The (copy of) the input string

//----------------------------------------------------------------------------
// Tokenizer::Constructors
//----------------------------------------------------------------------------
public:
   ~Tokenizer( void );              // Destructor

   Tokenizer(                       // Constructor
     const char*       string);     // The source string

   Tokenizer(                       // Constructor
     const char*       string,      // The source string
     const char*       delim);      // The token delimiter, NULL for default

private:                            // Bitwise copy is prohibited
   Tokenizer(const Tokenizer&);     // Disallowed copy constructor
Tokenizer&
   operator=(const Tokenizer&);     // Disallowed assignment operator

//----------------------------------------------------------------------------
// Tokenizer::Methods
//----------------------------------------------------------------------------
public:
const char*                         // The next token, NULL if none
   nextToken( void );               // Get next token

const char*                         // The remaining string, NULL if none
   remainder( void );               // Get remaining string
}; // class Tokenizer

#endif // TOKENIZER_H_INCLUDED
