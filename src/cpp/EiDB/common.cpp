//----------------------------------------------------------------------------
//
//       Copyright (C) 2002 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       common.cpp
//
// Purpose-
//       String functions which ignore case.
//
// Last change date-
//       2002/07/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <common.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "COMMON  " // Source filename, for debugging

#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if 0 // In private library
//----------------------------------------------------------------------------
//
// Subroutine-
//       memicmp
//
// Purpose-
//       memcmp, ignoring case.
//
//----------------------------------------------------------------------------
#ifndef _OS_WIN
int                                 // Resultant
   memicmp(                         // Memory compare, ignoring case
     const char*     string1,       // Source string
     const char*     string2,       // Compare string
     unsigned        length)        // Length
{
   int               result;        // Resultant
   int               c1;            // Character in string1
   int               c2;            // Character in string2

   int               i;

   result= 0;
   for(i= 0; i<length; i++)
   {
     c1= *string1;
     c2= *string2;

     result= tolower(c1) - tolower(c2);
     if( result != 0 )
       break;

     string1++;
     string2++;
   }

   return result;
}
#endif
#endif// In private library

#if 0 // In private library
//----------------------------------------------------------------------------
//
// Subroutine-
//       stricmp
//
// Purpose-
//       strcmp, ignoring case.
//
//----------------------------------------------------------------------------
#ifndef _OS_WIN
int                                 // Resultant
   stricmp(                         // String compare, ignoring case
     const char*     string1,       // Source string
     const char*     string2)       // Compare string
{
   int               result;        // Resultant
   int               c1;            // Character in string1
   int               c2;            // Character in string2

   for(;;)
   {
     c1= *string1;
     c2= *string2;
     result= tolower(c1) - tolower(c2);
     if( result != 0 )
       break;

     if( *string1 == '\0' )
       break;

     string1++;
     string2++;
   }

   return result;
}
#endif
#endif// In private library

#if 0 // In private library
//----------------------------------------------------------------------------
//
// Subroutine-
//       stristr
//
// Purpose-
//       strstr, ignoring case.
//
//----------------------------------------------------------------------------
char*                               // Resultant
   stristr(                         // Search for substring
     const char*     string,        // Source string
     const char*     substr)        // Substring
{
   unsigned          lString;       // (Remaining length of) string
   unsigned          lSubstr;       // Length of substring

   lString= strlen(string);
   lSubstr= strlen(substr);
   if( lSubstr == 0 )
     return (char*)string;

   while( lString >= lSubstr )
   {
     if( memicmp(string, substr, lSubstr) == 0 )
       return (char*)string;

     string++;
     lString--;
   }

   return NULL;
}
#endif// In private library

//----------------------------------------------------------------------------
//
// Subroutine-
//       strrev
//
// Purpose-
//       Reverse a string.
//
//----------------------------------------------------------------------------
#ifndef _OS_WIN
char*                               // Resultant
   strrev(                          // Memory compare, ignoring case
     char*           string)        // Source string
{
   int               bot, top;      // Indexes
   int               c;             // Temporary character holder

   bot= 0;
   top= strlen(string) - 1;
   while( bot < top )               // Reverse the string
   {
     c= string[top];
     string[top]= string[bot];
     string[bot]= c;

     bot++;
     top--;
   }

   return string;
}
#endif

#ifdef __cplusplus
} // extern "C"
#endif

