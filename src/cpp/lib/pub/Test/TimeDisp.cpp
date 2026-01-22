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
//       2026/01/21
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic
#include <exception>                // For std::exception
#include <new>                      // For std::bad_alloc, operator new
#include <cinttypes>                // For integer types
#include <clocale>                  // For setlocale

#include <endian.h>                 // For htobe64

#include <pub/TEST.H>               // For test functions and macros
#include <pub/Clock.h>              // For pub::Clock::now
#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/Dispatch.h"           // For pub::dispatch objects
#include <pub/Random.h>             // For pub::Random
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
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  OPT_RUNTIME= 10'000              // Default, 10.0 second test
,  OPT_ITEMS= 512                   // Default number of Items
,  OPT_TASKS= 8                     // Default number of Tasks

// These compile-time options are independent of the --trace parameter option.
,  USE_IDEBUG= false                // Enable internal debugging?
,  USE_ITRACE= false                // Enable internal tracing?
}; // enum

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class TimerItem;                    // For TimerItem*
class TimerTask;                    // For TimerTask*

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             error_count= 0; // Error counter
static int             running= 0;  // Test running indicator

static Event           test_start;  // The test start Event
static double          then= 0.0;   // The test start time

static void*           table= nullptr; // The Trace table
static TimerItem**     item_array= nullptr; // The Timer Item array
static TimerTask**     task_array= nullptr; // The Timer Task array

// Extended options
static int             opt_runtime= OPT_RUNTIME; // --runtime
static int             opt_items= OPT_ITEMS; // --items
static unsigned        opt_size= WorkerPool::get_size(); // --size
static int             opt_tasks= OPT_TASKS; // --tasks
static int             opt_trace= 0; // --trace
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"runtime",  required_argument, nullptr,    0} // --runtime=milliseconds
,  {"items",    required_argument, nullptr,    0} // --items=count
,  {"size",     required_argument, nullptr,    0} // --size=count
,  {"tasks",    required_argument, nullptr,    0} // --tasks=count
,  {"trace",    optional_argument, &opt_trace, 0x0800'0000} // --trace
,  {0, 0, 0, 0}                     // (End of option list)
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
     const char*       op,          // Operation
     uint16_t          task,        // Associated Task identifier
     uint16_t          item)        // Associated Item identifier
{
   memset(value, ' ', 8);           // Blank fill value operation
   memcpy(value, op, 5);               // Set operation type

   uint64_t task_item= uint64_t(task) << 32 | item;
   uint64_t* ptr_item= (uint64_t*)(value+8);
   *ptr_item= htobe64(task_item);

   trace(ident, line);
}

   Record(                          // Initialize the trace entry
     const char*       ident,       // The trace identiier (4 characters)
     uint32_t          line,        // The line number
     const char*       op,          // Operation
     uint16_t          task,        // Associated Task identifier
     uint16_t          item,        // Associated Item identifier
     uint16_t          next_task,   // Next Task scheduled
     uint16_t          last_item)   // Last Item processed
{
   memset(value, ' ', 8);           // Blank fill value operation
   memcpy(value, op, 5);            // Set operation type

   uint64_t task_item= uint64_t(task) << 32 | item;
   task_item |= uint64_t(next_task)   << 16 | last_item;
   uint64_t* ptr_item= (uint64_t*)(value+8);
   *ptr_item= htobe64(task_item);

   trace(ident, line);
}
};

static void
   trace(                           // Allocate and initialize trace Record
     const char*       ident,       // The trace identiier (4 characters)
     uint32_t          line,        // The line number
     const char*       op,          // Operation
     uint16_t          task,        // Associated Task identifier
     uint16_t          item)        // Associated Item identifier
{  if( USE_ITRACE ) {               // If tracing active
     Record* record= new Record(ident, line, op, task, item);
     (void)record;
   }
}

static void
   trace(                           // Allocate and initialize trace Record
     const char*       ident,       // The trace identiier (4 characters)
     uint32_t          line,        // The line number
     const char*       op,          // Operation
     uint16_t          task,        // Associated Task identifier
     uint16_t          item,        // Associated Item identifier
     uint16_t          next_task,   // Next Task scheduled
     uint16_t          last_item)   // Last Item processed
{  if( USE_ITRACE ) {               // If tracing active
     Record* record= new Record(ident, line, op, task, item
                               , next_task, last_item);
     (void)record;
   }
}

