//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2022 Frank Eskesen.
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
//       2022/11/27
//
//----------------------------------------------------------------------------
#include <assert.h>                 // For assert
#include <mutex>                    // For std::lock_guard

#include <pub/Clock.h>              // DispatchTTL completion time
#include <pub/Debug.h>              // For debugging
#include "pub/Dispatch.h"           // For dispatch objects, implemented
#include <pub/Latch.h>              // For pub::Latch, mutex substitute
#include <pub/List.h>               // For pub::AI_list
#include <pub/Named.h>              // For pub::Named, Timers is a Named Thread
#include <pub/Semaphore.h>          // For pub::Semaphore, Timers event
#include <pub/Thread.h>             // For pub::Thread, Timers is a Named Thread
#include <pub/Trace.h>              // For pub::Trace
#include <pub/Worker.h>             // For pub::Worker

using namespace _LIBPUB_NAMESPACE::debugging; // Enable debugging functions

namespace _LIBPUB_NAMESPACE::dispatch {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
// VERBOSE= 0                       // Verbosity, higher is more verbose

,  USE_XTRACE= true                 // Use extended tracing?
}; // enum

//----------------------------------------------------------------------------
// dispatch::Static attributes
//----------------------------------------------------------------------------
Latch                  Disp::mutex; // Termination mutex
Timers*                Disp::timers= nullptr;

#include "Dispatch.h"               // For Timers, DispatchTTL

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
//----------------------------------------------------------------------------
   Task::~Task( void )                        // Destructor
{
   Wait wait;
   Item item(Item::FC_UNDEF, &wait);

   Item* tail= itemList.fifo(&item); // Insert work Item
   if( tail != nullptr )            // If the list wasn't empty
     wait.wait();                   // Wait for completion
// else
//   itemList.reset();              // Nothing to do (not needed)
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
     if( (void*)item == __detail::__end ) {
       debugf("....%p (dummy head item)\n", item);
       break;
     }

     debugf("....%p -> %p\n", item, item->get_prev());
     item= item->get_prev();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       dispatch::Task::work
//
// Purpose-
//       Process all available Items
//
//----------------------------------------------------------------------------
void
   Task::work( void )               // Worker interface
{  if( HCDM ) traceh("Task(%p)::work()\n", this);

// Since this shouldn't occur and is handled properly if it does, skip it
// if( itemList.is_empty() )        // If nothing to do (should not occur)
//   return;                        // Do it quickly

   if( USE_XTRACE )
     Trace::trace(".DSP", "WORK", this, itemList.get_tail());

   for(auto it= itemList.begin(); it != itemList.end(); ++it) {
     if( USE_XTRACE )
       Trace::trace(".DSP", "ITER", this, it.get());

     if( it->fc < 0 ) {
       if( it->fc == Item::FC_UNDEF ) {
         // We use UNDEF to insure that a TASK has no work pending.
         // We therefore need to insure that all other work completes and
         // it == itemList.end().
         // In the usual case, all that's needed is the first `++i`.
         // In the unusual case, the UNDEF is posted out of sequence.
         auto post_it= it;

         while(++it != itemList.end() ) {
           // This code is not normally executed; the ++it ends the itemList
           Trace::trace(".DSP", "XTRA", this); // (Unexpected)
           if( it >= 0 ) {
             work(it.get());
           } else {
             if( it->fc == Item::FC_CHASE ) {
               it->post(Item::CC_NORMAL);
             } else {
               it->post(Item::CC_ERROR_FC);
             }
           }
         }

         post_it->post(Item::CC_ERROR_FC); // (~Task doesn't care)
         break;
       } else {
         it->post(Item::CC_ERROR_FC);
       }
     }
     else
       work(it.get());
   }

   if( USE_XTRACE )
     Trace::trace(".DSP", "IDLE", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       dispatch::Task::work
//
// Purpose-
//       Implement pure virtual method
//
//----------------------------------------------------------------------------
void
   Task::work(                      // Process
     Item*             item)        // This work Item
{  debugh("Task(%p)::work(%p) PVM\n", this, item);
   item->post();
}
}  // namespace _LIBPUB_NAMESPACE::dispatch
