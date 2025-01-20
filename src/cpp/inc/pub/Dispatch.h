//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2025 Frank Eskesen.
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
//       2025/01/19
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_DISPATCH_H_INCLUDED
#define _LIBPUB_DISPATCH_H_INCLUDED

#include <functional>               // For std::function

#include <pub/Latch.h>              // For pub::Latch
#include <pub/List.h>               // For pub::AI_list, for Item's base class
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
class Timers;                       // pub::dispatch::Timers (INTERNAL)

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
// pub::dispatch::Disp::Attributes
//----------------------------------------------------------------------------
private:
static Latch           mutex;       // Timers mutex
static Timers*         timers;      // The Timers Thread

//----------------------------------------------------------------------------
// pub::dispatch::Disp::Constructors
//----------------------------------------------------------------------------
public:
   Disp( void ) = delete;           // There are NO constructors

//----------------------------------------------------------------------------
// pub::dispatch::Disp::Methods
//----------------------------------------------------------------------------
static void
   debug( void );                    // Debugging display

//----------------------------------------------------------------------------
// pub::dispatch::Disp::cancel()
//
// Call this method to cancel a timer workUnit. If cancelled, the associated
// Item COMPLETES with a completion code of Item::CC_PURGE.
//----------------------------------------------------------------------------
static void
   cancel(                          // Cancel delay
     void*             token);      // Cancellation token

//----------------------------------------------------------------------------
// pub::dispatch::Disp::delay()
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
// pub::dispatch::Disp::enqueue()
//
// Insert an Item onto a Task's todo list. Tasks handle one Item at a time,
// under one execution Thread.
//----------------------------------------------------------------------------
static void
   enqueue(                         // Enqueue
     Task*             task,        // Onto this Task
     Item*             item);       // This Item

//----------------------------------------------------------------------------
// pub::dispatch::Disp::post()
//
// Pass the work Item to a different task for completion.
//----------------------------------------------------------------------------
static void
   post(                            // Post (using an internal Dispatch Task)
     Item*             item,        // This work Item
     int               _cc= 0);     // With this completion code

//----------------------------------------------------------------------------
// pub::dispatch::Disp::shutdown()
//
// Terminate Dispatcher delay processing. When this function is invoked, all
// current delay() operations are posted with the CC_PURGE completion code.
//----------------------------------------------------------------------------
static void
   shutdown( void );                // Terminate delay processing
}; // class pub::dispatch::Disp

//----------------------------------------------------------------------------
//
// Class-
//       pub::dispatch::Done
//
// Purpose-
//       The Dispatcher Done callback Object
//
//----------------------------------------------------------------------------
class Done {                        // The dispatch::Done callback Object
//----------------------------------------------------------------------------
// pub::dispatch::Done::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Done( void ) {}                  // Default constructor

   Done(const Done&) = delete;      // Disallowed copy constructor
   Done& operator=(const Done&) = delete; // Disallowed assignment operator

virtual
   ~Done( void ) {}                 // Destructor

//----------------------------------------------------------------------------
// pub::dispatch::Done::Methods
//----------------------------------------------------------------------------
public:
virtual void                        // OVERRIDE this method
   done(                            // Complete
     Item*             item) = 0;   // This work Item
}; // class pub::dispatch::Done

//----------------------------------------------------------------------------
//
// Class-
//       pub::dispatch::Item
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
// pub::dispatch::Item::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum CC                             // Completion codes
{  CC_NORMAL= 0                     // Normal (OK)
,  CC_PURGE= -1                     // Function purged
,  CC_ERROR= -2                     // Generic error
,  CC_ERROR_FC= -3                  // Invalid Function Code
,  CC_ERROR_IT= -4                  // Invalid Item Type
}; // enum CC

enum FC                             // Function codes
{  FC_VALID= 0                      // All user function codes are positive
,  FC_CHASE= -1                     // Chase (Handled by Dispatcher)
,  FC_UNDEF= -2                     // Undefined/invalid function code
}; // enum FC

//----------------------------------------------------------------------------
// pub::dispatch::Item::Attributes
//----------------------------------------------------------------------------
int                    fc= FC_VALID;  // Function code
int                    cc= CC_NORMAL; // Completion code
Done*                  done= nullptr; // Completion callback

//----------------------------------------------------------------------------
// pub::dispatch::Item::Constructors/destructor
//----------------------------------------------------------------------------
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
// pub::dispatch::Item::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debug(const char* info= "") const; // Debugging display

void
   post(                            // Complete the Work Item
     int               _cc= 0)      // With this completion code
{
   if( done ) {
     cc= _cc;
     done->done(this);
   } else {
     delete this;
   }
}
}; // class pub::dispatch::Item

//----------------------------------------------------------------------------
//
// Class-
//       pub::dispatch::Task
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
// pub::dispatch::Task::Attributes
//----------------------------------------------------------------------------
protected:
AI_list<Item>          itemList;    // The Work item list

//----------------------------------------------------------------------------
// pub::dispatch::Task::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Task( void );                    // Default constructor

   Task(const Task&) = delete;      // Disallowed copy constructor
   Task& operator=(const Task&) = delete; // Disallowed assignment operator

virtual
   ~Task( void );                   // Destructor

