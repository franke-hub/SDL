//----------------------------------------------------------------------------
//
//       Copyright (c) 2008 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//-----------------------------------------------------------------------------
//
// Title-
//       FSmanip.cpp
//
// Purpose-
//       Manipulate files.
//
// Last change date-
//       2008/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <com/define.h>
#include <com/Reader.h>
#include <com/Writer.h>

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     inpName= NULL; // The input file name
static int             addcomma= FALSE; // addcomma switch
static int             addspace= FALSE; // addspace switch
static int             evenodd= FALSE; // evenodd switch
static int             nulldel= FALSE; // nulldel switch
static int             oddeven= FALSE; // oddeven switch

static char            ioBUFF[65536]; // The input/output buffer
static char            eoBUFF[65536]; // The even/odd line I/O buffer

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
   error("FSmanip options inp-filename\n\n"
         "Copies the input file onto stdout.\n"
         "Carrage returns in the input file are removed.\n"
         "The first line, line 1, is odd numbered.\n");
   error("\n");
   error("options:\n"
         "-addcomma\tAdd comma between pairs\n"
         "-addspace\tAdd space between pairs\n"
         "-evenodd\tCombine even/odd pair lines into one output line\n"
         "-oddeven\tCombine odd/even pair lines into one output line\n"
         "-nulldel\tDelete null lines\n");
   error("\n");
   error("inp-filename\t(The input file name)\n");
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

       else if( strcmp("-addcomma", argp) == 0 )
         addcomma= TRUE;

       else if( strcmp("-addspace", argp) == 0 )
         addspace= TRUE;

       else if( strcmp("-evenodd", argp) == 0 )
         evenodd= TRUE;

       else if( strcmp("-nulldel", argp) == 0 )
         nulldel= TRUE;

       else if( strcmp("-oddeven", argp) == 0 )
         oddeven= TRUE;

       else                         // If invalid switch
       {
         ERROR= TRUE;
         error("Invalid parameter '%s'\n", argv[argi]);
       }
     }
     else                           // If filename parameter
     {
       if( inpName == NULL )
         inpName= argp;

       else
       {
         ERROR= TRUE;
         error("Unexpected file name '%s'\n", argv[argi]);
       }
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( verify )
   {
     error("-addcomma: %s\n", addcomma ? "TRUE" : "FALSE");
     error("-addspace: %s\n", addspace ? "TRUE" : "FALSE");
     error(" -evenodd: %s\n",  evenodd ? "TRUE" : "FALSE");
     error(" -nulldel: %s\n",  nulldel ? "TRUE" : "FALSE");
     error(" -oddeven: %s\n",  oddeven ? "TRUE" : "FALSE");
     error("     File: %s\n",  inpName);
   }

   if( !HELPI )
   {
     if( !evenodd  && !nulldel && !oddeven )
       error("Warning: No manipulation function selected.\n");

     if( inpName == NULL )
     {
       ERROR= TRUE;
       error("Error: No filename specified\n");
     }
   }

   if( addcomma && addspace )
   {
     ERROR= TRUE;
     error("Error: -addcomma and -addspace are mutually exclusive.\n");
   }

   if( evenodd && oddeven )
   {
     ERROR= TRUE;
     error("Error: -evenodd and -oddeven are mutually exclusive.\n");
   }

   if( evenodd || oddeven )
   {
     if( nulldel )
     {
       ERROR= TRUE;
       error("Error: -evenodd and -oddeven conflict with -nulldel.\n");
     }
   }
   else
   {
     if( addcomma )
     {
       ERROR= TRUE;
       error("Error: -addcomma requires -evenodd or -oddeven.\n");
     }

     if( addspace )
     {
       ERROR= TRUE;
       error("Error: -addspace requires -evenodd or -oddeven.\n");
     }
   }

   if( ERROR )
     error("\n");

   if( HELPI || ERROR )
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       manip
//
// Purpose-
//       Copy and manipulate the input file.
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   manip( void )                    // Copy the file
{
   char                delimit= '\0'; // Combination delimiter
   FileReader          reader(inpName); // File Reader
   FileWriter          writer(NULL);// File Writer

   int                 rc= 0;       // Return code

   //-------------------------------------------------------------------------
   // Test the file
   //-------------------------------------------------------------------------
   if( reader.getState() != reader.STATE_INPUT )
   {
     error("File(%s): NOT READABLE\n", inpName);
     return 2;
   }

   if( addcomma )
     delimit= ',';
   else if( addspace )
     delimit= ' ';

   //-------------------------------------------------------------------------
   // Copy the file
   //-------------------------------------------------------------------------
   try {
     for(;;)
     {
       rc= reader.readLine(ioBUFF, sizeof(ioBUFF));
       if( rc < 0 )
         break;

       if( oddeven )
       {
         writer.write(ioBUFF, strlen(ioBUFF));

         rc= reader.readLine(ioBUFF, sizeof(ioBUFF));
         if( rc < 0 )
         {
           error("Warning: Missing last even line\n");
           writer.writeLine("");
           break;
         }

         if( delimit != '\0' )
           writer.write(&delimit, 1);
         writer.writeLine(ioBUFF);
       }

       else if( evenodd )
       {
         rc= reader.readLine(eoBUFF, sizeof(eoBUFF));
         if( rc < 0 )
         {
           error("Warning: Missing last even line\n");
           writer.writeLine(ioBUFF);
           break;
         }

         writer.write(eoBUFF, strlen(eoBUFF));
         if( delimit != '\0' )
           writer.write(&delimit, 1);
         writer.writeLine(ioBUFF);
       }
       else
       {
         if( ioBUFF[0] != '\0' || !nulldel )
           writer.writeLine(ioBUFF);
       }
     }
   } catch(const char* s) {
     rc= 2;
     error("Exception: %s\n", s);
   } catch(...) {
     error("Exception occured\n");
   }

   if( rc != reader.RC_EOF )
   {
     error("Unexpected: %d= readLine()\n", rc);
     rc= 2;
   }
   else
     rc= 0;

   if( reader.close() != 0 )
   {
     rc= 2;
     error("Unable to close input file\n");
   }

   return rc;
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
   // Operate and return
   //-------------------------------------------------------------------------
   return manip();
}

