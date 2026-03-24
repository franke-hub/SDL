//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2026 Frank Eskesen.
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
//       Test_thr.cpp
//
// Purpose-
//       Test Thread function.
//
// Last change date-
//       2026/03/23
//
// Implementation notes-
//       We don't trace anything but can create a trace table for library use.
//
//----------------------------------------------------------------------------
#include <exception>                // For std::exception
#include <functional>               // For std::function
#include <iostream>                 // For std::cout, std::cerr
#include <memory>                   // For std::shared_ptr, ...
#include <mutex>                    // For std::lock_guard
#include <cstdio>                   // For printf
#include <cstdlib>                  // For random

#include <pub/Debug.h>              // For pub::debugging methods
#include "pub/Event.h"              // For pub::Event
#include "pub/Exception.h"          // For pub::Exception
#include <pub/Interval.h>           // For pub::Interval (performance debugging)
#include "pub/mutex.h"              // For pub::mutex
#include "pub/Named.h"              // For pub::Named (Threads)
#include "pub/Semaphore.h"          // For pub::Semaphore
#include "pub/System.h"             // For pub::System
#include "pub/Thread.h"             // For pub::Thread
#include "pub/Trace.h"              // For pub::Trace (when debugging)
#include "pub/Worker.h"             // For pub::Worker
#include "pub/Wrapper.h"            // For pub::Wrapper
#include "pub/utility.i"            // For pub::utility conversion routines

#define PUB _LIBPUB_NAMESPACE
using PUB::s2c;                     // String to char* utility
using PUB::Debug;
using PUB::Event;
using PUB::Exception;
using PUB::Interval;
using PUB::mutex;
using PUB::Named;
using PUB::Semaphore;
using PUB::Trace;                   // (When debugging)
using PUB::Thread;
using PUB::Worker;
using PUB::WorkerPool;
using PUB::Wrapper;
using namespace PUB::debugging;
using namespace PUB::System;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more

,  MAXDELETES=  16                  // Number of self-delete threads
,  MAXNOISY=  1000                  // Number of noisy threads
,  MAXQUIET= 25000                  // Number of quiet threads
,  TIMING=       1                  // Number of timing loops to run
}; // Generic enum

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static pub::mutex      alphaMutex;
static pub::mutex      betaMutex;
static Semaphore       alphaSemaphore(1);
static Semaphore       betaSemaphore(1);
static Semaphore       timedSemaphore(0);
static int             error_count= 0;
static Interval        interval;
static double          noisy_delay= 0.001; // Default noisy delay

static void*           table= nullptr; // The Trace table

