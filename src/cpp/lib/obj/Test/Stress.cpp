//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Stress.cpp
//
// Purpose-
//       Stress tests.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       Timings use old Thing.imp block sizes and run with Chrome active.
//
//       time Stress ( 20000000, 100000, 10) on bluebird
//          28.672 real     30.997
//        2:43.723 user   2:01.836
//           2.230 sys      10.530
//
//       time Stress (100000000, 100000, 10) on bluebird
//        2:18.018 real   2:35.485
//       13:31.267 user  10:42.162
//           8.845 sys    0:38.453
//
//----------------------------------------------------------------------------
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <random>
#include <string>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "com/Debug.h"
#include "obj/Allocator.h"          // For Allocator, Latch
#include "obj/Array.h"
#include "obj/Object.h"
#include "obj/Ref.h"
#include "obj/String.h"
#include "obj/Thread.h"
using namespace _OBJ_NAMESPACE;

#include "Thing.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define USE_DEBUG_THREAD true       // Operate debug thread?
#define USE_GC_SYNCH     true       // Use garbage collection synchronization?

#define USE_THING_CHECKING
#undef  USE_THING_CHECKING

#define ITERATIONS  100000000
#define THING_COUNT    100000
#define THREAD_COUNT       10

#if false // if true, run short test
#undef  ITERATIONS
#undef  THING_COUNT
#undef  THREAD_COUNT

#define ITERATIONS    1000000
#define THING_COUNT     10000
#define THREAD_COUNT        4
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static size_t          iterations= ITERATIONS;
static size_t          things=     THING_COUNT;
static size_t          threads=    THREAD_COUNT;

int                    readyThreads= 0; // Number of ready TestThreads
static Latch           readyMutex;  // Single threaded initialization
static Latch           gc_mutex;    // Single threaded garbage collection

static std::condition_variable
                       ready_cv;    // Startup condition variable
static std::mutex      cv_mutex;    // Protects ready_cv

// Statistics
static std::atomic<size_t> error_count(0); // Number of errors encountered
static std::atomic<size_t> total_alloc(0); // Number of Ref allocations
static std::atomic<size_t> total_inuse(0); // Number of Refs in use
static std::atomic<size_t> total_delet(0); // Number of Ref deletions

//----------------------------------------------------------------------------
//
// Subroutine-
//       synchronized_gc
//
// Purpose-
//       Run synchronized garbage collection.
//
//----------------------------------------------------------------------------
static inline void
   synchronized_gc( void )          // Run synchronized garbage collection
{
   #if USE_GC_SYNCH
     std::lock_guard<decltype(gc_mutex)> lock(gc_mutex);

     while( Ref::gc() )
       ;
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       tod
//
// Purpose-
//       Current time since epoch as a double
//
//----------------------------------------------------------------------------
static inline double                // Time since epoch
   tod( void )                      // Get current time
{
   int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
       std::chrono::system_clock::now().time_since_epoch() ).count();

   return (double)now / 1000.0;
}

//----------------------------------------------------------------------------
//
// Class-
//       TestThread
//
// Purpose-
//       Threaded stress test instance.
//
//----------------------------------------------------------------------------
class TestThread : public Thread {
//----------------------------------------------------------------------------
// TestThread::Attributes
//----------------------------------------------------------------------------
typedef Array_t<Thing, THING_COUNT>  ARRAY_OBJ;

const std::string      name;        // The Thread's name
bool                   operational= false; // Operational state

// The Thing (reference) array
Ref_t<ARRAY_OBJ>       thing_array;

// Statistical counters
int                    curCount= 0; // Current active Thing count
int                    minCount= 0; // Minimum active Thing count
int                    maxCount= 0; // Maximum active Thing count

size_t                 iteration;   // Current iteration counter
int                    joinFSM= 0;  // Join state

//----------------------------------------------------------------------------
// TestThread::Constructors
//----------------------------------------------------------------------------
public:
   ~TestThread( void )
{  debugf("TestThread::~TestThread %s\n", name.c_str()); }

