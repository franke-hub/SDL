//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       getenv.cpp
//
// Purpose-
//       Display the current envrironment.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/define.h>
#include <com/params.h>
#include <com/Unconditional.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter fault exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Parameter fault exit
{
   fprintf(stderr, "getenv\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Displays the environment variables\n");
   fprintf(stderr, "No parameters are expected\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   char*             argp;          // Argument pointer
   int               argi;          // Argument index

   int               error;         // Error encountered indicator
// int               verify;        // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no errors found
// verify= 0;                       // Default, no verification

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       argp++;                      // Skip over the switch char

//     if( swname("verify", argp) ) // If verify switch
//       verify= swatob("verify", argp); // Get switch value
//
//     else                         // If invalid switch
       {
         error= TRUE;
         fprintf(stderr, "Invalid parameter '%s'\n",
                         argv[argi]);
       }
     }
     else                           // If filename parameter
     {
       error= TRUE;
       fprintf(stderr, "Invalid parameter '%s'\n",
                       argv[argi]);
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( error )                      // If error encountered
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int                                 // Main return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   char**            sorted;        // Sorted environ
   char*             ptrC;          // -> char
   int               i, j, m;

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Function
   //-------------------------------------------------------------------------
   for(m=0; environ[m] != NULL; m++ ) // Count the number of variables
     ;

   sorted= (char**)must_malloc(m*sizeof(char*)); // Allocate the sort array
   for(i=0; i<m ; i++ )             // Populate the sort array
     sorted[i]= environ[i];

   for(i=0; i<m ; i++ )             // (Bubble) sort the array
   {
     for(j=i+1; j<m; j++)
     {
       if( strcmp(sorted[i], sorted[j]) > 0 )
       {
         ptrC= sorted[i];
         sorted[i]= sorted[j];
         sorted[j]= ptrC;
       }
     }
   }

   for(i=0; i<m ; i++ )
     printf("%s\n", sorted[i]);

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   return 0;
}
