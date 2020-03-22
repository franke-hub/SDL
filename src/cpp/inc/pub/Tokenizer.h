//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
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
//       2020/01/27
//
//----------------------------------------------------------------------------
#ifndef _PUB_TOKENIZER_H_INCLUDED
#define _PUB_TOKENIZER_H_INCLUDED

#include <string>                   // For std::string
#include "config.h"                 // For _PUB_NAMESPACE, ...

namespace _PUB_NAMESPACE {
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
typedef std::string    string;      // Using std::string within Tokenizer

protected:
string                 _delim;      // Holder for delim string
string                 _input;      // Holder for input string
const char*            delim;       // The token delimiter, may be nullptr
const char*            input;       // The input string

//----------------------------------------------------------------------------
// Tokenizer::Iterator::Attributes
//----------------------------------------------------------------------------
public:
class Iterator {                    // The Tokenizer iterator
public:
static const Iterator  _end;        // An end() iterator

protected:
const char*            input;       // The input string
size_t                 offset;      // The current offset
size_t                 length;      // The current token length
const char*            delim;       // The token delimiter, may be nullptr
size_t                 ldelim;      // strlen(delim)

//----------------------------------------------------------------------------
// Tokenizer::Iterator::Constructors
//----------------------------------------------------------------------------
public:
   Iterator(                        // Constructor
     const char*       _input,      // The source string
     const char*       _delim);     // The token delimiter, may be nullptr

   Iterator(const Iterator&) = default; // The default copy constructor
Iterator& operator=(const Iterator&) = default; // The default assignment operator

//----------------------------------------------------------------------------
// Tokenizer::Iterator::Operators
//----------------------------------------------------------------------------
string                              // The associated substring
   operator()( void );              // Get associated substring

bool                                // TRUE iff Iterators are equal
   operator==(                      // Is this Iterator equal to
     const Iterator&   that);       // That Iterator

bool                                // TRUE iff Iterators are not equal
   operator!=(                      // Is this Iterator not equal to
     const Iterator&   that);       // That Iterator

Iterator&
   operator++( void );              // Prefix ++operator

Iterator
   operator++( int );               // Postfix operator++, parameter ignored

//----------------------------------------------------------------------------
// Tokenizer::Iterator::Methods
//----------------------------------------------------------------------------
Iterator&                           // Always *this
   next( void );                    // The next Iterator

const char*                         // The remaining source
   remainder( void ) const          // Get remaining source
{  return input + offset; }
}; // class Tokenizer::Iterator

//----------------------------------------------------------------------------
// Tokenizer::Constructors
//----------------------------------------------------------------------------
public:
   Tokenizer(                       // Constructor
     const string&     input)       // The source string
:  _input(input), input(_input.c_str()), _delim(), delim(nullptr) {}

   Tokenizer(                       // Constructor
     const string&     input,       // The source string
     const string&     delim)       // The token delimiter
:  _input(input), input(_input.c_str()), _delim(delim), delim(_delim.c_str()) {}

   Tokenizer(const Tokenizer&) = default; // Copy constructor
Tokenizer& operator=(const Tokenizer&) = default; // Assignment operator

//----------------------------------------------------------------------------
// Tokenizer::Methods
//----------------------------------------------------------------------------
public:
Iterator                            // The begin Iterator
   begin( void )                    // Get begin Iterator
{  return Iterator(input, delim); }

Iterator                            // The end Iterator
   end( void )                      // Get end Iterator
{  return Iterator::_end; }

void
   reset(                           // Reset the Tokenizer string
     const string&     input)       // The replacement string
{
   _input= input;
   this->input= _input.c_str();
}
}; // class Tokenizer
}  // namespace _PUB_NAMESPACE
#endif // _PUB_TOKENIZER_H_INCLUDED
