//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2023 Frank Eskesen.
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
//       2023/04/29
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
#include <pub/Wrapper.h>            // For class Wrapper

#define PUB _LIBPUB_NAMESPACE
using PUB::Debug;                   // For Debug object
using namespace PUB::debugging;     // For debugging functions
using PUB::Wrapper;                 // For PUB::Wrapper class

//----------------------------------------------------------------------------
// Compile-time options
//----------------------------------------------------------------------------
enum // Compile time-options
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // Compile time-options

static const char*     OPT_LOCK= "/TestLock.cpp"; // Our lock name

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_reset= false; // Reset shared storage?

static struct option   opts[]=      // The getopt_long longopts parameter
{  {"reset",    no_argument,       &opt_reset,   true}
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static PUB::Lock*      pub_lock= nullptr; // Our named lock

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
   if( opt_verbose > 1 )
     debugh("[%6d] Main(%p)::Main\n", getpid(), this);
}

// Methods -------------------------------------------------------------------
        int  reset( void );         // Reset shared storage objects
virtual int  run( void );           // The mainline code
        int  spawned( void );       // Spawned process operation
}; // class Main =============================================================

//----------------------------------------------------------------------------
//
// Subroutine-
//       geterror
//
// Purpose-
//       Return strerror(errno)
//
//----------------------------------------------------------------------------
static inline const char* geterror( void ) { return strerror(errno); }

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
   rc= PUB::Lock::unlink(OPT_LOCK); // Remove the lock
   if( rc ) {
     errorf("%4d Lock::unlink(%s): %s\n", __LINE__, OPT_LOCK, geterror());
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
   if( opt_verbose > 1 )
     debugh("[%6d] Main(%p)::run()\n", getpid(), this);

   //-------------------------------------------------------------------------
   // If required, delete the message queue
   if( opt_reset )
     return reset();

   //-------------------------------------------------------------------------
   // Create the lock name (test for uniqueness)
   int
   rc= PUB::Lock::create(OPT_LOCK, O_CREAT | O_EXCL);
   if( rc ) {
     if( errno == EEXIST )          // If already open
       return spawned();            // We must be the spawned process

     errorf("%4d Lock::create(%s): %s\n", __LINE__, OPT_LOCK, geterror());
     return 1;
   }
   pub_lock= new PUB::Lock(OPT_LOCK); // Create the Lock
   pub_lock->lock();                // Grab the Lock

   //-------------------------------------------------------------------------
   // Create the child process
   int pid;
   rc= posix_spawn(&pid, argv[0], nullptr, nullptr, argv, environ);
   if( rc ) {
     errorf("%4d posix_spawn(%s): %s\n", __LINE__, argv[0], geterror());
     return 1;
   }
   if( opt_verbose > 1 )
     debugh("[%6d] spawned(%d)\n", getpid(), pid);

   //-------------------------------------------------------------------------
   // Test the lock
   if( opt_verbose )
     debugh("[%6d] 001\n", getpid());
   PUB::Thread::sleep(0.25);
   pub_lock->unlock();

   {{{{
     PUB::Thread::sleep(0.001);
     std::lock_guard<decltype(*pub_lock)> temp(*pub_lock);
     if( opt_verbose )
       debugh("[%6d] 003\n", getpid());
     PUB::Thread::sleep(0.25);
   }}}}

   {{{{
     PUB::Thread::sleep(0.001);
     std::lock_guard<decltype(*pub_lock)> temp(*pub_lock);
     if( opt_verbose )
       debugh("[%6d] 005\n", getpid());
     PUB::Thread::sleep(0.25);
   }}}}

   {{{{
     PUB::Thread::sleep(0.001);
     std::lock_guard<decltype(*pub_lock)> temp(*pub_lock);
     if( opt_verbose )
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

     errorf("%4d waitpid(%d): %s\n", __LINE__, pid, geterror());
     break;
   }

   //-------------------------------------------------------------------------
   // And we're done
   return reset();                  // Clean up and exit
}

//----------------------------------------------------------------------------
//
// Method-
//       Main::spawned
//
// Purpose-
//       Run the spawned process
//
//----------------------------------------------------------------------------
int                                 // Error count
   Main::spawned( void )            // Run the spawned process
{
   if( opt_verbose > 1 )
     debugh("[%6d] Main(%p)::spawned()\n", getpid(), this);

   //-------------------------------------------------------------------------
   // Create the lock
   pub_lock= new PUB::Lock(OPT_LOCK); // Create the Lock

   //-------------------------------------------------------------------------
   // Test the lock
   {{{{
     PUB::Thread::sleep(0.001);
     std::lock_guard<decltype(*pub_lock)> temp(*pub_lock);
     if( opt_verbose )
       debugh("[%6d] 002\n", getpid());
     PUB::Thread::sleep(0.25);
   }}}}

   {{{{
     PUB::Thread::sleep(0.001);
     std::lock_guard<decltype(*pub_lock)> temp(*pub_lock);
     if( opt_verbose )
       debugh("[%6d] 004\n", getpid());
     PUB::Thread::sleep(0.25);
   }}}}

   {{{{
     PUB::Thread::sleep(0.001);
     std::lock_guard<decltype(*pub_lock)> temp(*pub_lock);
     if( opt_verbose )
       debugh("[%6d] 006\n", getpid());
     PUB::Thread::sleep(0.25);
   }}}}

   return 0;
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
   Wrapper  tc= opts;               // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_info([]()
   {
     fprintf(stderr, "  --reset\tReset shared storage\n");
   });

   tc.on_init([](int argc, char* argv[])
   {
     // Disallow positional parameters
     int rc= 0;
     for(int argx= optind; argx < argc; argx++ ) {
       rc= 1;
       fprintf(stderr, "'%s' Positional parameter not supported\n", argv[argx]);
     }

     //-----------------------------------------------------------------------
     // Initialize/activate debugging trace
     Debug* debug= Debug::get();      // (To set file mode before use)
     debug->set_file_mode("ab");      // (Append so second PID doesn't truncate)
     debug->set_head(Debug::HEAD_TIME); // Include heading in time

     if( HCDM ) opt_hcdm= true;
     if( opt_hcdm )
       debug->set_mode(Debug::MODE_INTENSIVE); // Hard Core Debug Mode
     if( VERBOSE > opt_verbose )
       opt_verbose= VERBOSE;

     return rc;
   });

   tc.on_term([]()
   {
     if( pub_lock ) {
       delete pub_lock;
       pub_lock= nullptr;
     }
   });

   tc.on_main([tr](int argc, char* argv[])
   {
     if( opt_verbose > 1 ) {
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);
       debugf("[%6d] %s: %s %s\n", getpid(), __FILE__, __DATE__, __TIME__);
       debugf("[%6d] --hcdm(%d) --reset(%d) --verbose(%d)\n"
             , getpid(), opt_hcdm, opt_reset, opt_verbose);
     }

     Main main(argc, argv);
     int error_count= main.run();

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}
