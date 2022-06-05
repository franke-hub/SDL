//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_thr.cpp
//
// Purpose-
//       Test Thread function.
//
// Last change date-
//       2022/06/05
//
//----------------------------------------------------------------------------
#include <exception>                // For std::exception
#include <iostream>                 // For std::cout, std::cerr
#include <memory>                   // For std::shared_ptr, ...
#include <mutex>                    // For std::lock_guard
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For random
#include <string.h>

#include <pub/Debug.h>              // For pub::debugging methods
#include "pub/Event.h"              // For Event
#include "pub/Exception.h"          // For Exception
#include <pub/Interval.h>           // For Interval (performance debugging)
#include "pub/Mutex.h"              // For Mutex
#include "pub/Named.h"              // For Named (Threads)
#include "pub/Semaphore.h"          // For Semaphore
#include "pub/Thread.h"             // For Thread

#include "pub/Wrapper.h"            // For class Wrapper

using pub::Debug;
using pub::Event;
using pub::Exception;
using pub::Interval;
using pub::Mutex;
using pub::Named;
using pub::Semaphore;
using pub::Thread;
using pub::Wrapper;
using namespace pub::debugging;

#define opt_hcdm       pub::Wrapper::opt_hcdm
#define opt_verbose    pub::Wrapper::opt_verbose

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more

,  MAXHANGERS=  16                  // Number of hangy threads to generate
,  MAXNOISY=  1000                  // Number of noisy threads to generate
,  MAXQUIET= 25000                  // Number of quiet threads to generate
,  TIMING=       1                  // Number of timing loops to run
}; // Generic enum

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Mutex           alphaMutex;
static std::mutex      betaMutex;
static Semaphore       alphaSemaphore(1);
static Semaphore       betaSemaphore(1);
static Semaphore       blockedSemaphore(0);
static int             error_count= 0;
static Interval        interval;
static double          noisy_delay= 0.001; // Default noisy delay

//----------------------------------------------------------------------------
//
// Class
//       NoisyThread
//
// Purpose-
//       Define a simple, but noisy, Thread
//
//----------------------------------------------------------------------------
class NoisyThread : public Thread, public Named { // Noisy Thread class
protected:
double                 delay;       // Delay before exit
Event                  started;

public:
int                    stateControl;

public:
   NoisyThread(
     const char*       threadName,  // The Thread name
     double            delay= 0.001) // The exit delay
:  Thread(), Named(threadName)
,  delay(delay), stateControl(-1) { }

virtual void
   run(void)
{
   if( opt_verbose )
     debugf("%10.6f NoisyThread(%p).run(%s)\n", interval.stop(), this
           , get_name().c_str());

   // Indicate started
   setState(4);
   started.post();

   // Sleep (allow possible thread deletion)
   Thread::sleep(delay);

#if 0
   // Terminate, display current
   Thread* current= Thread::current();
   debugf("%10.6f NoisyThread(%p).exit(%s) %s\n", interval.stop(), this,
          get_name().c_str(), this == current ? "SAME" : "DIFF");
   fflush(stdout);
#endif
}

void
   safeStart(void)
{
   setState(1);
   started.reset();
   setState(2);
   start();
   started.wait();

   int stateControl= this->stateControl;
   if( stateControl != 4 )          // If not in posted state
   {
     debugf("%4d ERROR: NoisyThread(%p) fsm(%d)\n", __LINE__,
            this, stateControl);
     ::exit(EXIT_FAILURE);
   }
}

void
   setState(
     int               state)
{  stateControl= state; }
}; // class NoisyThread

//----------------------------------------------------------------------------
//
// Class
//       QuietThread
//
// Purpose-
//       Define a simple, but quiet, Thread
//
//----------------------------------------------------------------------------
class QuietThread : public Thread { // Quiet Thread class
public:
   QuietThread()
:  Thread() { }

virtual void
   run(void)
{
   Thread* current= Thread::current();
   if (this != current ) {
     error_count++;
     debugf("%4d ERROR: Thread(%p) Current(%p)\n", __LINE__, this, current);
   }
}
}; // class QuietThread

