//----------------------------------------------------------------------------
//
//       Copyright (c) 2021 Frank Eskesen.
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
//       Stress and timing test, comparing Ref/Object and std::shared_ptr
//
// Last change date-
//       2021/08/06
//
// Usage-
//       Stress {iterations {threads {things}}}
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

#include <com/Debug.h>
#include <obj/Latch.h>
#include <obj/Thread.h>

using namespace _OBJ_NAMESPACE;

#include "Thing.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= true                       // Hard Core Debug Mode?
}; // Generic enum

#define USE_DEBUG_THREAD false      // Operate debug thread?
#define USE_GC_SYNCH     true       // Use garbage collection synchronization?
#define THREAD_ARRAY       32       // Pre-defined thread array size

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
static double          started;     // Test start time
static double          elapsed;     // Test elapsed time

static size_t          iterations= ITERATIONS;
static size_t          things=     THING_COUNT;
static size_t          threads=    THREAD_COUNT;

static size_t          readyThreads= 0; // Number of ready TestThreads
static Latch           readyMutex;  // Single threaded initialization
static Latch           gc_mutex;    // Single threaded garbage collection

static std::condition_variable
                       ready_cv;    // Startup condition variable
static std::mutex      cv_mutex;    // Protects ready_cv

// Statistics
static std::atomic<size_t> error_count(0); // Number of errors encountered
static std::atomic<size_t> total_alloc(0); // Number of allocations
static std::atomic<size_t> total_inuse(0); // Number of in use elements
static std::atomic<size_t> total_delet(0); // Number of deallocations

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
   #ifdef USE_THING_OBJ
     std::lock_guard<decltype(gc_mutex)> lock(gc_mutex);

     while( obj::Ref::gc() )
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
const std::string      name;        // The Thread's name
bool                   operational= false; // Operational state

// The Thing (reference) array
typedef std::array<THING_PTR, THING_COUNT> ARRAY_OBJ;
ARRAY_OBJ*             thing_array;

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
{  debugf("TestThread::~TestThread %s\n", name.c_str());
   empty();
}

   TestThread(const char* _name) : Thread(), name(_name)
,  thing_array(new ARRAY_OBJ())
{  if( strcmp(_name,"bringup") != 0 ) start(); }

//----------------------------------------------------------------------------
// TestThread::Methods
//----------------------------------------------------------------------------
void
   debug( void )                    // Debug a thread
{
   debugf("Thread(%s) [%'6zd] %'6d operational(%s) joinFSM(%d)\n", name.c_str()
         , iteration, curCount, operational ? "true" : "false", joinFSM);
   if( operational ) {
     debugf("Min/Cur/Max: %d, %d, %d, %7.5f, %7.5f, %7.5f\n",
            minCount, curCount, maxCount,
            (float)minCount/(float)things/0.5,
            (float)curCount/(float)things/0.5,
            (float)maxCount/(float)things/0.5);
   }
}

