//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/Stress/Tdisp.cpp
//
// Purpose-
//       ~/src/cpp/inc/pub/Dispatcher.h Stress test
//
// Last change date-
//       2020/07/05
//
// Parameters-
//       --help        (Display help message)
//       --hcdm        (Hard Core Debug Mode)
//
//       --ditem=n     (Number of Dispatch::Items/Iteration)
//       --dtask=n     (Number of Dispatch::Tasks/Thread)
//       --first       (First completion terminates test)
//       --multi=n     (Thread count, alternatively)
//       --quick       (Run quick test)
//       --trace=n     (Trace table size)
//       --verbose{=n} (Debugging verbosity)
//
//       [0] Iteration count
//       [1] Thread count
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic
#include <exception>                // For std::exception
#include <random>                   // For randomization features

#include <assert.h>                 // For assert()
#include <ctype.h>                  // For isprint()
#include <errno.h>                  // For errno
#include <getopt.h>                 // For getopt_long()
#include <limits.h>                 // For INT_MAX, INT_MIN
#include <signal.h>                 // For signal
#include <stdarg.h>                 // For va_list
#include <stdio.h>                  // For printf
#include <string.h>                 // For strcmp
#include <unistd.h>                 // For getpid, ...

#include "pub/Dispatch.h"           // For pub::Dispatch (Which we test)
#include <pub/Debug.h>              // For pub::debugging::debugf, ...
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Event.h>              // For pub::Event
#include <pub/Interval.h>           // For pub::Interval
#include <pub/Named.h>              // For pub::Named (Threads are named)
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::utility::atol
#include <pub/macro/try_catch.h>    // For TRY_CATCH macro

// Global controls                  // Enable class access
using pub::Debug;                   // Enable pub::Debug
using pub::Trace;                   // Enable pub::Trace
using DispDisp= pub::Dispatch::Disp; // Alias pub::Dispatch::Disp
using DispItem= pub::Dispatch::Item; // Alias pub::Dispatch::Item
using DispTask= pub::Dispatch::Task; // Alias pub::Dispatch::Task
using DispWait= pub::Dispatch::Wait; // Alias pub::Dispatch::Wait

// Global controls                  // Enable subroutines access
using namespace pub::debugging;     // Enable pub::debugging subroutines

#include "Tdisp.h"                  // Common include (AFTER global controls)

//----------------------------------------------------------------------------
// Options (Note: opt_multi is both a positional and a keyword option)
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
static int             opt_index;   // Option index

//--------------------------------- // Common.h defined options
////// int             opt_hcdm= false; // --hcdm (Hard Core Debug Mode)
////// int             opt_first= false; // --first
////// uint32_t        opt_trace= TRACE_SIZE; // --trace argument
////// int             opt_verbose= -1; // --verbose

////// size_t          opt_iterations= ITERATIONS; // arg[0] Iteration count
////// int             opt_multi= TASK_COUNT;      // arg[1] Thread count

// getopt parameters ---------------------------------------------------------
static const char*     OSTR= ":";   // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",    no_argument,       &opt_help,    true} // --help (no argument)
,  {"hcdm",    no_argument,       &opt_hcdm,    true} // Hard Core Debug Mode

,  {"ditem",   required_argument, nullptr,      0} // Items per iteration
,  {"dtask",   required_argument, nullptr,      0} // Tasks per Thread
,  {"first",   no_argument,       &opt_first,   true} // First finish, untrace
,  {"multi",   required_argument, nullptr,      0} // Multithread count
,  {"quick",   no_argument,       nullptr,      0} // Quick test
,  {"trace",   required_argument, nullptr,      0} // Trace table size
,  {"verbose", optional_argument, &opt_verbose, 0} // Verbosity {=optional}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_HELP
,  OPT_HCDM

,  OPT_DITEM
,  OPT_DTASK
,  OPT_FIRST
,  OPT_MULTI
,  OPT_QUICK
,  OPT_TRACE
,  OPT_VERBOSE
,  OPT_SIZE
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
typedef void (*sig_handler_t)(int);
static sig_handler_t   sys1_handler= nullptr; // System SIGINT  signal handler
static sig_handler_t   usr1_handler= nullptr; // System SIGUSR1 signal handler
static sig_handler_t   usr2_handler= nullptr; // System SIGUSR2 signal handler

