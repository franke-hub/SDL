//----------------------------------------------------------------------------
//
//       Copyright (c) 2009 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       jumble.cpp
//
// Purpose-
//       Display all possiblities for a jumbled word.
//
// Last change date-
//       2009/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BRINGUP FALSE                // Testing bringup mode

#define MAXCOL 80                    // Maximum column length
#define MAXLEN 4096                  // Modified word length

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
int                    outcol= 0;    // Output column (external for debugging)

//----------------------------------------------------------------------------
//
// Subroutine-
//       display
//
// Purpose-
//       Display a string.
//
//----------------------------------------------------------------------------
static void
   display(                         // Display string
     int               length,      // The string length
     const char*       string)      // The jumbled word
{
   #if( BRINGUP == FALSE )
     if( (outcol+length) > MAXCOL )
     {
       printf("\n");
       outcol= 0;
     }
     else if( outcol > 0 )
       printf(" ");

     printf("%s", string);
     outcol += (length + 1);
   #else
     printf("%s\n", string);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       jumble
//
// Purpose-
//       Display all unique possibilities for a jumbled word.
//
//----------------------------------------------------------------------------
static void
   jumble(                          // Display jumbled possibilities
     int               index,       // The jumble index
     int               length,      // The length of the jumbled word
     const char*       string)      // The jumbled word
{
   char                modstr[MAXLEN]; // The modified word

   #if( BRINGUP )
     tracef("%2d, %s\n", index, string); // For debugging only
   #endif

   if( index >= length )
     display(length, string);
   else
   {
     #if( BRINGUP )
       tracef("%2d, %s => %2d, %s\n", index, string, index+1, string);
     #endif

     jumble(index+1, length, string);

     for(int x= index+1; x<length; x++)
     {
       strcpy(modstr, string);
       char C= modstr[x];
       if( modstr[index] != C )
       {
         int isValid= TRUE;         // If next not a duplicate
         char D= string[x];
         for(int y= x+1; y<length; y++)
         {
           if( D == string[y] )
           {
             isValid= FALSE;
             break;
           }
         }

         if( isValid )
         {
           modstr[x]= modstr[index];
           modstr[index]= C;
           #if( BRINGUP )
             tracef("%2d, %s => %2d, %s\n", index, string, index+1, modstr);
           #endif
           jumble(index+1, length, modstr);
         }
       }
     }
   }
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
     int               argc,        // Argument count
     const char*       argv[])      // Argument array
{
   int                 i;

   for(i= 1; i<argc; i++)
   {
     int L= strlen(argv[i]);
     if( L < MAXLEN )
     {
       jumble(0, L, argv[i]);
       if( outcol > 0 )
       {
         printf("\n");
         outcol= 0;
       }
     }
   }

   return 0;
}