   TestThread(const char* _name) : Thread(), name(_name)
,  thing_array(new ARRAY_OBJ())
{  if( strcmp(_name,"bringup") != 0 ) start(); }

//----------------------------------------------------------------------------
// TestThread::Methods
//----------------------------------------------------------------------------
void
   debug( void )                    // Debug a thread
{
   debugf("Thread(%s) [%6zd] %6d operational(%s) joinFSM(%d)\n", name.c_str(),
          iteration, curCount, operational ? "true" : "false", joinFSM);
   if( operational )
   {
     debugf("Min/Cur/Max: %d, %d, %d, %7.5f, %7.5f, %7.5f\n",
            minCount, curCount, maxCount,
            (float)minCount/(float)things/0.5,
            (float)curCount/(float)things/0.5,
            (float)maxCount/(float)things/0.5);
   }
}

void
   join( void )                     // Wait for this Thread to complete
{
   joinFSM++;                       // Waiting
   thread.join();
   joinFSM++;                       // Waited
}

void
   _run( void )                     // Test a thread
{
   debugf("Thread(%s) id(%s) started \n", name.c_str(),
          get_id_string().c_str());

   // Statistics
   int                 THREAD0= (name == "000");
   int*                slot= nullptr; // For testing random distribution

   // Initialize the Thing array
   int                 newCount;    // The number of ready threads
   {{{{
       // On many systems, memory allocation requires a thread lock.
       // It's much quicker to initialize one thread at a time.
       std::lock_guard<decltype(readyMutex)> lock(readyMutex);

       for(size_t i= 0; i<(*thing_array).size(); i += 2)
       {
         curCount++;
         minCount++;
         maxCount++;
         (*thing_array)[i]= new Thing(i);
       }

       total_alloc += curCount;
       total_inuse += curCount;

       readyThreads++;
       newCount= readyThreads;
   }}}}
   debugf("%4d Thread(%s) initialization complete\n", __LINE__, name.c_str());
   operational= true;

   if( THREAD0 )
   {
     debugf("%4d HCDM Stress\n", __LINE__); THREAD0= false; // Ignore stats

     size_t size= sizeof(int) * THING_COUNT;
     slot= (int*)malloc(size);
     memset(slot, 0, size);
   }

   if( newCount >= threads )        // If all threads are initialized
   {
     std::unique_lock<decltype(cv_mutex)> lock(cv_mutex);
     ready_cv.notify_all();
   } else {
     std::unique_lock<decltype(cv_mutex)> lock(cv_mutex);
     while( readyThreads != threads )
       ready_cv.wait(lock);
   }

   // Randomizer
   std::random_device  rd;          // Use to seed generator
   std::mt19937        mt(rd());    // Standard mersenne_twister_engine
   std::uniform_int_distribution<>
                       ud(0, (*thing_array).size()-1); // Uniform distributor

   // Randomized object allocation deletion
   for(iteration= 1; iteration<iterations; iteration++)
   {
     #if USE_GC_SYNCH
       {{{{ std::lock_guard<decltype(gc_mutex)> lock(gc_mutex);
       }}}}
     #endif

     if( iteration % (iterations/10) == 0 )
     {
////// synchronized_gc();           // Run garbage collector
       debugf("%4d Thread(%s) iteration %zd\n", __LINE__, name.c_str(),
              iteration);
     }

     size_t ix= ud(mt);             // Select an index
     if( THREAD0 )
       slot[ix]++;

     if( (*thing_array)[ix] == nullptr )  // If not present
     {
       (*thing_array)[ix]= new Thing(ix); // Add it
       curCount++;
       if( curCount > maxCount ) maxCount= curCount;

       total_alloc++;
       total_inuse++;
     } else {                       // If present
       (*thing_array)[ix]->check(__LINE__, ix); // Verify it
       (*thing_array)[ix]= nullptr; // Then remove it
       curCount--;
       if( curCount < minCount ) minCount= curCount;

       total_delet++;
       total_inuse--;
     }
   }

   operational= false;
   debugf("Thread(%s) complete: %d, %d, %d, %7.5f, %7.5f, %7.5f\n",
          name.c_str(), minCount, curCount, maxCount,
          (float)minCount/(float)things/0.5,
          (float)curCount/(float)things/0.5,
          (float)maxCount/(float)things/0.5);
   if( false )                      // (Optional) Debug at completion
   {
     debugf("%4d HCDM\n", __LINE__);
     Ref::debug_static();
     Thing::debug_static();
     Allocator::debug_static();
   }

   if( THREAD0 )
   {
     for(int ix= 0; ix<things; ix+=10)
     {
       debugf("[%8d] %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d\n", ix,
              slot[ix+0], slot[ix+1], slot[ix+2], slot[ix+3], slot[ix+4],
              slot[ix+5], slot[ix+6], slot[ix+7], slot[ix+8], slot[ix+9]);
     }
   }
}

virtual void
   run( void )                      // Test a thread
{
   try {
     _run();
   } catch(Exception& X) {
     error_count++;
     debugf("%4d Thread(%s) catch(%s) what(%s)\n", __LINE__, name.c_str(),
            X.string().c_str(), X.what());
   } catch(std::exception& X) {
     error_count++;
     debugf("%4d Thread(%s) catch(std::exception) what(%s)\n", __LINE__,
            name.c_str(), X.what());
   } catch(...) {
     error_count++;
     debugf("%4d Thread(%s) catch(...)\n", __LINE__, name.c_str());
   }

static Latch one_shot;
   if( error_count.load() && one_shot.try_lock() )
   {
     Ref::debug_static();
     Thing::debug_static();
     Allocator::debug_static();
   }
}
}; // class TestThread

