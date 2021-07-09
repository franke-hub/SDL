//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
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
//       2021/07/09
//
// Arguments: (For testtime only)
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

#include <inttypes.h>
#include <locale.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <exception>

#include <pub/Debug.h>
#include <pub/Event.h>
#include <pub/Interval.h>
#include <pub/Thread.h>

#include "pub/Dispatch.h"
using namespace _PUB_NAMESPACE;
using namespace _PUB_NAMESPACE::debugging;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <pub/ifmacro.h>

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::atomic<uint64_t>
                       rondesvous;  // Rondesvous bit map
static pub::Event      one;         // Synchronization event

//----------------------------------------------------------------------------
//
// Class-
//       Waiter
//
// Purpose-
//       Wait for event sequence.
//
//----------------------------------------------------------------------------
class Waiter : public pub::Worker { // Waiter
public:
virtual void
   work( void )                     // The Waiter function
{  one.wait(); }                    // Wait for event completion
}; // class Waiter

//----------------------------------------------------------------------------
//
// Class-
//       PassAlongTask
//
// Purpose-
//       Pass work to next Task in list.
//
//----------------------------------------------------------------------------
class PassAlongTask : public dispatch::Task {
protected:
dispatch::Task*        next;        // Next Task in list

public:
virtual
   ~PassAlongTask( void )
{
// IFHCDM( debugf("~PassAlongTask(%p) %2d\n", this, index); )
}

   PassAlongTask(
     dispatch::Task*   next)
: dispatch::Task()
, next(next)
{ }

virtual void
   work(
     dispatch::Item*   item)
{  next->enqueue(item); }           // Give the work to the next Task
}; // class PassAlongTask

//----------------------------------------------------------------------------
//
// Class-
//       RondesvousTask
//
// Purpose-
//       Report work item received, drive Done callback.
//
//----------------------------------------------------------------------------
class RondesvousTask : public dispatch::Task {
protected:
int                    index;       // Rondesvous identifier

public:
virtual
   ~RondesvousTask( void )
{
// IFHCDM( debugf("~RondesvousTask(%p) %2d\n", this, index); )
}