void
   empty( void )                    // Release all elements
{
   for(int ix= 0; ix<THING_COUNT; ix++) {
     if( (*thing_array)[ix] != nullptr ) { // If present
       (*thing_array)[ix]->check(__LINE__, ix); // Verify it
       (*thing_array)[ix]= nullptr; // Then remove it

       total_delet++;
       total_inuse--;
     }
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

   // Statistics, for random distribution verification
   int                 slot_stats= false;
   int*                slot= nullptr;

   if( name == "000" && false ) {   // (Random distribution already verified)
     size_t size= sizeof(int) * THING_COUNT;
     slot= (int*)malloc(size);
     if( slot ) {
       memset(slot, 0, size);
       slot_stats= true;
       debugf("%4d HCDM Slot statistics enabled\n", __LINE__);
     }
   }

   // Initialize the Thing array
   size_t              newCount;    // The number of ready threads
   {{{{
       // On many systems, memory allocation requires a thread lock.
       // It's much quicker to lock once and initialize one thread at a time.
       std::lock_guard<decltype(readyMutex)> lock(readyMutex);

       for(size_t i= 0; i<(*thing_array).size(); i += 2) {
         curCount++;
         minCount++;
         maxCount++;
         (*thing_array)[i]= MAKE_THING(i);
       }

       total_alloc += curCount;
       total_inuse += curCount;

       readyThreads++;
       newCount= readyThreads;
   }}}}
   debugf("%4d Thread(%s) initialization complete\n", __LINE__, name.c_str());
   operational= true;

   if( newCount >= threads ) {      // If all threads are initialized
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

   //-------------------------------------------------------------------------
   // Randomized object allocation/deletion
   for(iteration= 1; iteration<iterations; iteration++) {
     if( iteration % (iterations/10) == 0 ) {
       debugf("%4d Thread(%s) iteration %'zd\n", __LINE__, name.c_str()
             , iteration);
     }

     size_t ix= ud(mt);             // Select an index
     if( slot_stats )
       slot[ix]++;

     if( (*thing_array)[ix] == nullptr ) { // If not present
       (*thing_array)[ix]= MAKE_THING(ix);
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

   if( slot_stats ) {
     for(size_t ix= 0; ix<things; ix+=10) {
       debugf("[%8zd] %4d %4d %4d %4d %4d %4d %4d %4d %4d %4d\n", ix,
              slot[ix+0], slot[ix+1], slot[ix+2], slot[ix+3], slot[ix+4],
              slot[ix+5], slot[ix+6], slot[ix+7], slot[ix+8], slot[ix+9]);
     }

     free(slot);
   }
}

virtual void
   run( void )                      // Test a thread
{
   try {
     _run();
   } catch(Exception& X) {
     error_count++;
     debugf("%4d Thread(%s) catch(%s) what(%s)\n", __LINE__, name.c_str()
           , X.string().c_str(), X.what());
   } catch(std::exception& X) {
     error_count++;
     debugf("%4d Thread(%s) catch(std::exception) what(%s)\n", __LINE__
           , name.c_str(), X.what());
   } catch(...) {
     error_count++;
     debugf("%4d Thread(%s) catch(...)\n", __LINE__, name.c_str());
   }

   // If error, first completed thread displays some diagnostic information
   static Latch one_shot;
   if( error_count.load() && one_shot.try_lock() ) {
     #ifdef USE_THING_OBJ
       obj::Ref::debug_static();
     #endif
     Thing::debug_static();
   }
}
}; // class TestThread

// The test thread array
TestThread*            thread_array[THREAD_ARRAY];

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
{  start(); }

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
   while( operational ) {
     debugf("%4d Stress DebugThread waiting:\n", __LINE__);
     std::unique_lock<decltype(fg_mutex)> lock(fg_mutex);
     std::cv_status timeout= fg_cv.wait_for(lock, std::chrono::seconds(60));
     if( timeout == std::cv_status::timeout ) {
       debugf("%4d Stress DebugThread status:\n", __LINE__);
       #ifdef USE_THING_OBJ
         obj::Ref::debug_static();
       #endif
       Thing::debug_static();

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
     debugf("%4d catch(%s) what(%s)\n", __LINE__
           , X.string().c_str(), X.what());
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
   size_t              errorCount= 0; // Error counter

   debugf("Main: %s\n", Thread::get_id_string(std::this_thread::get_id()).c_str());

   // Initialize the Thing pool
   Thing* thing= new Thing();
   delete thing;
   Thing::debug_static();

   // Optional: Single-threaded test
   if( threads == 0 ) {
     TestThread bringup("bringup");
     bringup.run();

     // Static debugging
     #ifdef USE_THING_OBJ
       obj::Ref::debug_static();
     #endif
     Thing::debug_static();
     return 0;
   }

   //-------------------------------------------------------------------------
   // Multi-threaded stress test
   //-------------------------------------------------------------------------
   debugf("%14.3f Multi-thread started..\n", tod() - started);
   started= tod();                  // (Reset the clock)
   {{{{
     // Start all the TestThreads
     for(std::size_t i= 0; i<threads; i++) {
       char buffer[32];
       sprintf(buffer, "%.3zd", i);

       thread_array[i]= new TestThread(buffer);
     }

     // Wait for all the TestThreads to complete
     for(std::size_t i= 0; i<threads; i++)
       thread_array[i]->join();
     debugf("%14.3f ** All TestThreads completed **\n", tod());
     elapsed= tod() - started;
   }}}}

   // Complete any pending garbage collection
   Thing::debug_static();
   #ifdef USE_THING_OBJ
     debugf("\n\n");
     debugf("%14.3f ..Elapsed\n", tod() - started);
     debugf("Totals: alloc(%'zd), inuse(%'zd), delet(%'zd)\n",
            total_alloc.load(), total_inuse.load(), total_delet.load());
     obj::Ref::debug_static();
     for(std::size_t i= 0; i<threads; i++) {
       thread_array[i]->empty();
       thread_array[i]= nullptr;
     }

     debugf("\n\n");
     double then= tod();
     debugf("%14.3f ..Elapsed (thread empty)\n", then - started);
     debugf("%14.3f GC Started..  %'zd\n", then, total_inuse.load());
     while( obj::Ref::gc() )
       ;
     double now= tod();
     debugf("%14.3f ..GC Complete %'zd\n", now, obj::Ref::get_object_count());
     debugf("%14.3f ..GC Time\n", now - then);

     debugf("Totals: alloc(%'zd), inuse(%'zd), delet(%'zd)\n",
            total_alloc.load(), total_inuse.load(), total_delet.load());
   #else
     for(std::size_t i= 0; i<threads; i++) {
       thread_array[i]->empty();
       thread_array[i]= nullptr;
     }
     debugf("%14.3f ..Elapsed (thread empty)\n", tod() - started);
   #endif
   Thing::deallocate_all();
   debugf("%14.3f ..Elapsed (deallocate_all)\n", tod() - started);
   Thing::debug_static();

   #if USE_DEBUG_THREAD
     debugf("%4d Stress Terminating debugThread..\n", __LINE__);
     debugThread.terminate();
     debugThread.join();
     debugf("%4d Stress ..Terminated debugThread\n", __LINE__);
   #endif

   return errorCount;
}

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
   for(int argi=1; argi<argc; argi++) {
     char* argp= argv[argi];

     if( *argp == '-' ) {           // If switch format
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
     } else {
       switch(count) {
         case 0:
           if( strcmp(argp, "long") == 0 )
             iterations= 100000000;
           else if( strcmp(argp, "short") == 0 )
             iterations= 20000000;
           else
             iterations= atoi(argp);
           if( iterations < 10 )
             iterations= 10;
           break;

         case 1:
           threads= atoi(argp);
           if( threads > THREAD_ARRAY ) {
             error= true;
             fprintf(stderr, "threads(%zd) > maximum(%d)\n", threads
                           , THREAD_ARRAY);
           }
           break;

         case 2:
           things= atoi(argp);
           if( things < 16 ) {
             error= true;
             fprintf(stderr, "things(%zd) < minimum(%d)\n", things, 16);
           } else if( things > THING_COUNT ) {
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
   parm(argc, argv);                // Argument analysis
   debugf("STRESS Iterations(%'zd) Threads(%'zd) Things(%'zd)\n",
          iterations, threads, things);

   debugSetIntensiveMode();
   setlocale(LC_NUMERIC, "");       // Allows printf("%'zd\n", ...

   // Diagnostic information
   size_t storage= 0;
   debugf("%8zd= sizeof(Thing)\n", sizeof(Thing));
   storage += (sizeof(Thing) * things * threads) / 2;
   debugf("%'zd Storage usage\n", storage);

   // Run the stress test
   double started= tod();           // Test start time (local variable)
   debugf("%14.3f TC Started..\n", started);
   error_count += test_Thread();
   double now= tod();
   debugf("%14.3f ..TC Complete, %zd x %'zd\n", now, threads, iterations);
   double ops= (double)threads * (double)iterations;
   debugf("%14.3f ..TC Elapsed (test), %.1f Mops/sec\n", elapsed
         , (ops/elapsed)/1000000.0);
   debugf("%14.3f ..TC Elapsed (total)\n", now - started);
   if( error_count.load() == 0 )
     debugf("NO Errors\n");
   else if( error_count.load() == 1 )
     debugf(" 1 Error\n");
   else
     debugf("%2zd Errors\n", error_count.load());

   // Diagnostic wait
   if( false ) {
     debugf("Delay 10 seconds\n");
     std::this_thread::sleep_for(std::chrono::seconds(10));
     Thing::debug_static();
   }

   return (error_count.load() != 0);
}
