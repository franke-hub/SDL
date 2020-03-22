//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       format.cpp
//
// Purpose-
//       Format an input file.
//
// Last change date-
//       2012/01/01
//
// Options-
//       -fix:blank  [Remove empty lines]
//       -fix:bs     [Change "C\b\C", "_\bC", and "C\b_" sequences to "C"]
//       -mode:dos   [End each line with "\r\n"]
//       -mode:unix  [End each line with "\n"]
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum MODE                           // Output mode
{  MODE_NONE                        // No conversion
,  MODE_DOS                         // Convert to DOS format
,  MODE_UNIX                        // Convert to UNIX format
}; // enum MODE

enum FIX                            // Repair mode
{  FIX_NONE                         // No conversion
,  FIX_BLANK                        // Remove blank lines
,  FIX_BS                           // Convert "C\bC" into "C"
}; // enum MODE

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static int             fix=  FIX_NONE;  // Repair mode
static int             mode= MODE_NONE; // Convertion mode

//----------------------------------------------------------------------------
//
// Subroutine-
//       fixName
//
// Purpose-
//       Name associated with -fix parameter
//
//----------------------------------------------------------------------------
static const char*                  // -fix: parameter
   fixName( void )                  // Get -fix parameter name
{
   if( fix == FIX_BLANK )
     return "BLANK";
   if( fix == FIX_BS )
     return "BS";

   return "NONE";
}

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
   fprintf(stderr, "format {options} <input >output\n");
   fprintf(stderr, "\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "  -fix:blank Remove empty lines\n");
   fprintf(stderr, "  -fix:bs    Convert \"C\\bC\", \"_\\bC\", or \"C\\b_\",  into \"C\"\n");
   fprintf(stderr, "  -mode:dos  End each line with \\r\\n\n");
   fprintf(stderr, "  -mode:unix End each line with \\n\n");
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

   int                 error;       // Error encountered indicator
   int                 verify;      // Verification control

   //-------------------------------------------------------------------------
   // Defaults
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no errors found
   verify= 0;                       // Default, no verification

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for( argi=1; argi<argc; argi++ ) // Analyze variable controls
   {
     argp= argv[argi];              // Address the parameter
     if( strcmp(argp, "-help") == 0 )
       error= TRUE;

     else if( strcmp(argp, "-verify") == 0 )
       verify= TRUE;

     else if( strcmp(argp, "-fix:blank") == 0 )
       fix= FIX_BLANK;

     else if( strcmp(argp, "-fix:bs") == 0 || strcmp(argp, "-fix:BS") == 0 )
       fix= FIX_BS;

     else if( strcmp(argp, "-mode:dos") == 0 || strcmp(argp, "-mode:DOS") == 0 )
       mode= MODE_DOS;

     else if( strcmp(argp, "-mode:unix") == 0 || strcmp(argp, "-mode:UNIX") == 0 )
       mode= MODE_UNIX;

     else
     {
       error= TRUE;
       fprintf(stderr, "Invalid parameter '%s'\n", argp);
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( error )                      // If error encountered
     info();

   if( verify )
   {
     fprintf(stderr, "-fix:%s\n",  fixName());
     fprintf(stderr, "-mode:%s\n", modeName());
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
void
   inp2out( void )                  // Copy stdin to stdout
{
   int                 blankCount;  // Number of skipped blanks

   int P= getchar();                // Get first character
   if( mode != MODE_NONE && P == '\r' )
   {
     while( P == '\r' )
       P= getchar();
   }

   int O= '\n';                     // Last output character
   blankCount= 0;                   // Clear blank counter
   for(;;)
   {
     if( P < 0 )
       break;

     if( P == '\r' )
     {
       if( mode == MODE_NONE )
       {
         O= P;
         putchar(P);
       }

       P= getchar();
       continue;
     }

     if( P == '\n' )
     {
       blankCount= 0;

       if( fix == FIX_BLANK )
       {
         if( O == '\n' )
         {
           P= getchar();
           continue;
         }
       }

       if( mode != MODE_DOS )
         putchar(P);
       else
       {
         putchar('\r');
         putchar(P);
       }

       O= P;
       P= getchar();
       continue;
     }

     if( fix == FIX_BLANK )
     {
       if( O == '\n' )              // If still scanning
       {
         if( P == ' ' )
         {
           blankCount++;

           P= getchar();
           continue;
         }
         else                       // Non-blank found
         {
           while( blankCount > 0 )
           {
             putchar(' ');
             blankCount--;
           }
         }
       }
     }

     else if( fix == FIX_BS )
     {
       int C= getchar();
       if( C == '\b' )
       {
         C= getchar();
         if( P == '\b' )
         {
           putchar(P);
           O= P;
           P= C;
         }

         else if( P == '_' || C == '_' || P == C )
         {
           if( P == '_' )
             P= C;
         }

         else
         {
           putchar(P);
           putchar('\b');
           O= '\b';
           P= C;
         }
       }
       else
       {
         putchar(P);
         O= P;
         P= C;
       }

       continue;
     }

     putchar(P);
     O= P;
     P= getchar();
   }

   if( O != '\n' )
   {
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
