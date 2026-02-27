//----------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Test_pthread.cpp
//
// Purpose-
//       Test: pthread stress test.
//
// Last change date-
//       2026/02/26
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic
#include <exception>                // For std::exception
#include <thread>                   // For std::this_thread::sleep_for
#include <cinttypes>                // For integer types
#include <clocale>                  // For setlocale
#include <ctime>                    // For time, localtime

#include <pthread.h>                // For pthread, tested
#include <signal.h>                 // For signal controls

#include <pub/TEST.H>               // For test functions and macros
#include <pub/Clock.h>              // For pub::Clock::now
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Event.h>              // For pub::Event
#include <pub/Latch.h>              // For pub::Latch
#include <pub/Semaphore.h>          // For pub::Semaphore
#include <pub/Trace.h>              // For pub::Trace
#include "pub/utility.h"            // For pub::utility::to_void
#include "pub/utility.i"            // For pub::utility conversion subroutines
#include "pub/Wrapper.h"            // For class Wrapper
#include "Test_pthread.h"           // For local classes

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::Event;
using PUB::Latch;
using PUB::Semaphore;
using PUB::Wrapper;
using PUB::utility::to_void;

using std::atomic_size_t;

//----------------------------------------------------------------------------
// Typedefs, etc
//----------------------------------------------------------------------------
// Using PUB::Clock::now
static inline double now( void ) { return PUB::Clock::now(); }

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
static constexpr double OPT_RUNTIME= 10.0; // Default runtime

enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  OPT_LIMIT= 8192                  // Default maximum number of Tasks
,  OPT_TASKS= 32                    // Default number of Tasks

,  USE_ICHECK= true                 // Use self-checking?
,  USE_ITRACE= true                 // Use internal trace?
,  USE_XTRACE= false                // Use extended internal trace?
}; // enum

#define USE_DEBUG0 false            // Use DEBUG[0]? [[Latch v/ atomic ops]]
#define USE_TLTASK false            // Use tl_task field?

//----------------------------------------------------------------------------
// Forward method references
//----------------------------------------------------------------------------
static void sig_handler(int);       // The signal handler

