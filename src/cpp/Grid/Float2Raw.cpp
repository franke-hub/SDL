//----------------------------------------------------------------------------
//
//       Copyright (c) 2011-2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Float2Raw.cpp
//
// Purpose-
//       Convert a readable floating point file to a raw version.
//
// Last change date-
//       2017/01/01
//
// Input-
//       stdin: The readable floating point file.
//         (Terminal input not supported.)
//
// Output-
//       stdout: The raw floating point file.
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
   error("Float2Raw\n"
         "Input via stdin (Terminal input not supported.)\n"
         "Converts the readable input file into raw format.\n"
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
//       p2r
//
// Purpose-
//       Convert the input file into raw format.
//
//----------------------------------------------------------------------------
static void
   p2r( void )                      // Raw to print
{
   // Verify stdin
   struct stat buff;
   int fn= fileno(stdin);
   int rc= fstat(fn, &buff);
   if( rc != 0 )
   {
     error("ERROR: %d= stat(STDIN), ", rc);
     perror("stat(STDIN)");
     exit(EXIT_FAILURE);
   }

#ifndef _OS_WIN
   if( S_ISCHR(buff.st_mode) )
   {
     error("ERROR: terminal input not supported\n");
     exit(EXIT_FAILURE);
   }
#endif

   float output;
   for(;;)
   {
     int L= fscanf(stdin, " %f", &output);
     if( L <= 0 )
       break;

     L= fwrite(&output, sizeof(output), 1, stdout);
     if( L != 1 )
     {
       error("Error: File(STDOUT), write error(%d)\n", L);
       exit(EXIT_FAILURE);
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
     char*             argv[])      // Argument array
{
   parm(argc, argv);                // Parameter analysis
   p2r();                           // Print to raw

   return EXIT_SUCCESS;
}

