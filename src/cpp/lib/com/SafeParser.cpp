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
//       SafeParser.cpp
//
// Purpose-
//       Instantiate the SafeParser object.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdlib.h>                 // For NULL
#include <string.h>                 // For strdup

#include <com/Unconditional.h>

#include "com/SafeParser.h"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
const char*            emptyString= ""; // Empty string

//----------------------------------------------------------------------------
//
// Method-
//       SafeParser::~SafeParser
//
// Function-
//       Destructor.
//
//----------------------------------------------------------------------------
   SafeParser::~SafeParser( void )  // Destuctor
{
   setString(NULL);                 // Delete the copied string
}

//----------------------------------------------------------------------------
//
// Method-
//       SafeParser::SafeParser(const char*)
//
// Function-
//       Constructor.
//
//----------------------------------------------------------------------------
   SafeParser::SafeParser(          // Constuctor
     const char*       string)      // Initial string
:  Parser(emptyString)
{
   setString(string);
}

//----------------------------------------------------------------------------
//
// Method-
//       SafeParser::setString
//
// Function-
//       Replace the string.
//
//----------------------------------------------------------------------------
const char*                         // The string
   SafeParser::setString(           // Replace the string
     const char*       string)      // Initial string
{
   if( this->string != emptyString )// If not OUR empty string
   {
     free((void*)string);           // Free the copied string
     this->string= emptyString;     // Now use our emptyString
   }
   this->offset= 0;

   if( string == NULL )
     return this->string;

   this->string= must_strdup(string);

   return string;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       SafeParser::trim
//
// Purpose-
//       Remove leading and trailing blanks in our string
//
//----------------------------------------------------------------------------
const char*                         // -> Our resultant string
   SafeParser::trim( void )         // Remove leading and trailing blanks
{
   int i= strlen(string);
   while( i > 0 )                   // Remove trailing blanks
   {
     i--;
     if( string[i] != ' ' )
       break;

     ((char*)string)[i]= '\0';      // Remove trailing blank
   }

   while( current() == ' ' )        // Remove leading blanks
     offset++;

   return getString();
}