//----------------------------------------------------------------------------
//
// Subroutine-
//       sig_handler
//
// Purpose-
//       Handle signals.
//
//----------------------------------------------------------------------------
extern "C" void
   sig_handler(                     // Handle signals
     int               id)          // The signal identifier
{  debugh("\n\nsig_handler(%d) pid(%d)\n", id, getpid());

   switch(id) {                     // Handle the signal
     case SIGINT:
     case SIGUSR1:
     case SIGUSR2:
       if( task_array ) {           // Debug if initialized
         Main::debug(__LINE__);

         DispDisp::debug();
         return;
       }
       break;
   }

   debugh("Signal(%d) ignored\n", id);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Global initialization
//
//----------------------------------------------------------------------------
static int                          // Return code, 0 OK
   init(                            // Initialize
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize signal handling
   //-------------------------------------------------------------------------
   // Initialize signal handling
   sys1_handler= signal(SIGINT,  sig_handler);
   usr1_handler= signal(SIGUSR1, sig_handler);
   usr2_handler= signal(SIGUSR2, sig_handler);

   //-------------------------------------------------------------------------
   // Initialize/activate debugging trace (with options)
   setlocale(LC_NUMERIC, "");       // Allows printf("%'d\n", 123456789);

   Debug* debug= Debug::get();      // Activate debug tracing
// debug->set_head(Debug::HeadTime | Debug::HeadThread);
   debug->set_head(Debug::HeadTime); // All messages have thread identifier

   if( HCDM ) opt_hcdm= true;       // If HCDM compile-time, force opt_hcdm
   if( opt_hcdm ) {                 // If --hcdm option specified
     debug->set_mode(Debug::ModeIntensive); // Hard Core Debug Mode
     debugf("%4d HCDM.c pid(%d)\n", __LINE__, getpid());
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Global termination
//
//----------------------------------------------------------------------------
extern void
   term( void )                     // Terminate
{
   //-------------------------------------------------------------------------
   // Restore syste signal handlers
   signal(SIGINT,  sys1_handler);
   signal(SIGUSR1, usr1_handler);
   signal(SIGUSR2, usr2_handler);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm_int
//
// Purpose-
//       Convert parameter to integer, handling error cases
//
// Implementation note-
//       optarg: The argument string
//       opt_index: The argument index
//
//----------------------------------------------------------------------------
static int                          // The integer value
   parm_int( void )                 // Extract and verify integer value
{
   errno= 0; int value= pub::utility::atoi(optarg);
   if( errno ) {
     opt_help= true;
     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n", OPTS[opt_index].name, optarg);
     else if( *optarg == '\0' )
       fprintf(stderr, "--%s, no value specified\n", OPTS[opt_index].name);
     else
       fprintf(stderr, "--%s, format error: '%s'\n", OPTS[opt_index].name, optarg);
   }

   return value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter description.
//
//----------------------------------------------------------------------------
static int                          // Return code (Always 1)
   info( void)                      // Parameter description
{
   fprintf(stderr, "Tdisp <options> parameter ...\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --hcdm\tHard Core Debug Mode\n"
                   "\n"
                   "  --ditem=n\tNumber of Dispatch::Items/iteration\n"
                   "  --dtask=n\tNumber of Dispatch::Tasks/Thread\n"
                   "  --first\tThread completion disable tracing\n"
                   "  --multi=n\tNumber of Threads (Parameter [1])\n"
                   "  --quick\tRun quick test\n"
                   "  --trace=n\tTrace table size\n"
                   "  --verbose{=n}\tVerbosity, default 0\n"
                   "\nParameters:\n"
                   "  [0] Iteration count, default(%d)\n"
                   "  [1] Thread count, default(%d)\n" // Also --multi=
                   , ITERATIONS
                   , TASK_COUNT
          );

   return 1; // Always
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static int                          // Return code (0 if OK)
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 rc= 0;       // Resultant, default OK

   //-------------------------------------------------------------------------
   // Parameter analysis
   //-------------------------------------------------------------------------
   int C;                           // The option character
   while( (C= getopt_long(argc, argv, OSTR, OPTS, &opt_index)) != -1 )
   {
     switch( C )
     {
       case 0:
       {{{{
         switch( opt_index )
         {
           case OPT_HELP:           // These options set by getopt
           case OPT_HCDM:
           case OPT_FIRST:
             break;

           case OPT_DITEM:
             opt_ditem= parm_int();
             break;

           case OPT_DTASK:
             opt_dtask= parm_int();
             break;

           case OPT_MULTI:
             opt_multi= parm_int();
             break;

           case OPT_QUICK:
             opt_iterations= 1024;
             opt_multi= 2;
             break;

           case OPT_TRACE:
             opt_trace= parm_int();
             if( errno == 0 ) {     // If valid
               if( opt_trace < pub::Trace::TABLE_SIZE_MIN ) {
                 fprintf(stderr, "--trace(%u) set to MINIMUM(%u)\n"
                               , opt_trace, pub::Trace::TABLE_SIZE_MIN);
                 opt_trace= pub::Trace::TABLE_SIZE_MIN;
               } else if( opt_trace > pub::Trace::TABLE_SIZE_MAX ) {
                 fprintf(stderr, "--trace(%u) set to MAXIMUM(%u)\n"
                               , opt_trace, pub::Trace::TABLE_SIZE_MAX);
                 opt_trace= pub::Trace::TABLE_SIZE_MAX;
               }
             }
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= parm_int();
             break;

           default:
             opt_help= true;
             fprintf(stderr, "%4d Unexpected opt_index(%d) '%s'\n", __LINE__
                           , opt_index, optarg);
             break;
         }
         break;
       }}}}

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Option requires an argument '%s'.\n", __LINE__
                         , argv[optind-1]);
         else
           fprintf(stderr, "%4d Option requires an argument '-%c'.\n", __LINE__
                         , optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Unknown option '%s'.\n", __LINE__
                         , argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "%4d Unknown option '-%c'.\n", __LINE__, optopt);
         else
           fprintf(stderr, "%4d Unknown option character '0x%x'.\n", __LINE__
                         , (optopt & 0x00ff) );
         break;

       default:
         fprintf(stderr, "%4d ShouldNotOccur ('%c',0x%x).\n", __LINE__
                       , C, (C & 0x00ff) );
         break;
     }
   }

   // Handle positional options
   int x= 0;                        // Current relative index
   for(int i= optind; i<argc; i++) {
     errno= 0; size_t size= pub::utility::atol(argv[i]);
     if( errno ) {
       opt_help= true;
       printf("Argument[%d] '%s': Invalid value\n", i, argv[i]);
       rc= errno;
     }

     switch(x) {
       case 0:
         opt_iterations= size;
         break;

       case 1:
         opt_multi= (int)size;
         break;

       default:
         errno= EINVAL;             // Invalid argument
         printf("Argument[%d] '%s': Unexpected\n", i, argv[i]);
         rc= errno;
     }

     x++;
   }

   // Return sequence
   if( opt_help )                   // If --help or error detected
     rc= info();                    // (Always returns non-zero)

   return rc;
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
   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   int rc= parm(argc, argv);        // Argument analysis
   if( rc ) return rc;              // Return if invalid

   rc= init(argc, argv);            // Initialize
   if( rc ) return rc;              // Return if invalid

   // Initialization complete
   debugf("Tdisp.cpp: %s %s%s Iterations(%'zd) Threads(%u)\n"
          , __DATE__, __TIME__, opt_hcdm ? " HCDM" : ""
          , opt_iterations, opt_multi);

   if( opt_verbose >= 0 ) {         // If verbose, display --options
     #define TF pub::utility::to_ascii // TF: True or False

     debugf("--first(%s) --verbose(%d) --trace(%'u,0x%.8x)\n"
            , TF(opt_first), opt_verbose, opt_trace, opt_trace);
     debugf("--ditem(%'u) --dtask(%'u)\n"
            , opt_ditem, opt_dtask);

     #undef TF
   }

   //-------------------------------------------------------------------------
   // Mainline code: Dispatcher stress test
   //-------------------------------------------------------------------------
   rc= 2;                           // Default, exception return code
   TRY_CATCH(
     if( HCDM ) debugf("\n");
     if( false ) {                  // If true, tasks do nothing
       debugf("%4d HCDM.c == TRACE DISABLED ==\n", __LINE__);
       Trace::trace->deactivate();
     }

     // Initialize (Trace area, Thread array)
     if( HCDM ) debugh("%4d HCDM.c\n", __LINE__);
     Main::init();

     // Wait for all threads to finish starting (for synchronized start)
     if( HCDM ) debugh("%4d HCDM.c\n", __LINE__);
     Main::wait(__LINE__);

     // Run the test
     if( HCDM ) debugh("%4d HCDM.c\n", __LINE__);
     Main::post(__LINE__);          // Synchronized start
     if( HCDM ) debugh("%4d HCDM.c\n", __LINE__);

     elapsed= epoch_nano();         // (Global) elapsed time
     Main::wait(__LINE__);          // Wait for all Thread completions
     elapsed= epoch_nano() - elapsed;

     Main::post(__LINE__);          // Allow Threads to complete

     // Statistics display
     if( HCDM ) debugh("%4d HCDM.c\n", __LINE__);
     Main::stats();                 // Statistics

     // Termination cleanup
     if( HCDM ) debugh("%4d HCDM.c\n", __LINE__);
     Main::term();

     rc= 0;                         // Test sucessful
   )

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   term();

   return rc;
}