//----------------------------------------------------------------------------
// Extended options
//----------------------------------------------------------------------------
////// int             opt_hcdm= HCDM;       // (Wrapper built-in)
////// int             opt_verbose= VERBOSE; // (Wrapper built-in)
static int             opt_trace= 0; // --trace
static int             opt_block= 0; // --block
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"********", required_argument, nullptr,    0} // --(ignored)
,  {"block",    no_argument,       &opt_block, 1} // --block
,  {"delay",    required_argument, nullptr,    0} // --delay
,  {"trace",    optional_argument, &opt_trace, 0x0040'0000} // --trace
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
//
// Class
//       BasicThread
//
// Purpose-
//       Define our base Thread
//
// Implementation note-
//       Method start() does not return until running() has been invoked
//
//----------------------------------------------------------------------------
class BasicThread : public Thread, public Named { // Basic Thread class
Event                  is_running;  // (Set once, never reset)
Event                  is_started;  // (Set once, never reset)

public:
   BasicThread(const char* name= "Basic")
:  Thread(), Named(name), is_running(), is_started()
{  }

void
   running( void )
{  is_running.post(); is_started.wait(); }

void
   start( void )
{
   Thread::start();
   is_running.wait();
   is_started.post();
}
}; // class BasicThread

//----------------------------------------------------------------------------
//
// Class-
//       LambdaWorker
//
// Purpose-
//       Generic worker, used to create abnormal Thread termination conditions
//
//----------------------------------------------------------------------------
class LambdaWorker : public Worker {
typedef std::function<void(void)>   Function;
const Function         function;    // The Lambda function

public:
   LambdaWorker(                    // Constructor
     const Function&   _F)          // The Lambda function
:  Worker(), function(_F)
{  }

virtual void
   work( void )
{  function(); }                    // Invoke the Lambda function
}; // class LambdaWorker

//----------------------------------------------------------------------------
//
// Class-
//       LoopyThread
//
// Purpose-
//       Define a stoppable Thread
//
// Implementation notes-
//       Used for abnormal Thread termination testing.
//
//----------------------------------------------------------------------------
class LoopyThread : public BasicThread {
public:
bool                   operational= false; // TRUE while operational
bool                   self_delete= false; // Self-delete option
bool                   self_detach= false; // Self-detach option

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   LoopyThread( void )
:  BasicThread("LoopyThread")
{  if( opt_verbose > 1 ) debugf("LoopyThread(%p)!\n", this); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual
   ~LoopyThread( void )
{  if( opt_verbose > 1 ) debugf("LoopyThread(%p)~\n", this); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
virtual void
   run(void)
{
   operational= true;
   running();

   if( opt_verbose > 2 )
     debugf("LoopyThread(%p).run...\n", this);

   while( operational ) {
     Thread::sleep(1.0);
   }

   if( self_detach ) {
     if( opt_verbose > 1 )
       debugf("LoopyThread(%p):self-detach\n", this);

     detach();
   }

   if( self_delete ) {
     if( opt_verbose > 1 )
       debugf("LoopyThread(%p)::self-delete\n", this);

      delete this;
   }

   if( opt_verbose > 2 )
     debugf("LoopyThread(%p)...run\n", this);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   drive_destructor( void )         // Asynchronously drive the destructor
{
   LambdaWorker lw([this](void) {
     this->~LoopyThread();          // Asynchonously delete the LoopyThread
   }); // LambdaWorker

   WorkerPool::work(&lw);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   set_self_delete(void)
{  self_delete= true; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   set_self_detach(void)
{  self_detach= true; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   stop( void )
{  if( opt_verbose )
     debugf("LoopyThread(%p).stop\n", this);

   operational= false;
}
}; // class LoopyThread

//----------------------------------------------------------------------------
//
// Struct-
//       LoopyController
//
// Purpose-
//       Control a LoopyThread
//
// Implementation notes-
//       Used for abnormal Thread termination testing.
//
//----------------------------------------------------------------------------
struct LoopyController {
   LoopyThread*        loopy= nullptr; // The LoopyThread
   Event               init;        // Initial Event
   Event               stop;        // Stop Event

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   LoopyController( void )
:  loopy(new LoopyThread())
{  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   ~LoopyController( void )
{  delete loopy; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   post( void )                     // Post the stop Event
{  stop.post(); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   wait( void )                     // Wait for stop Event
{  stop.wait(); }
}; // struct LoopyController

//----------------------------------------------------------------------------
//
// Class
//       MutexThread
//
// Purpose-
//       Define a Thread used solely to test the Mutex object.
//
//----------------------------------------------------------------------------
class MutexThread : public BasicThread { // Mutex Thread class
public:
   MutexThread()
:  BasicThread("MutexThread") { }

virtual void
   run(void)
{
   running();

   if( opt_verbose )
     debugh("Before betaMutex.lock()\n");
   betaMutex.lock();

   {{{{
     if( opt_verbose )
       debugh("Before alphaMutex.lock()\n");
     std::lock_guard<decltype(alphaMutex)> lock(alphaMutex);

     if( opt_verbose )
       debugh("Before alphaMutex.unlock()\n");
   }}}}

   if( opt_verbose )
     debugh("sleep(1.0)...\n");
   Thread::sleep(1.0);
   if( opt_verbose )
     debugh("...sleep(1.0)\n");

   if( opt_verbose )
     debugh("Before betaMutex.unlock()\n");
   betaMutex.unlock();

   if( opt_verbose )
     debugh("done!\n");
}
}; // class MutexThread

//----------------------------------------------------------------------------
//
// Class
//       NoisyThread
//
// Purpose-
//       Define a simple, but noisy, Thread
//
//----------------------------------------------------------------------------
class NoisyThread : public BasicThread { // Noisy Thread class
protected:
double                 delay;       // Delay before exit

public:
   NoisyThread(
     const char*       threadName,  // The Thread name
     double            delay= 0.001) // The exit delay
:  BasicThread(threadName)
,  delay(delay)
{  }

virtual void
   run(void)
{
   running();

   if( opt_verbose > 2 )
     debugf("%12.6f NoisyThread(%p).run(%s)\n", interval.stop(), this
           , get_name().c_str());

   // Sleep (allow possible thread deletion)
   Thread::sleep(delay);

#if 0
   // Terminate, display current
   Thread* thread= Thread::current();
   debugf("%12.6f NoisyThread(%p).exit(%s) %s\n", interval.stop(), this,
          get_name().c_str(), this == thread ? "SAME" : "DIFF");
   fflush(stdout);
#endif
}
}; // class NoisyThread

//----------------------------------------------------------------------------
//
// Class
//       QuietThread
//
// Purpose-
//       Define a simple, but quiet, Thread
//
// Implementatin notes-
//       This Thread should not wait for start completion.
//
//----------------------------------------------------------------------------
class QuietThread : public Thread { // Quiet Thread class
public:
   QuietThread()
:  Thread()
{  }

virtual void
   run(void)
{
   Thread* thread= Thread::current();
   if (this != thread ) {
     error_count++;
     debugf("%4d ERROR: Thread(%p) Current(%p)\n", __LINE__, this, thread);
   }
}
}; // class QuietThread

//----------------------------------------------------------------------------
//
// Class
//       SelfDeletingThread
//
// Purpose-
//       This Thread is designed to run after being deleted.
//
//----------------------------------------------------------------------------
class SelfDeletingThread : public BasicThread { // SelfDeleting Thread class
public:
   SelfDeletingThread( void )
:  BasicThread("SelfDeleting")
{  }

virtual void
   run(void)
{
   running();

   if( opt_verbose > 2 )
     debugf("%12.6f SelfDeletingThread(%p).run()\n", interval.stop(), this);

   Thread* thread= Thread::current();  // The current Thread
   if( thread != this )                // This MUST BE correct
   {
     error_count++;
     debugf("%4d ERROR: SelfDeletingThread(%p) Current(%p)\n", __LINE__,
            this, thread);
     ::exit(EXIT_FAILURE);
   }

   // Test Thread::current
   thread= Thread::current();
   if( thread != this ) {
     error_count++;
     debugf("%4d ERROR: SelfDeletingThread(%p) != Current(%p)\n"
           , __LINE__, this, thread);
   }

   detach();
   thread= Thread::current();
   if( thread != this ) {           // Current should work when detached
     error_count++;
     debugf("%4d ERROR: SelfDeletingThread(%p) != Current(%p)\n", __LINE__
           , this, thread);
   }

   // After delete, don't reference the object
   delete this;

// This pragma shouldn't be necessary.
// We only use this as an  address. We don't use this-> anything.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wuse-after-free"
   thread= Thread::current();
   if( thread != nullptr ) {        // Current should not work when deleted
     error_count++;
     debugf("%4d ERROR: SelfDeletingThread(%p) Current(%p) != nullptr\n"
           , __LINE__, this, thread);
   }

   // Show work being done on a deleted Thread
   if( opt_verbose > 2 ) {
     debugf("%12.6f SelfDeletingThread(%p) exit\n", interval.stop(), this);
     fflush(stdout);
   }
#pragma GCC diagnostic pop
}
}; // class SelfDeletingThread

//----------------------------------------------------------------------------
//
// Class
//       SemaphoreThread
//
// Purpose-
//       Define a Thread used solely to test the Semaphore object.
//
//----------------------------------------------------------------------------
class SemaphoreThread : public BasicThread { // Semaphore Thread class
public:
   SemaphoreThread()
:  BasicThread("SemaphoreThread")
{  }

virtual void
   run(void)
{
   running();

   if( opt_verbose )
     debugh("Before betaSemaphore.wait()\n");
   betaSemaphore.wait();

   if( opt_verbose )
     debugh("Before alphaSemaphore.wait()\n");
   alphaSemaphore.wait();

   if( opt_verbose )
     debugh("Before alphaSemaphore.post()\n");
   alphaSemaphore.post();

   if( opt_verbose )
     debugh("sleep(1.0)...\n");
   Thread::sleep(1.0);
   if( opt_verbose )
     debugh("...sleep(1.0)\n");

   if( opt_verbose )
     debugh("Before betaSemaphore.post()\n");
   betaSemaphore.post();

   if( opt_verbose )
     debugh("Before timedSemaphore.wait(3.5)...\n");
   int rc= timedSemaphore.wait(3.5);
   if( opt_verbose )
     debugh("...%d= timedSemaphore.wait()\n", rc);

   if( opt_verbose )
     debugh("done!\n");
}
}; // class SemaphoreThread

//----------------------------------------------------------------------------
//
// Class
//       SleepThread
//
// Purpose-
//       Define a Thread used solely to test the Thread::sleep() function
//
//----------------------------------------------------------------------------
class SleepThread : public BasicThread { // Sleep Thread class
public:
   SleepThread()
:  BasicThread("SleepThread") { }

virtual void
   run(void)
{
   running();

   if( opt_verbose )
     debugh("Before sleep(1.234)\n");
   sleep(1.234);
   if( opt_verbose )
     debugh("*After sleep(1.234)\n");
}
}; // class SleepThread

//----------------------------------------------------------------------------
//
// Class
//       StandardThread
//
// Purpose-
//       Simple standard Thread. Verifies Thread::current()
//
//----------------------------------------------------------------------------
class StandardThread : public BasicThread { // Standard Thread class
public:
   StandardThread( void )
:  BasicThread("Standard")
{  }

virtual void
   run(void)
{
   running();

   if( opt_verbose > 2 )
     debugf("%12.6f StandardThread(%p).run()\n", interval.stop(), this);

   Thread* current= Thread::current(); // The current Thread
   if( current != this )            // This MUST BE correct
   {
     error_count++;
     debugf("%4d ERROR: StandardThread(%p) Current(%p)\n", __LINE__,
            this, current);
     ::exit(EXIT_FAILURE);
   }

   // Wait a little bit
   Thread::sleep(0.0125);

   current= Thread::current();
   if( current != this )            // This MUST BE correct
   {
     error_count++;
     debugf("%4d ERROR: StandardThread(%p) Current(%p)\n", __LINE__,
            this, current);
     ::exit(EXIT_FAILURE);
   }

   if( opt_verbose > 2 )
     debugf("%12.6f StandardThread(%p) exit\n", interval.stop(), this);
}
}; // class StandardThread

//----------------------------------------------------------------------------
//
// Subroutine-
//       selfDeletingThread
//
// Purpose-
//       Insure that a thread can complete even though the corresponding
//       object is deleted.
//
//----------------------------------------------------------------------------
static void
   selfDeletingThread(void)
{
   SelfDeletingThread* thread= new SelfDeletingThread();
   thread->start();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       standardThread
//
// Purpose-
//       Verify the correct operation of a standard Thread.
//
//----------------------------------------------------------------------------
static inline void
   standardThread(void)
{
   StandardThread standardThread;
   standardThread.start();
   standardThread.join();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Mutex
//
// Purpose-
//       Test mutual exclusion object.
//
//----------------------------------------------------------------------------
static inline void
   test_Mutex(void)
{
   MutexThread mutexThread;

   if( opt_verbose ) {
     debugf("\n");
     debugh("test_Mutex\n");
     debugh("Before alphaMutex.lock()\n");
   }
   alphaMutex.lock();

   if( opt_verbose )
     debugh("MutexThread.start()\n");
   mutexThread.start();

   if( opt_verbose )
     debugh("sleep(1.0)...\n");
   Thread::sleep(1.0);
   if( opt_verbose )
     debugh("...sleep(1.0)\n");

   if( opt_verbose )
     debugh("Before alphaMutex.unlock()\n");
   alphaMutex.unlock();

   {{{{
     if( opt_verbose )
       debugh("Before betaMutex.lock()\n");
     std::lock_guard<decltype(betaMutex)> lock(betaMutex);

     if( opt_verbose )
       debugh("Before betaMutex.unlock()\n");
   }}}}

   if( opt_verbose )
     debugh("mutexThread.join()\n");
   mutexThread.join();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Semaphore
//
// Purpose-
//       Test mutual exclusion object.
//
//----------------------------------------------------------------------------
static inline void
   test_Semaphore(void)
{
   SemaphoreThread semaphoreThread;

   if( opt_verbose ) {
     debugf("\n");
     debugh("test_Semaphore\n");
     debugh("Before alphaSemaphore.wait()\n");
   }
   alphaSemaphore.wait();

   if( opt_verbose )
     debugh("SemaphoreThread.start()\n");
   semaphoreThread.start();

   if( opt_verbose )
     debugh("sleep(1.0)...\n");
   Thread::sleep(1.0);
   if( opt_verbose )
     debugh("...sleep(1.0)\n");

   if( opt_verbose )
     debugh("Before alphaSemaphore.post()\n");
   alphaSemaphore.post();

   if( opt_verbose )
     debugh("Before betaSemaphore.wait()\n");
   betaSemaphore.wait();

   if( opt_verbose )
     debugh("Before betaSemaphore.post()\n");
   betaSemaphore.post();

   if( !opt_block ) {               // If not using timedSemaphore blocking
     if( opt_verbose )
       debugh("Before timedSemaphore.post()\n");
     timedSemaphore.post();
   }

   if( opt_verbose )
     debugh("SemaphoreThread.join()\n");
   semaphoreThread.join();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Sleep
//
// Purpose-
//       Thread thread sleep function.
//
//----------------------------------------------------------------------------
static inline void
   test_Sleep(void)
{
   // Test sleep in main SleepThread
   SleepThread sleepThread;
   sleepThread.start();
   sleepThread.join();

   // Test sleep in main thread
   if( opt_verbose )
     debugh("Before sleep(0.5)\n");
   Thread::sleep(0.5);
   if( opt_verbose )
     debugh("*After sleep(0.5)\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Stress
//
// Purpose-
//       Thread stress test.
//
//----------------------------------------------------------------------------
static inline void
   test_Stress(void)
{
   char                buffer[32];
   NoisyThread*        noisyArray[MAXNOISY];
   QuietThread*        quietArray[MAXQUIET];
   double              begin;       // Begin interval
   double              prior;       // Prior interval

   if( opt_verbose )
     debugf("\ntest_Stress\n");

   // This test requires an unusually large number active Threads
   pub::Thread::set_max_threads(MAXNOISY + MAXQUIET + 1'000);
   try {
     for(int count= 0; count<TIMING; count++) { // Timing loop, normally once
       interval.start();
       if( opt_verbose )
         debugf("%12.6f Creating %5d SelfDeleting threads\n"
               , interval.stop(), MAXDELETES);

       for(int i=0; i<MAXDELETES; i++)
         selfDeletingThread();

       if( opt_verbose )
         debugf("%12.6f Creating %5d Noisy threads\n"
               , interval.stop(), MAXNOISY);

       for(int i=0; i<MAXNOISY; i++) {
         sprintf(buffer, "%.4d", i);
         noisyArray[i]= new NoisyThread(buffer, noisy_delay);
         noisyArray[i]->start();
       }

       if( opt_verbose )
         debugf("%12.6f Creating %5d Quiet threads\n"
               , interval.stop(), MAXQUIET);

       for(int i=0; i<MAXQUIET; i++)
         quietArray[i]= new QuietThread();

       // Implementation note:
       // Since Debug and printf do not share mutexes, printf statements can
       // interfere with Debug output. The printf "\r" can elide all or part
       // of a Debug stdout display line.
       // The debug.out trace file should not show this interference.
       prior= interval.stop();
       begin= prior;
       if( opt_verbose ) {
         debugf("%12.6f Starting %5d Quiet threads\n", interval.stop()
               , MAXQUIET);
         fflush(stdout);
       }
       double maxstart= 0.0;
       double minstart= 99999.0;
       for(int i=0; i<MAXQUIET; i++) {
         quietArray[i]->start();
         double now= interval.stop();
         double del= now - prior;
         prior= now;
         if( minstart > del ) minstart= del;
         if( maxstart < del ) maxstart= del;
         if( opt_verbose > 1 ) {
           printf("%8d\r", i+1);
           if( (random() & 63) == 0 )
             fflush(stdout);        // CYGWIN: better performance if used
         }
       }
       if( opt_verbose > 1 ) debugf("\n");

       prior= interval.stop();
       double totstart= prior - begin;
       begin= prior;

       double maxjoin= 0.0;
       double minjoin= 99999.0;
       if( opt_verbose ) {
         debugf("%12.6f Joining  %5d Quiet threads\n", interval.stop()
               , MAXQUIET);
         fflush(stdout);
       }
       for(int i=0; i<MAXQUIET; i++) {
         if( HCDM && i == 0 )
           tracef("%12.6f [0]\n", interval.stop());
         quietArray[i]->join();
         double now= interval.stop();
         double del= now - prior;
         prior= now;
         if( minjoin > del ) minjoin= del;
         if( maxjoin < del ) maxjoin= del;
         if( opt_verbose > 1 ) {
           printf("%8d\r", i+1);
           if( (random() & 63) == 0 )
             fflush(stdout);        // CYGWIN: better performance if unused
         }
       }
       double totjoin= prior - begin;
       if( opt_verbose > 1 ) debugf("\n");

       if( opt_verbose )
         debugf("%12.6f Deleting %5d Quiet threads\n", interval.stop()
               , MAXQUIET);

       for(int i=0; i<MAXQUIET; i++)
         delete quietArray[i];

       if( opt_verbose )
         debugf("%12.6f Joining  %5d Noisy threads\n", interval.stop()
               , MAXNOISY);
       for(int i=0; i<MAXNOISY; i++) {
         noisyArray[i]->join();
         delete noisyArray[i];
         if( opt_verbose > 1 ) {
           printf("%8d\r", i+1);
           if( (random() & 63) == 0 )
             fflush(stdout);        // CYGWIN: better performance if unused
         }
       }
       if( opt_verbose > 1 ) debugf("\n");

       if( opt_verbose ) {
         debugf("%12.6f Joining  complete\n\n", interval.stop());
         debugf("maxstart(%12.6f) minstart(%12.6f) avgstart(%12.6f)\n",
                maxstart, minstart, (double)totstart / (double)MAXQUIET);
         debugf(" maxjoin(%12.6f)  minjoin(%12.6f)  avgjoin(%12.6f)\n",
                maxjoin,  minjoin, (double)totjoin / (double)MAXQUIET);
         debugf("totstart(%12.6f)  totjoin(%12.6f)\n", totstart, totjoin);
       }
     }
   }  catch(Exception& x) {
      debugf("Exception %s\n", x.to_string().c_str());
   }  catch(std::exception& x) {
      debugf("std::exception what(%s)\n", x.what());
   }  catch(const char* x) {
      debugf("Exception(char* %s)\n", x);
   }  catch(...) {
      debugf("Exception(...)\n");
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Termination
//
// Purpose-
//       Test (abnormal) Thread termination sequences.
//
//----------------------------------------------------------------------------
static inline void
   test_Termination(void)
{
   if( opt_verbose ) {
     debugf("\ntest_Termination\n");
     set_log_level(LL_NONE);        // Full stderr logging
   } else {
     set_log_level(LL_ALL);         // No stderr logging
   }
   log(LL_INFO, "\n");
   log(LL_INFO, "Test_thr.cpp test_Termination\n");

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   if( opt_verbose )
     debugf("..Terminate, auto-detach required\n");

   LoopyThread* loopy= new LoopyThread();
   loopy->set_self_delete();        // Delete before exit
   loopy->start();                  // Start the LoopyThread (from this Thread)
   loopy->stop();                   // Stop the LoopyThread
   loopy= nullptr;                  // (The Thread self-deleted)
   Thread::sleep(2.0);              // Allow (plenty of) time for complete stop

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   if( opt_verbose )
     debugf("..Terminate, auto-join required\n");

   LoopyController control;

   LambdaWorker lw([&control](void) {
     LoopyThread& loopy= *control.loopy;
     loopy.start();                 // Start the LoopyThread (from this Worker)
     control.init.post();           // Initialization complete
   }); // LambdaWorker lw

   WorkerPool::work(&lw);
   control.init.wait();
   control.loopy->stop();
   Thread::sleep(2.0);              // Allow (plenty of) time for complete stop
   delete control.loopy;            // Delete without join
   control.loopy= nullptr;          // (Avoid duplicate delete)
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
   //-------------------------------------------------------------------------
   // Initialize
   Wrapper  tc= opts;               // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Help informational display
   tc.on_info([]()
   {
     fprintf(stderr,
            "  --block\tDo not post timedSemaphore\n"
            "  --delay\t=(double) NoisyThread exit delay\n"
            "  --trace\t{=size} Trace table size\n"
            );
   });

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Initialization
   tc.on_init([tr](int, char**)
   {
     debug_set_head(Debug::HEAD_THREAD | Debug::HEAD_TIME);
     if( opt_hcdm )
       debug_set_mode(Debug::MODE_INTENSIVE);

     if( opt_trace )
       table= tr->init_trace("./trace.mem", opt_trace);

     return 0;
   });

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Parameter analysis
   tc.on_parm([tr](std::string P, const char* V)
   {
     if( P == "delay" ) {
       noisy_delay= tr->ptod(V);
     } else if( P == "trace" ) {
       if( V )
         opt_trace= tr->ptoi(V);
     } else if( P == "********" ) {
       // PLACEHOLDER
     } else {
       fprintf(stderr, "Invalid option '%s'\n", s2c(P));
       return 1;
     }

     return 0;
   });

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Termination
   tc.on_term([tr]()
   {
     if( table )
       tr->term_trace(table, opt_trace);
   });

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Mainline code
   tc.on_main([tr](int, char*[])
   {
     if( opt_verbose )
       debugh("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

     for(int i= 0; i<8; i++)        // Test Event object
       standardThread();            // This only tests current

     test_Mutex();
     test_Semaphore();
     test_Sleep();
     test_Termination();

     test_Stress();

     if( opt_verbose ) {
       debugf("\n");
       PUB::System::debug("Test_thr complete");

       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   opt_hcdm= HCDM;
   opt_verbose= VERBOSE;
   setlocale(LC_NUMERIC, "");       // Activates ' thousand separator

   return tc.run(argc, argv);
}
