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
//       Test_Thr.cpp
//
// Purpose-
//       Test Thread function.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <exception>                // For std::exception
#include <iostream>                 // For std::cout, std::cerr
#include <memory>                   // For std::shared_ptr, ...
#include <mutex>                    // For std::lock_guard
#include <stdio.h>                  // For printf
#include <stdlib.h>
#include <string.h>

#include <pub/Debug.h>              // For pub::debugging methods
#include "pub/Event.h"              // For Event
#include "pub/Exception.h"          // For Exception
#include <pub/Interval.h>           // For Interval (performance debugging)
#include "pub/Mutex.h"              // For Mutex
#include "pub/Named.h"              // For Named (Threads)
#include "pub/Semaphore.h"          // For Semaphore
#include "pub/Thread.h"             // For Thread

using pub::Debug;
using pub::Event;
using pub::Exception;
using pub::Interval;
using pub::Mutex;
using pub::Named;
using pub::Semaphore;
using pub::Thread;
using namespace pub::debugging;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include "pub/ifmacro.h"

#define MAXHANGERS        16        // Number of hangy threads to generate
#define MAXNOISY        1000        // Number of noisy threads to generate
#define MAXQUIET       25000        // Number of quiet threads to generate

#define TIMING         1            // Number of timing loops to run

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Mutex           alphaMutex;
static std::mutex      betaMutex;
static Semaphore       alphaSemaphore(1);
static Semaphore       betaSemaphore(1);
static Semaphore       blockedSemaphore(0);
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
   debugf("%10.6f NoisyThread(%p).run(%s)\n", interval.stop(), this,
          get_name().c_str());

   // Indicate started
   setState(4);
   started.post();

   // Sleep (allow possible thread deletion)
   Thread::sleep(delay);

   // Terminate, display current
#if 0
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
// setState(3);                     // (May already be in state 4)
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
// Thread* current= Thread::current();
// if (this != current )
//   debugf("%4d ERROR: Thread(%p) Current(%p)\n", __LINE__, this, current);

// tracef("%10.6f QuietThread.exit\n", interval.stop());
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
   debugf("%10.6f HangingThread(%p).run()\n", interval.stop(), this);

   Thread* current= Thread::current(); // The current Thread
   if( current != this )               // This MUST BE correct
   {
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
     debugf("%4d ERROR: HangingThread(%p) Current(%p)\n", __LINE__,
            this, current);
   }

   // Show work being done on a deleted Thread
   debugf("%10.6f HangingThread(%p) exit\n", interval.stop(), this);
   fflush(stdout);
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
   debugh("Before betaMutex.lock()\n");
   betaMutex.lock();

   {{{{
     debugh("Before alphaMutex.lock()\n");
     std::lock_guard<decltype(alphaMutex)> lock(alphaMutex);

     debugh("Before alphaMutex.unlock()\n");
   }}}}

   debugh("sleep(1.0)...\n");
   Thread::sleep(1.0);
   debugh("...sleep(1.0)\n");

   debugh("Before betaMutex.unlock()\n");
   betaMutex.unlock();

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
   debugh("Before betaSemaphore.wait()\n");
   betaSemaphore.wait();

   debugh("Before alphaSemaphore.wait()\n");
   alphaSemaphore.wait();

   debugh("Before alphaSemaphore.post()\n");
   alphaSemaphore.post();

   debugh("sleep(1.0)...\n");
   Thread::sleep(1.0);
   debugh("...sleep(1.0)\n");

   debugh("Before betaSemaphore.post()\n");
   betaSemaphore.post();

   debugh("Before blockedSemaphore.wait(3.5)...\n");
   int rc= blockedSemaphore.wait(3.5);
   debugh("...%d= blockedSemaphore.wait()\n", rc);

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
   debugh("Before sleep(1.234)\n");
   sleep(1.234);
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
   debugf("%10.6f StandardThread(%p).run()\n", interval.stop(), this);

   Thread* current= Thread::current(); // The current Thread
   if( current != this )            // This MUST BE correct
   {
     debugf("%4d ERROR: StandardThread(%p) Current(%p)\n", __LINE__,
            this, current);
     ::exit(EXIT_FAILURE);
   }

   // Wait a little bit
   Thread::sleep(0.0125);

   current= Thread::current();
   if( current != this )            // This MUST BE correct
   {
     debugf("%4d ERROR: StandardThread(%p) Current(%p)\n", __LINE__,
            this, current);
     ::exit(EXIT_FAILURE);
   }

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

   debugh("\n");
   debugh("testMutex\n");
   debugh("Before alphaMutex.lock()\n");
   alphaMutex.lock();

   debugh("thread.start()\n");
   mutexThread.start();

   debugh("sleep(1.0)...\n");
   Thread::sleep(1.0);
   debugh("...sleep(1.0)\n");

   debugh("Before alphaMutex.unlock()\n");
   alphaMutex.unlock();

   {{{{
     debugh("Before betaMutex.lock()\n");
     std::lock_guard<decltype(betaMutex)> lock(betaMutex);

     debugh("Before betaMutex.unlock()\n");
   }}}}

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

   debugh("\n");
   debugh("testSemaphore\n");
   debugh("Before alphaSemaphore.wait()\n");
   alphaSemaphore.wait();

   debugh("thread.start()\n");
   semaphoreThread.start();

   debugh("sleep(1.0)...\n");
   Thread::sleep(1.0);
   debugh("...sleep(1.0)\n");

   debugh("Before alphaSemaphore.post()\n");
   alphaSemaphore.post();

   debugh("Before betaSemaphore.wait()\n");
   betaSemaphore.wait();

   debugh("Before betaSemaphore.post()\n");
   betaSemaphore.post();

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
   debugh("Before sleep(0.5)\n");
   Thread::sleep(0.5);
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

   int                 i;