   RondesvousTask(
     int               index)
: dispatch::Task()
, index(index)
{ }

virtual void
   work(
     dispatch::Item*   item)
{
   uint64_t bitmap= ((uint64_t)1)<<index;

   uint64_t oldValue= rondesvous.load();
   for(;;)
   {
     uint64_t newValue= oldValue | bitmap;
     if( rondesvous.compare_exchange_strong(oldValue, newValue) )
       break;
   }

// debugf("%2d %.16" PRIx64 " %.16" PRIx64 "\n", index, bitmap, newValue);
   item->post();
}
}; // class RondesvousTask

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::throwf
//
// Purpose-
//       Write a diagnostic error message and throw an exception.
//
//----------------------------------------------------------------------------
static void
   throwf(                          // Abort with error message
     int               line,        // Line number
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
   _ATTRIBUTE_PRINTF(2,3)
   _ATTRIBUTE_NORETURN;

static void
   throwf(                          // Abort with error message
     int               line,        // Line number
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer

   fprintf(stderr, "%4d %s: ABORT: ", line, __FILE__);

   va_start(argptr, fmt);           // Initialize va_ functions
   vthrowf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test0000
//
// Purpose-
//       Bringup test.
//
//----------------------------------------------------------------------------
static int
   test0000(int, char**)            // Mainline code
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
   int                 result= 1;   // Resultant

   IFHCDM( debugf("%4d test0000\n", __LINE__); )

   dispatch::Item     item;         // Our work Item
   dispatch::Wait     wait;         // Our Wait object
   class Test0000Task : public dispatch::Task {
     protected:
       int* target;

     public:
       Test0000Task( int* foo ) : dispatch::Task(), target(foo) {}
       virtual void work(dispatch::Item* item) {
         IFHCDM( debugf("%4d Task.work\n", __LINE__); )
         *target= 0;
         item->post();
       }
   } task(&result);                 // Our Task

   item.done= &wait;                // Set Wait object
   task.enqueue(&item);             // Drive work
   IFHCDM( debugf("%4d waiting...\n", __LINE__); )
   wait.wait();                     // Wait for work completion
   task.reset();
   IFHCDM( debugf("%4d ...running\n", __LINE__); )

   // Verify that task.work was driven
   if( result != 0 )
     throwf(__LINE__, "result(%d) non-zero", result);
   if( item.cc != 0 )
     throwf(__LINE__, "cc(%d) non-zero", item.cc);

   // Verify delay and cancel
   wait.reset();
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

   return result;
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
static int
   test0001(int, char**)            // Mainline code
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
   RondesvousTask*     TASK[64];    // Rondesvous Task array
   dispatch::Item*     ITEM[64];    // Rondesvous Item array
   dispatch::Wait      WAIT[64];    // Rondesvous Wait array

   IFHCDM( debugf("%4d test0001\n", __LINE__); )

   // Initialize
   rondesvous.store(0);
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

   int64_t value= rondesvous.load();
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
//       test0002
//
// Purpose-
//       Bringup test: MAX_THREADS.
//
// Implementation note-
//       The implementation has been tested but not considered useful.
//
//----------------------------------------------------------------------------
static int
   test0002(int, char**)            // Mainline code
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
#if 0 // Implementation not exposed ==========================================
   IFHCDM( debugf("%4d test0002\n", __LINE__); )

   // MaxThreads testing
   WorkerPool::debug();

   one.reset();
   for(int i= 0; i<732; i++)
     WorkerPool::work(new Waiter());
   Thread::sleep(1.5);
   debugf("setMaxThreads(4096)\n");
   WorkerPool::setMaxThreads(4096);
   WorkerPool::debug();

   debugf("post()\n");
   one.post(0);
   Thread::sleep(1.5);
   WorkerPool::debug();

   debugf("setMaxThreads(64)\n");
   WorkerPool::setMaxThreads(64);
   WorkerPool::debug();

   debugf("setMaxThreads(4096)\n");
   WorkerPool::setMaxThreads(4096);
   WorkerPool::debug();

   one.reset();
   for(int i= 0; i<732; i++)
     WorkerPool::work(new Waiter());
   Thread::sleep(1.5);

   debugf("post()\n");
   one.post(0);
   Thread::sleep(1.5);
   WorkerPool::debug();
#endif // Implementation not exposed =========================================

// debugf("reset()\n");
   WorkerPool::reset();
   WorkerPool::debug();

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testtime
//
// Purpose-
//       Bringup test: Timing/stress test.
//
//----------------------------------------------------------------------------
static int
   testtime(                        // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   dispatch::Task      FINAL;       // The final Task
   dispatch::Task**    TASK;        // The PassAlongTask array
   dispatch::Item**    ITEM;        // The Item array
   dispatch::Wait**    WAIT;        // The Wait array

   IFHCDM( debugf("%4d testtime\n", __LINE__); )

   // Set defaults
   int LOOPS= 10240;                // Number of major iterations
   int MULTI= 160;                  // Number of elements queued per iteration
   int TASKS= 120;                  // Number of PassAlongTasks
// int HANGS= 2;                    // Number of no-wait requests

   // Parameter analysis
// if( argc > 4 )
//   HANGS= atoi(argv[4]);
   if( argc > 3 )
     TASKS= atoi(argv[3]);
   if( argc > 2 )
     MULTI= atoi(argv[2]);
   if( argc > 1 )
     LOOPS= atoi(argv[1]);
   debugf("%16d LOOPS\n", LOOPS);
   debugf("%16d MULTI\n", MULTI);
   debugf("%16d TASKS\n", TASKS);
// debugf("%16d HANGS\n", HANGS);

   // Create the Task array
   dispatch::Task* prior= &FINAL;
   TASK= new dispatch::Task*[TASKS];
   for(int i= TASKS-1; i >=0; i--)
   {
     dispatch::Task* task= new PassAlongTask(prior);
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

   // Run the test
// debug_set_mode(Debug::MODE_IGNORE);
   Interval interval;

   interval.start();
   for(int loop= 0; loop < LOOPS; loop++)
   {
     for(int multi= 0; multi < MULTI; multi++)
       TASK[0]->enqueue(ITEM[multi]);

     for(int multi= 0; multi < MULTI; multi++)
     {
       WAIT[multi]->wait();
       WAIT[multi]->reset();
     }
   }

   // Test complete
   double elapsed= interval.stop();
// debug_set_mode(Debug::MODE_INTENSIVE);
   debugf("%16.3f seconds elapsed\n", elapsed);
   double ops= (double)TASKS + 1.0;
   ops *= (double)MULTI;
   ops *= (double)LOOPS;
   setlocale(LC_ALL, "");           // Activates ' thousand separator
   debugf("%'16.3f ops/second\n", ops / elapsed);

   // Diagnostics
   dispatch::Disp::debug();

   // Cleanup
   FINAL.reset();
   for(int i= 0; i<TASKS; i++)
   {
     TASK[i]->reset();
     delete TASK[i];
   }

   for(int i= 0; i<MULTI; i++)
   {
     delete ITEM[i];
     delete WAIT[i];
   }

   delete WAIT;
   delete ITEM;
   delete TASK;

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
   int                 result= 0;

   debug_set_head(Debug::HEAD_THREAD); // Include thread in heading
   debug_set_file_mode("ab");       // Append trace file
   IFHCDM( debug_set_mode(Debug::MODE_INTENSIVE); )
   debugh("TestDisp started\n");

   try {
     if( argc > 1 && strcmp(argv[1], "-time") == 0 )
       result |= testtime(0, nullptr);
     else
     {
       if( true  ) result |= test0000(argc, argv);
       if( true  ) result |= test0001(argc, argv);
       if( true  ) result |= test0002(argc, argv);
       if( true  ) result |= testtime(argc, argv);
     }
   } catch(const char* x) {
     debugf("Exception const char*(%s)\n", x);
     result= 2;
   } catch(std::exception& x) {
     debugf("Exception exception(%s)\n", x.what());
     result= 2;
   } catch(...) {
     debugf("Exception ...\n");
     result= 2;
   }

   debugf("Result(%d)\n", result);

   return result;
}
