//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Dispatch.cpp
//
// Purpose-
//       Implement Dispatch object methods
//
// Last change date-
//       2023/05/25
//
//----------------------------------------------------------------------------
#include <assert.h>                 // For assert
#include <mutex>                    // For std::lock_guard

#include <pub/Clock.h>              // DispatchTTL completion time
#include <pub/Debug.h>              // For debugging
#include "pub/Dispatch.h"           // For dispatch objects, implemented
#include <pub/Latch.h>              // For pub::Latch, mutex substitute
#include "pub/List.h"               // For pub::AI_list
#include <pub/Named.h>              // For pub::Named, Timers is a Named Thread
#include <pub/Semaphore.h>          // For pub::Semaphore, Timers event
#include <pub/Thread.h>             // For pub::Thread, Timers is a Named Thread
#include <pub/Trace.h>              // For pub::Trace
#include <pub/Worker.h>             // For pub::Worker

// DEBUGGING: TODO REMOVE- - - - - - - - - - - - - - - - - - - - - - - - - - -
#include <stdio.h>                  // For sprintf
#include <pub/Reporter.h>           // For pub::Reporter
#include <pub/Statistic.h>          // For pub::Statistic
// DEBUGGING: TODO REMOVE- - - - - - - - - - - - - - - - - - - - - - - - - - -

using namespace _LIBPUB_NAMESPACE::debugging; // Enable debugging functions

namespace _LIBPUB_NAMESPACE::dispatch {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  USE_ITRACE= true                 // Use internal tracing?
}; // enum

//----------------------------------------------------------------------------
// dispatch::Static attributes
//----------------------------------------------------------------------------
Latch                  Disp::mutex; // Termination mutex
Timers*                Disp::timers= nullptr;

#include "Dispatch.hpp"             // For Timers, DispatchTTL

//----------------------------------------------------------------------------
// dispatch::DEBUGGING TODO:REMOVE
//----------------------------------------------------------------------------
static statistic::Active
                       undef_wait;  // FC_UNDEF wait counter
static Reporter::Record
                       undef_record; // Active FC_UNDEF counter

static struct StaticGlobal {
   StaticGlobal(void)               // Constructor
{
   undef_record.name= "Dispatch:undef";
   undef_record.on_report([]() {
     char buffer[128];
     statistic::Active& stat= undef_wait;
     sprintf(buffer, "{%8zd, %8zd, %8zd, %8zd}: "
            , stat.counter.load(), stat.current.load()
            , stat.maximum.load(), stat.minimum.load());
     return std::string(buffer) + undef_record.name;
   });

   undef_record.on_reset([]() {
     statistic::Active& stat= undef_wait;
     stat.counter.store(0);
     stat.current.store(0);
     stat.maximum.store(0);
     stat.minimum.store(0);
   });

   Reporter::get()->insert(&undef_record);
}  // StaticGlobal
}  staticGlobal;

//----------------------------------------------------------------------------
//
// Subroutine-
//       checkstop
//
// Purpose-
//       Force a segfault
//
//----------------------------------------------------------------------------
static inline void
   checkstop(                       // SEGFAULT
     const char*       info)        // Caller information
{
   debugf("CHECKSTOP(%s)\n", info);
   Reporter::Record* R= nullptr;
   debugf("Name(%s)\n", R->name.c_str());
}

