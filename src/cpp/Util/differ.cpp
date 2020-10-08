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
//       differ.cpp
//
// Purpose-
//       Compare files for equality.
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

#include <com/define.h>
#include <com/params.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BUF_SIZE    32768           // Input buffer size, in bytes

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static char          baseBuff[BUF_SIZE]; // Input buffer (base)
static char          compBuff[BUF_SIZE]; // Input buffer (compare)

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
   fprintf(stderr, "differ filename filename ...\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "filename ...\n");
   fprintf(stderr, "  The list files to compare\n");
   fprintf(stderr, "  Each file is compared with all other files\n");
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

   int               count;         // The number of files to compare
   int               error;         // Error encountered indicator
// int               verify;        // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   count= 0;                        // No files found yet
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
       count++;
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( count < 2 )                  // If too few files specified
   {
     error= TRUE;
     if( count == 0 )
       fprintf(stderr, "No filename specified\n");
     else
       fprintf(stderr, "Only one filename specified\n");
   }

   if( error )                      // If error encountered
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       differ
//
// Purpose-
//       Compare two files.
//
//----------------------------------------------------------------------------
static int                          // Return code
   differ(                          // Compare files
     char*           base,          // Base file
     char*           comp)          // Comparand
{
   int               result;        // Resultant

   FILE*             hBase;         // Handle[Base]
   FILE*             hComp;         // Handle[Comp]
   int               lBase;         // Length[Base]
   int               lComp;         // Length[Comp]
   int               L;             // Min(lBase, lComp)

   //-------------------------------------------------------------------------
   // Open the files
   //-------------------------------------------------------------------------
   if( strcmp(base,comp) == 0 )
   {
     fprintf(stderr, "File(%s) == File(%s) (same file)\n",
                     comp, base);
     return 0;
   }

   hBase= fopen(base, "rb");
   if( hBase == NULL )
   {
     fprintf(stderr, "File(%s) ", base);
     perror("open failed");
     return -1;
   }

   hComp= fopen(comp, "rb");
   if( hComp == NULL )
   {
     fprintf(stderr, "File(%s) ", comp);
     perror("open failed");

     fclose(hBase);
     return 1;
   }

   //-------------------------------------------------------------------------
   // Compare the files
   //-------------------------------------------------------------------------
   result= 0;
   for(;;)
   {
     lBase= fread(baseBuff, 1, BUF_SIZE, hBase);
     lComp= fread(compBuff, 1, BUF_SIZE, hComp);
     L= lBase;
     if( L > lComp )
       L= lComp;

     if( memcmp(baseBuff, compBuff, L) != 0 )
     {
       fprintf(stderr, "File(%s) != File(%s)\n",
                       base, comp);
       result= 1;
       break;
     }

     if( lBase != lComp )
     {
       if( lBase > lComp )
         fprintf(stderr, "File(%s) == File(%s) base, but File(%s) larger\n",
                         base, comp, base);
       else
         fprintf(stderr, "File(%s) == File(%s) base, but File(%s) larger\n",
                         base, comp, comp);
       result= 1;
       break;
     }

     if( L == 0 )
     {
       fprintf(stderr, "File(%s) == File(%s)\n",
                       base, comp);
       break;
     }
   }

   fclose(hComp);
   fclose(hBase);
   return result;
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
   int               rc;            // Called routine's return code
   int               returncd;      // This routine's return code

   int               i;
   int               j;

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   returncd= 0;
   for(i=1; i<argc; i++)
   {
     if( argv[i][0] == '-' )        // If this parameter is in switch format
       continue;

     for(j=i+1; j<argc; j++)
     {
       if( argv[j][0] == '-' )      // If this parameter is in switch format
         continue;

       rc= differ(argv[i], argv[j]);// Compare the files
       if( rc != 0 )                // If files differ
         returncd= 1;               // Indicate it

       if( rc < 0 )                 // If file[i] is invalid
         break;                     // No need to compare it any further
     }
   }

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   return returncd;
}