//----------------------------------------------------------------------------
// Statistics
//----------------------------------------------------------------------------
static constexpr size_t B= size_t(0x0001'0000'0000'0000); // A big number

static atomic_size_t   created_tasks(0); // Number of created Tasks
static atomic_size_t   destroy_tasks(0); // Number of destroyed Tasks
static atomic_size_t   running_tasks(0); // Number of running Tasks
static atomic_size_t   starting_tasks(0); // Number of starting Tasks
static atomic_size_t   max_running(0);  // Maximum number of running Tasks
static atomic_size_t   max_starting(0); // Maximum number of starting Tasks

#if USE_DEBUG0
static atomic_size_t   max_run_time(0); // Maximum run() time (nanoseconds)
static atomic_size_t   min_run_time(B); // Minimum run() time (nanoseconds)
static atomic_double   tot_run_time(0); // Total run() time (seconds)

static size_t          max_start_time=(0); // Maximum start() time (nanoseconds)
static size_t          min_start_time=(B); // Minimum start() time (nanoseconds)
static double          tot_start_time=(0); // Total start() time (seconds)
#else
static atomic_size_t   max_run_time(0); // Maximum run() time (nanoseconds)
static atomic_size_t   min_run_time(B); // Minimum run() time (nanoseconds)
static atomic_double   tot_run_time(0); // Total run() time (seconds)

static atomic_size_t   max_start_time(0); // Maximum start() time (nanoseconds)
static atomic_size_t   min_start_time(B); // Minimum start() time (nanoseconds)
static atomic_double   tot_start_time(0); // Total start() time (seconds)
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
#if USE_TLTASK
static __thread void*  tl_task= nullptr; // Thread-local Task*
#endif
static int             error_count= 0; // Error counter
static int             running= 0;  // Test running indicator

static double          then= 0.0;   // The test start time
static double          done= 0.0;   // The test completion time

static pthread_attr_t  attr_detached; // Detached attribute
static Semaphore       semaphore;   // MAX_TASKS Semaphore
static void*           table= nullptr; // The Trace table

// Signal handlers
typedef void           (*sig_handler_t)(int);
static sig_handler_t   sys1_handler= nullptr; // System SIGINT  signal handler
static sig_handler_t   sys2_handler= nullptr; // System SIGSEGV signal handler
static sig_handler_t   usr1_handler= nullptr; // System SIGUSR1 signal handler
static sig_handler_t   usr2_handler= nullptr; // System SIGUSR2 signal handler

// Extended options
static double          opt_runtime= OPT_RUNTIME; // --runtime
static int             opt_limit= OPT_LIMIT; // --limit
static int             opt_tasks= OPT_TASKS; // --tasks
static int             opt_trace= 0; // --trace
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"runtime",  required_argument, nullptr,        0} // --runtime=double
,  {"limit",    required_argument, nullptr,        0} // --limit=count
,  {"tasks",    required_argument, nullptr,        0} // --tasks=count
,  {"trace",    optional_argument, &opt_trace,  0x0040'0000} // --trace{=size}
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
const char* _month[]=
{  "Jan"
,  "Feb"
,  "Mar"
,  "Apr"
,  "May"
,  "Jun"
,  "Jul"
,  "Aug"
,  "Sep"
,  "Oct"
,  "Nov"
,  "Dec"
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       sno
//
// Purpose-
//       Should Not Occur condition occurred
//
//----------------------------------------------------------------------------
[[noreturn]]
static void
   sno(                             // Should Not Occur handler
     int               line,        // Failing line number
     const char*       op,          // Operation name
     int               rc)          // Failing return code
{
   debugh("%4d %s %d=%s ERR: %d:%s\n", line, __FILE__, rc, op
         , errno, strerror(errno));
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       diagnose
//
// Purpose-
//       Display diagnostic information
//
//----------------------------------------------------------------------------
static void
   diagnose(                        // Display diagnostics
     const char*       why= "")     // Caller identifier
{
   std::lock_guard<Debug> debug(*Debug::get());

   debugf("\n");
   debugf("%'16.3f diagnose(%s)\n",   now() - then, why);
   debugf("%'16zd Tasks created\n",   created_tasks.load());
   debugf("%'16zd Tasks destroyed\n", destroy_tasks.load());
   debugf("%'16zd Tasks running\n",   running_tasks.load());
   debugf("%'16zd Tasks starting\n",  starting_tasks.load());
   debugf("%'16zd max_running\n",  max_running.load());
   debugf("%'16zd max_starting\n", max_starting.load());
   debugf("%'16zd semaphore\n",    semaphore.get_count());

#if USE_DEBUG0
   debugf("\n");
   debugf("%'16zd max_run_time\n", max_run_time.load());
   debugf("%'16zd min_run_time\n", min_run_time.load());
   debugf("%'16.3f tot_run_time\n", tot_run_time.load());

   debugf("\n");
   debugf("%'16zd max_start_time\n", max_start_time);
   debugf("%'16zd min_start_time\n", min_start_time);
   debugf("%'16.3f tot_start_time\n", tot_start_time.load());
#else
   debugf("\n");
   debugf("%'16zd max_run_time\n", max_run_time.load());
   debugf("%'16zd min_run_time\n", min_run_time.load());
   debugf("%'16.3f tot_run_time\n", tot_run_time.load());

   debugf("\n");
   debugf("%'16zd max_start_time\n", max_start_time.load());
   debugf("%'16zd min_start_time\n", min_start_time.load());
   debugf("%'16.3f tot_start_time\n", tot_start_time.load());
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       inc_running
//
// Purpose-
//       Increment running_tasks counter
//
//----------------------------------------------------------------------------
static inline void
   inc_running( void )              // Increment the running_tasks count
{
   std::size_t new_running= ++running_tasks;
   std::size_t old_running= max_running.load();
   while( new_running > old_running )
     max_running.compare_exchange_weak(old_running, new_running);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       inc_starting
//
// Purpose-
//       Increment starting_tasks counter
//
//----------------------------------------------------------------------------
static inline void
   inc_starting( void )             // Increment the starting_tasks count
{
   std::size_t new_starting= ++starting_tasks;
   std::size_t old_starting= max_starting.load();
   while( new_starting > old_starting )
     max_starting.compare_exchange_weak(old_starting, new_starting);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       record_time
//
// Purpose-
//       Record maximum/minimum/total time interval (in nanoseconds)
//
//----------------------------------------------------------------------------
static inline void                  // (Might not be used)
   record_time(                     // Record maximum/minimum/total interval
     double            secs,        // The interval, in seconds
     size_t&           maxi,        // The maximum interval
     size_t&           mini,        // The minimum interval
     double&           total)       // The total interval
{
   if( USE_ICHECK && secs < 0 ) {
     debugf("secs(%.9f) < 0\n", secs);
     return;
   }

   const size_t time= secs * 1'000'000'000; // Interval (in nanoseconds)

   static Latch latch;
   std::lock_guard<Latch> lock(latch);

   if( time > maxi )
     maxi= time;

   if( time < mini )
     mini= time;

   total += secs;
}

static void
   record_time(                     // Record maximum/minimum/total interval
     double            secs,        // The interval, in seconds
     atomic_size_t&    maxi,        // The maximum interval
     atomic_size_t&    mini,        // The minimum interval
     atomic_double&    total)       // The total interval
{
   if( USE_ICHECK && secs < 0 ) {
     debugf("secs(%.9f) < 0\n", secs);
     return;
   }

   const size_t new_time= secs * 1'000'000'000; // Interval (in nanoseconds)
   const size_t old_maxi= maxi.load();
   const size_t old_mini= mini.load();
// Trace::trace(".REC", "secs", old_maxi, old_mini, new_time, new_time);

   size_t old_time= old_maxi;
   while( new_time > old_time )
     maxi.compare_exchange_weak(old_time, new_time);

   old_time= old_mini;
   while( new_time < old_time )
     mini.compare_exchange_weak(old_time, new_time);

   total += secs;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sig_handler
//
// Purpose-
//       Handle signals.
//
//----------------------------------------------------------------------------
static void
   sig_handler(                     // Handle signals
     int               id)          // The signal identifier
{
   static int recursion= 0;         // Signal recursion depth
   if( recursion ) {                // If signal recursion
     fprintf(stderr, "sig_handler(%d) recursion\n", id);
     fflush(stderr);
     exit(EXIT_FAILURE);
   }

   // Handle signal
   recursion++;                     // Disallow recursion
   const char* signame= "<<Unexpected>>";
   if( id == SIGINT ) signame= "SIGINT";
   else if( id == SIGSEGV ) signame= "SIGSEGV";
   else if( id == SIGUSR1 ) signame= "SIGUSR1";
   else if( id == SIGUSR2 ) signame= "SIGUSR2";
   errorf("sig_handler(%d) %s\n", id, signame);

   switch(id) {                     // Handle the signal
     case SIGINT:                   // (Console CTRL-C)
       running= false;
       exit(2);                     // Immediate exit
       break;

     case SIGSEGV:                  // (Program fault)
       Trace::trace(".BUG", __LINE__, signame);
       debug_set_mode(Debug::MODE_INTENSIVE);
       debug_backtrace();
       debugf("..terminated..\n");
       exit(EXIT_FAILURE);
       break;

     default:                       // (SIGUSR1 || SIGUSR2)
       Trace::trace(".SIG", __LINE__, signame);
       Debug::Mode mode= debug_get_mode();

       debug_set_mode(Debug::MODE_INTENSIVE);
       diagnose(signame);

       debug_set_mode(mode);
       break;
   }

   recursion--;
}

//----------------------------------------------------------------------------
//
// Struct-
//       Task
//
// Purpose-
//       The pthread_t container Task
//
//----------------------------------------------------------------------------
struct Task {
Event                  is_running;  // Task is running
Event                  is_started;  // Task is started

pthread_t              pthread{};   // The pthread_t
uint32_t               id= -1;      // The Task's identity

//----------------------------------------------------------------------------
// Task::constructor/destructor
   Task(                            // Constructor
     uint32_t          _identity)   // The Task's identity
:  id(_identity)
{  ++created_tasks;

   if( USE_ITRACE ) Trace::trace("=NEW", "TASK", this, id);
   if( opt_hcdm ) debugf("Task(%p)!\n", this);
}

   ~Task( void )                    // Destructor
{  ++destroy_tasks;

   if( USE_ITRACE ) Trace::trace("=DEL", "TASK", this, id);
#if USE_TLTASK
   tl_task= nullptr;
#endif
   if( opt_hcdm ) debugf("Task(%p)~\n", this);
}

//----------------------------------------------------------------------------
// Task::run: Run the pthread Task
Task*                               // (The next Task
   run(void)                        // Run the Task
{
   const char*    exit_code= "<RUN";
   const uint32_t id= this->id;
   const void*    that= to_void(this);

   if( USE_ITRACE ) Trace::trace("=TST", ">RUN", that, id);

   Task* next= nullptr;
   if( running ) {
     next= new Task(id);
   } else {
     if( opt_verbose ) {
       char buffer[32];
       *((int32_t*)buffer)= htobe32(id);
       buffer[4]= '\0';

       debugf("Task(%s) terminated\n", buffer);
       exit_code= "<XIT";
     }
   }

   // Task termination
   if( USE_ITRACE ) Trace::trace("=TST", "POST", that, id);
   semaphore.post();
   delete this;

   if( USE_ITRACE ) Trace::trace("=TST", exit_code, that, id);
   return next;
}

//----------------------------------------------------------------------------
// Task::start: Start the pthread
void
   start(void)                      // Start the pthread
{
   static atomic_size_t _starts(0); // Number of starts
   size_t starts= ++_starts;

   // Trace semaphore.wait once after running a while
   if( USE_ITRACE && starts == 40 ) Trace::trace("=TST", ">SEM", this);
   semaphore.wait();
   if( USE_ITRACE && starts == 40 ) Trace::trace("=TST", "<SEM", this);
   pthread_create(&pthread, &attr_detached, start_routine, this);

   if( USE_XTRACE ) Trace::trace("=TST", "SLOK", this);
   is_running.wait();
   is_started.post();
   if( USE_XTRACE ) Trace::trace("=TST", "SXIT", this);
}

//----------------------------------------------------------------------------
// Task::start_routine: Run the pthread
static void*
   start_routine(                   // Run the pthread
     void*             parm)        // With this parameter
{
   double then= now();              // Running timer

   Task* task= static_cast<Task*>(parm);
#if USE_TLTASK
   tl_task= task;
#endif
   int id= task->id;
   if( USE_ITRACE ) Trace::trace(".TST", ">THR", task, id);

   task->is_running.post();         // Startup sequencing
   task->is_started.wait();

   // Run the task
   inc_running();
   Task* next= task->run();
   --running_tasks;
   if( next ) {
     double then= now();
     inc_starting();
     next->start();
     --starting_tasks;
     record_time(now()-then, max_start_time, min_start_time, tot_start_time);
   }

   if( USE_ITRACE ) Trace::trace(".TST", "<THR", task, id);
   record_time(now()-then, max_run_time, min_run_time, tot_run_time);
   return nullptr;
}
}; // struct Task

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_timing
//
// Purpose-
//       Timing/stress test.
//
//----------------------------------------------------------------------------
static int
   test_timing( void )              // Timing test
{
   error_count= 0;                  // (No errors yet)

   if( opt_verbose ) {
     debugf("%'16.3f Runtime\n", opt_runtime);
     debugf("%'16d Tasks\n", opt_tasks);
   }

   //*************************************************************************
   // Run the timing test
   running= true;                   // Indicate running
   then= now();                     // Test start time
   if( USE_ITRACE ) Trace::trace(".STS", 0, "RUNNING=TRUE");
   if( opt_verbose > 1 )
     debugh("%'10.3f running= true\n", now() - then);

   // Initialize the Tasks
   // We can't start Tasks before the test is running since they will simply
   // terminate.
   for(int i= 0; i<opt_tasks; ++i) {
     char buffer[32];
     sprintf(buffer, "%.4d", i);
     int32_t id= *((int32_t*)buffer);

     Task* task= new Task(be32toh(id));
     task->start();
   }
   if( opt_verbose > 2 ) // (Distribution doesn't take much time)
     debugh("%'10.3f All Tasks started\n", now() - then);

   size_t ms= opt_runtime * 1000.0; // Time in milliseconds
   std::this_thread::sleep_for(std::chrono::milliseconds(ms));

   if( opt_verbose > 1 ) {          // Diagnostic while still running
     diagnose("while running==true");
   }

   running= false;                  // (But Tasks still need to complete)
   done= now();
   size_t operations= destroy_tasks.load(); // (Only count completed Tasks)

   if( USE_ITRACE ) Trace::trace(".STS", 0, "RUNNING=FALSE");
   if( opt_verbose > 1 )
     debugh("%'10.3f running= false\n", done - then);
   // Testing is complete
   //*************************************************************************

   // Report the results
   std::this_thread::sleep_for(std::chrono::milliseconds(1500));
   if( opt_verbose > 1 )
     diagnose("Test Complete");

   debugf("\n");
   double elapsed= done - then;
   double per_sec= double(operations)/elapsed;

   debugf("%'16.3f Elapsed\n", elapsed);
   debugf("%'16zd Operations\n", operations);
   debugf("%'16.0f Operations/second (Elapsed)\n", per_sec);

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
extern int
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   Wrapper  tc= opts;               // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_info([]()
   {
     // Options:, --help, --hcdm, and --verbose are displayed by Wrappper
     fprintf(stderr,
            "  --runtime\t=time In seconds (double)\n"
            "  --limit\t=count Maximum number of running Tasks\n"
            "  --tasks\t=count Number of Tasks\n"
            "  --trace\t{=size} Create internal trace file './trace.mem'\n"
            );
   });

   tc.on_init([tr](int, char**)
   {
     // Initialize internal data areas
     int rc= pthread_attr_init(&attr_detached);
     if( rc ) sno(__LINE__, "pthread_attr_init", rc);
     rc= pthread_attr_setdetachstate(&attr_detached, PTHREAD_CREATE_DETACHED);
     if( rc ) sno(__LINE__, "pthread_setdetachstate", rc);

     semaphore.reset(opt_limit);
     // Initialize signal handling
     debug_set_head(Debug::HEAD_THREAD | Debug::HEAD_TIME);
     if( opt_hcdm || opt_verbose > 1 )
       debug_set_mode(Debug::MODE_INTENSIVE);

     // Initialize signal handling
     if( opt_trace )
       table= tr->init_trace("./trace.mem", opt_trace);

     // Initialize signal handling
     sys1_handler= signal(SIGINT,  sig_handler);
     sys2_handler= signal(SIGSEGV, sig_handler);
     usr1_handler= signal(SIGUSR1, sig_handler);
     usr2_handler= signal(SIGUSR2, sig_handler);

     return 0;
   });

   tc.on_parm([tr](std::string P, const char* V)
   {
     if( P == "runtime" ) {
       opt_runtime= tr->ptod(V);
       if( opt_runtime < 1.0 ) {
         errorf("--runtime=%.3f invalid value\n", opt_runtime);
         errorf("  Valid range: %'d..%'.d\n", 1, 604'800);
         return 1;
       } else if( opt_runtime > 604'800 ) {
         errorf("--runtime=%.3f accepted, outside normal range\n"
               , opt_runtime);
         errorf("  Valid range: %'d..%'.d\n", 1, 604'800);
       }
     } else if( P == "limit" ) {
       opt_limit= tr->ptoi(V);
       if( opt_limit < 1 || opt_limit > 65535 ) {
         errorf("--limit=%d invalid value\n", opt_limit);
         errorf("  Valid range: %'d..%'.d\n", 1, 65535);
         return 1;
       }
     } else if( P == "tasks" ) {
       opt_tasks= tr->ptoi(V);
       if( opt_tasks < 1 || opt_tasks > 65535 ) {
         errorf("--tasks=%d invalid value\n", opt_tasks);
         errorf("  Valid range: %'d..%'.d\n", 1, 65535);
         return 1;
       }
     } else if( P == "trace" ) {
       if( V ) {
         opt_trace= tr->ptoi(V);
         if( opt_trace < 0x00010000 ) {
           errorf("--trace=0x%.8x (%d) invalid value\n", opt_trace, opt_trace);
           errorf("  Valid range: %'d..%'.d\n", 0x00010000, 0x7FFFFFFF);
           return 1;
         }
       }
     } else {
       fprintf(stderr, "Invalid option '%s'\n", s2c(P));
       return 1;
     }

     return 0;
   });

   tc.on_term([tr]()
   {
     // Restore system signal handlers
     if( sys1_handler ) signal(SIGINT,  sys1_handler);
     if( sys2_handler ) signal(SIGSEGV, sys2_handler);
     if( usr1_handler ) signal(SIGUSR1, usr1_handler);
     if( usr2_handler ) signal(SIGUSR2, usr2_handler);
     sys1_handler= sys2_handler= usr1_handler= usr2_handler= nullptr;

     // Terminate trace
     Trace::trace(".TST", __LINE__, "on_term");
     Trace::stop();

     if( table )
       tr->term_trace(table, opt_trace);

     // Cleanup internals
     int rc= pthread_attr_destroy(&attr_detached);
     if( rc )
       sno(__LINE__, "pthread_attr_init", rc);
   });

   tc.on_main([tr](int argc, char* argv[])
   {
     error_count= 0;

     if( optind < argc ) {
       debugf("Positional arguments are not allowed:\n");
       for(int i= optind; i<argc; ++i) {
         debugf("[%2d] '%s'\n", i, argv[i]);
       }
       return 1;
     }

     // Display the options
     if( opt_verbose > 1 ) {
       time_t gt= time(nullptr);
       struct tm* lt= localtime(&gt);

       debugf("Compiled: %s %s %s\n Started: %3s %2d %4d %.2d:%.2d:%.2d\n"
             , __DATE__, __TIME__, __FILE__
             , _month[lt->tm_mon], lt->tm_mday, lt->tm_year + 1900
             , lt->tm_hour, lt->tm_min, lt->tm_sec);

       // Compile-time options
       const char*
       option= "FALSE";
       #if USE_DEBUG0
         option= "TRUE";
       #endif
       debugf("%16s USE_DEBUG0 [Latch v. Atomic]\n", option);

       option= "FALSE";
       if( USE_ITRACE )
         option= "TRUE";
       debugf("%16s USE_ITRACE\n", option);

       // Parameter options
       debugf("%16d opt_hcdm\n", opt_hcdm);
       debugf("%16d opt_verbose\n", opt_verbose);
       debugf("%16d opt_limit\n", opt_limit);
       debugf("%16d opt_tasks\n", opt_tasks);
       debugf("%6s0x%.8x opt_trace (%'d)\n", "", opt_trace, opt_trace);
       debugf("\n");
     }

     try {
       error_count += test_timing();
     } catch(std::exception& x) {
       debugf("FAILED: Exception: exception(%s)\n", x.what());
       debug_backtrace();
       ++error_count;
     } catch(...) {
       debugf("FAILED: Exception: ...\n");
       debug_backtrace();
       ++error_count;
     }

     if( opt_hcdm || opt_verbose || error_count ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return int(error_count != 0);
   });

   //-----------------------------------------------------------------------
   // Run the test
   setlocale(LC_NUMERIC, "");       // Activates ' thousand separator
   opt_hcdm= HCDM;
   opt_verbose= VERBOSE;

   return tc.run(argc, argv);
}
