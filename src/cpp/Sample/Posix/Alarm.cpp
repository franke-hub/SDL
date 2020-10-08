//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Alarm.cpp
//
// Purpose-
//       Test the operation of alarm and setitimer
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <pthread.h>                // Must be first
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>

#include <com/Debug.h>

#ifdef _OS_BSD
#include <sys/signal.h>
#endif

#if defined(_OS_CYGWIN) || defined(_OS_LINUX)
#define sigvec struct sigaction     // Must follow sys/signal.h
#endif

#ifdef _OS_WIN
#include <signal.h>                 // Must follow com/Signal.h
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef USE_SLEEP
#define USE_SLEEP FALSE             // Use sleep?
#endif

#define SIG_COUNT 64                // Number of signal entries

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             posted= FALSE; // TRUE iff alarm went off

//----------------------------------------------------------------------------
//
// Subroutine-
//       tod
//
// Purpose-
//       Get the current time.
//
//----------------------------------------------------------------------------
static double
   tod( void )                      // The current time
{
   struct timeb        ticker;      // UTC time base

   ftime(&ticker);                  // UTC (since epoch)
   unsigned long secs= (unsigned long)ticker.time;
   unsigned long msec= (unsigned long)ticker.millitm;

   double result= (double)secs + double(msec)/1000.0;
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sleeper
//
// Purpose-
//       Wait for alarm event.
//
//----------------------------------------------------------------------------
static void
   sleeper( void )                  // Wait for alarm event
{
   size_t              count;       // Loop count

   for(count= 0; ; count++)
   {
     if( posted )
       break;

     #if( USE_SLEEP )
       sleep(10);
       debugf("%12.3f sleeper()\n", tod());
     #else
       if( (count%10000000000LL) == 0 )
         debugf("%12.3f sleeper()\n", tod());
     #endif
   }

   #if( !USE_SLEEP )
     debugf("%12.3f count(%zu)\n", tod(), count);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sigexit
//
// Function-
//       Signal handler
//
//----------------------------------------------------------------------------
static void
   sigexit(                         // Signal handler
     int               signalId)    // Signal identifier
{
   #ifdef HCDM
     debugf("%12.3f sigexit(%d)\n", tod(), signalId);
     Debug::get()->flush();
   #endif

   posted= TRUE;                    // POSTED
   fflush(stdout);                  // Force the standard buffers
   fflush(stderr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       initialize
//
// Purpose-
//       Set up signal handlers.
//
//----------------------------------------------------------------------------
#if defined(_OS_WIN)
static void
   initialize( void )               // Set up signal handlers
{
   int                 i;

   #ifdef HCDM
     printf("%8s= ::initialize()\n", "........");
   #endif

   for(i= 1; i<SIG_COUNT; i++) // Set up exits
   {
     signal(i, &sigexit);
   }
}
#else // _OS_BSD

static void
   initialize( void )               // Set up signal handlers
{
   sigvec              inpvec;      // Input vector
   sigvec              outvec;      // Output vector

   int                 i;

   #ifdef HCDM
     debugf("::initialize()\n");
   #endif

   memset(&inpvec, 0, sizeof(inpvec));
   inpvec.sa_handler= &sigexit;
   for(i= 1; i<SIG_COUNT; i++)      // Set up exits
   {
     sigaction(i, &inpvec, &outvec);
   }
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       testALARM
//
// Purpose-
//       Test the ALARM system call.
//
//----------------------------------------------------------------------------
static void
   testALARM( void )                // Test ALARM system call
{
   posted= FALSE;
   debugf("%12.3f alarm(5)\n", tod());
   alarm(5);

   debugf("%12.3f alarm(2)\n", tod());
   alarm(2);

   sleeper();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testSETITIMER
//
// Purpose-
//       Test the SETITIMER system call.
//
//----------------------------------------------------------------------------
static void
   testSETITIMER( void )            // Test ALARM system call
{
   itimerval itold, itnew;

   itnew.it_interval.tv_sec= 0;
   itnew.it_interval.tv_usec= 0;
   itnew.it_value.tv_sec= 3;
   itnew.it_value.tv_usec= 500000;

   posted= FALSE;
   debugf("%12.3f setitimer(3.5)\n", tod());
   setitimer(ITIMER_REAL, &itnew, &itold);

   itnew.it_value.tv_usec= 400000;
   debugf("%12.3f setitimer(3.4)\n", tod());
   setitimer(ITIMER_REAL, &itnew, &itold);
   sleeper();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       myThread
//
// Purpose-
//       Test the system calls running under a thread.
//
//----------------------------------------------------------------------------
static void
   myThread( void )                 // Test system calls under thread
{
   testALARM();
   testSETITIMER();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       asThread
//
// Purpose-
//       Test the system calls running under a thread.
//
//----------------------------------------------------------------------------
static void*
   asThread(void*)                  // Test system calls under thread
{
   debugf("asThread\n");
   myThread();
   return NULL;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testThread
//
// Purpose-
//       Test the system calls running under a thread.
//
//----------------------------------------------------------------------------
static void
   testThread( void )               // Test system calls under thread
{
   pthread_attr_t      tAttr;       // Thread attributes
   pthread_t           tid;         // Thread identifier

   pthread_attr_init(&tAttr);       // Initialize the attributes
   pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_JOINABLE);
   pthread_create(&tid, &tAttr, asThread, NULL);
   pthread_attr_destroy(&tAttr);    // Destroy the attributes

   pthread_join(tid, NULL);
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
   main(int, char**)                // Mainline code
//   int             argc,          // Argument count (Unused)
//   char*           argv[])        // Argument array (Unused)
{
   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   initialize();

   //-------------------------------------------------------------------------
   // Run the tests
   //-------------------------------------------------------------------------
   myThread();
   testThread();

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   return 0;
}

