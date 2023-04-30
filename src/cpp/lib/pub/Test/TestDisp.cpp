//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestDisp.cpp
//
// Purpose-
//       Test the Dispatch objects.
//
// Last change date-
//       2023/04/29
//
// Arguments: (For test_timing only)
//       TestDisp --timing          // (Only run timing test)
//       [1] 10240 Number of outer loops
//       [2]   160 Number of elements queued per loop
//       [3]   120 Number of "pass-along" Tasks
//       [4]     0 Number of elements left hanging per loop (NOT IMPLEMENTED)
//
//       [2]*[1]  Number of operations started
//       [2]*[4]  Number of ignored completions
//       [2]*([1]-[4]) Number of opertion completion waits
//       [2]*[1]*([3]+1) Number of operations
//
// Implementation notes-
//       See ./.TIMING for timing test information.
//
//----------------------------------------------------------------------------
#include <atomic>
#include <exception>

#include <inttypes.h>
#include <locale.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <pub/TEST.H>               // For test functions and macros
#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/Dispatch.h"           // For pub::dispatch objects, tested
#include <pub/Event.h>              // For pub::Event
#include <pub/Interval.h>           // For pub::Interval
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace
#include "pub/Wrapper.h"            // For class Wrapper

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

,  USE_PASSALONG_LAMBDA= false      // Use some PassAlongLambdaTasks?
,  USE_TRACE= false                 // Enable tracing?
}; // enum

//----------------------------------------------------------------------------
// Internal classes and subroutines
//----------------------------------------------------------------------------
#include "TestDisp.hpp"

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
std::atomic<uint64_t>  RondesvousTask::rondesvous= 0; // Rondesvous bit map

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static void*           table= nullptr; // The Trace table

// Extended options
static int             opt_error= false; // --error TODO: REMOVE
static int             opt_stress= false; // --stress
static int             opt_timing= false; // --timing
static int             opt_trace= 0; // --trace
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"stress",  no_argument,       &opt_stress,      true} // --stress
,  {"timing",  no_argument,       &opt_timing,      true} // --timing
,  {"trace",   optional_argument, &opt_trace, 0x00400000} // --trace
,  {"error",   no_argument,       &opt_error,       true} // --error
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       test0000
//
// Purpose-
//       Bringup test.
//
//----------------------------------------------------------------------------
static inline void line(int n)      // TODO: REMOVE
{  tracef("%4d ", n); }

