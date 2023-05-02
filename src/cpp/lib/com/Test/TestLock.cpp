//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestLock.cpp
//
// Purpose-
//       Test locking and latching functions.
//
// Last change date-
//       2020/10/03
//
// Implementation notes-
//       Functions are not declared static so that compilation completes
//       even when their calls are commented out.
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/Interval.h>
#include <com/Semaphore.h>
#include <com/Signal.h>
#include <com/Software.h>
#include <com/Thread.h>
#include <com/ThreadLogger.h>

#include "com/ThreadLock.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef USE_DEBUG_LOCK
#undef  USE_DEBUG_LOCK              // If defined, use ThreadLock::debug
#endif

#ifndef USE_MP_DEADLOCK
#undef  USE_MP_DEADLOCK             // If defined, test multiprocessor deadlock
#endif

#ifndef USE_UP_DEADLOCK
#undef  USE_UP_DEADLOCK             // If defined, test uniprocessor deadlock
#endif

#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#define DIM_ARRAY 10000             // Number of array elements
#define THREAD_COUNT 64             // Number of simultaneous Threads
                                    // Must be 8, 16, 32, or 64

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

#ifdef  USE_DEBUG_LOCK
  #define IFDEBUG(x) {x}
#else
  #define IFDEBUG(x) {}
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class MyThread;

//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
typedef void (*MP_TEST)(MyThread*);
typedef ThreadLock::Token Token;

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static long            errorCount= 0;
static ThreadLock      threadLock;
static Signal*         signal= NULL; // Signal handler

static Semaphore**     threadSequencer;
static MP_TEST         mp_test;

//----------------------------------------------------------------------------
//
// Class-
//       MySignal
//
// Purpose-
//       My signal handler.
//
//----------------------------------------------------------------------------
class MySignal : public Signal
{
public:
virtual int                         // Return code (0 iff handled)
   handle(                          // Signal handler
     SignalCode        signal)      // Signal code
{
   debugf("[%3ld][%3lx] Signal(%d) '%s' received\n",
          (long)Software::getPid(), (long)Software::getTid(),
          signal, getSignalName(signal));

   errorCount++;                    // This is an error!
   if( signal == Signal::SC_INTERRUPT ) // If user request
     threadLock.debug();            // Debug ThreadLock

   Thread::sleep(0.5);              // Wait just a bit
   return 1;                        // Exit, kill this thread
}
}; // class MySignal

//----------------------------------------------------------------------------
//
// Class-
//       MyThread
//
// Purpose-
//       Thread multiprocessor tests
//
//----------------------------------------------------------------------------
class MyThread : public Thread {
   // Attributes
   public:
   int                 index;       // Thread index

   // Constructor
   inline virtual ~MyThread( void ) {}
   inline MyThread(int index) : Thread(), index(index) {}

    //-------------------------------------------------------------------------
   // MyThread::line()
   public: inline void line(int n)
   {
     debugf("%4d [%2d] MyThread\n", n, index);
   }

   //-------------------------------------------------------------------------
   // MyThread::next()
   public: inline void next( void )
   {
     threadSequencer[index]->wait();
   }

   //-------------------------------------------------------------------------
   // MyThread::post()
   public: inline void post(int next)
   {
     threadSequencer[next]->post();
   }

   //-------------------------------------------------------------------------
   // MyThread::run()
   protected: virtual long run( void );

   //-------------------------------------------------------------------------
   // MyThread::runner() ** FOR BRINGUP SINGLE-THREAD TESTING **
   public: virtual void runner( void ) {
     run();
   }
}; // class MyThread

