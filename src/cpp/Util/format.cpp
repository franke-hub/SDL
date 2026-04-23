//----------------------------------------------------------------------------
//
//       Copyright (c) 2012-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//-----------------------------------------------------------------------------
//
// Title-
//       format.cpp
//
// Purpose-
//       Format stdin, writing to stdout.
//
// Last change date-
//       2026/04/20
//
// Implementation notes-
//       Trailing blanks are *always* removed.
//
// Options-
//       --hcdm        [Hard Core Debug Mode]
//       --verbose{:n} [Verbosity, higher is more verbose]
//
//       --fix:empty   [Remove completely blank lines]
//       // -fix:bs    [Change "C\b\C", "_\bC", and "C\b_" sequences to "C"]
//       --mode:dos    [End each line with "\r\n"]
//       --mode:unix   [End each line with "\n"]
//
//----------------------------------------------------------------------------
#include <cstdio>                   // For fprintf
#include <cstdlib>                  // For atoi, exit
#include <cstring>                  // For strcmp

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // Generic enum

enum MODE                           // Output mode
{  MODE_NONE                        // No conversion
,  MODE_DOS                         // Convert to DOS format
,  MODE_UNIX                        // Convert to UNIX format
}; // enum MODE

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static int             mode= MODE_NONE; // Conversion mode

static int             opt_hcdm= HCDM; // --hcdm
static int             opt_verbose= VERBOSE; // --verbose

static int             opt_empty= false; // --fix:empty

//----------------------------------------------------------------------------
//
// Subroutine-
//       modeName
//
// Purpose-
//       Name associated with -mode parameter
//
//----------------------------------------------------------------------------
static const char*                  // -mode: parameter
   modeName( void )                 // Get -mode parameter name
{
   if( mode == MODE_DOS )
     return "DOS";
   if( mode == MODE_UNIX )
     return "UNIX";
   return "NONE";
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
   fprintf(stderr, "format {options}\n"
           "  Copy stdin to stdout (with optional formatting)\n"
           "  (Trailing blanks are *always* removed)\n"
           "\n"
           "Options:\n"
           "  --hcdm\tHard Core Debug Mode\n"
           "  --verbose{:n}\tVerbosity, higher is more verbose\n"
           "\n"
           "  --fix:empty\tRemove empty lines\n"
//         "  --fix:bs\t"
//         "Convert \"C\\bC\", \"_\\bC\", or \"C\\b_\",  into \"C\"\n"
           "\n"
           "  --mode:dos\tEnd each line with \\r\\n\n"
           "  --mode:unix\tEnd each line with \\n\n"
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

   int                 help= false; // Help needed

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; ++argi ) { // Analyze variable controls
     argp= argv[argi];              // Address the parameter
     if( strcmp(argp, "--help") == 0 )
       help= true;
     else if( strcmp(argp, "--hcdm") == 0 )
       opt_hcdm= true;
     else if( memcmp(argp, "--verbose", 9) == 0 ) {
       opt_verbose= true;
       if( strlen(argp) > 9 ) {     // If parameter specified
         if( strlen(argp) == 10 || argp[9] != ':' ) { // If malformed
           help= true;
           fprintf(stderr, "Invalid parameter '%s'\n", argp);
           continue;
         } else {
           opt_verbose= atoi(argp+10);
         }
       }
     }
//   else if( strcasecmp(argp, "--fix:bs") == 0 )
//     fix= fix_bs;
     else if( strcasecmp(argp, "--fix:empty") == 0 )
       opt_empty= true;
     else if( strcasecmp(argp, "--mode:dos") == 0 )
       mode= MODE_DOS;
     else if( strcasecmp(argp, "--mode:unix") == 0 )
       mode= MODE_UNIX;
     else {
       help= true;
       fprintf(stderr, "Invalid parameter '%s'\n", argp);
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( help )                       // If assistance required
     info();

   if( opt_verbose ) {
     fprintf(stderr, "%5s --hcdm\n",    opt_hcdm ? "TRUE" : "FALSE");
     fprintf(stderr, "%5d --verbose\n", opt_verbose);

     fprintf(stderr, "%5s -fix:empty\n", opt_empty ? "TRUE" : "FALSE");
     fprintf(stderr, "%5s --mode\n", modeName());
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       inp2out
//
// Purpose-
//       Copy input file into output file.
//
//----------------------------------------------------------------------------
static void
   inp2out( void )                  // Copy stdin to stdout
{
   int blankCount= 0;               // Clear blank counter
   int O= '\n';                     // Last output character
   int I= getchar();                // Current input character
   for(;;) {
     if( I < 0 )
       break;

     if( I == '\r' ) {
       if( mode == MODE_NONE )
         putchar(I);

       I= getchar();
       continue;
     }

     if( I == '\n' ) {
       blankCount= 0;

       if( opt_empty ) {
         if( O == '\n' ) {
           I= getchar();
           continue;
         }
       }

       if( mode == MODE_DOS )
         putchar('\r');

       putchar(I);

       O= I;
       I= getchar();
       continue;
     }

     if( I == ' ' ) {
       ++blankCount;

       I= getchar();
       continue;
     }

     // The current character isn't '\r', '\n', or ' '
     while( blankCount > 0 ) {
       putchar(' ');
       --blankCount;
     }

     // Handle backspace
     if( false ) {                  // Not sure what's wanted here
       int C= getchar();
       if( C == '\b' ) {
         C= getchar();
         if( I == '\b' ) {
           putchar(I);
           O= I;
           I= C;
         } else if( I == '_' || C == '_' || I == C ) {
           if( I == '_' )
             I= C;
         } else {
           putchar(I);
           putchar('\b');
           O= '\b';
           I= C;
         }
       } else {
         putchar(I);
         O= I;
         I= C;
       }

       continue;
     }

     putchar(I);
     O= I;
     I= getchar();
   }

   if( O != '\n' ) {
     if( mode == MODE_DOS )
       putchar('\r');

     if( mode != MODE_NONE )
       putchar('\n');
   }
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
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Copy intput to output
   //-------------------------------------------------------------------------
   inp2out();

   return 0;
}
