//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2026 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Tokenizer.h
//
// Purpose-
//       Tokenizer object.
//
// Last change date-
//       2026/10/26
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_TOKENIZER_H_INCLUDED
#define _LIBPUB_TOKENIZER_H_INCLUDED

#include <string>                   // For std::string

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
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
// Tokenizer::Iterator::Attributes
//----------------------------------------------------------------------------
public:
class Iterator {                    // The Tokenizer iterator
public:
static const Iterator  _end;        // *THE* end() iterator

protected:
typedef std::string    string;      // Using std::string within Iterator

const char*            input;       // The input string
size_t                 offset;      // The current offset
size_t                 length;      // The current token length
const char*            delim;       // The token delimiters (default nullptr)
bool                   _quote= true; // Enable quoted strings

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
// Tokenizer::Iterator:Accessor methods
//----------------------------------------------------------------------------
void
   set_quote(bool quote)            // Enable quoted strings?
{  _quote= quote; }

//----------------------------------------------------------------------------
// Tokenizer::Iterator::Methods
//----------------------------------------------------------------------------
Iterator&                           // Always *this
   next( void );                    // The next Iterator

const char*                         // The remaining source
   remainder( void ) const          // Get remaining source
{  return input + offset; }
}; // class Tokenizer::Iterator

//============================================================================
// Tokenizer::Attributes
//----------------------------------------------------------------------------
protected:
typedef std::string    string;      // Using std::string within Tokenizer

string                 _delim;      // Holder for delim string
string                 _input;      // Holder for input string
const char*            delim;       // The token delimiter, may be nullptr
const char*            input;       // The input string

//----------------------------------------------------------------------------
// Tokenizer::Constructors
//----------------------------------------------------------------------------
public:
   Tokenizer(                       // Constructor
     const string&     _input_)     // The source string
:  _delim(), _input(_input_), delim(nullptr), input(_input.c_str())
{  }

   Tokenizer(                       // Constructor
     const string&     _input_,     // The source string
     const string&     _delim_)     // The token delimiter
:  _delim(_delim_), _input(_input_)
,  delim(_delim.c_str()), input(_input.c_str())
{  if( _delim == "" ) delim= nullptr; }

   Tokenizer(const Tokenizer& that) // Copy constructor
:  _delim(that._delim), _input(that._input)
,  delim(_delim.c_str()), input(_input.c_str())
{  if( _delim == "" ) delim= nullptr; }

Tokenizer& operator=(const Tokenizer& that) // Assignment operator
{
   _delim= that._delim;
   _input= that._input;
   delim= _delim == "" ? nullptr : _delim.c_str();
   input= _input.c_str();

   return *this;
}

//----------------------------------------------------------------------------
// Tokenizer::Methods
//----------------------------------------------------------------------------
Iterator                            // The begin Iterator
   begin( void )                    // Get begin Iterator
{  return Iterator(input, delim); }

Iterator                            // The end Iterator
   end( void )                      // Get end Iterator
{  return Iterator::_end; }

void
   reset(                           // Reset the Tokenizer string
     const string&     _input_)     // The replacement string
{  _input= _input_; this->input= _input.c_str(); }
}; // class Tokenizer
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_TOKENIZER_H_INCLUDED
