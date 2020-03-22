//----------------------------------------------------------------------------
//
//       Copyright (C) 2003 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       Wildstr.cpp
//
// Purpose-
//       String functions with wild cards.
//
// Last change date-
//       2003/03/16
//
// Description-
//       This set of routines provides for character matching with wild
//       character sequences.  A wild character matches itself or any other
//       character in the wild character sequence.  Two non-identical wild
//       characters match if any of the expanded sequences match.
//
//       Wild characters logically expand into one character, but this
//       character can be any of the expanded sequences.
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Wildstr.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "WILDSTR " // Source filename, for debugging

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static const char*   wildlist[256]= // Wild character list
   {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

//----------------------------------------------------------------------------
//
// Subroutine-
//       getWild
//
// Purpose-
//       Get the wildcard list for a character.
//
//----------------------------------------------------------------------------
const char*                         // Wildcard list
   getWild(                         // Get wildcard list
     int             wildchar)      // The wildcard character
{
   wildchar &= 0x00ff;              // Character mask
   return wildlist[wildchar];
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       setWild
//
// Purpose-
//       Set the wildcard list for character.
//
//----------------------------------------------------------------------------
const char*                         // Prior value for wildcard
   setWild(                         // Set wildcard character
     int             wildchar,      // The wildcard character
     const char*     list)          // The wildcard value list
{
   const char*       result;        // Prior wildcard list

   wildchar &= 0x00ff;              // Character mask
   assert( wildchar != 0 );         // '\0' can never be wild
   result= wildlist[wildchar];
   wildlist[wildchar]= list;

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       wildcmp
//
// Purpose-
//       Memory compare, with wildcards.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   wildcmp(                         // Look for string match
     const char*     source,        // Source string
     const char*     target,        // Compare string
     unsigned        length)        // Compare length
{
   int               result;        // Resultant
   char              strList[2];    // Short source
   char              subList[2];    // Short target
   const char*       wildstr;       // Wildstring list
   const char*       wildsub;       // Wildsubstring list
   const char*       initsub;       // Wildsubstring list

   result= 0;
   while( length > 0 )
   {
     result= *source - *target;
     if( result != 0 )
     {
       wildstr= getWild(*source);
       if( wildstr == NULL )
       {
         strList[0]= *source;
         strList[1]= '\0';
         wildstr= strList;
       }

       initsub= getWild(*target);
       if( initsub == NULL )
       {
         subList[0]= *target;
         subList[1]= '\0';
         initsub= subList;
       }

       while( *wildstr != '\0' )
       {
         wildsub= initsub;
         while( *wildsub != '\0' )
         {
           if( *wildstr == *wildsub )
           {
             result= 0;
             break;
           }

           wildsub++;
         }
         if( result == 0 )
           break;

         wildstr++;
       }

       if( result != 0 )
         break;
     }

     source++;
     target++;
     length--;
   }

   return result;
}

#ifdef HCDM
int
   HCDMwildcmp(
     const char*     source,
     const char*     target)
{
   int               result;

   result= wildcmp(source, target);
   printf("%8d= wildcmp(%s,%s)\n", result, source, target);
   return result;
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       wildseg
//
// Purpose-
//       Look for string segment match, with wildcards.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   wildseg(                         // Look for string segment match
     const char*     source,        // Source string
     const char*     target)        // Compare string
{
   int               result;        // Resultant
   char              strList[2];    // Short source
   char              subList[2];    // Short target
   const char*       wildstr;       // Wildstring list
   const char*       wildsub;       // Wildsubstring list
   const char*       initsub;       // Wildsubstring list

   result= 0;
   for(; *target != '\0';)
   {
     result= *source - *target;
     if( result != 0 )
     {
       wildstr= getWild(*source);
       if( wildstr == NULL )
       {
         strList[0]= *source;
         strList[1]= '\0';
         wildstr= strList;
       }

       initsub= getWild(*target);
       if( initsub == NULL )
       {
         subList[0]= *target;
         subList[1]= '\0';
         initsub= subList;
       }

       while( *wildstr != '\0' )
       {
         wildsub= initsub;
         while( *wildsub != '\0' )
         {
           if( *wildstr == *wildsub )
           {
             result= 0;
             break;
           }

           wildsub++;
         }
         if( result == 0 )
           break;

         wildstr++;
       }

       if( result != 0 )
         break;
     }

     source++;
     target++;
   }

   return result;
}

#ifdef HCDM
int
   HCDMwildseg(
     const char*     source,
     const char*     target)
{
   int               result;

   result= wildseg(source, target);
   printf("%8d= wildseg(%s,%s)\n", result, source, target);
   return result;
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       wildstr
//
// Purpose-
//       strstr with wildcards.
//
//----------------------------------------------------------------------------
char*                               // Resultant
   wildstr(                         // Search for substring
     const char*     string,        // Source string
     const char*     substr)        // Substring
{
   unsigned          lString;       // (Remaining length of) string
   unsigned          lSubstr;       // Length of substring

   lString= strlen(string);
   lSubstr= strlen(substr);
   while( lString >= lSubstr )
   {
     if( wildseg(string, substr) == 0 )
       return (char*)string;

     string++;
     lString--;
   }

   return NULL;
}

#ifdef HCDM
char*
   HCDMwildstr(
     const char*     string,
     const char*     substr)
{
   char*             result;

   result= wildstr(string, substr);
   printf("%p= wildstr(%s,%s)\n", result, string, substr);
   return result;
}
#endif

#ifdef __cplusplus
} // extern "C"
#endif

