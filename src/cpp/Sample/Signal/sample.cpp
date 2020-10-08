//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       sample.cpp
//
// Purpose-
//       Sample signal handler.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <com/Debug.h>
#include <com/define.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "SIGNAL  " // Source file, for debugging

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#define SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
static const char*     sigList[]=   // Signal list
{   /*00*/ "00 (Invalid)"
,   /*01*/ "Hangup"
,   /*02*/ "Interrupt"
,   /*03*/ "Quit"
,   /*04*/ "Illegal Instruction"
,   /*05*/ "Trace trap"
,   /*06*/ "Process abort"
,   /*07*/ "EMT Instruction"
,   /*08*/ "Floating point exception"
,   /*09*/ "Kill"
,   /*10*/ "Bus (specification) error"
,   /*11*/ "Segment violation"
,   /*12*/ "Bad argument to system call"
,   /*13*/ "No one to read pipe"
,   /*14*/ "Alarm clock timeout"
,   /*15*/ "Software termination signal"
,   /*16*/ "Ugent I/O channel condition"
,   /*17*/ "Stop"
,   /*18*/ "Interactive stop"
,   /*19*/ "Continue"
,   /*20*/ "Child stop or exit"
,   /*21*/ "Background read from control terminal"
,   /*22*/ "Background write to control terminal"
,   /*23*/ "I/O possible, or completed"
,   /*24*/ "CPU time limit exceeded"
,   /*25*/ "File size limit exceeded"
,   /*26*/ "(Invalid)"
,   /*27*/ "Input data in HFT ring buffer"
,   /*28*/ "Window size changed"
,   /*29*/ "Power fail restart"
,   /*30*/ "User signal 1"
,   /*31*/ "User signal 2"
,   /*32*/ "32 (Invalid)"
,   /*33*/ "33 (Invalid)"
,   /*34*/ "34 (Invalid)"
,   /*35*/ "35 (Invalid)"
,   /*36*/ "36 (Invalid)"
,   /*37*/ "37 (Invalid)"
,   /*38*/ "38 (Invalid)"
,   /*39*/ "39 (Invalid)"
,   /*40*/ "40 (Invalid)"
,   /*41*/ "41 (Invalid)"
,   /*42*/ "42 (Invalid)"
,   /*43*/ "43 (Invalid)"
,   /*44*/ "44 (Invalid)"
,   /*45*/ "45 (Invalid)"
,   /*46*/ "46 (Invalid)"
,   /*47*/ "47 (Invalid)"
,   /*48*/ "48 (Invalid)"
,   /*49*/ "49 (Invalid)"
,   /*50*/ "50 (Invalid)"
,   /*51*/ "51 (Invalid)"
,   /*52*/ "52 (Invalid)"
,   /*53*/ "53 (Invalid)"
,   /*54*/ "54 (Invalid)"
,   /*55*/ "55 (Invalid)"
,   /*56*/ "56 (Invalid)"
,   /*57*/ "57 (Invalid)"
,   /*58*/ "58 (Invalid)"
,   /*59*/ "59 (Invalid)"
,   /*60*/ "60 (Invalid)"
,   /*61*/ "61 (Invalid)"
,   /*62*/ "62 (Invalid)"
,   /*63*/ "63 (Invalid)"
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       sigHand
//
// Purpose-
//       Handle a signal.
//
//----------------------------------------------------------------------------
static void
   sigHand(                         // Handle a signal
     int             ident)         // Signal identifier
{
   fprintf(stderr, "\n");
   fprintf(stderr, "%s SIGNAL(%d) '%s' Received\n", __SOURCE__, ident,
                   size_t(ident) < ELEMENTS(sigList)
                     ? sigList[ident]
                     : "Unknown signal");

   fflush(stdout);                  // Force the standard buffers
   fflush(stderr);
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       logf
//
// Purpose-
//       Write message to log.
//
//----------------------------------------------------------------------------
static void
   vlogf(                           // Write log message
     const char*       fmt,         // PRINTF format descriptor
     va_list           argptr)      // PRINTF arguments
{
   char                buffer[512]; // Accumulator buffer
   const char*         threadName;  // Thread name

   threadName= __SOURCE__;
   vsprintf(buffer, fmt, argptr);   // Format the message
   fprintf(stderr, "%s: %s", threadName, buffer);
   fflush(stderr);
}

static void
   logf(                            // Write log message
     const char*       fmt,         // PRINTF format descriptor
                       ...)         // PRINTF argruments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vlogf(fmt, argptr);              // Write to log
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialization processing
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   signal(SIGHUP,    sigHand);      // Handle hangup
   signal(SIGINT,    sigHand);      // Handle interrupt
   signal(SIGQUIT,   sigHand);      // Handle quit
   signal(SIGILL,    sigHand);      // Handle illegal instruction
   signal(SIGABRT,   sigHand);      // Handle ABORT signal
   signal(SIGFPE,    sigHand);      // Handle floating point error
   signal(SIGKILL,   sigHand);      // Handle kill
   signal(SIGBUS,    sigHand);      // Handle specification fault
   signal(SIGSEGV,   sigHand);      // Handle segment violation
   signal(SIGPIPE,   sigHand);      // Handle no one to read pipe
   signal(SIGTERM,   sigHand);      // Handle termination signal
   signal(SIGSTOP,   sigHand);      // Handle stop
   signal(SIGTSTP,   sigHand);      // Handle interactive stop
   #ifdef SIGDANGER
     signal(SIGDANGER, sigHand);    // Handle page space exhaustion
   #endif
   #ifdef SIGSAK
     signal(SIGSAK,    sigHand);    // Handle attention key
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Termination processing
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit
//
//----------------------------------------------------------------------------
static void
   info(                            // Informational exit
     const char*     sourceName)    // The source fileName
{
   fprintf(stderr, "%s function <options>\n", sourceName);
   fprintf(stderr, "\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "-port:number\tPort number (default 65025)\n");
   fprintf(stderr, "-user:userid\tConnection userid\n");
   fprintf(stderr, "-pass:passwd\tConnection password\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Analyze parameters
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   int               error;         // TRUE if error encountered
   int               verify;        // TRUE if verify required

   int               i, j;

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   verify= FALSE;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp(argv[j], "-help") == 0 )
         error= TRUE;

       else
       {
         for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'h':              // -h (help)
               error= TRUE;
               break;

             case 'v':              // -v (verify)
               verify= TRUE;
               break;

             default:               // If invalid switch
               error= TRUE;
               fprintf(stderr, "Invalid switch '%c'\n", (int)argv[j][i]);
               break;
           }
         }
       }
     }
     else                           // Argument
     {
       error= TRUE;
       fprintf(stderr, "Invalid parameter: '%s'\n", argv[j]);
     }
   }

   //-------------------------------------------------------------------------
   // Check and validate
   //-------------------------------------------------------------------------
   if( error )
     info(argv[0]);

   if( verify )
   {
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
int                                 // Return code
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   //-------------------------------------------------------------------------
   // Parameter validation
   //-------------------------------------------------------------------------
   init();
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Process the function
   //-------------------------------------------------------------------------
   logf("Sleeping...\n");
   sleep(30);

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   term();
   return 0;
}