//----------------------------------------------------------------------------
//
// Method-
//       dispatch::Disp::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Disp::debug( void )              // Debugging display
{
   debugh("dispatch::debug()\n");
   WorkerPool::debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       dispatch::Disp::cancel
//
// Purpose-
//       Cancel delay event.
//
//----------------------------------------------------------------------------
void
   Disp::cancel(                    // Cancel
     void*             token)       // This timer event
{
   std::lock_guard<decltype(mutex)> lock(mutex);

   if( timers == nullptr )
     timers= new Timers();

   timers->cancel(token);
}

//----------------------------------------------------------------------------
//
// Method-
//       dispatch::Disp::enqueue
//
// Purpose-
//       Insert an item onto a Task's todo list.
//
//----------------------------------------------------------------------------
void
   Disp::enqueue(                   // Enqueue
     Task*             task,        // Onto this Task
     Item*             item)        // This work Item
{  task->enqueue(item); }

//----------------------------------------------------------------------------
//
// Method-
//       dispatch::Disp::delay
//
// Purpose-
//       Handle delay event request.
//
//----------------------------------------------------------------------------
void*                               // Cancellation token
   Disp::delay(                     // Delay for
     double            seconds,     // This many seconds, then
     Item*             workItem)    // Complete this work Item
{
   std::lock_guard<decltype(mutex)> lock(mutex);

   if( timers == nullptr )
     timers= new Timers();

   return timers->delay(seconds, workItem);
}

//============================================================================
//
// Method-
//       dispatch::Disp::wait
//
// Purpose-
//       Wait for all work to complete.
//
//----------------------------------------------------------------------------
void
   Disp::wait( void )               // Wait for all work to complete
{  if( HCDM ) traceh("Dispatch(*)::wait()...\n");

   if( timers != nullptr ) {
     timers->stop();
     timers->join();
     delete timers;
     timers= nullptr;
   }

   if( HCDM ) traceh("...Dispatch(*)::wait()\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       dispatch::Item::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   Item::debug(const char* info) const // Debugging display
{
   debugf("Item(%p)::debug(%s) fc(%d) cc(%d) done(%p)\n"
         , this, info, fc, cc, done);
}

//----------------------------------------------------------------------------
//
// Method-
//       dispatch::Done::done
//
// Purpose-
//       Implement pure virtual method
//
//----------------------------------------------------------------------------
void
   Done::done(                      // Process
     Item*             item)        // This work Item
{  if( HCDM ) traceh("Done(%p)::done(%p) PVM\n", this, item);
   delete item;
}

//============================================================================
//
// Method-
//       dispatch::Task::~Task
//
// Purpose-
//       Destructor.
//
// Implementation notes-
//       This is a destructor. Locking is not required.
//
//----------------------------------------------------------------------------
   Task::~Task( void )              // Destructor
{
   Item* tail= itemList.get_tail();
   if( tail ) {                     // If the list isn't empty
     Wait wait;
     Item item(Item::FC_UNDEF, &wait);
     enqueue(&item);

     if( USE_ITRACE )
       Trace::trace(".DSP", "wait", this, tail);

     undef_wait.inc();
     wait.wait();
     undef_wait.dec();

     if( USE_ITRACE )
       Trace::trace(".DSP", "wend", this, tail);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       dispatch::Task::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   Task::debug(const char* info) const        // Debugging display
{
   debugf("Task(%p)::debug(%s)\n", this, info);
   Item* item= itemList.get_tail();
   debugf("..itemList tail(%p)\n", item);
   while( item ) {
     if( (void*)item == (void*)&__detail::__end ) {
       debugf(">>%p (dummy head item)\n", item);
       break;
     }

     debugf(">>%p -> %p %d %d %p\n", item, item->get_prev()
           , item->fc, item->cc, item->done);
     item= item->get_prev();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       dispatch::Task::enqueue
//
// Purpose-
//       Insert an item onto a Task's todo list.
//
// Implementation notes-
//       For debugging only. REMOVE #if in Dispatch.h when not used.
//
//----------------------------------------------------------------------------
#if true                            // (DEBUGGING: Used to trace enqueue)
void
   Task::enqueue(                   // Enqueue
     Item*             item)        // This work Item
{
   if( USE_ITRACE )
     Trace::trace(".DSP", ".ENQ", this, item);

   Item* tail= itemList.fifo(item); // Insert work Item
   if( tail == nullptr )            // If the list was empty
     WorkerPool::work(this);        // Schedule this Task
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       dispatch::Task::work
//
// Purpose-
//       Process all available Items
//
// Implementation notes-
//       FC_UNDEF is used in ~Task to chase any enqueued operation.
//
//----------------------------------------------------------------------------
void
   Task::work( void )               // Worker interface
{  if( HCDM ) traceh("Task(%p)::work()\n", this);

   if( USE_ITRACE )
     Trace::trace(".DSP", "WORK", this, itemList.get_tail());

   Item* undef= nullptr;            // Any FC_UNDEF work item
   for(auto it= itemList.begin(); it != itemList.end(); ++it) {
     if( USE_ITRACE )
       Trace::trace(".DSP", ".DEQ", this, it.get());

     if( it->fc < 0 ) {
       if( it->fc == Item::FC_UNDEF ) {
         if( undef )                // If duplicate FC_UNDEF
           checkstop("duplicate FC_UNDEF");
         undef= it.get();
       } else if( it->fc == Item::FC_CHASE ) {
         it->post(Item::CC_NORMAL);
       } else {
         it->post(Item::CC_ERROR_FC);
       }
     }
     else
       work(it.get());
   }

   // Iteration complete (No itemList reference remain)
   if( undef )
     undef->post(Item::CC_ERROR_FC); // (~Task ignores return code)

   if( USE_ITRACE )
     Trace::trace(".DSP", "IDLE", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       dispatch::Task::work
//
// Purpose-
//       Implement Pure Virtual Method, should never be called.
//
//----------------------------------------------------------------------------
void
   Task::work(                      // Process
     Item*             item)        // This work Item
{  debugh("%4d dispatch::Task(%p)::work(%p) PVM\n", __LINE__, this, item);
   item->post();
}
}  // namespace _LIBPUB_NAMESPACE::dispatch
