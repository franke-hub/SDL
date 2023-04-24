//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2023 Frank Eskesen.
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
//       2023/04/23
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_DISPATCH_H_INCLUDED
#define _LIBPUB_DISPATCH_H_INCLUDED

#include <functional>               // For std::function

#include <pub/Latch.h>              // For pub::Latch
#include <pub/List.h>               // For pub::List, Item base class
#include <pub/Event.h>              // For pub::Wait
#include <pub/Worker.h>             // For pub::Worker

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace dispatch {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Done;                         // pub::dispatch::Done
class Item;                         // pub::dispatch::Item
class Task;                         // pub::dispatch::Task
class Timers;                       // pub::dispatch:Timers (INTERNAL)

//----------------------------------------------------------------------------
//
// Static class-
//       dispatch::Disp
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
static void
   enqueue(                         // Enqueue
     Task*             task,        // Onto this Task
     Item*             item);       // This Item

//----------------------------------------------------------------------------
// Disp::wait()
//
// Terminate Dispatcher processing, then wait for all associated work to
// complete. No new work will be processed after this function is called.
//----------------------------------------------------------------------------
static void
   wait( void );                    // Wait for all work to complete
}; // class Disp

//----------------------------------------------------------------------------
//
// Class-
//       dispatch::Done
//
// Purpose-
//       The Dispatcher Done callback Object
//
//----------------------------------------------------------------------------
class Done {                        // The dispatch::Done callback Object
//----------------------------------------------------------------------------
// Done::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Done( void ) {}                 // Destructor
   Done( void ) {}                  // Default constructor

   Done(const Done&) = delete;      // Disallowed copy constructor
   Done& operator=(const Done&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Done::Methods
//----------------------------------------------------------------------------
public:
virtual void                        // OVERRIDE this method
   done(                            // Complete
     Item*             item) = 0;   // This work Item
}; // class Done

//----------------------------------------------------------------------------
//
// Class-
//       Item
//
// Purpose-
//       The Dispatcher work Item Object.
//
// Implementation notes-
//       All negative function codes are handled internally by the Dispatcher.
//       They are not passed to DispatchTask::work().
//
//       When post() is invoked:
//         if done != nullptr, done->done(this) is invoked.
//         if done == nullptr, the Item is deleted.
//
//----------------------------------------------------------------------------
class Item : public AI_list<Item>::Link { // A dispatcher work item
//----------------------------------------------------------------------------
// Item::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum CC                             // Completion codes
{  CC_NORMAL= 0                     // Normal (OK)
,  CC_PURGE= -1                     // Function purged
,  CC_ERROR= -2                     // Generic error
,  CC_ERROR_FC= -3                  // Invalid function code
}; // enum CC

enum FC                             // Function codes
{  FC_VALID= 0                      // All user function codes are positive
,  FC_CHASE= -1                     // Chase (Handled by Dispatcher)
,  FC_UNDEF= -2                     // Undefined/invalid function code
}; // enum FC

//----------------------------------------------------------------------------
// Item::Attributes
//----------------------------------------------------------------------------
public:
int                    fc= FC_VALID;  // Function code
int                    cc= CC_NORMAL; // Completion code
Done*                  done= nullptr; // Completion callback

//----------------------------------------------------------------------------
// Item::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Item( void ) = default;          // Default constructor

   Item(                            // Constructor
     Done*             done)        // -> Done callback object
:  done(done) { }

   Item(                            // Constructor
     int               fc,          // Function code
     Done*             done= nullptr) // -> Done callback object
:  fc(fc), done(done) { }

virtual
   ~Item( void ) = default;         // Destructor

private:
   Item(const Item&) = delete;      // Disallowed copy constructor
   Item& operator=(const Item&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Item::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debug(const char* info= "") const; // Debugging display

void
   post(                            // Complete the Work Item
     int               user_cc= 0)  // With this completion code
{
   if( done ) {
     cc= user_cc;
     done->done(this);
   } else {
     delete this;
   }
}
}; // class Item

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
// Task::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   Task( void )                     // Default constructor
:  Worker(), itemList() {}

virtual
   ~Task( void );                   // Destructor

