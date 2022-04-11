//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2022 Frank Eskesen.
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
//       2022/04/05
//
// Implementation notes-
//       DispatchDone.h, DispatchItem.h, and DispatchTask.h are integral parts
//       of the Dispatcher. Their include is guaranteed.
//
//----------------------------------------------------------------------------
#ifndef _PUB_DISPATCH_H_INCLUDED
#define _PUB_DISPATCH_H_INCLUDED

#include <pub/Latch.h>              // For mutex

#include "pub/DispatchDone.h"       // Dispatch Done/Wait object
#include "pub/DispatchItem.h"       // Dispatch work Item object
#include "pub/DispatchTask.h"       // Dispatch Task object

namespace _PUB_NAMESPACE::dispatch {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Timers;                       // Dispatch: Timers Thread (INTERNAL)

//----------------------------------------------------------------------------
//
// Static class-
//       pub::dispatch::Disp
//
// Purpose-
//       Work dispatcher. (All methods are static.)
//
//----------------------------------------------------------------------------
class Disp {                        // The dispatcher (static class)
//----------------------------------------------------------------------------
// Disp::Attributes
//----------------------------------------------------------------------------
private:
static Latch           mutex;       // Timers mutex
static Timers*         timers;      // The Timers Thread

//----------------------------------------------------------------------------
// Disp::Constructors
//----------------------------------------------------------------------------
public:
   Disp( void ) = delete;           // There are NO constructors

//----------------------------------------------------------------------------
// Disp::Methods
//----------------------------------------------------------------------------
static void
   debug( void );                    // Debugging display

//----------------------------------------------------------------------------
// Disp::cancel()
//
// Call this method to cancel a timer workUnit. If cancelled, the associated
// Item COMPLETES with a completion code of Item::CC_PURGE.
//----------------------------------------------------------------------------
static void
   cancel(                          // Cancel delay
     void*             token);      // Cancellation token

//----------------------------------------------------------------------------
// Disp::delay()
//
// The associated Item completes after the specified number of seconds with
// a completion code of CC_NORMAL. The delay may be cancelled by calling the
// cancel() function with the resultant cancellation token.
//----------------------------------------------------------------------------
static void*                        // Cancellation token
   delay(                           // Delay for
     double            seconds,     // This many seconds, then
     Item*             item);       // Complete this work Item

//----------------------------------------------------------------------------
// Disp::enqueue()
//
// Insert an Item onto a Task's todo list. Tasks handle one Item at a time,
// under one execution Thread.
//----------------------------------------------------------------------------
static inline void
   enqueue(                         // Enqueue
     Task*             task,        // Onto this Task
     Item*             item)        // This Item
{  task->enqueue(item); }

//----------------------------------------------------------------------------
// Disp::wait()
//
// Terminate Dispatcher processing, then wait for all associated work to
// complete. No new work will be processed after this function is called.
//----------------------------------------------------------------------------
static void
   wait( void );                    // Wait for all work to complete
}; // class Disp
}  // namespace _PUB_NAMESPACE::dispatch
#endif // _PUB_DISPATCH_H_INCLUDED