//----------------------------------------------------------------------------
//
// Class
//       HangingThread
//
// Purpose-
//       This Thread is designed to run after being deleted.
//
//----------------------------------------------------------------------------
class HangingThread : public Thread { // Hanging Thread class
public:
Event                  started;     // Posted when started

public:
   HangingThread( void )
:  Thread(), started() {}

virtual void
   run(void)
{
   if( HCDM || opt_hcdm )
     debugf("%10.6f HangingThread(%p).run()\n", interval.stop(), this);

   Thread* current= Thread::current(); // The current Thread
   if( current != this )               // This MUST BE correct
   {
     error_count++;
     debugf("%4d ERROR: HangingThread(%p) Current(%p)\n", __LINE__,
            this, current);
     ::exit(EXIT_FAILURE);
   }

   // Indicate started
   started.post(0);

   // Once this sleep() operation is complete, the Thread should not exist.
   Thread::sleep(0.250);            // Delay past thread deletion

   current= Thread::current();
   if( current != nullptr )
   {
     error_count++;
     debugf("%4d ERROR: HangingThread(%p) Current(%p)\n", __LINE__,
            this, current);
   }

   // Show work being done on a deleted Thread
   if( HCDM || opt_hcdm ) {
     debugf("%10.6f HangingThread(%p) exit\n", interval.stop(), this);
     fflush(stdout);
   }
}
}; // class HangingThread

//----------------------------------------------------------------------------
//
// Class
//       MutexThread
//
// Purpose-
//       Define a Thread used solely to test the Mutex object.
//
//----------------------------------------------------------------------------
class MutexThread : public NoisyThread { // Mutex Thread class
public:
   MutexThread()
:  NoisyThread("MutexThread") { }

virtual void
   run(void)
{
   if( HCDM || opt_hcdm )
     debugh("Before betaMutex.lock()\n");
   betaMutex.lock();

   {{{{
     if( HCDM || opt_hcdm )
       debugh("Before alphaMutex.lock()\n");
     std::lock_guard<decltype(alphaMutex)> lock(alphaMutex);

     if( HCDM || opt_hcdm )
       debugh("Before alphaMutex.unlock()\n");
   }}}}

   if( HCDM || opt_hcdm )
     debugh("sleep(1.0)...\n");
   Thread::sleep(1.0);
   if( HCDM || opt_hcdm )
     debugh("...sleep(1.0)\n");

   if( HCDM || opt_hcdm )
     debugh("Before betaMutex.unlock()\n");
   betaMutex.unlock();

   if( HCDM || opt_hcdm )
     debugh("done!\n");
}
}; // class MutexThread

