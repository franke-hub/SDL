//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Tokenizer.cpp
//
// Purpose-
//       Tokenizer object methods.
//
// Last change date-
//       2021/03/02
//
//----------------------------------------------------------------------------
#include <stdexcept>                // For std::out_of_range exception, ...
#include <string>                   // For std::string

#include <ctype.h>                  // For isspace
#include <string.h>                 // For memcmp, strstr

#include <pub/Debug.h>

#include "pub/Tokenizer.h"

namespace _PUB_NAMESPACE {
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
     const char*       _delim)      // The token delimiter, may be nullptr
:  input(_input), offset(0), length(0), delim(_delim), ldelim(0)
{
   if( delim )
     ldelim= strlen(delim);

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
   if( delim )                      // If delimiter specified
   {
     if( ldelim != 0 )              // Skip leading delimiters
     {
       while( memcmp(delim, input + offset, ldelim) == 0 )
         offset += ldelim;
     }

     const char* origin= input + offset;
     if( *origin == '\0' )          // If Nothing left
       length= 0;
     else
     {
       const char* ending= strstr(origin+1, delim);
       if( ending )
         length= ending - origin;
       else
         length= strlen(origin);
     }
   } else {                         // If whitespace delimiter
     while( isspace(input[offset]) ) // Skip leading whitespace
       offset++;

     const char* origin= input + offset;
     if( *origin == '\0' )          // If nothing left
       length= 0;                   // Nothing left
     else                           // Return
     {
       int quote= 0;                // Not quoted
       if( *origin == '\'' || *origin == '\"' )
         quote= *origin;
       const char* ending= origin + 1;
       if( quote ) {                // If quoted
         while( *ending != '\0' && *ending != quote )
           ending++;
         if( *ending == quote )
           ending++;
       } else {
         while( *ending != '\0' && !isspace(*ending) )
           ending++;
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
   if( *(input+offset) == '\'' || *(input+offset) == '\"' ) { // Quoted string
     size_t size= length-1;
     if( *(input+offset) == *(input+offset+size) )
       size--;
     string result(input+offset+1, size);
     return result;
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
} // namespace _PUB_NAMESPACE
