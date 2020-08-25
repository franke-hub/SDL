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
//       TestLock.cpp
//
// Purpose-
//       Test Lock.h
//
// Last change date-
//       2020/08/23
//
//----------------------------------------------------------------------------
#include "pub/Lock.h"               // The test object

#include <mutex>                    // For std::lock_guard
#include <ctype.h>                  // For isprint()
#include <errno.h>                  // For errno
#include <fcntl.h>                  // For O_* constants
#include <getopt.h>                 // For getopt_long()
#include <limits.h>                 // For INT_MAX, INT_MIN
#include <spawn.h>                  // For posix_spawn functions
#include <stdio.h>                  // For printf
//nclude <stdlib.h>                 // For various
#include <string.h>                 // For strerror
#include <unistd.h>                 // For getpid, ...
#include <sys/stat.h>               // For S_* constants and macros
#include <sys/wait.h>               // For waitpid

#include <pub/Debug.h>              // For debugging functions
#include <pub/Exception.h>          // For pub::Exception, std::exception
#include <pub/Thread.h>             // For pub::Thread::sleep
using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging functions

//----------------------------------------------------------------------------
// Compile-time options
//----------------------------------------------------------------------------
enum // Compile time-options
{  HCDM= false                      // Hard Core Debug Mode?
}; // Compile time-options

static const char*     OPT_LOCK= "/TestLock.cpp"; // Our lock name

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
static int             opt_hcdm= false; // Hard Core Debug Mode
static int             opt_index;   // Option index

static int             opt_reset= false; // Reset shared storage
static int             opt_verbose= -1; // --verbose=n (Verbosity)

static const char*     OSTR= ":";   // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",     no_argument,       &opt_help,    true}
,  {"hcdm",     no_argument,       &opt_hcdm,    true}

,  {"reset",    no_argument,       &opt_reset,   true}
,  {"verbose",  optional_argument, &opt_verbose,  0}
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_HELP
,  OPT_HCDM

,  OPT_RESET
,  OPT_VERBOSE
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static pub::Lock*      pub_lock= nullptr; // Our named lock

//============================================================================
// Class: Main (Mainline code)
//----------------------------------------------------------------------------
class Main {                        // Mainline code
public: // Attributes --------------------------------------------------------
int                    argc;        // Argument count
char**                 argv;        // Argument array

// Constructor ---------------------------------------------------------------
   Main(int argc, char* argv[])
:  argc(argc), argv(argv)
{
// if( opt_verbose > 0 )            // (For reference)
//   debugh("[%6d] Main(%p)::Main()\n", getpid(), this);
// debugh("[%6d] %4d HCDM\n", getpid(), __LINE__); // (For reference)
   if( opt_verbose > 0 )
     debugh("[%6d] Main(%p)::Main\n", getpid(), this);
}

// Methods -------------------------------------------------------------------
        int  process( void );       // Spawned process operation
        int  reset( void );         // Reset shared storage objects
virtual int  run( void );           // The mainline code
}; // class Main =============================================================