static inline int
   test0000(int, char**)            // Mainline code
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
   int                 error_count= 1; // Error count

   if( opt_verbose ) debugf("%4d test0000\n", __LINE__);

   // Basic function test ====================================================
   if( opt_verbose ) debugf("\n%4d Basic function test\n", __LINE__);
   dispatch::Item     item;         // Our work Item
   dispatch::Wait     wait;         // Our Wait object
   class Test0000Task : public dispatch::Task {
     protected:
       int* target;

     public:
       Test0000Task( int* foo ) : dispatch::Task(), target(foo) {}
       virtual void work(dispatch::Item* item) {
         *target= 0;
         item->post();
       }
   } task(&error_count);            // Our Task

   item.done= &wait;                // Set Wait object
   task.enqueue(&item);             // Drive work
   if( opt_verbose ) debugf("%4d waiting...\n", __LINE__);
   wait.wait();                     // Wait for work completion
   task.reset();
   if( opt_verbose ) debugf("%4d ...running\n", __LINE__);

   if( error_count != 0 )
     throwf(__LINE__, "result(%d) non-zero", error_count);
   if( item.cc != 0 )
     throwf(__LINE__, "cc(%d) non-zero", item.cc);

   // Lambda function test ===================================================
   if( opt_verbose ) debugf("\n%4d Lambda function test\n", __LINE__);
   int not_done= true;
   int not_task= true;
   PUB::Event event;                // Our completion item
   dispatch::LambdaDone l_done([&](dispatch::Item* item_) { // Method done()
     if( &item != item_ )
       throwf("%4d &item(%p) item_(%p)\n", __LINE__, &item, item_);
     not_done= false;
     event.post();
   });

   dispatch::LambdaTask l_task([&](dispatch::Item* item_) { // Method work()
     not_task= false;
     item_->post();
   });

   item.cc= -1;                     // Set error result
   item.done= &l_done;
   l_task.enqueue(&item);           // Drive work
   if( opt_hcdm && opt_verbose ) debugf("%4d waiting...\n", __LINE__);
   event.wait();                    // Wait for event
   if( opt_hcdm && opt_verbose ) debugf("%4d ...running\n", __LINE__);

   if( item.cc != 0 )
     throwf(__LINE__, "cc(%d) non-zero", item.cc);
   if( not_task )
     throwf(__LINE__, "not_task (l_task.work not driven)");
   if( not_done )
     throwf(__LINE__, "not_done (l_done.done not driven)");

   // Verify delay and cancel ================================================
   if( opt_verbose ) debugf("\n%4d delay/cancel function tests\n", __LINE__);
   wait.reset();
   item.done= &wait;                // Set Wait object
   Interval interval;
   interval.start();
   dispatch::Disp::delay(3.025, &item); // Note: Extra time for Clock granule
   wait.wait();
   double elapsed= interval.stop();
   if( elapsed < 3.0 || elapsed > 3.1 )
     throwf(__LINE__, "delay 3.0<elapsed(%e)<3.1", elapsed);
   if( item.cc != 0 )
     throwf(__LINE__, "cc(%d) non-zero", item.cc);

   wait.reset();
   interval.start();
   void* cancel= dispatch::Disp::delay(3.025, &item);
   Thread::sleep(1.001);
   dispatch::Disp::cancel(cancel);
   wait.wait();
   elapsed= interval.stop();
   if( elapsed < 1.0 || elapsed > 1.1 )
     throwf(__LINE__, "delay 1.0<elapsed(%e)<1.1", elapsed);
   if( item.cc != dispatch::Item::CC_PURGE )
     throwf(__LINE__, "cc(%d) invalid", item.cc);

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test0001
//
// Purpose-
//       Bringup test: Rondesvous Task.
//
//----------------------------------------------------------------------------
static inline int
   test0001(int, char**)            // Mainline code
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
   RondesvousTask*     TASK[64];    // Rondesvous Task array
   dispatch::Item*     ITEM[64];    // Rondesvous Item array
   dispatch::Wait      WAIT[64];    // Rondesvous Wait array

   if( opt_verbose ) debugf("\n%4d test0001\n", __LINE__);

   // Initialize
   RondesvousTask::rondesvous.store(0);
   for(int i= 0; i<64; i++)
   {
     TASK[i]= new RondesvousTask(i);
     ITEM[i]= new dispatch::Item(0, &WAIT[i]);
   }

   // Drive work
   for(int i= 0; i<64; i++)
     dispatch::Disp::enqueue(TASK[i], ITEM[i]); // Drive work

   // Wait for completion
   for(int i= 0; i<64; i++)
     WAIT[i].wait();

   int64_t value= RondesvousTask::rondesvous.load();
   if( value != (-1) )
     throwf(__LINE__, "Work incomplete %" PRIx64, value);

   // Terminate
   for(int i= 0; i<64; i++)
   {
     TASK[i]->reset();
     delete(TASK[i]);
     TASK[i]= nullptr;

     delete(ITEM[i]);
     ITEM[i]= nullptr;
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_error
//
// Purpose-
//       Error test TODO: REMOVE
//
//----------------------------------------------------------------------------
static inline int
   test_error(int, char**)          // Error test TODO: REMOVE
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
   if( opt_verbose ) debugf("\n%4d test_error\n", __LINE__);
   int                 error_count= 0; // Error count

   if( opt_hcdm )
     throw std::runtime_error("test runtime_error");

   error_count += VERIFY( "test_error always fails" == nullptr );

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_stress
//
// Purpose-
//       Stress test
//
//----------------------------------------------------------------------------
static inline int
   test_stress(int, char**)         // Stress test
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
   if( opt_verbose ) debugf("\n%4d test_stress\n", __LINE__);

   debugf("test_stress NOT CODED YET\n"); // TODO: CODE

   return 0;
}

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
   test_timing(                     // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   dispatch::Task*     FINAL;       // The final Task
   dispatch::Task**    TASK;        // The PassAlongTask array
   dispatch::Item**    ITEM;        // The Item array
   dispatch::Wait**    WAIT;        // The Wait array

   if( opt_verbose ) debugf("\n%4d test_timing\n", __LINE__);

   // Set defaults
   int LOOPS= 10240;                // Number of major iterations
   int MULTI= 160;                  // Number of elements queued per iteration
   int TASKS= 120;                  // Number of PassAlongTasks
// int HANGS= 2;                    // Number of no-wait requests

   // Parameter analysis
// if( argc > optind + 3 )
//   HANGS= atoi(argv[optind + 3]);
   if( argc > optind + 2 )
     TASKS= atoi(argv[optind + 2]);
   if( argc > optind + 1 )
     MULTI= atoi(argv[optind + 1]);
   if( argc > optind + 0 )
     LOOPS= atoi(argv[optind + 0]);
   if( opt_verbose || opt_timing ) {
     debugf("%16d LOOPS\n", LOOPS);
     debugf("%16d MULTI\n", MULTI);
     debugf("%16d TASKS\n", TASKS);
//   debugf("%16d HANGS\n", HANGS);
   }

   // Create the Task array
   FINAL= new PassAlongTask(nullptr);
   dispatch::Task* prior= FINAL;
   TASK= new dispatch::Task*[TASKS];
   for(int i= 0; i < TASKS; ++i)
   {
     dispatch::Task* task= nullptr;
     if( USE_PASSALONG_LAMBDA && (i & 1) )
       task= new PassAlongLambdaTask(prior);
     else
       task= new PassAlongTask(prior);
     prior= task;
     TASK[i]= task;
   }

   // Create the ITEM and WAIT arrays
   ITEM= new dispatch::Item*[MULTI];
   WAIT= new dispatch::Wait*[MULTI];
   for(int i= 0; i < MULTI; i++)
   {
     WAIT[i]= new dispatch::Wait();
     ITEM[i]= new dispatch::Item(0, WAIT[i]);
   }

   // Debugging display
   if( USE_TRACE || (opt_hcdm && opt_verbose > 1) ) {
     debugf("TASKS: %d\n", TASKS);
     for(int i= 0; i<TASKS; ++i ) {
       debugf("[%3d] %p->%p\n", i, TASK[i], ((PassAlongTask*)TASK[i])->next);
     }
     debugf("[%3d] %p [FINAL]\n", TASKS, FINAL);

     debugf("MULTI: %d\n", MULTI);
     for(int i= 0; i<MULTI; ++i ) {
       debugf("[%3d] ITEM(%p)->WAIT(%p)\n", i, ITEM[i], WAIT[i]);
     }
   }

   // Run the test
   Interval interval;

   interval.start();
   for(int loop= 0; loop < LOOPS; loop++)
   {
     for(int multi= 0; multi < MULTI; multi++) {
       if( USE_TRACE )
         Trace::trace(".ENQ", ">>>>", (void*)intptr_t(multi), ITEM[multi]);

       TASK[TASKS-1]->enqueue(ITEM[multi]);
     }

     for(int multi= 0; multi < MULTI; multi++)
     {
       if( USE_TRACE )
         Trace::trace(".DEQ", "<<<<", (void*)intptr_t(multi), ITEM[multi]);

       WAIT[multi]->wait();
       WAIT[multi]->reset();
     }
   }

   // Test complete
   double elapsed= interval.stop();
   double ops= (double)TASKS + 1.0;
   ops *= (double)MULTI;
   ops *= (double)LOOPS;
   setlocale(LC_NUMERIC, "");       // Activates ' thousand separator
   if( opt_verbose || opt_timing ) {
     debugf("%'16.3f seconds elapsed\n", elapsed);
     debugf("%'16.3f ops/second\n", ops / elapsed);
   }

   // Diagnostics
   if( opt_hcdm || opt_verbose ) {
     debugf("\n");
     dispatch::Disp::debug();
   }

   // Cleanup
   FINAL->reset();
   for(int i= 0; i<TASKS; i++)
   {
     TASK[i]->reset();
     delete TASK[i];
   }
   delete FINAL;

   for(int i= 0; i<MULTI; i++)
   {
     delete ITEM[i];
     delete WAIT[i];
   }

   delete[] WAIT;
   delete[] ITEM;
   delete[] TASK;

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
     fprintf(stderr, "  --stress\tRun stress test\n");
     fprintf(stderr, "  --timing\tRun timing test\n");
     if( USE_TRACE )
       fprintf(stderr,
              "  --trace\t{=size} Create internal trace file './trace.mem'\n"
              );
   });

   tc.on_parm([tr](std::string P, const char* V)
   {
     if( P == "trace" ) {
       if( V )
         opt_trace= tr->ptoi(V);
     }

     return 0;
   });

   tc.on_init([tr](int, char**)
   {
     debug_set_head(Debug::HEAD_THREAD); // Include thread in heading
     if( opt_hcdm )
       debug_set_mode(Debug::MODE_INTENSIVE);

     if( USE_TRACE && opt_trace )
       table= tr->init_trace("./trace.mem", opt_trace);

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
     try {
       if( opt_verbose )
         debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

       if( opt_timing ) {
         error_count= test_timing(argc, argv);
       } else if( opt_stress ) {
         error_count= test_stress(argc, argv);
       } else if( opt_error ) {     // TODO: REMOVE
         error_count += test_error(argc, argv); // TODO: REMOVE
       } else {
         static const char* static_argv[]= { "100", "100", "100" };
         argv= (char**)static_argv;
         argc= 3;
         optind= 0;

         if( true  ) error_count += test0000(argc, argv);
         if( true  ) error_count += test0001(argc, argv);
         if( true  ) error_count += test_timing(argc, argv);
       }
     } catch(const char* x) {
       debugf("FAILED: Exception: const char*(%s)\n", x);
       ++error_count;
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
