//----------------------------------------------------------------------------
//
//       Copyright (c) 2013-2020 Frank Eskesen.
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
//       2020/01/05
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
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <exception>

#include <com/Atomic.h>
#include <com/Clock.h>
#include <com/Debug.h>
#include <com/Thread.h>
#include <com/ThreadLogger.h>

#include "com/Dispatch.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef logf
#define logf traceh                 // Alias for trace w/header
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Dispatch*       disp;        // Our dispatcher
static ATOMIC64        rondesvous;  // Rondesvous bit map

//----------------------------------------------------------------------------
//
// Class-
//       PassAlongTask
//
// Purpose-
//       Pass work to next Task in list.
//
//----------------------------------------------------------------------------
class PassAlongTask : public DispatchTask {
protected:
DispatchTask*          next;        // Next Task in list

public:
virtual
   ~PassAlongTask( void )
{
// IFHCDM( debugf("~PassAlongTask(%p) %2d\n", this, index); )
}

   PassAlongTask(
     DispatchTask*     next)
: DispatchTask()
, next(next)
{}

virtual void
   work(
     DispatchItem*     item)
{
   disp->enqueue(next, item);       // Give the work to the next Task
}
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
class RondesvousTask : public DispatchTask {
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
: DispatchTask()
, index(index)
{}

virtual void
   work(
     DispatchItem*     item)
{
   uint64_t bitmap= ((uint64_t)1)<<index;

   int cc;
   uint64_t oldValue;
   uint64_t newValue;
   do
   {
     oldValue= rondesvous;
     newValue= oldValue | bitmap;
     cc= csd(&rondesvous, oldValue, newValue);
   } while( cc != 0 );

// debugf("%2d %.16" PRIx64 " %.16" PRIx64 "\n", index, bitmap, newValue);
   item->post(0);
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
   test0000(                        // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 result= 1;   // Resultant

   IFHCDM( debugf("%4d test0000\n", __LINE__); )

   DispatchItem       item;         // Our work Item
   DispatchWait       wait;         // Our Wait object
   class Test0000Task : public DispatchTask {
     protected:
       int* target;

     public:
       Test0000Task( int* foo ) : DispatchTask(), target(foo) {}
       virtual void work(DispatchItem* item) {
         IFHCDM( debugf("%4d task.work\n", __LINE__); )
         *target= 0;
         item->post(0);
       }
   } task(&result);                 // Our Task Block

   item.setDone(&wait);             // Set Done callback object
   disp->enqueue(&task, &item);     // Drive work
   IFHCDM( debugf("%4d waiting...\n", __LINE__); )
   wait.wait();                     // Wait for work completion
   IFHCDM( debugf("%4d ...running\n", __LINE__); )

   // Verify that task.work was driven
   if( result != 0 )
     throwf(__LINE__, "result(%d) non-zero", result);
   if( item.getCC() != 0 )
     throwf(__LINE__, "cc(%d) non-zero", item.getCC());

   // Verify delay and cancel
   wait.reset();
   double tod= Clock::current();
   disp->delay(3.001, &item);
   wait.wait();
   double elapsed= Clock::current() - tod;
   if( elapsed < 3.0 || elapsed > 4.0 )
     throwf(__LINE__, "elapsed(%e)", elapsed);
   if( item.getCC() != 0 )
     throwf(__LINE__, "cc(%d) non-zero", item.getCC());

   wait.reset();
   tod= Clock::current();
   void* cancel= disp->delay(3.001, &item);
   Thread::sleep(1.001);
   disp->cancel(cancel);
   wait.wait();
   elapsed= Clock::current() - tod;
   if( elapsed < 1.0 || elapsed > 2.0 )
     throwf(__LINE__, "elapsed(%e)", elapsed);
   if( item.getCC() != DispatchItem::CC_ERROR )
     throwf(__LINE__, "cc(%d) invalid", item.getCC());

   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test0001
//
// Purpose-
//       Bringup test: Rondesvous task.
//
//----------------------------------------------------------------------------
static int
   test0001(                        // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   RondesvousTask*     Task[64];    // RondesvousTask array
   DispatchItem*       Item[64];    // RondexvousItem array
   DispatchWait        Wait[64];    // RondexvousWait array

   IFHCDM( debugf("%4d test0001\n", __LINE__); )

   // Initialize
   rondesvous= 0;
   for(int i= 0; i<64; i++)
   {
     Task[i]= new RondesvousTask(i);
     Item[i]= new DispatchItem(0, &Wait[i]);
   }

   // Drive work
   for(int i= 0; i<64; i++)
   {
     disp->enqueue(Task[i], Item[i]); // Drive work
   }

   // Wait for completion
   for(int i= 0; i<64; i++)
   {
     if( rondesvous == (-1) )
       break;

     Thread::sleep(0.001);
   }

   int64_t value= rondesvous;
   if( value != (-1) )
     throwf(__LINE__, "Work incomplete %" PRIx64, value);

   // The rondesvous may be complete but the task may still be running.
   // We have to wait for Wait completion too.
   for(int i= 0; i<64; i++)
   {
     Wait[i].wait();
   }

   // Terminate
   for(int i= 0; i<64; i++)
   {
     Task[i]->reset();
     delete(Task[i]);
     Task[i]= NULL;

     delete(Item[i]);
     Item[i]= NULL;
   }

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
   DispatchTask        FINAL;       // The final Task
   DispatchTask**      Task;        // The PassAlongTask array
   DispatchItem**      Item;        // The Item array
   DispatchWait**      Wait;        // The Wait array

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
   debugf("%8d LOOPS\n", LOOPS);
   debugf("%8d MULTI\n", MULTI);
   debugf("%8d TASKS\n", TASKS);
// debugf("%8d HANGS\n", HANGS);

   // Create the Task array
   DispatchTask* prior= &FINAL;
   Task= (DispatchTask**)malloc(TASKS * sizeof(DispatchTask*));
   for(int i= TASKS-1; i >=0; i--)
   {
     DispatchTask* task= new PassAlongTask(prior);
     prior= task;
     Task[i]= task;
   }

   // Create the Item and Wait arrays
   Item= (DispatchItem**)malloc(MULTI * sizeof(DispatchItem*));
   Wait= (DispatchWait**)malloc(MULTI * sizeof(DispatchWait*));
   for(int i= 0; i < MULTI; i++)
   {
     Wait[i]= new DispatchWait();
     Item[i]= new DispatchItem(0, Wait[i]);
   }

   // Run the test
   double start= Clock::current();

   for(int loop= 0; loop < LOOPS; loop++)
   {
     for(int multi= 0; multi < MULTI; multi++)
       disp->enqueue(Task[0], Item[multi]);

     for(int multi= 0; multi < MULTI; multi++)
     {
       Wait[multi]->wait();
       Wait[multi]->reset();
     }
   }

   // Test complete
   double elapsed= Clock::current() - start;
   debugf("%8.3f seconds elapsed\n", elapsed);

   // Diagnostics
   disp->debug();

   // Cleanup
   FINAL.reset();
   for(int i= 0; i<TASKS; i++)
   {
     Task[i]->reset();
     delete Task[i];
   }

   for(int i= 0; i<MULTI; i++)
   {
     delete Item[i];
     delete Wait[i];
   }

   free(Wait);
   free(Item);
   free(Task);

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

   Debug::set(new ThreadLogger());  // Initialize ThreadLogger
   IFHCDM( debugSetIntensiveMode(); )
   disp= new Dispatch();

   try {
     if( argc > 1 && strcmp(argv[1], "-time") == 0 )
       result |= testtime(0, NULL);
     else
     {
       if( TRUE  ) result |= test0000(argc, argv);
       if( TRUE  ) result |= test0001(argc, argv);
       if( TRUE  ) result |= testtime(argc, argv);
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

   delete disp;
   debugf("Result(%d)\n", result);
// delete Debug::set(NULL);         // Terminate ThreadLogger (not needed)

   return result;
}