//----------------------------------------------------------------------------
//
// Class-
//       TimerThread
//
// Purpose-
//       Background Thread that sets and clears `running`
//
//----------------------------------------------------------------------------
class TimerThread : public PUB::Thread {
public:
   TimerThread( void ) = default;
   ~TimerThread( void ) = default;

virtual void
   run( void )
{
   test_start.post();
// running= true;                   // Set in main task, test_timing()
// then= PUB::Clock::now();         // Set in main task, test_timing()

   Thread::sleep((double)opt_runtime/1000.0); // (Run the test)

   if( opt_hcdm || opt_verbose > 1 ) { // WorkerPool debug while still running
     debugf("\n");
     PUB::WorkerPool::debug();
   }

   running= false;

   test_start.reset();
}
}; // class TimerThread
static TimerThread timer_thread;    // *THE* TimerThread

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

   // Verify enqueued for this Task; Record last Item processed
   if( USE_IDEBUG ) {
     error_count += VERIFY(timer_item->next_task == identity);
     last_item= timer_item->identity;
   }

   // On test completion, post Item
   if( !running ) {                 // If test completed
     if( opt_hcdm )
       tracef("%4d Task[%3d] Item[%3d] POST\n", __LINE__
             , identity, timer_item->identity);
     else
       trace(".TST", __LINE__, "POST ", identity, timer_item->identity);

     if( USE_IDEBUG )
       timer_item->next_task= -identity; // Indicate POSTED (by this Task)

     item->post();                  // Post the Item
     return;
   }

   // Count this work item
   trace(".TST", __LINE__, "COUNT", identity, timer_item->identity);
   ++timer_item->task_count[identity]; // The Item saw us
   ++item_count[timer_item->identity]; // We saw the Item

   // Enqueue on next Task
   uint32_t next= random.modulus((uint32_t)opt_tasks);
   if( USE_IDEBUG )
     timer_item->next_task= next;
   trace(".TST", __LINE__, "QUEUE", identity, timer_item->identity
        , next, last_item);
   task_array[next]->enqueue(item);
}
}; // struct TimerTask

//----------------------------------------------------------------------------
//
// Subroutine-
//       BINGO
//
// Purpose-
//       Test everything we can. (Invoke from GDB)
//
//----------------------------------------------------------------------------
extern const char*                  // Return something
   BINGO( void );                   // Test
const char*                         // Return something
   BINGO( void )                    // Timing test
{
   PUB::WorkerPool::debug("BINGO");
   PUB::Thread::static_debug("BINGO");

   debugf("\n");
   for(int task_ix= 0; task_ix < opt_tasks; ++task_ix)
     task_array[task_ix]->debug("TASKS");

   debugf("\n");
   for(int item_ix= 0; item_ix < opt_items; ++item_ix)
     item_array[item_ix]->debug("ITEMS");

   return "BANGO";
};

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
   if( opt_hcdm || opt_verbose ) {
     debugf("%s\n", "test_timing");

     debugf("%'16.3f Runtime\n", (double)opt_runtime/1000.0);
     debugf("%'16d Items\n", opt_items);
     debugf("%'16d Tasks\n", opt_tasks);
   }

   error_count= 0;                  // (No errors yet)

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

   // Start the TimerThread; Start the timing test
   int item_ix= 0;
   int task_ix= 0;

   timer_thread.start();
   test_start.wait();               // Wait for running state
   running= true;                   // Indicate running
   then= PUB::Clock::now();         // Test start time

   // Distribute the TimerItems
   // Note: We can't distribute these Items until the test is running, or the
   // Items will simply get posted and discarded.
   // While initializing, the test runs with fewer active items.
   while( item_ix < opt_items ) {
     if( task_ix >= opt_tasks )
       task_ix= 0;

     trace(".TST", __LINE__, "QUEUE", item_ix, task_ix);
     item_array[item_ix]->next_task= task_ix;
     task_array[task_ix++]->enqueue(item_array[item_ix++]);
   }

   // Wait for the TimerThread to complete
   timer_thread.join();
   double now= PUB::Clock::now();

   // Wait for all TimerItem completions
   for(item_ix= 0; item_ix<opt_items; ++item_ix) {
     if( opt_hcdm )
       tracef("%4d Task[%3d] Item[%3d] WAIT\n", __LINE__, -1, item_ix);
     else
       trace(".TST", __LINE__, "WAIT ", -1, item_ix);
     item_array[item_ix]->wait.wait();
   }

   // Completion analysis
   debugf("\n");
