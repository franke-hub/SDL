//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       istring.cpp
//
// Purpose-
//       String functions which ignore case.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include <com/Debug.h>

#include "com/istring.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifdef __cplusplus
extern "C" {
#endif

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
     const char*       string1,     // Source string
     const char*       string2,     // Compare string
     size_t            length)      // Length
{
   int                 result;      // Resultant
   int                 c1;          // Character in string1
   int                 c2;          // Character in string2

   result= 0;
   for(size_t i= 0; i<length; i++)
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

#ifdef HCDM
int
   HCDMmemicmp(
     const char*       string1,
     const char*       string2,
     size_t            length)      // Length
{
   int result= memicmp(string1, string2, length);
   debugf("%8d= memicmp(%s,%s,%ld)\n", result, string1, string2, (long)length);
   return result;
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       strichr
//
// Purpose-
//       strchr, ignoring case.
//
//----------------------------------------------------------------------------
char*                               // Resultant
   strichr(                         // String compare, ignoring case
     const char*       string,      // Source string
     int               match)       // Compare string
{
   int MATCH= toupper(match);

   for(;;)
   {
     if( toupper(*string) == MATCH )
       break;

     if( *string == '\0' )
     {
       string= NULL;
       break;
     }

     string++;
   }

   return (char*)string;
}

#ifdef HCDM
char*                               // Resultant
   HCDMstrichr(
     const char*       string,
     int               match)
{
   char* result= strichr(string, match);
   debugf("%p= strichr(%s,%c)\n", result, string, match);
   return result;
}
#endif

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
     const char*       string1,     // Source string
     const char*       string2)     // Compare string
{
   int                 result;      // Resultant
   int                 c1;          // Character in string1
   int                 c2;          // Character in string2

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

#ifdef HCDM
int
   HCDMstricmp(
     const char*       string1,
     const char*       string2)
{
   int result= stricmp(string1, string2);
   debugf("%8d= stricmp(%s,%s)\n", result, string1, string2);
   return result;
}
#endif

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
     const char*       string,      // Source string
     const char*       substr)      // Substring
{
   unsigned            lString;     // (Remaining length of) string
   unsigned            lSubstr;     // Length of substring

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

#ifdef HCDM
char*
   HCDMstristr(
     const char*       string1,
     const char*       string2)
{
   char* result= stristr(string1, string2);
   debugf("%p= stristr(%s,%s)\n", result, string1, string2);
   return result;
}
#endif

#ifdef __cplusplus
} // extern "C"
#endif

