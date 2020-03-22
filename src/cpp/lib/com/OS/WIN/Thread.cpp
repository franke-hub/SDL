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
//       OS/WIN/Thread.cpp
//
// Purpose-
//       Thread object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <windows.h>

#include <com/Atomic.h>
#include <com/define.h>
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
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define HASHSIZE                256 // Size of hash table (power of 2)

//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
typedef unsigned long  Token;       // Attribute (token)

//----------------------------------------------------------------------------
//
// Struct-
//       Object
//
// Purpose-
//       Describe the hidden Thread object.
//
//----------------------------------------------------------------------------
enum FSM                            // Finite State Machine states
{  FSM_IDLE= 0                      // IDLE (None of the ACTIVE states)
,  FSM_ACTIVE                       // ACTIVE (Running)
,  FSM_WAIT                         // ACTIVE (Running, thread waiting)
,  FSM_JOIN                         // ACTIVE (Complete, JOIN required)
}; // enum FSM

struct Object : public AU_List<Object>::Link // Hidden Object
{
   Thread*             thread;      // -> Thread object

   long                compCode;    // The completion code
   ATOMIC32            fsm;         // Finite State Machine
   HANDLE              handle;      // WIN Thread Handle
   DWORD               ident;       // WIN Thread Identifier
   int                 priority;    // The Thread's priority
   unsigned long       stack;       // The starting stack size
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Latch           latch;       // Local latch
static SECURITY_ATTRIBUTES
                       threadSecurityAttributes;

static AU_List<Object> list;                 // The list of Objects
static _SystemThread   main;                 // The main Thread

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
     debugf(">>thread: %p\n", O->thread);
     debugf(">>   FSM: %d\n", O->fsm);
     debugf(">> CCode: %ld\n", O->compCode);
     debugf(">>  HAND: %p\n", O->handle);
     debugf(">> ident: %.8lx\n", (long)O->ident);
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
     debugf(">>  [%p] Thread(%p) ident(%.8lx) fsm(%d)\n",
            O, O->thread, (long)O->ident, O->fsm);
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

   if( O->handle != NULL )
     ::CloseHandle(O->handle);

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
static DWORD                        // Return code
WINAPI                              // This is a WINAPI routine
   threadDriver(                    // Drive the Thread
     LPVOID            parm)        // -> Object
{
   Object*             O= (Object*)parm; // -> Object
   Thread*             thread= O->thread; // -> Thread

   IFHCDM(
     debugf("_Thread(%p)::threadDriver() %.8lx [%p]\n",
            thread, (long)::GetCurrentThreadId(), O);
     debugObject(O, thread);
   )

   if( thread != NULL )
     O->compCode= _SystemThread::run(thread);

   IFHCDM(
     debugf("%8ld= _Thread(%p)::run() [%p]\n", O->compCode, O->thread, O);
   )

   AutoLatchSHR lock(latch);

   setFSM(O, FSM_JOIN);
   if( O->thread == NULL )          // If Thread::~Thread has been run
     destroyObject(O);

   return O->compCode;
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::cancel
//
// Purpose-
//       Terminate Thread processing.
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
     if( O->ident == ::GetCurrentThreadId() ) // If should have used exit
     {
       latch.releaseXCL();          // There is no exit from this scope!
       ::ExitThread(-1);            // Just use exit
     }

     if( O->handle != NULL )        // If the thread is active
       ::TerminateThread(O->handle, -1); // Terminate the Thread
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::create
//
// Purpose-
//       Create a Thread.
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
   if( thread == &main )
   {
     O->ident= ::GetCurrentThreadId();

     // Initialize threadSecurityAttributes
     threadSecurityAttributes.nLength= sizeof(threadSecurityAttributes);
     threadSecurityAttributes.lpSecurityDescriptor= NULL;
     threadSecurityAttributes.bInheritHandle= TRUE;
   }

   //-------------------------------------------------------------------------
   // Atomically add this Object to the Object list
   //-------------------------------------------------------------------------
   list.fifo(O);

   IFHCDM(
     debugSetIntensiveMode();
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

   DWORD ident= ::GetCurrentThreadId(); // Set thread identifier
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
     debugf("%p= _Thread::current() %.8lx\n", thread, (long)ident);
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
   Object*             link;        // Working Object*
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
         ;;                         // The Object will be deleted when complete
         break;

       case FSM_JOIN:               // If completed, not waiting
       case FSM_IDLE:               // If not active
         destroyObject(O);
         break;

       case FSM_WAIT:               // If waiting for completion
         // The Thread has an active waiter, normally the creating Thread.
         // However, some other thread is deleting this Thread now.
         // This is problematic. The waiter thinks the Thread still exists.
         // We will delete the object when the wait completes.
         ::CloseHandle(O->handle);  // This should cause the wait to complete

         // Complain about this usage error
         debugf("Thread(%p) (%ld) called Thread(%p)::~Thread(),\n"
                "but Thread(%p)::wait() is currently running.\n",
                current(), (long)::GetCurrentThreadId(), thread, thread);

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

   ::ExitThread(returnCode);        // Exit from the current Thread
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::getPriority
//
// Purpose-
//       Return the Thread's priority
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

   return O->ident;
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
//       Update the Thread's priority
//
//----------------------------------------------------------------------------
void
   _SystemThread::setPriority(      // Set priority
     Thread*           thread,      // For this Thread
     int               priority)    // To this value
{
   Object*             O= (Object*)(thread->object); // Get the Object

   O->priority= priority;
   // TODO: Make it actually work
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
   unsigned            msec;        // Sleep time in milliseconds

   if( secs > (0xffffffffU/1000U) ) // If more than 4,000,000 or so seconds
     msec= INFINITE - 1;            // That's almost infinite
   else
   {
     msec= secs * 1000;             // Account for seconds
     msec += (nsec+500000)/1000000; // Account for nanoseconds

     if( msec == 0 && nsec != 0 )   // Roll up first millisecond
       msec= 1;
   }

   ::Sleep(msec);                   // Suspend the current Thread
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::start
//
// Purpose-
//       Start the Thread
//
//----------------------------------------------------------------------------
void
   _SystemThread::start(            // Start
     Thread*           thread)      // This Thread
{
   Object*             O= (Object*)(thread->object); // Get the Object

   IFHCDM(
     debugf("_Thread(%p)::start() [%p]\n", thread, O);
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

   if( O->handle != NULL )
     CloseHandle(O->handle);

   O->handle= ::CreateThread(       // Create Thread
                  &threadSecurityAttributes,
                  O->stack,         // The stack size
                  threadDriver,     // -> Thread Driver
                  O,                // Driver argument
                  0,                // Create Active
                  &O->ident);       // -> Thread identifier
   if (O->handle == NULL)
     throwf("%4d %s Thread(%p)", __LINE__, __FILE__, thread);

   if( O->priority != 0 )
     setPriority(thread, 0);        // Activate the Thread priority
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
     for(;;)                          // Wait for Thread completion
     {
       int rc= WaitForSingleObject(O->handle, INFINITE);
       if( rc != WAIT_TIMEOUT )
         break;
     }

     AutoLatchSHR lock(latch);
     if( O->thread == NULL )        // If Thread::~Thread has been run
     {
       destroyObject(O);            // Delete the associated Object
       throwf("%4d %s Thread(%p) Usage error", __LINE__, __FILE__, thread);
     }

     O->ident= 0;
     setFSM(O, FSM_IDLE);

     ::CloseHandle(O->handle);      // The handle has no further use
     O->handle= NULL;
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
   ::Sleep(0);                      // Yield control to another Thread
}

