//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2012 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Parser.i
//
// Purpose-
//       Parser inline methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef PARSER_I_INCLUDED
#define PARSER_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Parser::current
//
// Function-
//       Extract the current character.
//
//----------------------------------------------------------------------------
int                                 // The current character
   Parser::current( void ) const    // Extract the current character
{
   return string[offset] & 0x00ff;
}

//----------------------------------------------------------------------------
//
// Method-
//       Parser::next
//
// Function-
//       Extract the next character.
//
//----------------------------------------------------------------------------
int                                 // The next character
   Parser::next( void )             // Extract the next character
{
   if( string[offset] == '\0' )
     return 0;

   offset++;
   return string[offset] & 0x00ff;
}

#endif // PARSER_I_INCLUDED
