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
//       TimeDisp.cpp
//
// Purpose-
//       Dispatcher timing test.
//
// Last change date-
//       2026/03/17
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic
#include <exception>                // For std::exception
#include <new>                      // For std::bad_alloc, operator new
#include <cinttypes>                // For integer types
#include <clocale>                  // For setlocale
#include <cstdlib>                  // For atexit
#include <ctime>                    // For time, localtime

#include <endian.h>                 // For htobe64
#include <sys/signal.h>             // For signal, ...

#include <pub/TEST.H>               // For test functions and macros
#include <pub/Clock.h>              // For pub::Clock::now
#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/Dispatch.h"           // For pub::dispatch objects
#include <pub/Random.h>             // For pub::Random
#include "pub/System.h"             // For pub::System::debug
#include "pub/Thread.h"             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace
#include "pub/utility.i"            // For pub::utility conversion subroutines
#include "pub/Wrapper.h"            // For class Wrapper
#include "pub/Worker.h"             // For pub::WorkerPool methods

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::Wrapper;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
static constexpr double OPT_RUNTIME= 10.0; // Default runtime

enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  OPT_ITEMS= 512                   // Default number of Items
,  OPT_TASKS= 8                     // Default number of Tasks

// These compile-time options are independent of the --trace parameter option.
// When setting USE_QTRACE==true always set USE_ITRACE==true
,  USE_DETAIL= false                // Use detailed debugging?
,  USE_ICHECK= false                // Enable internal checking?
,  USE_ITRACE= false                // Enable internal tracing?
,  USE_QTRACE= false                // Enable method queue tracing?
}; // enum

//----------------------------------------------------------------------------
// Forward class references
//----------------------------------------------------------------------------
class TimerItem;                    // For TimerItem*
class TimerTask;                    // For TimerTask*

//----------------------------------------------------------------------------
// Forward method references
//----------------------------------------------------------------------------
static void sig_handler(int);       // The signal handler

static void
   queue(                           // Enqueue TimerItem onto TimerTask
     int               line,        // Caller's line number
     TimerTask*        this_task,   // The running task, nullptr if TimerThread
     TimerTask*        next_task,   // Next TimerTask
     TimerItem*        next_item);  // Next TimerItem

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             error_count= 0; // Error counter
static int             running= 0;  // Test running indicator

static double          then= 0.0;   // The test start time
static double          done= 0.0;   // The test completion time

static void*           table= nullptr; // The Trace table
static TimerItem**     item_array= nullptr; // The Timer Item array
static TimerTask**     task_array= nullptr; // The Timer Task array

// Signal handlers
typedef void           (*sig_handler_t)(int);
static sig_handler_t   sys1_handler= nullptr; // System SIGINT  signal handler
static sig_handler_t   sys2_handler= nullptr; // System SIGSEGV signal handler
static sig_handler_t   usr1_handler= nullptr; // System SIGUSR1 signal handler
static sig_handler_t   usr2_handler= nullptr; // System SIGUSR2 signal handler

// Extended options
static double          opt_runtime= OPT_RUNTIME; // --runtime
static int             opt_items= OPT_ITEMS; // --items
static int             opt_retest= false; // --retest
static int             opt_size= WorkerPool::get_size(); // --size
static int             opt_tasks= OPT_TASKS; // --tasks
static int             opt_trace= 0; // --trace
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"runtime",  required_argument, nullptr,        0} // --runtime=double
,  {"retest",   no_argument,       &opt_retest, true} // --retest
,  {"items",    required_argument, nullptr,        0} // --items=count
,  {"size",     required_argument, nullptr,        0} // --size=count
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
// Struct-
//       Record
//
// Purpose-
//       Trace record
//
// Implementation notes-
//       Invoking trace with USE_ITRACE==false is optimized out.
//
//----------------------------------------------------------------------------
struct Record : public PUB::Trace::Record {
void
   operator delete(void*)           // Deallocate a Record
{  }                                // (Never needed)

void*
   operator new(size_t size) throw() // Allocate a new Record
{
   if( USE_ITRACE && table ) {      // If tracing active
     return PUB::Trace::table->allocate(size);
   }

   return nullptr;
}