// The test thread array
static Array_t<TestThread, THREAD_COUNT> thread_array; // The Thread array

//----------------------------------------------------------------------------
//
// Class-
//       DebugThread
//
// Purpose-
//       Threaded stress test debugging thread.
//
//----------------------------------------------------------------------------
class DebugThread : public Thread {
//----------------------------------------------------------------------------
// DebugThread::Attributes
//----------------------------------------------------------------------------
public:
int                    operational= true; // Thread operational

// Controls used to synchronize termination
std::mutex             fg_mutex;    // Protects fg_cv
std::condition_variable
                       fg_cv;       // The foreground condition variable

//----------------------------------------------------------------------------
// DebugThread::Constructors
//----------------------------------------------------------------------------
public:
   DebugThread( void ) : Thread()
,  fg_mutex(), fg_cv()
{  start();
}

//----------------------------------------------------------------------------
// DebugThread::Methods
//----------------------------------------------------------------------------
void
   terminate( void )                // Terminate the thread
{
   operational= false;

   std::unique_lock<decltype(fg_mutex)> lock(fg_mutex);
   fg_cv.notify_all();
}

void
   _run( void )                     // Background debugging thread
{
   debugf("%4d Stress DebugThread started\n", __LINE__);

   // Operate the thread
   while( operational )
   {
     debugf("%4d Stress DebugThread waiting:\n", __LINE__);
     std::unique_lock<decltype(fg_mutex)> lock(fg_mutex);
     std::cv_status timeout= fg_cv.wait_for(lock, std::chrono::seconds(60));
     if( timeout == std::cv_status::timeout )
     {
       debugf("%4d Stress DebugThread status:\n", __LINE__);
       Ref::debug_static();
       Thing::debug_static();
       Allocator::debug_static();

#if 0  // Hard Core Debug Mode
       for(std::size_t i= 0; i<thread_array.size(); i++)
       {
         Ref_t<TestThread> ref= thread_array[i];
         if( ref.get() )
           ref->debug();
         else
           debugf("TestThread[%zd] deleted\n", i);
       }
#endif

       // Implementation note: This GC invocation is intended to limit the
       // number of extended pages in the RefLink and Thing Allocators.
       synchronized_gc();           // Run garbage collector
     }
   }

   debugf("%4d Stress DebugThread exiting\n", __LINE__);
}

virtual void
   run( void )                      // Test a thread
{
   try {
     _run();
   } catch(Exception& X) {
     error_count++;
     debugf("%4d catch(%s) what(%s)\n", __LINE__, X.string().c_str(), X.what());
   } catch(std::exception& X) {
     error_count++;
     debugf("%4d catch(std::exception) what(%s)\n", __LINE__, X.what());
   } catch(...) {
     error_count++;
     debugf("%4d catch(...)\n", __LINE__);
   }
}
}; // class DebugThread

