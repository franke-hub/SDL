//----------------------------------------------------------------------------
//
//       Copyright (c) 2011-2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Raw2Float.cpp
//
// Purpose-
//       Convert a raw floating point file to a printable version.
//
// Last change date-
//       2012/01/01
//
// Input-
//       stdin: The raw floating point file.
//         (Terminal input not supported.)
//
// Output-
//       stdout: The readable value file.
//
//----------------------------------------------------------------------------
#include <stdarg.h>                 // For va_list
#include <math.h>                   // For sqrt
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <sys/stat.h>               // For struct stat

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             inpSize;     // The input file cols per row

//----------------------------------------------------------------------------
//
// Subroutine-
//       error
//
// Purpose-
//       Write a message onto stderr
//
//----------------------------------------------------------------------------
static void
   error(                           // Write error message
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

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
   error("Raw2Float filename\n\n"
         "Converts the raw input file into readable numbers.\n"
         );
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
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   char*               argp;        // Argument pointer
   int                 argi;        // Argument index

   int                 ERROR;       // Error encountered indicator
   int                 HELPI;       // Help encountered indicator
   int                 verify;      // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   ERROR= FALSE;                    // Set defaults
   HELPI= FALSE;
   verify= FALSE;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter

     if( *argp == '-' )             // If this parameter is in switch format
     {
       if( strcmp("-help", argp) == 0 )  // If help request
         HELPI= TRUE;

       else if( strcmp("-verify", argp) == 0 )
         verify= TRUE;

       else                         // If invalid switch
       {
         ERROR= TRUE;
         error("Invalid parameter '%s'\n", argv[argi]);
       }
     }
     else                           // If filename parameter
     {
       ERROR= TRUE;
       error("Unexpected file name '%s'\n", argv[argi]);
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( !HELPI )
   {
     struct stat s;
     int fn= fileno(stdin);
     int rc= fstat(fn, &s);
     if( rc != 0 )
     {
       ERROR= TRUE;
       error("ERROR: %d= stat(STDIN), ", rc);
       perror("stat(STDIN)");
     }
     else
     {
       inpSize= s.st_size;       // Get the file size
       inpSize >>= 2;            // inpSize /= 4
       inpSize= (int)sqrt((double)inpSize); // Now ony X x X
       if( s.st_size != (inpSize*inpSize*4) )
       {
         ERROR= TRUE;
         error("Error: File(STDIN) size(%d) not Row == Col\n", s.st_size);
       }

       if( s.st_size == 0 )
       {
         ERROR= TRUE;
         error("Error: File(STDIN) empty\n");
       }
     }
   }

   if( HELPI || ERROR )
   {
     if( ERROR )
       error("\n");

     info();
   }

   if( verify )
   {
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       r2p
//
// Purpose-
//       Print the raw input file
//
//----------------------------------------------------------------------------
static void
   r2p( void )                      // Raw to print
{
   float* line= (float*)malloc(inpSize * sizeof(float));
   for(;;)
   {
     int L= fread(line, sizeof(float), inpSize, stdin);
     if( L <= 0 )
       break;

     if( L != inpSize )
     {
       error("Error: File(STDIN), read error(%d) %d\n", L, inpSize);
       break;
     }

     for(int i= 0; i<inpSize; i++)
     {
       if( i != 0 )
         printf(" ");

       printf("%10.6f", line[i]);
     }
     printf("\n");
   }

   free(line);
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
     char*             argv[])      // Argument array
{
   parm(argc, argv);                // Parameter analysis
   r2p();                           // Raw to print

   return EXIT_SUCCESS;
}

