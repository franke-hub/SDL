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
//       DispatchTask.h
//
// Purpose-
//       Standard Dispatch Task object.
//
// Last change date-
//       2022/09/02
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_DISPATCHTASK_H_INCLUDED
#define _LIBPUB_DISPATCHTASK_H_INCLUDED

#include <functional>               // For std::function

#include "pub/Dispatch.h"           // The Dispatcher
#include "pub/DispatchItem.h"       // Dispatch work Item
#include "pub/Worker.h"             // Worker Thread Pool

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace dispatch {
//----------------------------------------------------------------------------
//
// Class-
//       dispatch::Task
//
// Purpose-
//       The Dispatch Task.
//
// Notes-
//       Multiple threads may simultaneously add work items to the work Item
//       list. They are processed in FIFO order by the worker, one Item at a
//       time, in single-threaded mode, by the work() method.
//
//       A Worker either completes the work Item by invoking its post() method
//       or enqueues it onto another Task.
//
//----------------------------------------------------------------------------
class Task : public Worker {        // Dispatch Task
//----------------------------------------------------------------------------
// Task::Attributes
//----------------------------------------------------------------------------
protected:
AI_list<Item>          itemList;    // The Work item list

//----------------------------------------------------------------------------
// Task::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Task( void ) {}                 // Destructor
   Task( void )                     // Default constructor
:  Worker(), itemList() {}

   Task(const Task&) = delete;      // Disallowed copy constructor
   Task& operator=(const Task&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Task::Methods
//----------------------------------------------------------------------------
public:
inline bool                         // TRUE iff Task has no enqueued work
   is_idle( void ) const            // Is this Task idle?
{
   return itemList.is_empty();
}

inline bool                         // TRUE iff Task has enqueued work
   is_busy( void ) const            // Is this Task busy?
{
   return !itemList.is_empty();
}

virtual void
   debug(const char* info) const;   // Debugging display

void debug( void ) { debug(""); }

void
   enqueue(                         // Enqueue
     Item*             item)        // This work Item
{
   Item* tail= itemList.fifo(item); // Insert work Item
   if( tail == nullptr )            // If the list was empty
     WorkerPool::work(this);        // Schedule this Task
}

void
   reset( void )                    // Reset the itemList
{  itemList.reset(); }

virtual void                        // The Worker interface
   work( void );                    // Drain work from Task

protected:
virtual void                        // (IMPLEMENT this method)
   work(                            // Process
     Item*             item);       // This work Item
}; // class Task

//----------------------------------------------------------------------------
//
// Class-
//       dispatch::LambdaTask
//
// Purpose-
//       A Dispatch Task where the function is constructor-defined
//
// Sample usage (which performs the default action)-
//       LambdaTask task([this](Item* item) {
//         // Your code goes here
//         item->post();
//      });
//
//----------------------------------------------------------------------------
class LambdaTask : public Task {    // Dispatch Lambda Task
public:
typedef std::function<void(Item*)> function_t; // Work handler

protected:
function_t             callback;    // The Work item handler

public:
virtual
   ~LambdaTask( void ) = default;   // Destructor
   LambdaTask(function_t f)         // Lambda constructor
:  Task(), callback(f) {}

protected:
virtual void                        // (Callback constructor-defined)
   work(                            // Process
     Item*             item)        // This work Item
{  callback(item); }
}; // class LambdaTask
}  // namespace dispatch
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DISPATCHTASK_H_INCLUDED
