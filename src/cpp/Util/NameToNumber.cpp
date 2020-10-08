//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       NameToNumber.cpp
//
// Purpose-
//       Convert a name to a number.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     letter= "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const int       number[]=
{    1   // A
,    2   // B
,    3   // C
,    4   // D
,    5   // E
,    6   // F
,    7   // G
,    8   // H
,    9   // I
,   10   // J
,   20   // K
,   30   // L
,   40   // M
,   50   // N
,   60   // O
,   70   // P
,   80   // Q
,   90   // R
,  100   // S
,  200   // T
,  300   // U
,  400   // V
,  500   // W
,  600   // X
,  700   // Y
,  800   // Z
,  900   // -
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       GetLetter
//
// Purpose-
//       Get the value of a letter
//
//----------------------------------------------------------------------------
static int                          // The letter value
   getLetter(                       // Get letter value
     int             L)             // The letter
{
   int               i;

   L= toupper(L);
   for(i= 0; letter[i] != '\0'; i++)
   {
     if( L == letter[i] )
       return number[i];
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       string
//
// Purpose-
//       Get the value of a string
//
//----------------------------------------------------------------------------
static int                          // The string value
   string(                          // Get string value
     const char*     string)        // The string
{
   int               result;        // Resultant

   result= 0;
   while( *string != '\0' )
   {
     result += getLetter(*string & 0x00ff);
     string++;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int                          // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     const char*     argv[])        // Argument array
{
   int               word= 0;       // Word value
   int               total= 0;      // Total value
   int               i;

   for(i= 1; i<argc; i++)
   {
     word= string(argv[i]);
     printf("%10d %s\n", word, argv[i]);
     total += word;
   }
   printf("%10d\n", total);

   return 0;
}

