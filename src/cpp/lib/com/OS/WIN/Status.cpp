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
//       OS/WIN/Status.cpp
//
// Purpose-
//       Status object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <windows.h>

#include <com/Barrier.h>
#include <com/define.h>
#include <com/Debug.h>
#include <com/Thread.h>
#include <com/Unconditional.h>

#include "com/Status.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       Object
//
// Purpose-
//       Define the hidden Status Object
//
//----------------------------------------------------------------------------
enum FSM                            // Finite State Machine states
{  FSM_INIT= 0                      // INITial state (not posted)
,  FSM_WAIT                         // WAITing (waiter exists)
,  FSM_POST                         // POSTed
}; // enum FSM

struct Object                       // Hidden Status object
{
   int                 fsm;         // Finite State Machine
   HANDLE              handle;      // Semaphore handle
   int                 refCount;    // WAIT reference count
}; // struct Object

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // Mutual exclusion object
static SECURITY_ATTRIBUTES
                       statusSecurityAttributes; // (Initialized in Static())

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

   #ifdef HCDM
     debugf("%p= Status::createObject()\n", O);
   #endif

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
   #ifdef HCDM
     debugf("%s= Status::destroyObject(%p)\n", "........", O);
   #endif

   if( O->handle != NULL )
     ::CloseHandle(O->handle);

   free(O);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Status::~Status
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Status::~Status( void )          // Destructor
{
   #ifdef HCDM
     debugf("%8s= Status(%p)::~Status\n", "", this);
   #endif

   Object* O= (Object*)handle;
   handle= NULL;
   if( O != NULL )
     destroyObject(O);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Status::Status
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Status::Status( void )           // Constructor
:  handle(NULL)
,  value(0)
{
   #ifdef HCDM
     debugf("%8s= Status(%p)::Status\n", "", this);
   #endif

   // Initialize threadSecurityAttributes
   statusSecurityAttributes.nLength= sizeof(statusSecurityAttributes);
   statusSecurityAttributes.lpSecurityDescriptor= NULL;
   statusSecurityAttributes.bInheritHandle= TRUE;

   Object* O= createObject();
   O->fsm= FSM_INIT;
   O->handle= ::CreateEvent(        // Create a WIN event handle
                  &statusSecurityAttributes,
                  TRUE,             // Manual reset
                  FALSE,            // Default NOT signalled
                  NULL);            // Unnamed event
   if (O->handle == NULL)
     throwf("No Storage");

   handle= O;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Status::wait
//
// Purpose-
//       Wait until post invoked
//
//----------------------------------------------------------------------------
long
   Status::wait( void )             // Wait for status
{
   Object*             O= (Object*)handle; // -> Object
   int                 fsm;         // Current state
   HANDLE              handle;      // Current handle

   {{{{
     AutoBarrier lock(barrier);

     #ifdef HCDM
       debugf("%8s= Status(%p)::wait()...fsm(%d)\n", "", this, O->fsm);
       debugf("%p= Thread::current\n", Thread::current());
     #endif

     if (O->fsm == FSM_INIT)        // If this is a new wait event
       O->fsm= FSM_WAIT;

     fsm= O->fsm;                   // Save the state
     handle= O->handle;             // Save event handle

     if( fsm == FSM_WAIT )
       O->refCount++;
   }}}}

   if( fsm == FSM_WAIT )            // If we need to wait
   {
     for(;;)                        // Wait for Event
     {
       int rc= WaitForSingleObject(handle, INFINITE);
       if( rc != WAIT_TIMEOUT )
         break;
     }

     AutoBarrier lock(barrier);
     O->refCount--;
   }

   #ifdef HCDM
     debugf("%8ld= ...Status(%p)::wait()\n", value, this);
   #endif
   return value;                    // Return the post status
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Status::post
//
// Purpose-
//       Post the status update.
//
//----------------------------------------------------------------------------
void
   Status::post(                    // Post the Status
     long              status)      // The status value
{
   Object*             O= (Object*)handle; // -> Object

   AutoBarrier lock(barrier);

   #ifdef HCDM
     debugf("%8s= Status(%p)::post(%ld) fsm(%d)\n", "", this, status, O->fsm);
     debugf("%p= Thread::current\n", Thread::current());
   #endif

   if( O->fsm == FSM_POST )
     throwf("%4d %s MultiplePostException", __LINE__, __FILE__);

   int fsm= O->fsm;                 // Prior state
   O->fsm= FSM_POST;                // Set POSTed state
   value= status;                   // Set the post code

   if( fsm == FSM_WAIT )            // If was in WAIT state
     ::SetEvent(O->handle);         // Post the Mutex
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Status::reset
//
// Purpose-
//       Reset the Status Object.
//
//----------------------------------------------------------------------------
void
   Status::reset( void )            // Reset the Status Object
{
   Object*             O= (Object*)handle; // -> Object

   AutoBarrier lock(barrier);

   #ifdef HCDM
     debugf("%8s= Status(%p)::reset() fsm(%d)\n", "", this, O->fsm);
     debugf("%p= Thread::current\n", Thread::current());
   #endif

   switch( O->fsm )
   {
     case FSM_INIT:
       break;

     case FSM_WAIT:
       throwf("%4d %s InvalidResetException", __LINE__, __FILE__);
       break;

     case FSM_POST:
       if( O->refCount != 0 )
         throwf("%4d %s InvalidResetException", __LINE__, __FILE__);

       O->fsm= FSM_INIT;            // Reset the state
       ::ResetEvent(O->handle);     // Reset the mutex
       break;

     default:
       throwf("%4d %s FSM(%d)", __LINE__, __FILE__, O->fsm);
       break;
   }
}

