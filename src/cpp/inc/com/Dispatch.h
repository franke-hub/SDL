//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Dispatch.h
//
// Purpose-
//       Work dispatcher.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       Class names beginning with Dispatch are reserved for this Dispatcher.
//       DispatchTask.h, DispatchItem.h, and DispatchDone.h are integral parts
//       of the Dispatcher. Their include is guaranteed.
//
//----------------------------------------------------------------------------
#ifndef DISPATCH_H_INCLUDED
#define DISPATCH_H_INCLUDED

#include <com/Barrier.h>            // Held whenever attributes change
#include <com/List.h>               // For DispatchTask.h, DispatchItem.h

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class DispatchTask;                 // Dispatch: Task Block
class DispatchItem;                 // Dispatch: work Item
class DispatchDone;                 // Dispatch: Done callback Object

class DispatchMaster;               // Dispatch: Master Thread (INTERNAL)
class DispatchThread;               // Dispatch: Worker Thread (INTERNAL)
class DispatchTimers;               // Dispatch: Timers Thread (INTERNAL)

//----------------------------------------------------------------------------
//
// Class-
//       Dispatch
//
// Purpose-
//       Work dispatcher.
//
//----------------------------------------------------------------------------
class Dispatch {                    // The dispatcher
friend class DispatchMaster;        // For Barrier access
friend class DispatchTimers;        // For Barrier access

//----------------------------------------------------------------------------
// Dispatch::Attributes
//----------------------------------------------------------------------------
protected:
Barrier                barrier;     // Protect thread pointers
DispatchMaster*        master;      // The DispatchMaster Thread
DispatchTimers*        timers;      // The DispatchTimers Thread

//----------------------------------------------------------------------------
// Dispatch::Constructors
//----------------------------------------------------------------------------
public:
   ~Dispatch( void );               // Destructor
   Dispatch( void );                // Constructor

private:                            // Bitwise copy is prohibited
   Dispatch(const Dispatch&);       // Disallowed copy constructor
   Dispatch& operator=(const Dispatch&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// Dispatch::Methods
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Debugging display

//----------------------------------------------------------------------------
// Dispatch::cancel()
//
// Call this method to cancel a timer workUnit. If cancelled, the associated
// DispathItem COMPLETES with a completion code of CC_ERROR.
//----------------------------------------------------------------------------
void
   cancel(                          // Cancel delay
     void*             token);      // Cancellation token

//----------------------------------------------------------------------------
// Dispatch::delay()
//
// The associated DispatchItem completes after the specified number of seconds
// with a completion code of CC_NORMAL. The delay may be cancelled by calling
// the cancel() function with the resultant cancellation token.
//----------------------------------------------------------------------------
void*                               // Cancellation token
   delay(                           // Delay for
     double            seconds,     // This many seconds, then
     DispatchItem*     workItem);   // Complete this WorkItem

//----------------------------------------------------------------------------
// Dispatch::enqueue()
//
// Add the associated DispatchItem to the ordered list of work units to
// be handled by the specified DispatchTask.
//----------------------------------------------------------------------------
void
   enqueue(                         // Add Item to list of tasks for Task
     DispatchTask*     task,        // -> Task
     DispatchItem*     item);       // -> Item

//----------------------------------------------------------------------------
// Dispatch::wait()
//
// Terminate Dispatcher processing, then wait for all associated work to
// complete. No new work will be processed after this function is called.
//----------------------------------------------------------------------------
void
   wait( void );                    // Wait for all work to complete
}; // class Dispatch

#include "DispatchTask.h"           // Task Block object
#include "DispatchItem.h"           // Work Item object
#include "DispatchDone.h"           // Standard Done objects

#endif // DISPATCH_H_INCLUDED