// double nominal= (double)opt_runtime/1000.0;
   double elapsed= now - then;
// debugf("%'16.2f Nominal\n", nominal);
   debugf("%'16.2f Elapsed\n", elapsed);

   size_t item_count= 0;
   size_t task_count= 0;
   for(item_ix= 0; item_ix<opt_items; ++item_ix) {
     for(task_ix= 0; task_ix<opt_tasks; ++task_ix) {
       task_count += item_array[item_ix]->task_count[task_ix];
     }
   }

   for(task_ix= 0; task_ix<opt_tasks; ++task_ix) {
     for(item_ix= 0; item_ix<opt_items; ++item_ix) {
       item_count += task_array[task_ix]->item_count[item_ix];
     }
   }

   if( item_count != task_count ) {
     error_count += VERIFY( item_count == task_count );
     debugf("%'16zd Item count\n", item_count);
     debugf("%'16zd Task count\n", task_count);
   }

   // (Enqueue + Dequeue) Operations/second, includes possible task scheduling
// debugf("%'16.0f Operations/second (nominal)\n", (double)item_count/nominal);
   debugf("%'16.0f Operations/second (elapsed)\n", (double)item_count/elapsed);

   // Diagnostics
   if( opt_hcdm || opt_verbose ) { // WorkerPool debug after test completes
     debugf("\n");
     PUB::WorkerPool::debug();
   }

   // Cleanup
   for(item_ix= 0; item_ix<opt_items; ++item_ix) {
     delete item_array[item_ix];
     item_array[item_ix]= nullptr;
   }
   free(item_array);
   item_array= nullptr;

   for(task_ix= 0; task_ix<opt_tasks; ++task_ix) {
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
            "  --runtime\t=time In milliseconds\n"
            "  --items\t=count Number of Items\n"
            "  --size\t=count WorkerThread pool size\n"
            "  --tasks\t=count Number of Tasks\n"
            "  --trace\t{=size} Create internal trace file './trace.mem'\n"
            );
   });

   tc.on_parm([tr](std::string P, const char* V)
   {
     if( P == "trace" ) {
       if( V )
         opt_trace= tr->ptoi(V);
     } else if( P == "runtime" ) {
       opt_runtime= tr->ptoi(V);
       if( opt_runtime < 100 )
         opt_runtime= 100;
     } else if( P == "items" ) {
       opt_items= tr->ptoi(V);
       if( opt_items < 1 )
         opt_items= 1;
     } else if( P == "size" ) {
       opt_size= (unsigned)tr->ptoi(V);
       PUB::WorkerPool::set_size(opt_size);
     } else if( P == "tasks" ) {
       opt_tasks= tr->ptoi(V);
       if( opt_tasks < 1 )
         opt_tasks= 1;
     } else {
       fprintf(stderr, "Invalid option '%s'\n", s2c(P));
       return 1;
     }

     return 0;
   });

   tc.on_init([tr](int, char**)
   {
     debug_set_head(Debug::HEAD_THREAD | Debug::HEAD_TIME);
     if( opt_hcdm )
       debug_set_mode(Debug::MODE_INTENSIVE);

     if( opt_trace )
       table= tr->init_trace("./trace.mem", opt_trace);

     setlocale(LC_NUMERIC, "");     // Activates ' thousand separator

     return 0;
   });

   tc.on_term([tr]()
   {
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
       if( opt_hcdm || opt_verbose ) {
         debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

         debugf("%16d opt_hcdm\n", opt_hcdm);
         debugf("%16d opt_verbose\n", opt_verbose);

         debugf("%'16d opt_runtime\n", opt_runtime);
         debugf("%'16d opt_items\n", opt_items);
         debugf("%'16u opt_size\n", opt_size);
         debugf("%'16d opt_tasks\n", opt_tasks);
         debugf("%6s0x%.8x opt_trace\n", "", opt_trace);
       }

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
   opt_hcdm= HCDM;
   opt_verbose= VERBOSE;
   return tc.run(argc, argv);
}
