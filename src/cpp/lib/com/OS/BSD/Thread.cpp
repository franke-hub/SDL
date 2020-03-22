//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/BSD/Thread.cpp
//
// Purpose-
//       Thread object methods.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <pthread.h>                // Must be first

#include <errno.h>
#include <sched.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/Atomic.h>
#include <com/Debug.h>
#include <com/Latch.h>
#include <com/List.h>
#include <com/Unconditional.h>

#include "com/Thread.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard-Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Versioning
//----------------------------------------------------------------------------
#ifndef PTHREAD_CREATE_JOINABLE
#define PTHREAD_CREATE_JOINABLE PTHREAD_CREATE_UNDETACHED
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       Object
//
// Purpose-
//       Define the hidden Thread Object.
//
//----------------------------------------------------------------------------
enum FSM                            // Finite State Machine states
{  FSM_IDLE= 0                      // IDLE, (None of the ACTIVE states)
,  FSM_ACTIVE                       // ACTIVE (Running)
,  FSM_WAIT                         // ACTIVE (Running, thread waiting)
,  FSM_JOIN                         // ACTIVE (Complete, JOIN required)
}; // enum FSM

struct Object : public AU_List<Object>::Link // Hidden Object
{
   Thread*             thread;      // -> Associated Thread

   long                compCode;    // The completion code
   ATOMIC32            fsm;         // Finite State Machine
   pthread_t           ident;       // Thread identifier
   int                 priority;    // Thread priority
   unsigned long       stack;       // The starting stack size
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Latch           latch;       // List Latch object
static AU_List<Object> list;        // Object list
static _SystemThread   main_;       // The main Thread

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::debugObject
//
// Purpose-
//       Display an Object
//
//----------------------------------------------------------------------------
#ifdef HCDM
static void
   debugObject(                     // Display a Object
     Object*           O,           // -> Object to display
     Thread*           thread)      // -> Thread to display
{
   AutoLatchXCL lock(latch);

   debugf("_Thread(%p)::debugObject(%p)\n", thread, O);
   if( O != NULL )
   {
     debugf(">> chain: %p\n", O->getPrev());
     debugf(">>   FSM: %d\n", O->fsm);
     debugf(">> CCode: %ld\n", O->compCode);
     debugf(">> ident: %p\n", (void*)O->ident);
     debugf(">> prior: %.8lx\n", (long)O->priority);
     debugf(">> stack: %.8lx\n", (long)O->stack);
   }
}

static inline void
   debugObjectList( void )          // Display the Object list
{
   AutoLatchXCL lock(latch);

   debugf(">>Object list:\n");
   for(Object* O= list.getTail(); O != NULL; O= O->getPrev())
     debugf(">>  [%p] Thread(%p) ident(%p) fsm(%d)\n",
            O, O->thread, (void*)O->ident, O->fsm);
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::createObject
//
// Purpose-
//       Allocate space for the hidden Object.
//
//----------------------------------------------------------------------------
static Object*                      // -> Object
   createObject( void )             // Allocate the hidden Objects
{
   Object*             O;

   O= (Object*)must_malloc(sizeof(Object));
   memset(O, 0, sizeof(Object));

   IFHCDM( debugf("%p= _Thread::createObject()\n", O); )

   return O;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::destroyObject
//
// Purpose-
//       Release space for the hidden Objects.
//
//----------------------------------------------------------------------------
static void
   destroyObject(                   // Delete the hidden Object
     Object*           O)           // -> Object
{
   IFHCDM( debugf("_Thread(%p)::destroyObject(%p)\n", O->thread, O); )

   free(O);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::setFSM
//
// Purpose-
//       Update the thread state.
//
// Implementation notes-
//       SHR or XCL latch held.
//
//----------------------------------------------------------------------------
static inline int                   // The prior state
   setFSM(                          // Update the state
     Object*           O,           // -> Object
     int               fsm)         // The new state
{
   int                 oldValue;    // Current state
   int                 cc;          // Condition code

   do
   {
     oldValue= O->fsm;
     cc= csw(&O->fsm, oldValue, fsm);
   } while( cc != 0 );

   IFHCDM(
     debugf("%3d= _Thread(%p)::setFSM(%d) [%p]\n", oldValue, O->thread, fsm, O);
   )

   return oldValue;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::threadDriver
//
// Purpose-
//       Drive the Thread.
//
//----------------------------------------------------------------------------
extern "C" {
static void*                        // Always NULL
   threadDriver(                    // Drive the Thread
     void*             parm)        // -> Object
{
   Object*             O= (Object*)parm; // -> Object
   Thread*             thread= O->thread; // -> Thread

   IFHCDM(
     debugf("_Thread(%p)::threadDriver() %p [%p]\n",
            thread, (void*)pthread_self(), O);
     debugObject(O, thread);
   )

   if( thread != NULL )
     O->compCode= _SystemThread::run(thread);

   IFHCDM(
     debugf("%8ld= _Thread(%p)::run() [%p]\n", O->compCode, O->thread, O);
   )

   AutoLatchSHR lock(latch);

   setFSM(O, FSM_JOIN);             // Thread complete
   if( O->thread == NULL )          // If Thread::~Thread has been run
     destroyObject(O);

   return NULL;
}
} // extern "C"

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::cancel
//
// Purpose-
//       Terminate thread processing.
//
//----------------------------------------------------------------------------
void
   _SystemThread::cancel(           // Terminate Thread processing
     Thread*           thread)      // This Thread
{
   IFHCDM( debugf("_Thread(%p)::cancel()\n", thread); )

   AutoLatchXCL lock(latch);

   Object* O= (Object*)(thread->object); // Thread Object*
   if( O != NULL )
   {
     if( O->ident == pthread_self() ) // If should have used exit
     {
       latch.releaseXCL();          // There is no exit from this scope!
       pthread_exit(NULL);          // Just use exit
     }

     if( O->ident != 0 )            // If the thread is active
       pthread_cancel(O->ident);    // Terminate the thread
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::create
//
// Purpose-
//       Create a Thread
//
//----------------------------------------------------------------------------
void
   _SystemThread::create(           // Create
     Thread*           thread)      // This Thread
{
   Object*             O;           // -> Object

   O= createObject();
   thread->object= O;               // Set the Thread->Object ref
   O->thread= thread;               // Set the Object->Thread ref
   O->fsm= FSM_IDLE;                // Start in IDLE State
   if( thread == &main_ )
     O->ident= pthread_self();

   //-------------------------------------------------------------------------
   // Atomically add this Object to the Object list
   //-------------------------------------------------------------------------
   list.fifo(O);                    // Add element to list

   IFHCDM(
//   debugSetIntensiveMode();
     debugf("_Thread(%p)::create() [%p]\n", thread, O);
     debugObject(O, thread);
   )
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::current
//
// Purpose-
//       Return the current Thread
//
//----------------------------------------------------------------------------
Thread*                             // -> Current Thread
   _SystemThread::current( void )   // Return the current Thread
{
   Thread*             thread= NULL;// Resultant
   Object*             O;           // -> Object

   pthread_t ident= pthread_self(); // Set thread identifier
   {{{{
     AutoLatchSHR lock(latch);

     for(O= list.getTail(); O != NULL; O= O->getPrev())
     {
       if( O->ident == ident )
       {
         thread= O->thread;
         break;
       }
     }
   }}}}

   #if defined(HCDM) && FALSE
     debugf("%p= _Thread::current() %p\n", thread, (void*)ident);
     debugObject(O, thread);
   #endif

   return thread;                   // Return the associated Thread
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::destroy
//
// Purpose-
//       Destroy a Thread
//
//----------------------------------------------------------------------------
void
   _SystemThread::destroy(          // Destroy
     Thread*           thread)      // This Thread
{
// Object*             link;        // Working Object*
   Object*             O= (Object*)(thread->object); // Thread Object*

   IFHCDM(
     debugf("_Thread(%p)::destroy() [%p]\n", thread, O);
     debugObject(O, thread);
   )

   if( O != NULL )
   {
     AutoLatchXCL lock(latch);

     list.remove(O);                // Remove Object from list

     thread->object= NULL;
     O->thread= NULL;
     switch( O->fsm )
     {
       case FSM_ACTIVE:             // If currently running
         pthread_detach(O->ident);  // No synchronization is possible
         break;

       case FSM_JOIN:               // If completed, not waiting
         pthread_detach(O->ident);  // No synchronization is possible
         destroyObject(O);
         break;

       case FSM_IDLE:               // If not active
         destroyObject(O);
         break;

       case FSM_WAIT:               // If waiting for completion
         // The Thread has an active waiter, normally the creating Thread.
         // However, some other thread is deleting this Thread now.
         // This is problematic. The waiter thinks the Thread still exists.
         // We will delete the object when the wait completes.
         pthread_detach(O->ident);  // This should cause the wait to complete

         // Complain about this usage error
         debugf("Thread(%p) (%p) called Thread(%p)::~Thread(),\n"
                "but Thread(%p)::wait() is currently running.\n",
                current(), (void*)pthread_self(), thread, thread);

         // We don't want to throw an exception during delete
         break;                     // So we just exit

       default:
         throwf("%4d %s Thread(%p) FSM(%d)", __LINE__, __FILE__, thread, O->fsm);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::exit
//
// Purpose-
//       Exit from the current Thread.
//
//----------------------------------------------------------------------------
void
   _SystemThread::exit(             // Exit from the current Thread
     long              returnCode)  // The thread return code
{
   IFHCDM( debugf("_Thread::exit()\n"); )

   pthread_exit((void*)returnCode); // Exit from the current Thread
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::getPriority
//
// Purpose-
//       Return the priority
//
//----------------------------------------------------------------------------
int                                 // The Thread's priority
   _SystemThread::getPriority(      // Get priority
     const Thread*     thread)      // For this Thread
{
   Object*             O= (Object*)(thread->object); // Get the Object

   return O->priority;
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::getStackSize
//
// Purpose-
//       Return the stack size
//
//----------------------------------------------------------------------------
unsigned long                       // The Thread's stack size
   _SystemThread::getStackSize(     // Get stack size
     const Thread*     thread)      // For this Thread
{
   Object*             O= (Object*)(thread->object); // Get the Object

   return O->stack;
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::getThreadID
//
// Purpose-
//       Return the thread identifier
//
//----------------------------------------------------------------------------
unsigned long                       // The thread identifier
   _SystemThread::getThreadID(      // Get thread identifier
     const Thread*     thread)      // For this Thread
{
   Object*             O= (Object*)(thread->object); // Get the Object

   return (unsigned long)O->ident;
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::run
//
// Purpose-
//       Call protected method Thread::run()
//
//----------------------------------------------------------------------------
long                                // The Thread's return code
   _SystemThread::run(              // Run
     Thread*           thread)      // This Thread
{
   return thread->run();
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::setPriority
//
// Purpose-
//       Update the priority
//
//----------------------------------------------------------------------------
void
   _SystemThread::setPriority(      // Update priority
     Thread*           thread,      // For this Thread
     int               priority)    // With this change
{
   Object*             O= (Object*)(thread->object); // Get the Object

   AutoLatchXCL lock(latch);

   O->priority += priority;

   if( O->fsm == FSM_ACTIVE )       // If the Thread is running
   {
     if( O->ident == 0 )            // If not started
       return;                      // Ignore setPriority

     // Update the priority
     int policy= sched_getscheduler(0);
     int min_priority= sched_get_priority_min(policy);
     int max_priority= sched_get_priority_max(policy);
     int new_priority= (min_priority + max_priority) / 2;

     if( max_priority >= min_priority )
     {
       new_priority += O->priority;
       if( new_priority < min_priority )
         new_priority= min_priority;
       if( new_priority > max_priority )
         new_priority= max_priority;
     }
     else
     {
       new_priority -= O->priority;
       if( new_priority > min_priority )
         new_priority= min_priority;
       if( new_priority < max_priority )
         new_priority= max_priority;
     }

     sched_param param;
     param.sched_priority= new_priority;
     pthread_setschedparam(O->ident, policy, &param); // (Error IGNORED)
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::setStackSize
//
// Purpose-
//       Update the stack size
//
//----------------------------------------------------------------------------
void
   _SystemThread::setStackSize(     // Set stack size
     Thread*           thread,      // For this Thread
     unsigned long     size)        // To this size
{
   Object*             O= (Object*)(thread->object); // Get the Object

   O->stack= size;
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::sleep
//
// Purpose-
//       Suspend the current Thread.
//
//----------------------------------------------------------------------------
void
   _SystemThread::sleep(            // Suspend the current Thread
     unsigned          secs,        // For this many seconds
     unsigned          nsec)        // and this many nanoseconds
{
   unsigned            usec= (nsec+500) / 1000; // Time in microseconds

   ::sleep(secs);                   // Seconds sleep
   if( usec > 0 )                   // If some number of microseconds
     ::usleep(usec);                // Microsecond sleep
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::start
//
// Purpose-
//       Start the Thread.
//
//----------------------------------------------------------------------------
void
   _SystemThread::start(            // Start
     Thread*           thread)      // This Thread
{
   Object*             O= (Object*)(thread->object); // Get the Object
   pthread_attr_t      tAttr;       // Thread attributes

   int                 rc;          // Called routine return code

   IFHCDM(
     debugf("_Thread(%p)::start()\n", thread);
     debugObject(O, thread);
   )

   {{{{
     AutoLatchSHR lock(latch);

     if( O->fsm != FSM_IDLE )
       throwf("%4d %s Thread(%p) FSM(%d)", __LINE__, __FILE__, thread, O->fsm);

     O->ident= 0;
     O->compCode= (-1);
     setFSM(O, FSM_ACTIVE);
   }}}}

   rc= pthread_attr_init(&tAttr);   // Initialize the attributes
   if( rc != 0 )
     throwf("%4d %s Thread(%p) rc(%d)", __LINE__, __FILE__, thread, rc);

   pthread_attr_setdetachstate(&tAttr, PTHREAD_CREATE_JOINABLE);
   pthread_attr_setstacksize(&tAttr, O->stack);

   int counter= 0;
   rc= pthread_create(&O->ident, &tAttr, threadDriver, O);
   while( rc == EAGAIN )
   {
     counter++;
     if( counter > 100 )
     {
       fprintf(stderr, "%4d Thread(%p)::start, (retrying)\n", __LINE__, thread);
       counter= 0;
     }

     usleep(100000);
     rc= pthread_create(&O->ident, &tAttr, threadDriver, O);
   }
   if( rc != 0 )
     throwf("%4d %s Thread(%p) rc(%d)", __LINE__, __FILE__, thread, rc);

   if( O->priority != 0 )
     setPriority(thread, 0);        // Activate the Thread priority

   pthread_attr_destroy(&tAttr);    // Destroy the attributes
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::wait
//
// Purpose-
//       Wait for a Thread to complete
//
//----------------------------------------------------------------------------
long                                // The Thread's return code
   _SystemThread::wait(             // Wait for completion
     Thread*           thread)      // For this Thread
{
   Object*             O= (Object*)(thread->object); // Get the Object
   int                 oldValue;    // Prior FSM

   IFHCDM( debugf("_Thread(%p)::wait() [%p]\n", thread, O); )

   {{{{
     AutoLatchXCL lock(latch);

     oldValue= O->fsm;
     if( oldValue == FSM_ACTIVE )
       setFSM(O, FSM_WAIT);

     if( oldValue == FSM_WAIT )
       throwf("%4d %s Thread(%p) Usage error", __LINE__, __FILE__, thread);
   }}}}

   if( oldValue == FSM_ACTIVE || oldValue == FSM_JOIN )
   {
     int rc= pthread_join(O->ident, NULL);
     if( rc != 0 )
       throwf("%4d %s Thread(%p) System error", __LINE__, __FILE__, thread);

     AutoLatchSHR lock(latch);
     if( O->thread == NULL )        // If Thread::~Thread has been run
     {
       destroyObject(O);            // Delete the associated Object
       throwf("%4d %s Thread(%p) Usage error", __LINE__, __FILE__, thread);
     }

     O->ident= 0;
     setFSM(O, FSM_IDLE);
   }

   return O->compCode;
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::yield
//
// Purpose-
//       Yield control to another Thread
//
//----------------------------------------------------------------------------
void
   _SystemThread::yield( void )     // Yield control to another Thread
{
   ::sched_yield();                 // Yield control to another Thread
}