//----------------------------------------------------------------------------
// pub::dispatch::Task::Methods
//----------------------------------------------------------------------------
virtual void
   debug(const char* info= "") const; // Debugging display

void
   enqueue(                         // Enqueue
     Item*             item)        // This work Item
{
   Item* tail= itemList.fifo(item); // Insert work Item
   if( tail == nullptr )            // If the list was empty
     WorkerPool::work(this);        // Schedule this Task
}

protected:
virtual void                        // (IMPLEMENT this method)
   work(                            // Process
     Item*             item);       // This work Item

private:
void                                // The Worker interface
   work( void ) final;              // Drain work from Task
}; // class pub::dispatch::Task

//----------------------------------------------------------------------------
//
// Class-
//       pub::dispatch::Wait
//
// Purpose-
//       The Wait until done Object
//
// Notes-
//       This Object cannot cannot be shared, but can be reused by calling the
//       reset method once the wait has been satisfied.
//
// Implementation notes-
//       The Wait object uses the done() method to convert the Done object
//       into a wait/post object, where posting occurs in the done() method.
//       The overridden done() method now "belongs" to Wait.
//
//----------------------------------------------------------------------------
class Wait : public Done {          // The dispatcher Wait until Done Object
//----------------------------------------------------------------------------
// pub::dispatch::Wait::Attributes
//----------------------------------------------------------------------------
private:
Event                  event;       // For wait/post

//----------------------------------------------------------------------------
// pub::dispatch::Wait::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Wait( void ) : Done() {}         // Constructor

   Wait(const Wait&) = delete;      // Disallowed copy constructor
   Wait& operator=(const Wait&) = delete; // Disallowed assignment operator

virtual
   ~Wait( void ) {}                 // Destructor

//----------------------------------------------------------------------------
// pub::dispatch::Wait::Methods
//----------------------------------------------------------------------------
void
   reset( void )                    // Reset for re-use
{  event.reset(); }

int32_t                             // The event code (Always positive)
   wait( void )                     // Wait for work Item completion
{  return event.wait(); }

private:
virtual void
   done(                            // Complete
     Item*             item) final  // This work Item
{  event.post(item->cc); }
}; // class pub::dispatch::Wait

//----------------------------------------------------------------------------
//
// Typedef-
//       pub::dispatch::Work_i
//
// Purpose-
//       Work method interface
//
//----------------------------------------------------------------------------
typedef std::function<void(Item*)>
                        Work_i;     // The work method interface

//----------------------------------------------------------------------------
//
// Class-
//       pub::dispatch::LambdaDone
//
// Purpose-
//       The Dispatcher Done callback Object
//
// Implementation notes-
//       The LambdaDone object uses the done() method to provide an
//       alternate method for overriding it.
//       The original done() method now "belongs" to LambdaDone.
//
//----------------------------------------------------------------------------
class LambdaDone : public Done {    // The dispatch::LambdaDone callback Object
//----------------------------------------------------------------------------
// pub::dispatch::LambdaDone::Attributes
//----------------------------------------------------------------------------
protected:
Work_i                 do_done;     // The completion handler

//----------------------------------------------------------------------------
// pub::dispatch::LambdaDone::Constructors/destructor
//----------------------------------------------------------------------------
public:
   LambdaDone( void )               // Default constructor
:  Done() {}                        // (Callback not initialized)

   LambdaDone(Work_i f)             // Constructor
:  Done(), do_done(f) {}

   LambdaDone(const LambdaDone&) = delete; // Disallowed copy constructor
   LambdaDone& operator=(const LambdaDone&) = delete; // Disallowed assignment operator

virtual
   ~LambdaDone( void ) = default;   // Destructor

//----------------------------------------------------------------------------
// pub::dispatch::LambdaDone::Methods
//----------------------------------------------------------------------------
void
   on_done(Work_i f)                // Replace do_done
{  do_done= f; }

virtual void
   done(                            // Complete
     Item*             item) final  // This work Item
{  do_done(item); }
}; // class pub::dispatch::LambdaDone

//----------------------------------------------------------------------------
//
// Class-
//       pub::dispatch::LambdaTask
//
// Purpose-
//       A Dispatch Task using a std::function
//
// Implementation notes-
//       The LambdaTask object uses the work() method to provide an
//       alternate method for overriding it.
//       The original work() method now "belongs" to LambdaTask.
//
// Sample usage (which performs the default action)-
//       LambdaTask task([this](Item* item) {
//         // Your code goes here
//         item->post();
//      });
//
//----------------------------------------------------------------------------
class LambdaTask : public Task {    // Dispatch Lambda Task
protected:
Work_i                 do_work;     // The Work item handler

public:
   LambdaTask( void )               // Default constructor
:  Task()
{  }

   LambdaTask(                      // Instantiate work method
     Work_i            f)           // With this lambda function
:  Task(), do_work(f) {}

virtual
   ~LambdaTask( void ) = default;   // Destructor

//----------------------------------------------------------------------------
// pub::dispatch::LambdaTask::Methods
//----------------------------------------------------------------------------
void
   on_work(                         // Instantiate work method
     Work_i            f)           // With this lambda function
{  do_work= f; }

virtual void
   work(                            // Process
     Item*             item) final  // This work Item
{  do_work(item); }
}; // class pub::dispatch::LambdaTask
}  // namespace dispatch
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_DISPATCH_H_INCLUDED
