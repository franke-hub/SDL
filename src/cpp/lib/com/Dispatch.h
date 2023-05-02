//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Dispatch.h
//
// Purpose-
//       Work dispatcher, including local definitions.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <com/Debug.h>
#include <com/Clock.h>              // In DispatchTTL
#include <com/Semaphore.h>          // In DispatchTimers
#include <com/StatusThread.h>       // (Base class)

#include "com/Dispatch.h"           // Include visible class definitions

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef logf
#define logf traceh                 // Alias for trace w/header
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Class-
//       DispatchDTL
//
// Purpose-
//       DispatchThreadLink, used by DispatchMaster for DispatchThread objects
//
//----------------------------------------------------------------------------
class DispatchDTL : public AU_List<DispatchDTL>::Link { // The DispatchThread AU_Link
protected:
DispatchThread*        thread;      // -> Associated Thread

public:
   ~DispatchDTL( void )             // Destructor
{
}

   DispatchDTL(                     // Constructor
     DispatchThread*   thread)      // The associated DispatchThread
:  AU_List<DispatchDTL>::Link()
,  thread(thread)
{
}

DispatchThread*
   getThread( void )
{
   return thread;
}
}; // class DispatchDTL

//----------------------------------------------------------------------------
//
// Class-
//       DispatchTTL
//
// Purpose-
//       Dispatch Timer Thread Link: Keep track of delay request.
//
//----------------------------------------------------------------------------
class DispatchTTL : public List<DispatchTTL>::Link
                  , public AU_List<DispatchTTL>::Link {
//----------------------------------------------------------------------------
// DispatchTTL::Attributes
//----------------------------------------------------------------------------
public:
Clock                  time;        // The proposed completion time
DispatchItem*          item;        // The work Item

//----------------------------------------------------------------------------
// DispatchTTL::Constructors
//----------------------------------------------------------------------------
public:
   ~DispatchTTL( void );            // Destructor
   DispatchTTL(                     // Constructor
     const Clock&      time,        // Completion time
     DispatchItem*     item);       // Work Item
}; // class DispatchTTL

//----------------------------------------------------------------------------
//
// Class-
//       DispatchMaster
//
// Purpose-
//       Dispatch master thread
//
//----------------------------------------------------------------------------
class DispatchMaster : public StatusThread { // The DispatchMaster thread
//----------------------------------------------------------------------------
// DispatchMaster::Attributes
//----------------------------------------------------------------------------
protected:
Dispatch*              owner;       // The associated Dispatch object
AU_List<DispatchDTL>   dtlList;     // List of DispatchDTL objects
AU_List<DispatchTask>  taskList;    // List of DispatchTask elements

unsigned               count;       // The number of allocated Threads

//----------------------------------------------------------------------------
// DispatchMaster::Constructors
//----------------------------------------------------------------------------
public:
   ~DispatchMaster( void );         // Destructor
   DispatchMaster(                  // Constructor
     Dispatch*         owner);      // The associated Dispatch object

//----------------------------------------------------------------------------
// DispatchMaster::Methods
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Debugging display

inline void
   enqueue(                         // Schedule work
     DispatchTask*     task)        // -> Task
{
   IFHCDM( logf("DispatchMaster(%p)::enqueue(%p)\n", this, task); )

   DispatchTask* tail= taskList.fifo(task); // Insert Task on our Task list
   if( tail == NULL )
     post();
}

void
   done(                            // A DispatchThread completed
     DispatchThread*    thread);    // -> DispatchThread

virtual void
   stop( void );                    // Terminate processing

virtual void
   work( void );                    // Operate the Thread
}; // class DispatchMaster

//----------------------------------------------------------------------------
//
// Class-
//       DispatchThread
//
// Purpose-
//       Dispatch thread
//
//----------------------------------------------------------------------------
class DispatchThread : public StatusThread { // The DispatchThread object
//----------------------------------------------------------------------------
// DispatchThread::Attributes
//----------------------------------------------------------------------------
protected:
DispatchDTL            link;        // The associated DispatchDTL
Dispatch*              dispatch;    // The associated Dispatch object
DispatchMaster*        owner;       // The associated DispatchMaster
DispatchTask*          task;        // The target Task

//----------------------------------------------------------------------------
// DispatchThread::Constructors
//----------------------------------------------------------------------------
public:
   ~DispatchThread( void );         // Destructor
   DispatchThread(                  // Constructor
     Dispatch*         dispatch,    // The associated Dispatch
     DispatchMaster*   owner);      // The associated DispatchMaster

//----------------------------------------------------------------------------
// DispatchThread::Methods
//----------------------------------------------------------------------------
public:
inline DispatchDTL*                 // -> DispatchDTL
   getDispatchDTL( void )           // Get DispatchDTL
{
   return &link;
}

void
   debug( void ) const;             // Debugging display

inline void
   enqueue(                         // Schedule work
     DispatchTask*     task)        // -> Task
{
   IFHCDM( logf("DispatchThread(%p)::enqueue(%p)\n", this, task); )

   this->task= task;
   post();
}

virtual void
   post( void );                    // Signal work available

virtual long
   run( void );                     // Operate the thread

virtual void
   stop( void );                    // Terminate processing

virtual long                        // The Thread's return code
   wait( void );                    // Wait for Thread to complete

virtual void
   work( void );                    // Process work unit
}; // class DispatchThread

//----------------------------------------------------------------------------
//
// Class-
//       DispatchTimers
//
// Purpose-
//       Handle time delay requests.
//
//----------------------------------------------------------------------------
class DispatchTimers : public NamedThread {
//----------------------------------------------------------------------------
// DispatchTimers::Enumerations and typedefs
//----------------------------------------------------------------------------
protected:
enum FSM                            // Finite State Machine states
{  FSM_RESET= 0                     // RESET, not initialized
,  FSM_START                        // START, not run
,  FSM_READY                        // READY, operational
,  FSM_CLOSE                        // CLOSE, terminating
}; // enum FSM

//----------------------------------------------------------------------------
// DispatchTimers::Attributes
//----------------------------------------------------------------------------
protected:
Dispatch*              owner;       // The associated Dispatch object
int                    fsm;         // Finite State Machine
Semaphore              event;       // Synchronization event object
List<DispatchTTL>      list;        // Ordered list of pending events
AU_List<DispatchTTL>   pend;        // Atomic list of pending events

//----------------------------------------------------------------------------
// DispatchTimers::Constructors
//----------------------------------------------------------------------------
public:
   ~DispatchTimers( void );         // Destructor
   DispatchTimers(                  // Constructor
     Dispatch*         owner);      // The associated Dispatch object

//----------------------------------------------------------------------------
// DispatchTimers::Accessor methods
//----------------------------------------------------------------------------
public:
inline List<DispatchTTL>&           // The list
   getList( void )                  // Get list reference
{  return list; }

//----------------------------------------------------------------------------
// DispatchTimers::Methods
//----------------------------------------------------------------------------
public:
void
   cancel(                          // Cancel
     void*             token);      // This timer event

void*                               // Cancellation token
   delay(                           // Delay for
     double            seconds,     // This many seconds, then
     DispatchItem*     item);       // Complete this work Item

virtual int                         // Return code (0 OK)
   notify(                          // Terminate the DispatchTimers
     int               code);       // Notification code

virtual long                        // Operate the Thread
   run();
}; // class DispatchTimers