//----------------------------------------------------------------------------
//
// Class
//       SemaphoreThread
//
// Purpose-
//       Define a Thread used solely to test the Semaphore object.
//
//----------------------------------------------------------------------------
class SemaphoreThread : public NoisyThread { // Semaphore Thread class
public:
   SemaphoreThread()
:  NoisyThread("SemaphoreThread") { }

virtual void
   run(void)
{
   if( HCDM || opt_hcdm )
     debugh("Before betaSemaphore.wait()\n");
   betaSemaphore.wait();

   if( HCDM || opt_hcdm )
     debugh("Before alphaSemaphore.wait()\n");
   alphaSemaphore.wait();

   if( HCDM || opt_hcdm )
     debugh("Before alphaSemaphore.post()\n");
   alphaSemaphore.post();

   if( HCDM || opt_hcdm )
     debugh("sleep(1.0)...\n");
   Thread::sleep(1.0);
   if( HCDM || opt_hcdm )
     debugh("...sleep(1.0)\n");

   if( HCDM || opt_hcdm )
     debugh("Before betaSemaphore.post()\n");
   betaSemaphore.post();

   if( HCDM || opt_hcdm )
     debugh("Before blockedSemaphore.wait(3.5)...\n");
   int rc= blockedSemaphore.wait(3.5);
   if( HCDM || opt_hcdm )
     debugh("...%d= blockedSemaphore.wait()\n", rc);

   if( HCDM || opt_hcdm )
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
class SleepThread : public NoisyThread { // Sleep Thread class
public:
   SleepThread()
:  NoisyThread("SleepThread") { }

virtual void
   run(void)
{
   if( HCDM || opt_hcdm )
     debugh("Before sleep(1.234)\n");
   sleep(1.234);
   if( HCDM || opt_hcdm )
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
class StandardThread : public Thread { // Standard Thread class
public:
   StandardThread( void )
:  Thread() { }

virtual void
   run(void)
{
   if( HCDM || opt_hcdm )
     debugf("%10.6f StandardThread(%p).run()\n", interval.stop(), this);

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

   if( HCDM || opt_hcdm )
     debugf("%10.6f StandardThread(%p) exit\n", interval.stop(), this);
}
}; // class StandardThread

//----------------------------------------------------------------------------
//
// Subroutine-
//       hangingThread
//
// Purpose-
//       Insure that a thread can complete even though the corresponding
//       object is deleted.
//
//----------------------------------------------------------------------------
static void
   hangingThread(void)
{
   auto hangingThread= std::make_shared<HangingThread>();

   hangingThread->start();
   hangingThread->started.wait();   // Do not return before run invoked!
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
   StandardThread      standardThread;

   standardThread.start();
   standardThread.join();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testMutex
//
// Purpose-
//       Test mutual exclusion object.
//
//----------------------------------------------------------------------------
static inline void
   testMutex(void)
{
   MutexThread         mutexThread;

   if( opt_verbose ) {
     debugh("\n");
     debugh("testMutex\n");
     debugh("Before alphaMutex.lock()\n");
   }
   alphaMutex.lock();

   if( opt_verbose )
     debugh("thread.start()\n");
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
     debugh("thread.join()\n");
   mutexThread.join();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testSemaphore
//
// Purpose-
//       Test mutual exclusion object.
//
//----------------------------------------------------------------------------
static inline void
   testSemaphore(void)
{
   SemaphoreThread     semaphoreThread;

   if( opt_verbose ) {
     debugh("\n");
     debugh("testSemaphore\n");
     debugh("Before alphaSemaphore.wait()\n");
   }
   alphaSemaphore.wait();

   if( opt_verbose )
     debugh("thread.start()\n");
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

   if( opt_verbose )
     debugh("thread.join()\n");
   semaphoreThread.join();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testSleep
//
// Purpose-
//       Thread thread sleep function.
//
//----------------------------------------------------------------------------
static inline void
   testSleep(void)
{
   SleepThread         sleepThread;

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
//       testStress
//
// Purpose-
//       Thread stress test.
//
//----------------------------------------------------------------------------
static inline void
   testStress(void)
{
   char                buffer[32];
   NoisyThread*        noisyArray[MAXNOISY];
   QuietThread*        quietArray[MAXQUIET];
   double              begin;       // Begin interval
   double              prior;       // Prior interval

   try {
     for(int count= 0; count<TIMING; count++) { // Timing loop, normally once
       interval.start();
       if( opt_verbose ) {
         debugf("\n");
         debugf("%10.6f %4d Creating hanging threads\n", interval.stop()
               , __LINE__);
       }
       for(int i=0; i<MAXHANGERS; i++)
         hangingThread();

       if( opt_verbose ) {
         debugf("\n");
         debugf("%10.6f %4d Creating Noisy threads\n", interval.stop()
               , __LINE__);
       }
       for(int i=0; i<MAXNOISY; i++)
       {
         sprintf(buffer, "%.4d", i);
         noisyArray[i]= new NoisyThread(buffer, noisy_delay);
         noisyArray[i]->safeStart();
       }

       if( opt_verbose ) {
         debugf("\n");
         debugf("%10.6f %4d Creating Quiet threads\n", interval.stop()
               , __LINE__);
       }
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
         debugf("%10.6f %4d Starting Quiet threads\n", interval.stop()
               , __LINE__);
         fflush(stdout);
       }
       double maxstart= 0.0;
       double minstart= 99999.0;
       for(int i=0; i<MAXQUIET; i++)
       {
         quietArray[i]->start();
         double now= interval.stop();
         double del= now - prior;
         prior= now;
         if( minstart > del ) minstart= del;
         if( maxstart < del ) maxstart= del;
         if( opt_verbose ) {
           printf("%8d\r", i+1);
           if( (random() & 63) == 0 )
             fflush(stdout);        // CYGWIN: better performance if used
         }
       }
       prior= interval.stop();
       double totstart= prior - begin;
       begin= prior;

       double maxjoin= 0.0;
       double minjoin= 99999.0;
       if( opt_verbose ) {
         debugf("\n");
         debugf("%10.6f %4d Joining Quiet threads\n", interval.stop()
               , __LINE__);
         fflush(stdout);
       }
       for(int i=0; i<MAXQUIET; i++)
       {
         if( HCDM && i == 0 )
           tracef("%10.6f [0]\n", interval.stop());
         quietArray[i]->join();
         double now= interval.stop();
         double del= now - prior;
         prior= now;
         if( minjoin > del ) minjoin= del;
         if( maxjoin < del ) maxjoin= del;
         if( opt_verbose ) {
           printf("%8d\r", i+1);
           if( (random() & 63) == 0 )
             fflush(stdout);        // CYGWIN: better performance if unused
         }
       }
       double totjoin= prior - begin;

       if( opt_verbose ) {
         debugf("\n");
         debugf("%10.6f %4d Deleting Quiet threads\n", interval.stop()
               , __LINE__);
       }
       for(int i=0; i<MAXQUIET; i++)
         delete quietArray[i];

       if( opt_verbose )
         debugf("%10.6f %4d Joining Noisy threads\n", interval.stop()
               , __LINE__);
       for(int i=0; i<MAXNOISY; i++)
       {
         noisyArray[i]->join();
         delete noisyArray[i];
       }

       if( opt_verbose ) {
         debugf("%10.6f %4d All threads completed\n", interval.stop(), __LINE__);
         debugf("maxstart(%10.6f) minstart(%10.6f) avgstart(%10.6f)\n",
                maxstart, minstart, (double)totstart / (double)MAXQUIET);
         debugf(" maxjoin(%10.6f)  minjoin(%10.6f)  avgjoin(%10.6f)\n",
                maxjoin,  minjoin, (double)totjoin / (double)MAXQUIET);
         debugf("totstart(%10.6f)  totjoin(%10.6f)\n", totstart, totjoin);
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
   Wrapper  tc;                     // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_init([](int argc, char* argv[])
   {
     if( HCDM ) opt_hcdm= true;
     if( VERBOSE > opt_verbose ) opt_verbose= VERBOSE;

     debug_set_head(Debug::HEAD_THREAD);
     debug_set_head(Debug::HEAD_TIME);
     if( opt_hcdm )
       debug_set_mode(Debug::MODE_INTENSIVE);

     if( optind < argc )
       noisy_delay= atof(argv[optind]);

     return 0;
   });

   //-------------------------------------------------------------------------
   // Define the tests
   tc.on_main([tr](int, char*[])
   {
     if( opt_verbose )
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

     for(int i= 0; i<8; i++)        // Test Event object
       standardThread();

     testMutex();
     testSemaphore();
     testSleep();
     testStress();

     if( opt_verbose ) {
       debugf("\n");
       Thread::static_debug("");

       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}