   Task(const Task&) = delete;      // Disallowed copy constructor
   Task& operator=(const Task&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Task::Methods
//----------------------------------------------------------------------------
virtual void
   debug(const char* info= "") const; // Debugging display

void
   enqueue(                         // Enqueue
     Item*             item)        // This work Item
#if false                           // TRUE for (preferred) inline version
{
   Item* tail= itemList.fifo(item); // Insert work Item
   if( tail == nullptr )            // If the list was empty
     WorkerPool::work(this);        // Schedule this Task
}
#else
   ;                                // FALSE for outline (debugging) version
#endif

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
//       dispatch::LambdaDone
//
// Purpose-
//       The Dispatcher Done callback Object
//
//----------------------------------------------------------------------------
class LambdaDone : public Done {    // The dispatch::LambdaDone callback Object
//----------------------------------------------------------------------------
// LambdaDone::Attributes
//----------------------------------------------------------------------------
public:
typedef std::function<void(Item*)> function_t; // Work handler

protected:
function_t             callback;    // The Work item handler

//----------------------------------------------------------------------------
// LambdaDone::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~LambdaDone( void ) = default;   // Destructor
   LambdaDone( void )               // Default constructor
:  Done() { }                       // (Callback not initialized)
   LambdaDone(function_t f)         // Constructor
:  Done(), callback(f) {}

   LambdaDone(const LambdaDone&) = delete; // Disallowed copy constructor
   LambdaDone& operator=(const LambdaDone&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// LambdaDone::Methods
//----------------------------------------------------------------------------
public:
void
   on_done(function_t f)            // Replace callback
{  callback= f; }

virtual void
   done(                            // Complete
     Item*             item)        // This work Item
{  callback(item); }
}; // class LambdaDone

//----------------------------------------------------------------------------
//
// Class-
//       Wait
//
// Purpose-
//       The Wait until done Object
//
// Notes-
//       This Object cannot cannot be shared, but can be reused by calling the
//       reset method once the wait has been satisfied.
//
//----------------------------------------------------------------------------
class Wait : public Done {          // The dispatcher Wait until Done Object
//----------------------------------------------------------------------------
// Wait::Attributes
//----------------------------------------------------------------------------
private:
Event                  event;       // For wait/post

//----------------------------------------------------------------------------
// Wait::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Wait( void ) {}                 // Destructor
   Wait( void ) : Done() {}         // Constructor

   Wait(const Wait&) = delete;      // Disallowed copy constructor
   Wait& operator=(const Wait&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Wait::Methods
//----------------------------------------------------------------------------
public:
virtual void
   done(                            // Complete
     Item*)                         // This (ignored) work Item
{  event.post(); }

void
   reset( void )                    // Reset for re-use
{  event.reset(); }

void
   wait( void )                     // Wait for work Item completion
{  event.wait(); }
}; // class Wait

//----------------------------------------------------------------------------
//
// Class-
//       dispatch::LambdaTask
//
// Purpose-
//       A Dispatch Task using a std::function
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
   LambdaTask( void )               // Default constructor
:  Task(), callback([](Item*) {}) {}

   LambdaTask(function_t f)         // Lambda constructor
:  Task(), callback(f) {}

virtual
   ~LambdaTask( void ) = default;   // Destructor

//----------------------------------------------------------------------------
// LambdaTask::Methods
//----------------------------------------------------------------------------
void
   on_work(function_t f)
{  callback= f; }

protected:
virtual void                        // (Callback constructor-defined)
   work(                            // Process
     Item*             item)        // This work Item
{  callback(item); }
}; // class LambdaTask
}  // namespace dispatch
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DISPATCH_H_INCLUDED
