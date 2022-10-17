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
//       2022/10/05
//
//----------------------------------------------------------------------------
#include "Dispatch.h"               // For namespace pub::dispatch definitions

namespace _LIBPUB_NAMESPACE::dispatch {
//----------------------------------------------------------------------------
// dispatch::Static attributes
//----------------------------------------------------------------------------
Latch                  Disp::mutex; // Termination mutex
Timers*                Disp::timers= nullptr;

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
   traceh("dispatch::debug()\n");
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

//----------------------------------------------------------------------------
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

   if( timers != nullptr )
   {
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
   tracef("Item(%p)::debug(%s) fc(%d) cc(%d) done(%p)\n"
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
   tracef("Task(%p)::debug(%s)\n", this, info);
   Item* item= itemList.get_tail();
   tracef("..itemList %p\n", item);
   while( item && (void*)item != __detail::__end ) {
     Item* cast= dynamic_cast<Item*>(item);
//tracef("%4d TASK cast(%p)->%p item(%p)\n", __LINE__, cast, cast->get_prev(), item);
     if( cast ) {
//tracef("%4d TASK Item(%p)->%p IS an Item\n", __LINE__, cast, cast->get_prev());
//cast->debug();
//tracef("%4d HCDM\n", __LINE__);
       item= cast->get_prev();      // (TODO: NOTE: Can't use cast->_prev)
     } else {
       tracef("Item(%p)->?? Not an Item\n", item);
       item= nullptr;
     }
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

   for(auto it= itemList.begin(); it != itemList.end(); ++it) {
     if( it->fc < 0 )
     {
       int cc= Item::CC_NORMAL;
       switch(it->fc)
       {
         case Item::FC_CHASE:
           break;

         case Item::FC_TRACE:
           traceh("Task(%p):trace(%p)\n", this, it.get());
           break;

         default:
           cc= Item::CC_ERROR_FC;
           break;
       }

       it->post(cc);
     }
     else
       work(it.get());
   }
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
{  if( HCDM ) traceh("Task(%p)::work(%p) PVM\n", this, item);
   item->post();
}
}  // namespace _LIBPUB_NAMESPACE::dispatch
