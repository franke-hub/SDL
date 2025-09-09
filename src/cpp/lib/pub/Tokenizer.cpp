//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Tokenizer.cpp
//
// Purpose-
//       Tokenizer and Tokenizer::Iterator object methods.
//
// Last change date-
//       2025/09/08
//
//----------------------------------------------------------------------------
#include <stdexcept>                // For std::out_of_range exception, ...
#include <string>                   // For std::string
#include <cctype>                   // For isspace
#include <cstring>                  // For memcmp, strstr

#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/Tokenizer.h"          // For pub::Tokenizer, implemented

using namespace pub::debugging;     // For debugging

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Static data areas
//----------------------------------------------------------------------------
const Tokenizer::Iterator
                      Tokenizer::Iterator::_end("", nullptr);

//----------------------------------------------------------------------------
//
// Method-
//       Tokenizer::Iterator::Iterator
//
// Purpose-
//       Constructors
//
//----------------------------------------------------------------------------
   Tokenizer::Iterator::Iterator(   // Constructor
     const char*       _input,      // The source string
     const char*       _delim)      // The token delimiter, usually nullptr
:  input(_input), offset(0), length(0), delim(_delim)
{
   if( delim ) {                    // If delimiter specified
     _quote= false;                 // Disable quotes
     if( *delim == '\0' )           // If delimiter == empty string
       delim= nullptr;              // Treat as isspace delimiter
   }

   next();
}

//----------------------------------------------------------------------------
//
// Method-
//       Tokenizer::Iterator::next
//
// Purpose-
//       Return the next token, always *this
//
//----------------------------------------------------------------------------
Tokenizer::Iterator&                 // The next Iterator, always *this
   Tokenizer::Iterator::next( void ) // Get next Iterator
{
   offset += length;                // Skip over the current token, if any
   if( delim ) {                    // If delimiters specified
     // Skip leading delimiters
     unsigned C= *((unsigned char*)input + offset);
     while( strchr(delim, C) != nullptr ) {
       if( C == 0 ) {
         length= 0;
         return *this;
       }

       ++offset;
       C= *((unsigned char*)input + offset);
     }

     // Find first trailing delimiter (or *ending == '\0')
     const char* origin= input + offset;
     const char* ending= origin + 1;
     C= *(unsigned char*)ending;
     while( strchr(delim, C) == nullptr ) { // strchr(delim, 0) is never nullptr
       ++ending;
       C= *(unsigned char*)ending;
     }
     length= ending - origin;
   } else {                         // If whitespace delimiter
     while( isspace(input[offset]) ) // Skip leading whitespace
       ++offset;

     const char* origin= input + offset;
     const char* ending= origin;
     unsigned Q= *(unsigned char*)origin;
     if( _quote && (Q == '\'' || Q == '\"') ) { // If beginning quote
       ++ending;                    // Skip beginning quote
       unsigned E= *(unsigned char*)ending;
       while( E != 0 && E != Q ) {
         ++ending;
         E= *(unsigned char*)ending;
       }
       if( E == Q )                 // If ending quote
         ++ending;                  // Include it in the string
       length= ending - origin;
     } else {                       // If quotes are disabled or not present
       unsigned E= *(unsigned char*)ending;
       while( E != 0 && !isspace(E) ) {
         ++ending;
         E= *(unsigned char*)ending;
       }
       length= ending - origin;
     }
   }

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Tokenizer::Iterator::operators
//
// Purpose-
//       Implement Tokenizer::Iterator operators
//
//----------------------------------------------------------------------------
std::string                         // The associated substring
   Tokenizer::Iterator::operator()( void ) // Get associated substring
{
   // Handle quoted string
   if( _quote ) {                   // If quotes are enabled
     unsigned Q= *((unsigned char*)input+offset);
     if( Q == '\'' || Q == '\"') {
       size_t size= length-1;
       if( Q == *((unsigned char*)input+offset+size) )
         size--;
       string result(input+offset+1, size); // (The string without quotes)
       return result;
     }
   }

   string result(input+offset, length);
   return result;
}

bool                                // TRUE iff Iterators are equal
   Tokenizer::Iterator::operator==( // Is this Iterator equal to
     const Iterator&   that)        // That Iterator
{  return strcmp(input+offset, that.remainder()) == 0; }

bool                                // TRUE iff Iterators are not equal
   Tokenizer::Iterator::operator!=( // Is this Iterator not equal to
     const Iterator&   that)        // That Iterator
{  return strcmp(input+offset, that.remainder()) != 0; }

Tokenizer::Iterator&
   Tokenizer::Iterator::operator++( void ) // Prefix ++operator
{  return next(); }

Tokenizer::Iterator
   Tokenizer::Iterator::operator++( int ) // Postfix operator++, parameter ignored
{
   Iterator result(*this);
   next();
   return result;
}
} // namespace _LIBPUB_NAMESPACE