//----------------------------------------------------------------------------
//
// Subroutine-
//       testLatch
//
// Purpose-
//       Test Latch functions
//
//----------------------------------------------------------------------------
static void
   testLatch( void )                // Test Latch functions
{
   debugf("\n");
   debugf("%s %4d: testLatch()\n", __FILE__, __LINE__);

   // Function complete
   debugf("%s %4d: testLatch() complete\n", __FILE__, __LINE__);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       mpDeadlockTest
//
// Purpose-
//       Test ThreadLock deadlock detector
//
//----------------------------------------------------------------------------
static void
   mpDeadlockTest(                  // Test ThreadLock deadlock
     MyThread*         thread)      // For this thread
{
   debugSetIntensiveMode();         // An abort is expected
   int index= thread->index;

   thread->next();                  // Wait for GO signal
   switch( index )
   {
     case 0:
       thread->line(__LINE__);
       threadLock.obtainSHR("A");
       threadLock.obtainXCL("A-XCL"); // No dependencies
       thread->post(1);
       thread->line(__LINE__);
       thread->next();
       thread->post(1);
       thread->line(__LINE__);
       threadLock.obtainXCL("B");
       break;

     case 1:
       thread->line(__LINE__);
       threadLock.obtainSHR("B");
       threadLock.obtainXCL("B-XCL"); // No dependencies
       thread->post(2);
       thread->line(__LINE__);
       thread->next();
       thread->post(2);
       thread->line(__LINE__);
       threadLock.obtainXCL("C");
       break;

     case 2:
       thread->line(__LINE__);
       threadLock.obtainSHR("C");
       threadLock.obtainXCL("C-XCL"); // No dependencies
       thread->post(0);
       thread->line(__LINE__);
       thread->next();
       thread->line(__LINE__);
       IFDEBUG( threadLock.debug(); )
       threadLock.obtainXCL("A");
       IFDEBUG( threadLock.debug(); )
       break;

     default:
       throwf("%4d [%2d] ShouldNotOccur\n", __LINE__, index);
       break;
   }

   threadLock.threadExit();
   errorCount++;                    // This SHOULD NOT OCCUR
   debugf("%4d [%2d] mpDeadlockTest DID NOT DEADLOCK\n", __LINE__, index);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testLockMPdeadlock
//
// Purpose-
//       Test ThreadLock cyclic deadlock.
//
//----------------------------------------------------------------------------
static inline void                  // (Conditionally tested)
   testLockMPdeadlock( void )       // Test Lock functions on multiprocessor
{
   debugf("\n");
   debugf("%s %4d: testLockMPdeadlock()\n", __FILE__, __LINE__);
   mp_test= mpDeadlockTest;

   Semaphore SA(0);
   Semaphore SB(0);
   Semaphore SC(0);

   Semaphore* array[3]= {&SA, &SB, &SC};
   threadSequencer= array;

   MyThread TA(0);
   MyThread TB(1);
   MyThread TC(2);

   TA.start();
   TB.start();
   TC.start();

   Thread::sleep(0.5);
   SA.post();

   TA.wait();
   TB.wait();
   TC.wait();

   // Function complete
   debugf("%s %4d: testLockMP()deadlock complete\n", __FILE__, __LINE__);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       mpStandardTest
//
// Purpose-
//       Test ThreadLock in MP mode
//
//----------------------------------------------------------------------------
static void
   mpStandardTest(                  // Test ThreadLock deadlock
     MyThread*         thread)      // For this thread
{
   debugSetIntensiveMode();         // An abort is expected
   int index= thread->index;

   thread->next();                  // Wait for GO signal
   switch( index )
   {
     case 0:
       thread->line(__LINE__);
       threadLock.obtainSHR("A");
       threadLock.obtainXCL("A-XCL"); // No dependencies
       thread->post(1);
       thread->line(__LINE__);
       thread->next();
       thread->post(1);
       thread->line(__LINE__);
       threadLock.obtainXCL("B");
       break;

     case 1:
       thread->line(__LINE__);
       threadLock.obtainSHR("B");
       threadLock.obtainXCL("B-XCL"); // No dependencies
       thread->post(2);
       thread->line(__LINE__);
       thread->next();
       thread->post(2);
       thread->line(__LINE__);
       threadLock.obtainXCL("C");
       break;

     case 2:
       thread->line(__LINE__);
       threadLock.obtainSHR("C");
       threadLock.obtainXCL("C-XCL"); // No dependencies
       thread->post(0);
       thread->line(__LINE__);
       thread->next();
       thread->line(__LINE__);
       IFDEBUG( threadLock.debug(); )
//     threadLock.obtainXCL("A");
//     IFDEBUG( threadLock.debug(); )
       break;

     default:
       throwf("%4d [%2d] ShouldNotOccur\n", __LINE__, index);
       break;
   }

   threadLock.threadExit();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testLockMP
//
// Purpose-
//       Test ThreadLock cyclic deadlock.
//
//----------------------------------------------------------------------------
static void
   testLockMP( void )               // Test Lock functions on multiprocessor
{
   debugf("\n");
   debugf("%s %4d: testLockMP()\n", __FILE__, __LINE__);
   mp_test= mpStandardTest;

   Semaphore SA(0);
   Semaphore SB(0);
   Semaphore SC(0);

   Semaphore* array[3]= {&SA, &SB, &SC};
   threadSequencer= array;

   MyThread TA(0);
   MyThread TB(1);
   MyThread TC(2);

   TA.start();
   TB.start();
   TC.start();

   Thread::sleep(0.5);
   SA.post();

   TA.wait();
   TB.wait();
   TC.wait();

   // Function complete
   debugf("%s %4d: testLockMP() complete\n", __FILE__, __LINE__);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testLockUP
//
// Purpose-
//       Test Lock functions on uniprocessor
//
//----------------------------------------------------------------------------
static void
   testLockUP( void )               // Test Lock functions on uniprocessor
{
   debugf("\n");
   debugf("%s %4d: testLockUP()\n", __FILE__, __LINE__);
   debugSetIntensiveMode();

   // Obtain some locks
   debugf("Obtain SHR\n");
   Token t1= threadLock.obtainSHR("SHR lock");
   IFDEBUG( threadLock.debug(); )

   debugf("Attempt SHR\n");
   Token t2= threadLock.attemptSHR("SHR lock");
   IFDEBUG( threadLock.debug(); )

   debugf("Obtain XCL\n");
   Token t3= threadLock.obtainXCL("XCL lock");
   IFDEBUG( threadLock.debug(); )

   debugf("Attempt XCL\n");
   Token t4= threadLock.attemptXCL("XCL lock");
   IFDEBUG( threadLock.debug(); )

   debugf("t1(%p) t2(%p) t3(%p) t4(%p)\n", t1, t2, t3, t4);

   #ifdef USE_UP_DEADLOCK
     debugf("Obtain XCL [deadlock]\n");
     if( FALSE )                    // Test SHR=>XCL deadlock?
       threadLock.obtainXCL("SHR lock");
     else if( FALSE )               // Test XCL=>SHR deadlock?
       threadLock.obtainSHR("XCL lock");
     else                           // XCL=>XCL deadlock
       threadLock.obtainXCL("XCL lock");
     threadLock.debug();
     throwf("DID NOT DEADLOCK");
   #endif

   // Release the locks
   debugf("Release SHR\n");
   threadLock.release(t1);
   IFDEBUG( threadLock.debug(); )

   debugf("Release SHR\n");
   threadLock.release(t2);
   IFDEBUG( threadLock.debug(); )

   debugf("Release XCL\n");
   threadLock.release(t3);
   IFDEBUG( threadLock.debug(); )

   //-------------------------------------------------------------------------
   // Test lock table expansion
   debugf("Expanding lock table...\n");
   for(int i= 0; i<2048; i++)
   {
     char lockName[32];
     sprintf(lockName, "%.5d", i);
     if( (i%10) == 0 )
       threadLock.obtainXCL(lockName);
     else
       threadLock.obtainSHR(lockName);
   }

   IFDEBUG( threadLock.debug(); )   // Verify expansion
   threadLock.threadExit();         // Release all the locks
   IFDEBUG( threadLock.debug(); )   // Verify contraction

   //-------------------------------------------------------------------------
   // Test lock table contraction
   debugf("Contracting lock table...\n");
   for(int i= 0; i<1721; i++)
   {
     Token t= threadLock.obtainSHR("Lock");
     threadLock.release(t);
   }
   IFDEBUG( threadLock.debug(); )   // Verify contraction took place

   // Function complete
   debugf("%s %4d: testLockUP() complete\n", __FILE__, __LINE__);
}

//----------------------------------------------------------------------------
//
// Method-
//       MyThread::run
//
// Purpose-
//       Run multiprocessor test.
//
//----------------------------------------------------------------------------
long
   MyThread::run( void )            // Test Lock functions on uniprocessor
{
   //-----------------------------------------------------------------------
   // Display thread index/identifier correlator
   debugf("%4d [%2d] %lx MyThread::run()\n", __LINE__, index,
          (long)Software::getTid());

   //-----------------------------------------------------------------------
   // Run the test
   (*mp_test)( this);

   return 0;                      // Always good
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
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
   // When debugging, the default debug object is set when in
   // Hard Core Debug Mode. We create an alternate debug object.
   // Note: the log file namme must != "debug.out"
   Debug::set(new ThreadLogger("debug.log")); // Start logging
// debugSetIntensiveMode();

   // Set Signal handler
   signal= new MySignal();

   // Run the tests
   testLatch();
   testLockUP();
   testLockMP();
   #ifdef USE_MP_DEADLOCK
     testLockMPdeadlock();          // Also tests Signal backtrace
   #endif

   debugf("%s complete, ", __FILE__);
   delete signal;
   signal= NULL;

   if( errorCount == 0 )
     debugf("NO ");
   else
     debugf("%ld ", errorCount);
   debugf("Error%s\n", errorCount == 1 ? "" : "s");

   // Close our debugging object.
   Thread::sleep(1.0);
// delete Debug::set(NULL);         // Terminate ThreadLogger (not needed)

   return errorCount;
}