   Record(                          // Initialize the trace entry
     const char*       ident,       // The trace identiier (4 characters)
     uint32_t          line,        // The line number
     const char*       op,          // Operation (max char[8])
     uint16_t          this_task_ix, // Associated Task identifier
     uint16_t          this_item_ix) // Associated Item identifier
{
   memset(value, ' ', 8);           // Blank fill value operation
   memcpy(value, op, strlen(op));   // Set operation type

   uint64_t task_info= (uint64_t(this_task_ix) << 48)
                     | (uint64_t(this_item_ix) << 32);
   uint64_t* ptr_item= (uint64_t*)(value+8);
   *ptr_item= htobe64(task_info);

   trace(ident, line);
}

   Record(                          // Initialize the trace entry
     const char*       ident,       // The trace identiier (4 characters)
     uint32_t          line,        // The line number
     const char*       op,          // Operation (max char[8])
     uint16_t          this_task_ix, // Associated Task identifier
     uint16_t          this_item_ix, // Associated Item identifier
     uint16_t          next_task_ix, // Next Task identifier
     uint16_t          next_item_ix) // Next Item identifier

{
   memset(value, ' ', 8);           // Blank fill value operation
   memcpy(value, op, strlen(op));   // Set operation type

   uint64_t task_info= (uint64_t(this_task_ix) << 48)
                     | (uint64_t(this_item_ix) << 32)
                     | (uint64_t(next_task_ix) << 16)
                     | (uint64_t(next_item_ix));
   uint64_t* ptr_item= (uint64_t*)(value+8);
   *ptr_item= htobe64(task_info);

   trace(ident, line);
}
};

static inline void
   trace(                           // Allocate and initialize trace Record
     const char*       ident,       // The trace identiier (4 characters)
     uint32_t          line,        // The line number
     const char*       op,          // Operation (max char[8])
     uint16_t          this_task_ix, // Current Task identifier
     uint16_t          this_item_ix) // Current Item identifier
{  if( USE_ITRACE ) {               // If tracing active
     Record* record= new Record(ident, line, op, this_task_ix, this_item_ix);
     (void)record;
   }
}

