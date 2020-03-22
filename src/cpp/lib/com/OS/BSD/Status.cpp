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
//       OS/BSD/Status.cpp
//
// Purpose-
//       Status object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <errno.h>
#include <semaphore.h>
#include <stdlib.h>
#include <string.h>

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
//       Define the hidden Status Object.
//
//----------------------------------------------------------------------------
enum FSM                            // Finite State Machine states
{  FSM_INIT= 0                      // INITial state (not posted)
,  FSM_WAIT                         // WAITing (waiter exists)
,  FSM_POST                         // POSTed
}; // enum FSM

struct Object                       // Hidden Status Object
{
   FSM                 fsm;         // Finite State Machine
   sem_t               mutex;       // The physical semaphore
   int                 refCount;    // WAIT reference count
}; // struct Object

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Barrier         barrier= BARRIER_INIT; // Mutual exclusion object

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
   {
     sem_destroy(&O->mutex);
     destroyObject(O);
   }
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

   Object* O= createObject();
   O->fsm= FSM_INIT;
   int rc= sem_init(&O->mutex, 0, 0);  // Initialize (LOCKED)
   if( rc < 0 )
     throwf("%4d %s %d= sem_init()", __LINE__, __FILE__, rc);

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
   FSM                 fsm;         // Current state

   {{{{
     AutoBarrier lock(barrier);

     #ifdef HCDM
       debugf("%8s= Status(%p)::wait()...fsm(%d)\n", "", this, O->fsm);
       debugf("%p= Thread::current\n", Thread::current());
     #endif

     if (O->fsm == FSM_INIT)        // If this is a new wait event
       O->fsm= FSM_WAIT;

     fsm= O->fsm;                   // Save the state

     if( fsm == FSM_WAIT )
       O->refCount++;
   }}}}

   if( fsm == FSM_WAIT )            // If a wait is required
   {
     for(;;)
     {
       int rc= sem_wait(&O->mutex); // Obtain control
       if( rc == 0 )
       {
         if( O->fsm == FSM_WAIT )   // Unexplained linux glitch
           continue;

         break;
       }

       if( errno != EINTR )
         throwf("%4d %s Unexpected error(%d)", __LINE__, __FILE__, errno);
     }

     AutoBarrier lock(barrier);
     O->refCount--;
   }

   #ifdef HCDM
     debugf("%8ld= ...Status(%p)::wait()\n", value, this);
   #endif
   return value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Status::post
//
// Purpose-
//       Post the Status.
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
     sem_post(&O->mutex);           // Post the Mutex
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Status::reset
//
// Purpose-
//       Reset the Status object.
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
       break;

     default:
       throwf("%4d %s FSM(%d)", __LINE__, __FILE__, O->fsm);
       break;
   }
}

