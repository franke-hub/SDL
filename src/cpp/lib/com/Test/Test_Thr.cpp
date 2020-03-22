//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
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
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Clock.h>
#include <com/Debug.h>
#include <com/define.h>
#include "com/Mutex.h"
#include "com/Semaphore.h"
#include "com/Status.h"
#include "com/Thread.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define MAXHANGERS               16 // Number of hangy threads to generate
#define MAXNOISY               1000 // Number of noisy threads to generate
#define MAXQUIET              25000 // Number of quiet threads to generate

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Mutex         alphaMutex;
static Mutex         betaMutex;
static Semaphore     alphaSemaphore;
static Semaphore     betaSemaphore;
static Semaphore     blockedSemaphore(0);

static enum {
   DoNothing,                       // Don't do anything
}                    quietFSM;      // What to do in Quiet::run()

//----------------------------------------------------------------------------
//
// Class
//       MutexThread
//
// Purpose-
//       Define a Thread used solely to test the Mutex object.
//
//----------------------------------------------------------------------------
class MutexThread : public Thread { // Mutex Thread class
public:
   MutexThread()
:  Thread()
{
}

virtual long
   run(void);
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
class NoisyThread : public Thread { // Noisy Thread class
public:
int                  stateControl;

private:
const char*          const threadName;
Status               started;

public:
   NoisyThread(
     const char*     threadName);

public:
const char*
   getName(void) const
{  return threadName;
}

void
   setState(
     int               state)
{
   #ifdef HCDM
     debugf("NoisyThread(%p)::setState(%d)\n", this, state);
   #endif

   stateControl= state;
}

void
   safeStart(void);

virtual long
   run(void);
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
:  Thread()
{
}

virtual long
   run(void);
}; // class QuietThread

//----------------------------------------------------------------------------
//
// Class
//       SemaphoreThread
//
// Purpose-
//       Define a Thread used solely to test the Semaphore object.
//
//----------------------------------------------------------------------------
class SemaphoreThread : public Thread { // Semaphore Thread class
public:
   SemaphoreThread()
:  Thread()
{
}

virtual long
   run(void);
}; // class SemaphoreThread

//----------------------------------------------------------------------------
//
// Method-
//       MutexThread::run
//
// Purpose-
//       Drive a thread.
//
//----------------------------------------------------------------------------
long
   MutexThread::run(void)           // Drive a thread
{
   int               i;

   debugf("Thread: Before betaMutex.reserve()\n");
   betaMutex.reserve();

   debugf("Thread: Before alphaMutex.reserve()\n");
   alphaMutex.reserve();

   debugf("Thread: Before alphaMutex.release()\n");
   alphaMutex.release();

   debugf("Thread: yield() loop\n");
   for(i=0; i<256; i++)
     Thread::yield();

   debugf("Thread: Before betaMutex.release()\n");
   betaMutex.release();

   debugf("Thread: done!\n");

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       NoisyThread::NoisyThread
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   NoisyThread::NoisyThread(        // Construct a thread
     const char      *threadName)
:  Thread()
,  threadName(threadName)
,  stateControl(-1)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NoisyThread::safeStart
//
// Purpose-
//       Safely start a thread.
//
//----------------------------------------------------------------------------
void
   NoisyThread::safeStart(void)     // Safely start this thread
{
   setState(1);
   started.reset();
   setState(2);
   start();
// setState(3);                     // (May already be in state 4)
   started.wait();
}

//----------------------------------------------------------------------------
//
// Method-
//       NoisyThread::run
//
// Purpose-
//       Drive a thread.
//
//----------------------------------------------------------------------------
long
   NoisyThread::run(void)           // Drive a thread
{
   Thread*           current;       // The current Thread

   debugf("%4d %s %14.3f Thread(%s).run()\n", __LINE__, "Test_Thr.cpp",
          Clock::current(), getName());

   // Once this post() operation is complete, the Thread might not exist.
   // Since getName() uses the Thread object, it should precede post().
   setState(4);
   started.post(0);

   // The Thread object may have been deleted by now, making
   // (Thread::current() == NULL)!
   // Thread::yield() makes the probability of this higher.
   Thread::yield();
   current= Thread::current();
   if( current != this && current != NULL )
   {
     debugf("%4d %s Thread(%p) Current(%p)\n", __LINE__,
            "Test_Thr.cpp", this, current);
     ::exit(EXIT_FAILURE);
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       QuietThread::run
//
// Purpose-
//       Drive a thread.
//
//----------------------------------------------------------------------------
long
   QuietThread::run(void)           // Drive a thread
{
   if (this != Thread::current())
   {
     debugf("%4d %s Thread(%p) Current(%p)\n", __LINE__, "Test_Thr.cpp",
            this, Thread::current());
   }

   switch(quietFSM)
   {
     case DoNothing:
       break;

     default:
       debugf("Invalid quietFSM(%d)\n", quietFSM);
       break;
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       SemaphoreThread::run
//
// Purpose-
//       Drive a thread.
//
//----------------------------------------------------------------------------
long
   SemaphoreThread::run(void)       // Drive a thread
{
   int               i;

   debugf("Thread: Before betaSemaphore.wait()\n");
   betaSemaphore.wait();

   debugf("Thread: Before alphaSemaphore.wait()\n");
   alphaSemaphore.wait();

   debugf("Thread: Before alphaSemaphore.post()\n");
   alphaSemaphore.post();

   debugf("Thread: yield() loop\n");
   for(i=0; i<256; i++)
     Thread::yield();

   debugf("Thread: Before betaSemaphore.post()\n");
   betaSemaphore.post();

   debugf("Thread: %14.3f Before blockedSemaphore.wait(3.45678)\n",
          Clock::current());
   int rc= blockedSemaphore.wait(3.45678);
   debugf("Thread: %14.3f %d= blockedSemaphore.wait()\n",
          Clock::current(), rc);

   debugf("Thread: done!\n");

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::testMutex
//
// Purpose-
//       Test mutual exclusion object.
//
//----------------------------------------------------------------------------
static inline void
   testMutex(void)
{
   MutexThread       mutexThread;

   int               i;

   debugf("\n");
   debugf("testMutex\n");
   debugf("Test:   Before alphaMutex.reserve()\n");
   alphaMutex.reserve();

   debugf("Test:   thread.start()\n");
   mutexThread.start();

   debugf("Test:   yield() loop\n");
   for(i=0; i<256; i++)
     Thread::yield();
   debugf("Test:   yield() done\n");

   debugf("Test:   Before alphaMutex.release()\n");
   alphaMutex.release();

   debugf("Test:   Before betaMutex.reserve()\n");
   betaMutex.reserve();

   debugf("Test:   Before betaMutex.release()\n");
   betaMutex.release();

   debugf("Test:   thread.wait()\n");
   mutexThread.wait();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::testSemaphore
//
// Purpose-
//       Test mutual exclusion object.
//
//----------------------------------------------------------------------------
static inline void
   testSemaphore(void)
{
   SemaphoreThread   semaphoreThread;

   int               i;

   debugf("\n");
   debugf("testSemaphore\n");
   debugf("Test:   Before alphaSemaphore.wait()\n");
   alphaSemaphore.wait();

   debugf("Test:   thread.start()\n");
   semaphoreThread.start();

   debugf("Test:   yield() loop\n");
   for(i=0; i<256; i++)
     Thread::yield();
   debugf("Test:   yield() done\n");

   debugf("Test:   Before alphaSemaphore.post()\n");
   alphaSemaphore.post();

   debugf("Test:   Before betaSemaphore.wait()\n");
   betaSemaphore.wait();

   debugf("Test:   Before betaSemaphore.post()\n");
   betaSemaphore.post();

   debugf("Test:   thread.wait()\n");
   semaphoreThread.wait();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::hangingThread
//
// Purpose-
//       Insure that a thread can complete even though the
//       corresponding object is deleted.
//
//----------------------------------------------------------------------------
static int
   hangingThread(void)
{
   NoisyThread       noisyThread("Hanging Thread");

   noisyThread.safeStart();
   return noisyThread.stateControl;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::standardThread
//
// Purpose-
//       Verify the correct operation of a standard Thread.
//
//----------------------------------------------------------------------------
static void
   standardThread(void)
{
   NoisyThread       noisyThread("Standard Thread");
   int               stateControl;

   noisyThread.stateControl= (-1);
   noisyThread.safeStart();
   stateControl= noisyThread.stateControl;

   if( stateControl != 4 )
   {
     debugf("%4d %s Statecontrol(%d) != 4\n",
            __LINE__, "Test_Thr.cpp", stateControl);
     exit(EXIT_FAILURE);
   }

   noisyThread.wait();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::testThread
//
// Purpose-
//       Thread stress test.
//
//----------------------------------------------------------------------------
static inline void
   testThread(void)
{
   char              str[MAXNOISY][8];

   NoisyThread*      noisyArray[MAXNOISY];
   QuietThread*      quietThread;

   int               i;
   int               rc;

try {
   debugf("\n");
   debugf("%4d %s Hanging threads\n", __LINE__, "Test_Thr.cpp");
   for(i=0; i<MAXHANGERS; i++)
   {
     rc= hangingThread();
     if( rc != 4 )
       debugf("%4d %s Statecontrol(%d) != 4\n", __LINE__, "Test_Thr.cpp", rc);
   }

   debugf("\n");
   debugf("%4d %s Noisy threads\n", __LINE__, "Test_Thr.cpp");
   for(i=1; i<=MAXNOISY; i++)
   {
     sprintf(str[i-1], "%.4d", i);
     noisyArray[i-1]= new NoisyThread(str[i-1]);
     noisyArray[i-1]->setStackSize(0x00010000);
   }

   for(i=1; i<=MAXNOISY; i++)
     noisyArray[i-1]->safeStart();

   quietFSM= DoNothing;
   debugf("\n");
   debugf("%4d %s Quiet threads\n", __LINE__, "Test_Thr.cpp");
   for(i=1; i<=MAXQUIET; i++)
   {
     quietThread= new QuietThread();
     quietThread->start();
     quietThread->wait();
     delete quietThread;
     printf("%8d\r", i);
   }
   printf("\n");

   debugf("%4d %s Noisy thread wait\n", __LINE__, "Test_Thr.cpp");
   for(i=1; i<=MAXNOISY; i++)
     noisyArray[i-1]->wait();

   debugf("All threads are complete\n");
} catch (const char* x) {
   debugf("Exception(%s)\n", x);
} catch (...) {
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
     int             argc,          // Argument count
     char           *argv[])        // Argument array
{
   debugf("Thread bringup test\n");
   for(int i= 0; i<8; i++)          // Test Status object
     standardThread();

//   testMutex();
//   testSemaphore();
   testThread();

   return 0;
}