//----------------------------------------------------------------------------
//
// Subroutine-
//       oops
//
// Purpose-
//       Return strerror(errno)
//
//----------------------------------------------------------------------------
static inline const char* oops( void ) { return strerror(errno); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static int                          // Return code (0 OK)
   init(                            // Initialize
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   //-------------------------------------------------------------------------
   // Initialize/activate debugging trace
   Debug* debug= Debug::get();      // (To set file mode before use)
   debug->set_file_mode("ab");      // (Append so second PID doesn't truncate)
   debug->set_head(Debug::HEAD_TIME); // Include heading in time

   if( HCDM ) opt_hcdm= true;       // If HCDM compile-time, force opt_hcdm
   if( opt_hcdm ) {                 // If --hcdm option specified
     debug->set_mode(Debug::MODE_INTENSIVE); // Hard Core Debug Mode
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       terminate
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{
   if( pub_lock ) {
     delete pub_lock;
     pub_lock= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       to_integer
//
// Purpose-
//       Convert string to integer, handling error cases
//
// Implementation note-
//       Leading or trailing blanks are NOT allowed.
//
//----------------------------------------------------------------------------
static int                          // The integer value
   to_integer(                      // Extract and verify integer value
     const char*       inp)         // From this string
{
   errno= 0;
   char* strend;                    // Ending character
   long value= strtol(inp, &strend, 0);
   if( strend == inp || *inp == ' ' || *strend != '\0' )
     errno= EINVAL;
   else if( value < INT_MIN || value > INT_MAX )
     errno= ERANGE;

   return value;
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
   int value= to_integer(optarg);
   if( errno ) {
     opt_help= true;
     if( errno == ERANGE )
       fprintf(stderr, "--%s, range error: '%s'\n", OPTS[opt_index].name, optarg);
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
   fprintf(stderr, "%s <options>\n"
                   "Options:\n"
                   "  --help\tThis help message\n"
                   "  --hcdm\tHard Core Debug Mode\n"

                   "  --reset\tReset shared storage\n"
                   "  --verbose{=n}\tVerbosity, default 0\n"
                   , __FILE__
          );

   return 1;
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
           case OPT_HELP:           // These options handled by getopt
           case OPT_HCDM:
           case OPT_RESET:
             break;

           case OPT_VERBOSE:
             if( optarg )
               opt_verbose= parm_int();
             break;

           default:
             fprintf(stderr, "%4d Unexpected opt_index(%d)\n", __LINE__,
                             opt_index);
             break;
         }
         break;
       }}}}

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Option requires an argument '%s'.\n", __LINE__,
                           argv[optind-1]);
         else
           fprintf(stderr, "%4d Option requires an argument '-%c'.\n", __LINE__,
                           optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "%4d Unknown option '%s'.\n", __LINE__,
                           argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "%4d Unknown option '-%c'.\n", __LINE__, optopt);
         else
           fprintf(stderr, "%4d Unknown option character '0x%x'.\n", __LINE__,
                           (optopt & 0x00ff));
         break;

       default:
         fprintf(stderr, "%4d ShouldNotOccur ('%c',0x%x).\n", __LINE__,
                         C, (C & 0x00ff));
         break;
     }
   }

   // Disallow positional parameters
   for(int i= optind; i < argc; i++ ) {
     opt_help= true;
     fprintf(stderr, "'%s' Positional parameter not supported\n", argv[i]);
   }

   // Return sequence
   int rc= 0;
   if( opt_help )
     rc= info();
   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Main::process
