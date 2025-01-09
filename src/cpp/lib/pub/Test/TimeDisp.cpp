//----------------------------------------------------------------------------
//
//       Copyright (c) 2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TimeDisp.cpp
//
// Purpose-
//       Dispatcher timing test.
//
// Last change date-
//       2025/01/09
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic
#include <exception>                // For std::exception
#include <new>                      // For std::bad_alloc
#include <cinttypes>                // For integer types
#include <clocale>                  // For setlocale

#include <pub/TEST.H>               // For test functions and macros
#include <pub/Clock.h>              // For pub::Clock::now
#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/Dispatch.h"           // For pub::dispatch objects, timed
#include <pub/Random.h>             // For pub::Random
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace
#include "pub/utility.i"            // For pub::utility conversion subroutines
#include "pub/Wrapper.h"            // For class Wrapper

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::Wrapper;

using PUB::s2c;                     // String to char* conversion

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  OPT_RUNTIME= 10'000              // Default, 10.0 second test
,  OPT_ITEMS= 512                   // Number of Items
,  OPT_TASKS= 8                     // Number of Tasks
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
static int             running= 0;  // Test running indicator
static Event           test_start;  // The test start Event

static void*           table= nullptr; // The Trace table
static TimerItem**     item_array= nullptr; // The Timer Item array
static TimerTask**     task_array= nullptr; // The Timer Task array

// Extended options
static int             opt_runtime= OPT_RUNTIME; // --runtime= milliseconds
static int             opt_items= OPT_ITEMS; // --items= count
static int             opt_tasks= OPT_TASKS; // --tasks= count
static int             opt_trace= 0; // --trace
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"runtime", required_argument, nullptr,    0} // --runtime
,  {"items",   required_argument, nullptr,    0} // --items
,  {"tasks",   required_argument, nullptr,    0} // --tasks
,  {"trace",   optional_argument, &opt_trace, 0x00400000} // --trace
,  {0, 0, 0, 0}                     // (End of option list)
};

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
   running= true;
   test_start.post();
// double then= pub::Clock::now();

   Thread::sleep((double)opt_runtime/1000.0); // (Run the test)

   running= false;
// double now= pub::Clock::now();
   test_start.reset();
// debugf("%'16.2f TimerThread elapsed\n", now - then);
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
pub::dispatch::Wait    wait;        // The Item's Wait object

size_t                 identity;    // The Item's identity
size_t*                task_count;  // COUNTER: Number of times handled/Task

public:
   TimerItem(                       // The Timer Item
     size_t            _identity)   // The Timer Item's identity
:  PUB::dispatch::Item(&wait), identity(_identity)
,  task_count((size_t*)malloc(sizeof(size_t) * opt_tasks))
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
   debugf("TimerItem(%p).debug(%s) id(%zd)\n", this, info, identity);

   for(int i= 0; i<opt_tasks; ++i) {
     debugf("..[%3d] %'16zd\n", i, task_count[i]);
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
pub::Random            random;      // Random number generator

size_t                 identity;    // The Task's identity
size_t*                item_count;  // COUNTER: Number of times handled/Item

   TimerTask(                       // The Timer Task
     size_t            _identity)   // The Timer Task's identity
:  PUB::dispatch::Task(), random(), identity(_identity)
,  item_count((size_t*)malloc(sizeof(size_t) * opt_items))
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

virtual void
   debug(                           // Debugging display
     const char*       info= "")    // Header information
{
   debugf("TimerTask(%p).debug(%s) id(%zd)\n", this, info, identity);

   for(int i= 0; i<opt_items; ++i) {
     debugf("..[%3d] %'16zd\n", i, item_count[i]);
   }
}

virtual void
   work(                            // Process
     PUB::dispatch::Item* item)     // This work Item
{
   if( !running ) {                 // If test completed
     item->post();                  // Post the Item
     return;
   }

   // Count this work item
   struct TimerItem* timer_item= (TimerItem*)item; // (We only get TimerItems)
   ++timer_item->task_count[identity]; // The Item saw us
   ++item_count[timer_item->identity]; // We saw the Item

   // Enqueue on next Task
   size_t next= random.modulus((uint32_t)opt_tasks);
   task_array[next]->enqueue(item);
}
}; // struct TimerTask

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
   if( opt_verbose ) {
     debugf("%s\n", "test_timing");

     debugf("%'16.2f Runtime\n", (double)opt_runtime/1000.0);
     debugf("%'16d Items\n", opt_items);
     debugf("%'16d Tasks\n", opt_tasks);
   }

   int error_count= 0;              // Error counter

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

   double then= pub::Clock::now();
   timer_thread.start();
   test_start.wait();               // Wait for running state

   // Distribute the TimerItems
   while( item_ix < opt_items ) {
     if( task_ix >= opt_tasks )
       task_ix= 0;

     task_array[task_ix++]->enqueue(item_array[item_ix++]);
   }

   // Wait for the TimerThread to complete
   timer_thread.join();
   double now= pub::Clock::now();

   // Wait for all TimerItem completions
   for(item_ix= 0; item_ix<opt_items; ++item_ix) {
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
// debugf("%'16.2f Operations/second (nominal)\n", (double)item_count/nominal);
   debugf("%'16.2f Operations/second (elapsed)\n", (double)item_count/elapsed);

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
     fprintf(stderr,
            "  --runtime\t=time In milliseconds\n"
            "  --items\t=count Number of Items\n"
            "  --tasks\t=count Number of Tasks\n"
            );

     if( USE_ITRACE )
       fprintf(stderr,
              "  --trace\t{=size} Create internal trace file './trace.mem'\n"
              );
   });

   tc.on_parm([tr](std::string P, const char* V)
   {
     if( USE_ITRACE && P == "trace" ) {
       if( V )
         opt_trace= tr->ptoi(V);
     } else if( P == "runtime" ) {
       opt_runtime= tr->ptoi(V);
       if( opt_runtime < 1000 )
         opt_runtime= 1000;
     } else if( P == "items" ) {
       opt_items= tr->ptoi(V);
       if( opt_items < 1 )
         opt_items= 1;
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

     if( USE_ITRACE && opt_trace )
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
     int error_count= 0;

     if( optind < argc ) {
       debugf("Positional arguments not allowed:\n");
       for(int i= optind; i<argc; ++i) {
         debugf("[%2d] '%s'\n", i, argv[i]);
       }
       return 1;
     }

     try {
       if( opt_verbose ) {
         debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);
         if( USE_ITRACE )
           debugf("%6s0x%.8x opt_trace\n", "", opt_trace);
       }

       error_count= test_timing();
     } catch(std::exception& x) {
       debugf("FAILED: Exception: exception(%s)\n", x.what());
       ++error_count;
     } catch(...) {
       debugf("FAILED: Exception: ...\n");
       ++error_count;
     }

     if( opt_verbose || error_count ) {
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
