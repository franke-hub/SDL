//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2021 Frank Eskesen.
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
//       2021/07/21
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
enum { HCDM= false };               // Hard Core Debug Mode?
#define IFHCDM(x) if( HCDM ) {x}

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             fsm= 0;      // Our Finite State Machine
static int             REPS= 4;     // Iteration count
static sem_t           semA;        // Our Semaphore[A]
static sem_t           semB;        // Our Semaphore[B]
static sem_t           semC;        // Our Semaphore[C]
static pthread_t       tidA;        // Thread id[A]
static pthread_t       tidB;        // Thread id[B]
static pthread_t       tidC;        // Thread id[C]

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
   timeval tv;
   gettimeofday(&tv, nullptr);
   double tod = (double)tv.tv_sec;
   tod += (double)tv.tv_usec / 1000000.0;

   return tod;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sem
//
// Purpose-
//       Print semaphore values
//
//----------------------------------------------------------------------------
static void
   sem(                             // Print semaphore values
     int               line,        // At this file line
     const char*       tid)         // In this thread
{
   int A, B, C;                     // The semaphore values
   sem_getvalue(&semA, &A);
   sem_getvalue(&semB, &B);
   sem_getvalue(&semC, &C);
   printf("%12.3f [%s] %4d SEM {%d,%d,%d}\n", tod(), tid, line, A, B, C);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       post
//
// Purpose-
//       Post a semaphore
//
//----------------------------------------------------------------------------
static void
   post(                            // Post a semaphore
     int               line,        // At this file line
     const char*       tid,         // In this thread
     const char*       sid)         // For this semaphore id
{
   sem_t* sem= nullptr;

   switch(*sid) {
     case 'A':
       sem= &semA;
       break;

     case 'B':
       sem= &semB;
       break;

     case 'C':
       sem= &semC;
       break;

     default:
       printf("%4d [%s] SHOULD NOT OCCUR(%s)\n", __LINE__, tid, sid);
       exit(1);
   }

   sem_post(sem);
   int A, B, C;                     // The semaphore values
   sem_getvalue(&semA, &A);
   sem_getvalue(&semB, &B);
   sem_getvalue(&semC, &C);
   IFHCDM(
     printf("%12.3f [%s] %4d HCDM post %s {%d,%d,%d}\n", tod(), tid, line, sid
           , A, B, C);
   )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       wait
//
// Purpose-
//       Wait for a semaphore
//
//----------------------------------------------------------------------------
static void
   wait(                            // Wait for semaphore
     int               line,        // At this file line
     const char*       tid,         // In this thread
     const char*       sid)         // For this semaphore id
{
   sem_t* sem= nullptr;

   switch(*sid) {
     case 'A':
       sem= &semA;
       break;

     case 'B':
       sem= &semB;
       break;

     case 'C':
       sem= &semC;
       break;

     default:
       printf("%4d [%s] SHOULD NOT OCCUR(%s)\n", __LINE__, tid, sid);
       exit(1);
   }

   int A, B, C;                     // The semaphore values
   sem_getvalue(&semA, &A);
   sem_getvalue(&semB, &B);
   sem_getvalue(&semC, &C);
   IFHCDM(
     printf("%12.3f [%s] %4d HCDM wait %s {%d,%d,%d}\n", tod(), tid, line, sid
           , A, B, C);
   )
   sem_wait(sem);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       done
//
// Purpose-
//       Thread exit message
//
//----------------------------------------------------------------------------
static void
   done(                            // Write thread exit message
     int               line,        // At this file line
     const char*       tid)         // In this thread
{
   int A, B, C;                     // The semaphore values
   sem_getvalue(&semA, &A);
   sem_getvalue(&semB, &B);
   sem_getvalue(&semC, &C);
   IFHCDM(
     printf("%12.3f [%s] %4d HCDM done ! {%d,%d,%d}\n", tod(), tid, line
           , A, B, C);
   )
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
     const char*       tid,         // Thread ID
     int               old,         // Current value
     int               chg)         // Updated value
{
   wait(__LINE__, tid, "A");

   int fsm= ::fsm;
   if( old != fsm ) {
     printf("Error: Thread(%s) expected(%d) got(%d)\n", tid, old, fsm);
     throw "ShouldNotOccur";
   }

   IFHCDM(
     printf("%12.3f [%s] %4d HCDM FSM: %d=>%d\n", tod(), tid, __LINE__
           , old, chg);
   )
   ::fsm= chg;

   post(__LINE__, tid, "C");

   wait(__LINE__, tid, "B");
   post(__LINE__, tid, "A");

   wait(__LINE__, tid, "C");
   post(__LINE__, tid, "B");
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
   myThread(                        // Test system calls under thread
     const char*       tid)         // The Thread identifier
{
   IFHCDM(printf("myThread (%s)\n", tid);) // Hello

   //-------------------------------------------------------------------------
   // Initialize semaphores
   IFHCDM(sem(__LINE__, tid);)
   if( *tid == 'A' ) {
     post(__LINE__, tid, "C");
     wait(__LINE__, tid, "A");
     post(__LINE__, tid, "A");
   } else if( *tid == 'B' ) {
     wait(__LINE__, tid, "B");
     post(__LINE__, tid, "A");
   } else if( *tid == 'C' ) {
     wait(__LINE__, tid, "C");
     post(__LINE__, tid, "B");
   } else {
     printf("%12.3f [%s] %4d SHOULD NOT OCCUR\n", tod(), tid, __LINE__);
     sem(__LINE__, tid);
     exit(1);
   }

   //-------------------------------------------------------------------------
   // Simple semaphore operation
#if 1
   IFHCDM(printf("\n\n%12.3f [%s] %4d HCDM alpha\n", tod(), tid, __LINE__);)
   IFHCDM(sem(__LINE__, tid);)

   if( *tid == 'B' ) {
     wait(__LINE__, tid, "C");
     post(__LINE__, tid, "B");
   } else if( *tid == 'C' ) {
     done(__LINE__, tid);
     return;
   }
   for(int i= 0; i<REPS; i++) {
     wait(__LINE__, tid, "A");
     usleep(50000);                 // (Only for time delay)
     post(__LINE__, tid, "C");

     wait(__LINE__, tid, "B");
     post(__LINE__, tid, "A");

     wait(__LINE__, tid, "C");
     post(__LINE__, tid, "B");
   }
   if( *tid == 'A' ) {
     wait(__LINE__, tid, "A");      // (Wait for tid B to wait for sid C)
     post(__LINE__, tid, "C");      // (Allows tid B to complete)
     wait(__LINE__, tid, "B");      // (Wait for tid B to complete)
     post(__LINE__, tid, "A");      // (Allow tid A to start)
   }
#endif

   //-------------------------------------------------------------------------
   // Triple semaphore operation for forced event sequencing
   IFHCDM(printf("\n\n%12.3f [%s] %4d HCDM beta\n", tod(), tid, __LINE__);)
   IFHCDM(sem(__LINE__, tid);)
#if 0
   post(__LINE__, tid, "B");
   done(__LINE__, tid);
   return;
#endif

   if( *tid == 'A' ) {
     for(int i= 0; i<REPS; i++) {
       setFSM(tid, 0, 1);
       setFSM(tid, 2, 3);
       setFSM(tid, 4, 5);
       setFSM(tid, 6, 7);
       setFSM(tid, 8, 9);
     }
     post(__LINE__, tid, "C");
   } else if( *tid == 'B' ) {
     wait(__LINE__, tid, "C");
     post(__LINE__, tid, "B");
     for(int i= 0; i<REPS; i++) {
       setFSM(tid, 1, 2);
       setFSM(tid, 3, 4);
       setFSM(tid, 5, 6);
       setFSM(tid, 7, 8);
       setFSM(tid, 9, 0);
     }
   }

   done(__LINE__, tid);
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
   asThread(void* tid)               // Test system calls under thread
{
   try {
     myThread((const char*)tid);
   } catch( const char* X ) {
     printf("Exception(%s)\n", X);
     exit(1);
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
   main(                            // Mainline code
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   //-------------------------------------------------------------------------
   // Get parameter (repetition count)
   //-------------------------------------------------------------------------
   if( argc > 1 ) {                 // If a parameter was specified
     REPS= atoi(argv[1]);           // Set repetition count
     if( REPS <= 0 ) {              // If invalid
       fprintf(stderr, "Invalid repetition count '%s'\n", argv[1]);
       exit(1);
     }
   }

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   int rc= sem_init(&semA, 0, 0);   // Initialize (LOCKED)
   IFHCDM(printf("%d= sem_init(A)\n", rc);)
   rc= sem_init(&semB, 0, 0);       // Initialize (LOCKED)
   IFHCDM(printf("%d= sem_init(B)\n", rc);)
   rc= sem_init(&semC, 0, 0);       // Initialize (LOCKED)
   IFHCDM(printf("%d= sem_init(C)\n", rc);)

   //-------------------------------------------------------------------------
   // Run the tests
   //-------------------------------------------------------------------------
   pthread_attr_t      tAttr;       // Thread attributes

   pthread_attr_init(&tAttr);       // Initialize the attributes
   pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_JOINABLE);
   pthread_create(&tidA, &tAttr, asThread, (void*)"A");
   pthread_create(&tidB, &tAttr, asThread, (void*)"B");
   pthread_create(&tidC, &tAttr, asThread, (void*)"C");
   pthread_attr_destroy(&tAttr);    // Destroy the attributes

   //-------------------------------------------------------------------------
   // Function complete
   //-------------------------------------------------------------------------
   pthread_join(tidA, NULL);
   pthread_join(tidB, NULL);
   pthread_join(tidC, NULL);
   IFHCDM(sem(__LINE__, "*");)

   int A, B, C;                     // The semaphore values
   sem_getvalue(&semA, &A);
   sem_getvalue(&semB, &B);
   sem_getvalue(&semC, &C);
   if( A != 1 || B != 1 || C != 0 ) {
     printf("%4d Unexpected semaphore values\n", __LINE__);
     fsm= -1;
   }

   sem_destroy(&semC);
   sem_destroy(&semB);
   sem_destroy(&semA);

   if( fsm == 0 )
     printf("%12.3f OK!\n", tod());
   else {
     printf("Error: expected(%d) fsm(%d) at end\n", 0, fsm);
     fsm= 2;
   }

   return fsm;
}
