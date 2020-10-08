//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Semaphore.cpp
//
// Purpose-
//       Test the operation of pthread and semaphore
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <pthread.h>                // Must be first

#include <sched.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/timeb.h>

#include <com/Debug.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             fsm;         // Our Finite State Machine
static sem_t           fsmA;        // Our Semaphore
static sem_t           fsmB;        // Our Semaphore
static sem_t           fsmC;        // Our Semaphore
static pthread_t       tidA;        // Thread id "A"
static pthread_t       tidB;        // Thread id "B"

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
//       setFSM
//
// Purpose-
//       Update the current FSM
//
//----------------------------------------------------------------------------
static void
   setFSM(                          // Update the FSM
     int               old,         // Current value
     int               chg)         // Updated value
{
   sem_wait(&fsmA);
   sem_post(&fsmC);

   int fsm= ::fsm;
   if( old != fsm )
   {
     debugf("Error: expected(%d) got(%d)\n", old, fsm);
     throw "ShouldNotOccur";
   }

   debugf("%12.3f [%p] FSM %d=>%d\n", tod(), (void*)pthread_self(), old, chg);
   ::fsm= chg;

   sem_wait(&fsmB);
   sem_post(&fsmA);

   sem_wait(&fsmC);
   sem_post(&fsmB);
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
   int                 i;
   pthread_t tid= pthread_self();

   //-------------------------------------------------------------------------
   // Simple semaphore operation
   for(i= 0; i<8; i++)
   {
     debugf("%12.3f [%p] %4d HCDM\n", tod(), (void*)tid, __LINE__);
     sem_wait(&fsmA);
     debugf("%12.3f [%p] %4d HCDM\n", tod(), (void*)tid, __LINE__);
     usleep(50000);
     sem_post(&fsmA);
   }

   //-------------------------------------------------------------------------
   // Triple semaphore operation for forced event sequencing
   if( tid == tidA )
   {
     for(i= 0; i<8; i++)
     {
       setFSM(0, 1);
       setFSM(2, 3);
       setFSM(4, 5);
       setFSM(6, 7);
       setFSM(8, 9);
     }
     sem_post(&fsmC);
   }
   else if( tid == tidB )
   {
     sem_wait(&fsmC);
     for(i= 0; i<8; i++)
     {
       setFSM(1, 2);
       setFSM(3, 4);
       setFSM(5, 6);
       setFSM(7, 8);
       setFSM(9, 0);
     }
   }
   else
     debugf("Invalid TID(%p) neither A(%p) nor B(%p)\n",
            (void*)tid, (void*)tidA, (void*)tidB);
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
   debugf("asThread (%p)\n", (void*)pthread_self());
   try {
     myThread();
   } catch( const char* X ) {
     debugf("Exception(%s)\n", X);
   }

   return NULL;
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
   fsm= (-1);
   int rc= sem_init(&fsmA, 0, 0);   // Initialize (LOCKED)
   debugf("%d= sem_init()\n", rc);
   sem_init(&fsmB, 0, 0);           // Initialize (LOCKED)
   sem_init(&fsmC, 0, 0);           // Initialize (LOCKED)

   //-------------------------------------------------------------------------
   // Run the tests
   //-------------------------------------------------------------------------
   pthread_attr_t      tAttr;       // Thread attributes

   pthread_attr_init(&tAttr);       // Initialize the attributes
   pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_JOINABLE);
   pthread_create(&tidB, &tAttr, asThread, NULL);
   pthread_create(&tidA, &tAttr, asThread, NULL);
   pthread_attr_destroy(&tAttr);    // Destroy the attributes

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   debugf("tidA(%p)\n", (void*)tidA);
   debugf("tidB(%p)\n", (void*)tidB);
   fsm= 0;
   sem_post(&fsmA);
   sem_post(&fsmB);

   pthread_join(tidA, NULL);
   pthread_join(tidB, NULL);

   sem_destroy(&fsmC);
   sem_destroy(&fsmB);
   sem_destroy(&fsmA);

   if( fsm != 0 )
     debugf("Error: expected(%d) got(%d) at end\n", 0, fsm);

   return 0;
}

