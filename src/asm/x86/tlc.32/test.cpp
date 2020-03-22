//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       test.cpp
//
// Purpose-
//       Sample code generator.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
volatile int           junk[32];

//----------------------------------------------------------------------------
//
// Subroutine-
//       local
//
// Purpose-
//       Local subroutine.
//
//----------------------------------------------------------------------------
extern "C" volatile int             // Return code
   local(                           // Sample code
     volatile int*     junk)
{
   int                 temp;

   int                 i;

   for(i= 0; i<32; i++)
     junk[i]= i;

   temp= 0;
   for(i= 0; i<32; i++)
     temp += junk[i] * 3;

   return temp;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test
//
// Purpose-
//       Sample code generator.
//
//----------------------------------------------------------------------------
extern "C" int                      // Return code
   test( void )                     // Sample code
{
   char                string[128];
   volatile int        (*function)(volatile int*);

   function= &local;

   printf("%s %d\n", "Test OK", (*function)(junk));
   fgets(string, sizeof(string), stdin);

/***
   if( junk[0] < junk[1] )
     printf("TRUE\n");

   if( (unsigned)junk[0] < (unsigned)junk[1] )
     printf("TRUE\n");

   printf("%d\n", junk[0] * junk[1]);
   printf("%d\n", junk[0] / junk[1]);
   printf("%d\n", junk[0] % junk[1]);
***/

   return 0;
}

