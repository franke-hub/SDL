//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Wildchar.cpp
//
// Purpose-
//       Wildchar object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/Debug.h>
#include "com/Wildchar.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define MAX_CHAR 256                // Max character value
#define MAX_MASK 255                // Max character mask

//----------------------------------------------------------------------------
//
// Method-
//       Wildchar::~Wildchar
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Wildchar::~Wildchar( void )      // Destructor
{
   #ifdef HCDM
     debugf("Wildchar(%p)::~Wildchar()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Wildchar::Wildchar
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Wildchar::Wildchar( void )       // Constructor
{
   int                 i;

   #ifdef HCDM
     debugf("Wildchar(%p)::Wildchar()\n", this);
   #endif

   for(i= 0; i<MAX_CHAR; i++)
     wildlist[i]= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Wildchar::get
//
// Purpose-
//       Extract wild character list for character.
//
//----------------------------------------------------------------------------
const char*                         // Resultant
   Wildchar::get(                   // Extract wild character list
     int               wild)        // For this character
{
   wild &= MAX_MASK;
   return wildlist[wild];
}

//----------------------------------------------------------------------------
//
// Method-
//       Wildchar::set
//
// Purpose-
//       Replace wild character list for character.
//
//----------------------------------------------------------------------------
const char*                         // Resultant (prior wild card list)
   Wildchar::set(                   // Replace wild character list
     int               wild,        // For this character
     const char*       list)        // Using this list
{
   const char*         result;      // Resultant

   wild &= MAX_MASK;
   if( wild == 0 )
     throw "ValueException";

   result= wildlist[wild];
   wildlist[wild]= list;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Wildchar::compare
//
// Purpose-
//       Compare characters with wildcard replacement.
//
//----------------------------------------------------------------------------
inline int                          // Resultant <0, =0, >0)
   Wildchar::compare(               // Compare characters using wild cards
     int               source,      // Source character
     int               target)      // Target character
{
   int                 result;      // Resultant
   char                srcList[2];  // Short source list
   char                tgtList[2];  // Short target list
   const char*         srcWild;     // Source wild substring
   const char*         tgtWild;     // Target wild substring
   const char*         subWild;     // Target wild substring (working value)

   source &= MAX_MASK;
   target &= MAX_MASK;
   result= source - target;
   if( result != 0 )
   {
     srcWild= wildlist[source];     // Source wild list
     tgtWild= wildlist[target];     // Target wild list
     if( srcWild != NULL || tgtWild != NULL ) // If wild list replacement
     {
       if( srcWild == NULL )
       {
         srcList[0]= source;
         srcList[1]= '\0';
         srcWild= srcList;
       }

       if( tgtWild == NULL )
       {
         tgtList[0]= target;
         tgtList[1]= '\0';
         tgtWild= tgtList;
       }

       while( *srcWild != '\0' )    // Compare using wild card replacement
       {
         subWild= tgtWild;
         while( *subWild != '\0' )
         {
           if( *srcWild == *subWild )
           {
             result= 0;
             break;
           }

           subWild++;
         }
         if( result == 0 )
           break;

         srcWild++;
       }
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Wildchar::compare
//
// Purpose-
//       Compare strings with wildcard replacement.
//
//----------------------------------------------------------------------------
int                                 // Resultant <0, =0, >0)
   Wildchar::compare(               // Compare strings using wild cards
     const char*       source,      // Source string
     const char*       target)      // Target string
{
   int                 result;      // Resultant

   for(;;)
   {
     result= compare(*source,*target);
     if( result != 0 )
       break;

     if( *source == '\0' )
       break;

     source++;
     target++;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Wildchar::compare
//
// Purpose-
//       Compare memory with wildcard replacement.
//
//----------------------------------------------------------------------------
inline int                          // Resultant <0, =0, >0)
   Wildchar::compare(               // Compare memory using wild cards
     const char*       source,      // Source data area
     const char*       target,      // Target data area
     unsigned          length)      // Data area length
{
   int                 result;      // Resultant

   result= 0;
   while( length > 0 )
   {
     result= compare(*source,*target);
     if( result != 0 )
       break;

     source++;
     target++;
     length--;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Wildchar::strstr
//
// Purpose-
//       strstr with wildcard replacement.
//
//----------------------------------------------------------------------------
char*                               // Resultant <0, =0, >0)
   Wildchar::strstr(                // strstr with wild card replacement
     const char*       string,      // Source string
     const char*       substr)      // Target substring
{
   unsigned            length;      // Remaining string length
   unsigned            sublen;      // Substring length

   length= strlen(string);
   sublen= strlen(substr);
   while( length >= sublen )
   {
     if( compare(string,substr,sublen) == 0 )
       return (char*)string;

     string++;
     length--;
   }

   return NULL;
}