try {
for(int count= 0; count<TIMING; count++) { // Timing loop, normally once
   interval.start();
   debugf("\n");
   debugf("%10.6f %4d Creating hanging threads\n", interval.stop(), __LINE__);
   for(i=0; i<MAXHANGERS; i++)
     hangingThread();

   debugf("\n");
   debugf("%10.6f %4d Creating Noisy threads\n", interval.stop(), __LINE__);
   for(i=0; i<MAXNOISY; i++)
   {
     sprintf(buffer, "%.4d", i);
     noisyArray[i]= new NoisyThread(buffer, noisy_delay);
     noisyArray[i]->safeStart();
   }

   debugf("\n");
   debugf("%10.6f %4d Creating Quiet threads\n", interval.stop(), __LINE__);
   for(i=0; i<MAXQUIET; i++)
     quietArray[i]= new QuietThread();

   // Implementation note:
   // Since Debug and printf do not share mutexes, printf statements can
   // interfere with Debug output. The printf "\r" can elide all or part
   // of a Debug stdout display line.
   // The debug.out trace file should not show this interference.
   prior= interval.stop();
   begin= prior;
   debugf("%10.6f %4d Starting Quiet threads\n", interval.stop(), __LINE__);
   fflush(stdout);
   double maxstart= 0.0;
   double minstart= 99999.0;
   for(i=0; i<MAXQUIET; i++)
   {
     quietArray[i]->start();
     double now= interval.stop();
     double del= now - prior;
     prior= now;
     if( minstart > del ) minstart= del;
     if( maxstart < del ) maxstart= del;
     IFHCDM( tracef("%10.6f %10.6f %6d %p\n", now, del, i, quietArray[i]); )
     printf("%8d\r", i+1);
     fflush(stdout);                // CYGWIN: better performance if used
   }
   printf("\n");
   prior= interval.stop();
   double totstart= prior - begin;
   begin= prior;

   double maxjoin= 0.0;
   double minjoin= 99999.0;
   debugf("%10.6f %4d Joining Quiet threads\n", interval.stop(), __LINE__);
   fflush(stdout);
   for(i=0; i<MAXQUIET; i++)
   {
     IFHCDM( if( i == 0 ) tracef("%10.6f [0]\n", interval.stop()); )
     quietArray[i]->join();
     double now= interval.stop();
     double del= now - prior;
     prior= now;
     if( minjoin > del ) minjoin= del;
     if( maxjoin < del ) maxjoin= del;
     IFHCDM( tracef("%10.6f %6d %p\n", interval.stop(), i, quietArray[i]); )
     printf("%8d\r", i+1);
//   fflush(stdout);                // CYGWIN: better performance if unused
   }
   printf("\n");
   double totjoin= prior - begin;

   debugf("%10.6f %4d Deleting Quiet threads\n", interval.stop(), __LINE__);
   for(i=0; i<MAXQUIET; i++)
     delete quietArray[i];

   debugf("%10.6f %4d Joining Noisy threads\n", interval.stop(), __LINE__);
   for(i=0; i<MAXNOISY; i++)
   {
     noisyArray[i]->join();
     delete noisyArray[i];
   }

   debugf("%10.6f %4d All threads completed\n", interval.stop(), __LINE__);
   debugf("maxstart(%10.6f) minstart(%10.6f) avgstart(%10.6f)\n",
          maxstart, minstart, (double)totstart / (double)MAXQUIET);
   debugf(" maxjoin(%10.6f)  minjoin(%10.6f)  avgjoin(%10.6f)\n",
          maxjoin,  minjoin, (double)totjoin / (double)MAXQUIET);
   debugf("totstart(%10.6f)  totjoin(%10.6f)\n", totstart, totjoin);
}
} catch(Exception& x) {
   debugf("Exception %s\n", x.to_string().c_str());
} catch(std::exception& x) {
   debugf("std::exception what(%s)\n", x.what());
} catch(const char* x) {
   debugf("Exception(char* %s)\n", x);
} catch(...) {
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
   debugf("Thread bringup test\n");
   debug_set_head(Debug::HeadThread);
   debug_set_head(Debug::HeadTime);
// debug_set_mode(Debug::ModeIntensive);
   if( argc > 1 )
     noisy_delay= atof(argv[1]);
   debugf("%10.6f noisy_delay\n", noisy_delay);

   for(int i= 0; i<8; i++)          // Test Event object
     standardThread();

   testMutex();
   testSemaphore();
   testSleep();
   testStress();

   return 0;
}