//
// Purpose-
//       Run the spawned process
//
//----------------------------------------------------------------------------
int                                 // Error count
   Main::process( void )            // Run the spawned process
{
   if( opt_verbose > 0 )
     debugh("[%6d] Main(%p)::process()\n", getpid(), this);

   //-------------------------------------------------------------------------
   // Create the lock
   pub_lock= new pub::Lock(OPT_LOCK); // Create the Lock

   //-------------------------------------------------------------------------
   // Test the lock
   {{{{
     pub::Thread::sleep(0.001);
     std::lock_guard<decltype(*pub_lock)> temp(*pub_lock);
     debugh("[%6d] 002\n", getpid());
     pub::Thread::sleep(0.25);
   }}}}

   {{{{
     pub::Thread::sleep(0.001);
     std::lock_guard<decltype(*pub_lock)> temp(*pub_lock);
     debugh("[%6d] 004\n", getpid());
     pub::Thread::sleep(0.25);
   }}}}

   {{{{
     pub::Thread::sleep(0.001);
     std::lock_guard<decltype(*pub_lock)> temp(*pub_lock);
     debugh("[%6d] 006\n", getpid());
     pub::Thread::sleep(0.25);
   }}}}

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Main::reset
//
// Purpose-
//       Reset the shared storage objects
//
//----------------------------------------------------------------------------
int                                 // Error count
   Main::reset( void )              // Reset the message queues
{
   int error_count= 0;

   int
   rc= pub::Lock::unlink(OPT_LOCK); // Remove the lock
   if( rc ) {
     errorf("%4d Lock::unlink(%s): %s\n", __LINE__, OPT_LOCK, oops());
     error_count++;
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Method-
//       Main::run
//
// Purpose-
//       The mainline code, with try/catch wrapper
//
//----------------------------------------------------------------------------
int                                 // Error count
   Main::run( void )                // Mainline code
{
   if( opt_verbose > 0 )
     debugh("[%6d] Main(%p)::run()\n", getpid(), this);

   //-------------------------------------------------------------------------
   // If required, delete the message queue
   if( opt_reset )
     return reset();

   //-------------------------------------------------------------------------
   // Create the lock name (test for uniqueness)
   int
   rc= pub::Lock::create(OPT_LOCK, O_CREAT | O_EXCL);
   if( rc ) {
     if( errno == EEXIST ) {        // If already open
       rc= process();               // We must be the spawned process
       term();
       exit(rc);
     }

     errorf("%4d Lock::create(%s): %s\n", __LINE__, OPT_LOCK, oops());
     return 1;
   }
   pub_lock= new pub::Lock(OPT_LOCK); // Create the Lock
   pub_lock->lock();                // Grab the Lock

   //-------------------------------------------------------------------------
   // Create the child process
   int pid;
   rc= posix_spawn(&pid, argv[0], nullptr, nullptr, argv, environ);
   if( rc ) {
     errorf("%4d posix_spawn(%s): %s\n", __LINE__, argv[0], oops());
     return 1;
   }
   if( opt_verbose > 1 )
     debugh("[%6d] spawned(%d)\n", getpid(), pid);

   //-------------------------------------------------------------------------
   // Test the lock
   debugh("[%6d] 001\n", getpid());
   pub::Thread::sleep(0.25);
   pub_lock->unlock();

   {{{{
     pub::Thread::sleep(0.001);
     std::lock_guard<decltype(*pub_lock)> temp(*pub_lock);
     debugh("[%6d] 003\n", getpid());
     pub::Thread::sleep(0.25);
   }}}}

   {{{{
     pub::Thread::sleep(0.001);
     std::lock_guard<decltype(*pub_lock)> temp(*pub_lock);
     debugh("[%6d] 005\n", getpid());
     pub::Thread::sleep(0.25);
   }}}}

   {{{{
     pub::Thread::sleep(0.001);
     std::lock_guard<decltype(*pub_lock)> temp(*pub_lock);
     debugh("[%6d] 007\n", getpid());
   }}}}

   //-------------------------------------------------------------------------
   // Wait for child process completion
   for(;;) {
     errno= 0;
     pid_t wait= waitpid(pid, &rc, 0);
     if( wait == pid )
       break;

     if( errno == EINTR )
       continue;

     errorf("%4d waitpid(%d): %s\n", __LINE__, pid, oops());
     break;
   }

   //-------------------------------------------------------------------------
   // And we're done
   return reset();                  // Clean up and exit
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

   if( opt_verbose >= 0 ) {
     debugf("[%6d] %s: %s %s\n", getpid(), __FILE__, __DATE__, __TIME__);
     debugf("[%6d] --hcdm(%d) --reset(%d) --verbose(%d)\n"
            , getpid(), opt_hcdm, opt_reset, opt_verbose);
   }

   //-------------------------------------------------------------------------
   // Operate
   //-------------------------------------------------------------------------
   Main main(argc, argv);
   rc= main.run();

   //-------------------------------------------------------------------------
   // Terminate
   //-------------------------------------------------------------------------
   debugf("%d Error%s\n", rc, rc == 1 ? "" : "s");
   term();                          // Termination cleanup

   if( rc )
     rc= 1;
   return rc;
}