#if USE_DEBUG_THREAD
static DebugThread     debugThread; // Our debug thread
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Thread
//
// Purpose-
//       Instantiate the test Threads.
//
//----------------------------------------------------------------------------
static int                          // Number of errors encountered
   test_Thread( void )              // Test Thread.h
{
   debugf("Now testing Thread.h\n");
   size_t              errorCount= 0; // Error counter

   debugf("Main: %s\n", Thread::get_id_string(std::this_thread::get_id()).c_str());

   // Initialize the Thing pool
   Thing* thing= new Thing();
   delete thing;
   Thing::debug_static();

   // Get starting object count
   size_t old_object_count= Ref::get_object_count();
   debugf("%3zd= get_object_count@%4d\n", Ref::get_object_count(), __LINE__);

   // Optional: Single-threaded test
   if( threads == 0 ) {
     TestThread bringup("bringup");
     bringup.run();

     // Static debugging
     Ref::debug_static();
     Thing::debug_static();
     Allocator::debug_static();
     return 0;
   }

   //-------------------------------------------------------------------------
   // Multi-threaded stress test
   //-------------------------------------------------------------------------
   {{{{
     // Start all the TestThreads
     for(std::size_t i= 0; i<thread_array.size(); i++)
     {
       char buffer[32];
       sprintf(buffer, "%.3zd", i);

       thread_array[i]= new TestThread(buffer);
     }

     // Wait for all the TestThreads to complete
     for(std::size_t i= 0; i<thread_array.size(); i++)
       thread_array[i]->join();

     debugf("** All TestThreads completed **\n");
     for(std::size_t i= 0; i<thread_array.size(); i++)
       thread_array[i]= nullptr;
   }}}}

   // Complete any pending garbage collection
   Ref::debug_static();
   Thing::debug_static();
   Allocator::debug_static();

   double then= tod();
   debugf("%14.3f GC Started..  %zd\n", then, total_inuse.load());
   while( Ref::gc() )
     ;
   double now= tod();
   debugf("%14.3f ..GC Complete %zd\n", now, Ref::get_object_count());
   debugf("%14.3f GC Elapsed\n", now - then);

   debugf("Totals: %zd, %zd, %zd\n",
          total_alloc.load(), total_inuse.load(), total_delet.load());

   // Verify garbage collection completed
   size_t new_object_count= Ref::get_object_count();
   if( new_object_count != old_object_count )
   {
     errorCount++;
     debugf("ERROR: Ref::get_object_count(%zd) old_object_count(%zd)\n",
            new_object_count, old_object_count);
   }

   #if USE_DEBUG_THREAD
     debugf("%4d Stress Terminating debugThread..\n", __LINE__);
     debugThread.terminate();
     debugThread.join();
     debugf("%4d Stress ..Terminated debugThread\n", __LINE__);
   #endif

   // Static debugging
   Ref::debug_static();
   Thing::debug_static();
   Allocator::debug_static();

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Include-
//       Thing.imp
//
// Purpose-
//       The static Thing implementation.
//
//----------------------------------------------------------------------------
#ifdef USE_THING_CHECKING
#undef USE_DEBUG_MESSAGES
static Latch           stress_oneShot;
void
   thing_check(size_t count)        // Called from Thing::operator new
{
   if( (count % 1048596) != 0 )     // If thing object list is not growing
     return;

   if( stress_oneShot.lock_word == 0 && stress_oneShot.try_lock() )
   {
     #ifdef USE_DEBUG_MESSAGES
       debugf("\n\n******** ALLOCATION OVERFLOW %8zd\n", count);
       Ref::debug_static();
       Thing::debug_static();

       for(int i= 0; i<threads; i++)
         thread_array[i]->debug();
     #endif

     // Place holder

     #ifdef USE_DEBUG_MESSAGES
       debugf("\n\n........ ALLOCATION OVERFLOW %8zd\n\n\n",
              Thing::get_allocated());
     #endif

     stress_oneShot.unlock();
   }
}
#endif // USE_THING_CHECKING implementation

#include "Thing.imp"                // Define static Thing attributes

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Print a description of what this program does, then exit.
//
//----------------------------------------------------------------------------
static void
   info( void )                     // Accumulate a name
{
   fprintf(stderr, "Stress Options {iterations {threads {things}}}\n");
   fprintf(stderr, "Options:\n"
           "  -h  (Write this help message)\n"
////       "  -v  (Verbose)\n"
          );

   exit(EXIT_FAILURE);
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
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 count= 0;    // Non-switch argument count
   int                 error= false; // Error encountered indicator
// int                 verbose = false; // Verbose indicator

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   for(int argi=1; argi<argc; argi++)
   {
     char* argp= argv[argi];

     if( *argp == '-' )             // If switch format
     {
       argp++;                      // Skip over the switch char

       if( strcmp(argp, "h") == 0 || strcmp(argp, "H") == 0 )
         info();

//     if( strcmp(argp, "h") == 0 || strcmp(argp, "H") == 0 )
//       verbose= true;
//     else
       {
         error= true;
         fprintf(stderr, "Invalid parameter '%s'\n", argv[argi]);
       }
     }
     else
     {
       switch(count)
       {
         case 0:
           if( strcmp(argp, "long") == 0 )
             iterations= 100000000;
           else if( strcmp(argp, "short") == 0 )
             iterations= 20000000;
           else
             iterations= atoi(argp);
           break;

         case 1:
           threads= atoi(argp);
           if( threads > THREAD_COUNT )
           {
             error= true;
             fprintf(stderr, "threads(%zd) > maximum(%d)\n", threads, THREAD_COUNT);
           }
           break;

         case 2:
           things= atoi(argp);
           if( things < 16 )
           {
             error= true;
             fprintf(stderr, "things(%zd) < minimum(%d)\n", things, 16);
           }
           else if( things > THING_COUNT )
           {
             error= true;
             fprintf(stderr, "things(%zd) > maximum(%d)\n", things, THING_COUNT);
           }
           break;

         default:
           error= true;
           fprintf(stderr, "Unexpected parameter: '%s'\n", argp);
           break;
       }

       count++;
     }
   }

   //-------------------------------------------------------------------------
   // Completion analysis
   //-------------------------------------------------------------------------
   if( error )                      // If error encountered
     info();
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
     int             argc,          // Argument count
     char*           argv[])        // Argument list
{
   Debug debug;
   debugSetIntensiveMode();

   parm(argc, argv);                // Argument analysis
   debugf("STRESS Iterations(%zd) Threads(%zd) Things(%zd)\n",
          iterations, threads, things);

   // Diagnostic information
   debugf("%8zd= sizeof(Ref)\n", sizeof(Ref));
   debugf("%8zd= sizeof(Ref_t<Thing>)\n", sizeof(Ref_t<Thing>));
   debugf("%8zd= sizeof(Thing)\n", sizeof(Thing));
   size_t storage= sizeof(Ref_t<Thing>) * things * threads;
   storage += (sizeof(Thing) * things * threads) / 2;
   debugf("%zd Storage usage\n", storage);

   // Run the stress test
   double then= tod();
   debugf("%14.3f TC Started..\n", then);
   error_count += test_Thread();
   double now= tod();
   debugf("%14.3f ..TC Complete\n", now);
   debugf("%14.3f ..TC Elapsed\n", now - then);
   if( error_count.load() == 0 )
     debugf("NO Errors\n");
   else if( error_count.load() == 1 )
     debugf(" 1 Error\n");
   else
     debugf("%2zd Errors\n", error_count.load());

   // Diagnostic wait
   if( false )
   {
     debugf("Delay 10 seconds\n");
     std::this_thread::sleep_for(std::chrono::seconds(10));
     Ref::debug_static();
     Thing::debug_static();
   }

   return (error_count.load() != 0);
}