static inline void
   trace(                           // Allocate and initialize trace Record
     const char*       ident,       // The trace identiier (4 characters)
     uint32_t          line,        // The line number
     const char*       op,          // Operation (max char[8])
     uint16_t          this_task_ix, // Current Task identifier
     uint16_t          this_item_ix, // Current Item identifier
     uint16_t          next_task_ix, // Next Task identifier
     uint16_t          next_item_ix) // Next Item identifier
{  if( USE_ITRACE ) {               // If tracing active
     Record* record= new Record(ident, line, op, this_task_ix, this_item_ix
                               , next_task_ix, next_item_ix);
     (void)record;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       at_exit
//
// Purpose-
//       Handle atexit notification
//
//----------------------------------------------------------------------------
static void
   at_exit( void )
{
   if( opt_verbose > 2 )
     printf("at_exit\n");

   Trace::trace(".TST", __LINE__, "at_exit");
   Trace::stop();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       bad_alloc
//
// Purpose-
//       Throw bad_alloc iff allocation fails
//
//----------------------------------------------------------------------------
static inline void
   bad_alloc(void* alloc)
{  if( alloc == nullptr ) throw std::bad_alloc(); }

//----------------------------------------------------------------------------
//
// Class-
//       TimerItem
//
// Purpose-
//       The test Item
//
//----------------------------------------------------------------------------
class TimerItem : public PUB::dispatch::Item {
public:
PUB::dispatch::Wait    wait;        // The Item's Wait object
size_t*                task_count;  // COUNTER: Number of times handled/Task

uint16_t               identity= -1;  // The Item's identity
uint16_t               next_task= -1; // The next task to run

public:
   TimerItem(                       // The Timer Item
     uint16_t          _identity)   // The Timer Item's identity
:  PUB::dispatch::Item(&wait)
,  task_count((size_t*)malloc(sizeof(size_t) * opt_tasks)), identity(_identity)
{
   bad_alloc(task_count);           // Verify storage allocated
   memset(task_count, 0, sizeof(size_t) * opt_tasks);
}

   ~TimerItem( void )
{  free(task_count); }

virtual void
   debug(                           // Debugging display
     const char*       info= "") const // Header information
{
   debugf("TimerItem(%p).debug(%s) id(%3d) next_task(%3d)\n", this
         , info, identity, (int16_t)next_task);

   if( true ) {
     for(int i= 0; i<opt_tasks; ++i) {
       debugf("..[%3d] %'16zd\n", i, task_count[i]);
     }
   }
}
}; // class TimerItem

//----------------------------------------------------------------------------
//
// Class-
//       TimerTask
//
// Purpose-
//       The test Task
//
//----------------------------------------------------------------------------
class TimerTask : public PUB::dispatch::Task {
public:
PUB::Random            random;      // Random number generator
size_t*                item_count;  // COUNTER: Number of times handled/Item

uint16_t               identity= -1;  // The Task's identity
uint16_t               last_item= -1; // The last Item processed

   TimerTask(                       // The Timer Task
     uint16_t          _identity)   // The Timer Task's identity
:  PUB::dispatch::Task(), random()
,  item_count((size_t*)malloc(sizeof(size_t) * opt_items)), identity(_identity)
{
   bad_alloc(item_count);           // Verify storage allocated
   memset(item_count, 0, sizeof(size_t) * opt_items);

   if( true )
     random.set_seed(_identity + 1); // (Seed 0 doesn't randomize)
   else
     random.randomize();
}

   ~TimerTask( void )
{  free(item_count); }

//----------------------------------------------------------------------------
// TimerTask::debug: Write debugging message
virtual void
   debug(                           // Debugging display
     const char*       info= "") const // Header information
{
   debugf("TimerTask(%s).debug(%s) id(%3d) last_item(%3d)\n", s2c(v2s(this))
         , info, identity, last_item);
   PUB::dispatch::Task::debug("TimerTask");

   for(int i= 0; i<opt_items; ++i) {
     debugf("..[%3d] %'16zd\n", i, item_count[i]);
   }
}

//----------------------------------------------------------------------------
// TimerTask::work: Process work Item
virtual void
   work(                            // Process
     PUB::dispatch::Item* item)     // This work Item
{
   TimerItem* timer_item= (TimerItem*)item; // (We only get TimerItems)

   trace(".TST", __LINE__, "WORK", identity, timer_item->identity);

   // Verify Item enqueued for this Task; Record last Item processed
   if( USE_ICHECK ) {
     error_count += VERIFY(timer_item->next_task == identity);
     last_item= timer_item->identity;
   }

   // On test completion, post Item
   if( !running ) {                 // If test completed
     if( opt_hcdm )
       tracef("%4d Task[%3d] Item[%3d] POST\n", __LINE__
             , identity, timer_item->identity);
     trace(".TST", __LINE__, "POST", identity, timer_item->identity);

     if( USE_ICHECK )
       timer_item->next_task= -identity; // Indicate POSTED (by this Task)

     item->post();                  // Post the Item
     return;
   }

   // Count this work item
   ++timer_item->task_count[identity]; // The Item saw us
   ++item_count[timer_item->identity]; // We saw the Item

   // Enqueue on next Task
   uint32_t next= random.modulus((uint32_t)opt_tasks);
   queue(__LINE__, this, task_array[next], timer_item);
}
}; // struct TimerTask

//----------------------------------------------------------------------------
//
// Subroutine-
//       BINGO
//
// Purpose-
//       DEBUG everything we can.
//
//----------------------------------------------------------------------------
static void
   BINGO( void )                    // Debug everything
{
   System::debug("BINGO", USE_DETAIL);

   if( USE_DETAIL ) {
     debugf("\n");
     for(int task_ix= 0; task_ix < opt_tasks; ++task_ix)
       task_array[task_ix]->debug("TASKS");

     debugf("\n");
     for(int item_ix= 0; item_ix < opt_items; ++item_ix)
       item_array[item_ix]->debug("ITEMS");
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       queue
//
// Purpose-
//       Enqueue TimerItem onto TimerTask
//
//----------------------------------------------------------------------------
static void
   queue(                           // Enqueue TimerItem onto TimerTask
     int               line,        // Caller's line number
     TimerTask*        this_task,   // The running task, nullptr if TimerThread
     TimerTask*        next_task,   // Next TimerTask
     TimerItem*        next_item)   // Next TimerItem
{
   if( USE_QTRACE ) {
     int this_task_ix= -1;
     int this_item_ix= -1;
     if( this_task ) {
       this_task_ix= this_task->identity;
       this_item_ix= this_task->last_item;
     }
     int next_task_ix= next_task->identity;
     int next_item_ix= next_item->identity;

     const char* status= "QUEUE-R"; // (Usually) already running
     if( !next_task->is_running() )
       status= "QUEUE-S";           // (Usually) requires start
     trace(".TST", line, status, this_task_ix, this_item_ix
                 , next_task_ix, next_item_ix);
   }

   if( USE_ICHECK )
     next_item->next_task= next_task->identity;

   next_task->enqueue(next_item);
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
   const char* signame= "SIG????";
   if( id == SIGINT ) signame= "SIGINT";
   else if( id == SIGSEGV ) signame= "SIGSEGV";
   else if( id == SIGUSR1 ) signame= "SIGUSR1";
   else if( id == SIGUSR2 ) signame= "SIGUSR2";

   static int recursion= 0;         // Signal recursion depth
   if( recursion ) {                // If signal recursion
     Trace::trace(".TST", ">SIG", id, signame);
     fprintf(stderr, "sig_handler(%d:%s) recursion\n", id, signame);
     fflush(stderr);
     exit(EXIT_FAILURE);
   }

   // Handle signal
   recursion= 1;                    // Disallow recursion
   errorf("sig_handler(%d:%s)\n", id, signame);
   Trace::trace(".TST", "=SIG", id, signame);

   switch(id) {                     // Handle the signal
     case SIGINT:                   // (Console CTRL-C)
       BINGO();                     // Debug everything
       exit(2);                     // Immediate exit
       break;

     case SIGSEGV:                  // (Program fault)
       debug_set_mode(Debug::MODE_INTENSIVE);
       debug_backtrace();
       debugf("..terminated..\n");
       exit(EXIT_FAILURE);
       break;

     default:                       // (SIGUSR1 || SIGUSR2)
       Debug::Mode mode= debug_get_mode();

       debug_set_mode(Debug::MODE_INTENSIVE);
       System::debug(signame);

       debug_set_mode(mode);
       break;                       // (No configured action)
   }
   recursion= 0;
}

//----------------------------------------------------------------------------
//
// Class-
//       TimerThread
//
// Purpose-
//       Run the timing test
//
//----------------------------------------------------------------------------
class TimerThread : public PUB::Thread {
public:
   TimerThread( void ) = default;
   ~TimerThread( void ) = default;

//----------------------------------------------------------------------------
virtual void
   run( void )
{
   // Start the test
   running= true;                   // Indicate running
   then= PUB::Clock::now();         // Test start time
   if( opt_hcdm || opt_verbose > 1 )
     debugh("%'10.3f running= true\n", PUB::Clock::now() - then);

   // Distribute the TimerItems
   // We can't distribute Items before the test is running since they will
   // simply get posted and discarded.
   int item_ix= 0;
   int task_ix= 0;
   while( item_ix < opt_items ) {
     if( task_ix >= opt_tasks )
       task_ix= 0;

     queue(__LINE__, nullptr, task_array[task_ix++], item_array[item_ix++]);
   }

   if( opt_hcdm || opt_verbose > 2 ) // (Distribution doesn't take much time)
     debugh("%'10.3f All Items distributed\n", PUB::Clock::now() - then);

   Thread::sleep(opt_runtime);      // (Run the test)

   if( opt_hcdm || opt_verbose > 1 ) { // WorkerPool debug while still running
     debugh("TimerThread\n");
     PUB::System::debug("while running==true");
   }

   running= false;                  // (But the test isn't 100% complete)
   if( opt_hcdm || opt_verbose > 1 )
     debugh("%'10.3f running= false\n", PUB::Clock::now() - then);

   if( USE_ITRACE )
     Trace::trace(".TST", __LINE__, "RUNNING=FALSE");
}
}; // class TimerThread
static TimerThread timer_thread;    // *THE* TimerThread

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

   // Display the options
   if( opt_retest )
      debugf("**************** REGRESSION TEST\n");

   if( opt_hcdm || opt_verbose > 1 ) {
     time_t gt= time(nullptr);
     struct tm* lt= localtime(&gt);

     debugf("Compiled: %s %s %s\n Started: %3s %2d %4d %.2d:%.2d:%.2d\n"
           , __DATE__, __TIME__, __FILE__
           , _month[lt->tm_mon], lt->tm_mday, lt->tm_year + 1900
           , lt->tm_hour, lt->tm_min, lt->tm_sec);
     debugf("%16d opt_hcdm\n", opt_hcdm);
     debugf("%16d opt_verbose\n", opt_verbose);
     debugf("%6s0x%.8x opt_trace (%'d)\n", "", opt_trace, opt_trace);
     debugf("\n");
   }

   if( opt_hcdm || opt_verbose ) {
     debugf("%'16.3f Runtime\n", opt_runtime);
     debugf("%'16d Items\n", opt_items);
     debugf("%'16d Tasks\n", opt_tasks);
     debugf("%'16d Size\n",  opt_size);
   }

   // Initialize the Item array
   item_array= (TimerItem**)malloc(sizeof(TimerItem*) * opt_items);
   bad_alloc(item_array);           // Verify allocated

   for(int i= 0; i<opt_items; ++i) {
     item_array[i]= new TimerItem(i);
   }

   // Initialize the Task array
   task_array= (TimerTask**)malloc(sizeof(TimerTask*) * opt_tasks);
   bad_alloc(task_array);           // Verify allocated

   for(int i= 0; i<opt_tasks; ++i) {
     task_array[i]= new TimerTask(i);
   }

   //*************************************************************************
   // Start the TimerThread (running the timing test)
   timer_thread.start();

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Wait for the TimerThread to complete
   timer_thread.join();
   if( opt_hcdm || opt_verbose > 1 )
     debugh("%'10.3f Join complete\n", PUB::Clock::now() - then);

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Wait for all TimerItem completions
   for(int item_ix= 0; item_ix<opt_items; ++item_ix) {
     if( opt_hcdm )
       tracef("%4d Task[%3d] Item[%3d] WAIT\n", __LINE__, -1, item_ix);
     trace(".TST", __LINE__, "WAIT ", -1, item_ix);
     item_array[item_ix]->wait.wait();
   }

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Testing is complete
   done= PUB::Clock::now();
   if( opt_hcdm || opt_verbose > 1 ) {
     debugh("%'10.3f Test complete\n", done - then);

     // We've waited for all TimerItems, but some TimerTasks (which run under
     // WorkerThreads) might have been stopped in that process. Stopping
     // schedules the Thread delete doesn't wait for completion. This normally
     // isn't a problem because Worker.cpp termination has a built-in delay.
     // We delay at this point in order to get coherent WorkerPool status.
     Thread::sleep(2.0);            // (Allows pending deletes to complete)
     debugf("\n");
     System::debug("Test complete");
   }
   //*************************************************************************

   // Count the operations (cross-checking the item_count and task_count)
   size_t task_count= 0;
   for(int item_ix= 0; item_ix<opt_items; ++item_ix) {
     for(int task_ix= 0; task_ix<opt_tasks; ++task_ix) {
       task_count += item_array[item_ix]->task_count[task_ix];
     }
   }

   size_t item_count= 0;
   for(int task_ix= 0; task_ix<opt_tasks; ++task_ix) {
     for(int item_ix= 0; item_ix<opt_items; ++item_ix) {
       item_count += task_array[task_ix]->item_count[item_ix];
     }
   }

   if( item_count != task_count ) {
     error_count += VERIFY( item_count == task_count );
     debugf("%'16zd Item count\n", item_count);
     debugf("%'16zd Task count\n", task_count);
   }

   // Report the results
   // Note: Operations added to a task while a task is running are processed
   // use the same Worker. While this increases the reported average queue
   // length, we don't have enough instrumentation to quantify this effect.
   debugf("\n");
   double elapsed= done - then;
   double per_sec= double(item_count)/elapsed;
   double workers= PUB::WorkerPool::get_workers();

   debugf("%'16.2f Elapsed\n", elapsed);
   debugf("%'16zd Operations\n", item_count);
   debugf("%'16.0f Workers\n", workers);
   debugf("%'16.0f Operations/second (Elapsed)\n", per_sec);
   debugf("%'16.2f Operations/worker (Average queue length)\n"
         , double(item_count) / workers);

   // Cleanup
   for(int item_ix= 0; item_ix<opt_items; ++item_ix) {
     delete item_array[item_ix];
     item_array[item_ix]= nullptr;
   }
   free(item_array);
   item_array= nullptr;

   for(int task_ix= 0; task_ix<opt_tasks; ++task_ix) {
     delete task_array[task_ix];
     task_array[task_ix]= nullptr;
   }
   free(task_array);
   task_array= nullptr;

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
            "  --items\t=count Number of Items\n"
            "  --size\t=count WorkerThread pool size\n"
            "  --tasks\t=count Number of Tasks\n"
            "  --trace\t{=size} Create internal trace file './trace.mem'\n"
            );
   });

   tc.on_init([tr](int, char**)
   {
     atexit(at_exit);               // Handle atexit

     debug_set_head(Debug::HEAD_THREAD | Debug::HEAD_TIME);
     if( opt_hcdm || opt_verbose > 1 )
       debug_set_mode(Debug::MODE_INTENSIVE);

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
     if( P == "items" ) {
       opt_items= tr->ptoi(V);
       if( opt_items < 1 || opt_items >= 0x00010000 ) {
         errorf("--items=%d invalid value\n", opt_items);
         errorf("  Valid range: %'d..%'.d\n", 1, 65535);
         return 1;
       }
     } else if( P == "runtime" ) {
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
     } else if( P == "size" ) {
       opt_size= (unsigned)tr->ptoi(V);
       if( opt_size < 0 || opt_size >= 0x00010000 ) {
         errorf("--size=%d invalid value\n", opt_size);
         errorf("  Valid range: %'d..%'.d\n", 0, 65535);
         return 1;
       }
       PUB::WorkerPool::set_size(opt_size);
     } else if( P == "tasks" ) {
       opt_tasks= tr->ptoi(V);
       if( opt_tasks < 1 || opt_tasks >= 0x00010000 ) {
         errorf("--tasks=%d invalid value\n", opt_tasks);
         errorf("  Valid range: %'d..%'.d\n", 1, 65535);
         return 1;
       }
     } else if( P == "trace" ) {
       if( opt_verbose > 0 )
         errorf("Warning: --trace option used but USE_ITRACE == false\n");

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
     if( opt_verbose > 2 )
       printf("on_term\n");

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
