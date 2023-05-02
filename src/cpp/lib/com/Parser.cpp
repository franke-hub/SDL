//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Parser.cpp
//
// Purpose-
//       Instantiate the Parser object.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // For isspace()
#include <math.h>                   // For pow()

#include <com/define.h>             // For FALSE, TRUE
#include "com/Parser.h"

//----------------------------------------------------------------------------
//
// Method-
//       Parser::~Parser
//
// Function-
//       Destructor.
//
//----------------------------------------------------------------------------
   Parser::~Parser( void )          // Destuctor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Parser::Parser(const char*)
//
// Function-
//       Constructor.
//
//----------------------------------------------------------------------------
   Parser::Parser(                  // Constuctor
     const char*       string)      // Initial string
:  string("")
,  offset(0)
{
   if( string == NULL )
     return;

   this->string= string;
}

//----------------------------------------------------------------------------
//
// Method-
//       Parser::getString
//
// Function-
//       Extract the remainder of the string.
//
//----------------------------------------------------------------------------
const char*                         // The string remainder
   Parser::getString( void ) const  // Extract the string remainder
{
   return string + offset;
}

//----------------------------------------------------------------------------
//
// Method-
//       Parser::setString
//
// Function-
//       Replace the string.
//
//----------------------------------------------------------------------------
const char*                         // The string
   Parser::setString(               // Replace the string
     const char*       string)      // Initial string
{
   this->string= "";
   this->offset= 0;

   if( string == NULL )
     return this->string;

   this->string= string;
   return string;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Parser::findSpace
//
// Purpose-
//       Skip until space in string.
//
//----------------------------------------------------------------------------
const char*                         // -> Next blank
   Parser::findSpace( void )        // Find blank
{
   int                 C;

   C= current();
   while( !isspace(C) && C != '\0' )
     C= next();

   return string + offset;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Parser::skipSpace
//
// Purpose-
//       Skip over spaces in string.
//
//----------------------------------------------------------------------------
const char*                         // -> Next non-blank
   Parser::skipSpace( void )        // Skip over blanks in string
{
   int                 C;

   C= current();
   while( isspace(C) && C != '\0' )
     C= next();

   return string + offset;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Parser::toDec32
//
// Purpose-
//       Extract a decimal value.
//
//----------------------------------------------------------------------------
int32_t                             // Return value
   Parser::toDec32( void )          // Extract decimal value
{
   int32_t             result;      // Resultant
   int                 sign;        // Sign of resultant
   int                 C;           // The current character

   skipSpace();                     // Skip leading spaces
   C= current();                    // Get current character
   sign= 1;                         // Default, positive number
   if( C == '-' )                   // If negative number sign
   {
     sign= (-1);
     next();
   }
   else if( C == '+' )              // If positive number sign
     C= next();

   result= 0;
   for(;;)
   {
     if( C < '0' || C > '9' )       // If invalid decimal character
       break;                       // Done

     result *= 10;
     result += C - '0';
     C= next();
   }

   return result * sign;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Parser::toDec64
//
// Purpose-
//       Extract a decimal value.
//
//----------------------------------------------------------------------------
int64_t                             // Return value
   Parser::toDec64( void )          // Extract decimal value
{
   int64_t             result;      // Resultant
   int                 sign;        // Sign of resultant
   int                 C;           // The current character

   skipSpace();                     // Skip leading spaces
   C= current();                    // Get current character
   sign= 1;                         // Default, positive number
   if( C == '-' )                   // If negative number sign
   {
     sign= (-1);
     next();
   }
   else if( C == '+' )              // If positive number sign
     C= next();

   result= 0;
   for(;;)
   {
     if( C < '0' || C > '9' )       // If invalid decimal character
       break;                       // Done

     result *= 10;
     result += C - '0';
     C= next();
   }

   return result * sign;
}

//----------------------------------------------------------------------------
//
// Method-
//       Parser::toDouble
//
// Function-
//       Extract double value.
//
//----------------------------------------------------------------------------
double                              // The double value
   Parser::toDouble( void )         // Parse double value
{
   double              result;      // Resultant
   double              sign;        // Sign of resultant
   double              divisor;     // Divisor
   double              exponent;    // Exponent
   int                 decimal;     // TRUE if decimal point found
   int                 C;           // The current character

   skipSpace();                     // Skip leading spaces
   C= current();                    // Get current character

   sign= 1.0;                       // Default, positive number
   if( C == '-' )                   // If negative number sign
   {
     sign= -1.0;
     C= next();
   }
   else if( C == '+' )              // If positive number sign
     C= next();

   decimal= FALSE;
   divisor= 1.0;
   result= 0.0;
   for(;;)
   {
     if( C == '.' )                 // If decimal point
     {
       if( decimal )                // If this is the second one
         break;                     // Done (can't continue)

       decimal= TRUE;
       next();
       continue;
     }

     if( C < '0' || C > '9' )       // If invalid numeric character
       break;                       // Done

     if( decimal )                  // If decimal point was encounterd
       divisor *= 10.0;             // Adjust the divisor

     result *= 10.0;
     result += C - '0';
     C= next();
   }
   if( C == 'e' || C == 'E' )
   {
     exponent= toDec();
     if( exponent < 0 )
       divisor *= pow(10.0, (double)-exponent);
     else
       divisor /= pow(10.0, (double)exponent);
   }

   return (sign*result)/divisor;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Parser::toHex32
//
// Purpose-
//       Extract a hex value.
//
//----------------------------------------------------------------------------
int32_t                             // Return value
   Parser::toHex32( void )          // Extract hex value
{
   int32_t             result;      // Resultant
   int                 nibble;      // Next nibble
   int                 C;           // The current character

   skipSpace();                     // Skip leading spaces
   C= current();                    // Get current character
   result= 0;
   for(;;)
   {
     if( C >= '0' && C <= '9' )     // If numeric value
       nibble= C - '0';             // Extract it

     else if( C >= 'a' && C <= 'f' )// If range a..f
       nibble= C - 'a' + 10;        // Extract it

     else if( C >= 'A' && C <= 'F' )// If range A..F
       nibble= C - 'A' + 10;        // Extract it

     else                           // If invalid hex character
       break;                       // Done

     result *= 16;
     result += nibble;
     C= next();
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Parser::toHex64
//
// Purpose-
//       Extract a hex value.
//
//----------------------------------------------------------------------------
int64_t                             // Return value
   Parser::toHex64( void )          // Extract hex value
{
   int64_t             result;      // Resultant
   int                 nibble;      // Next nibble
   int                 C;           // The current character

   skipSpace();                     // Skip leading spaces
   C= current();                    // Get current character
   result= 0;
   for(;;)
   {
     if( C >= '0' && C <= '9' )     // If numeric value
       nibble= C - '0';             // Extract it

     else if( C >= 'a' && C <= 'f' )// If range a..f
       nibble= C - 'a' + 10;        // Extract it

     else if( C >= 'A' && C <= 'F' )// If range A..F
       nibble= C - 'A' + 10;        // Extract it

     else                           // If invalid hex character
       break;                       // Done

     result *= 16;
     result += nibble;
     C= next();
   }

   return result;
}

