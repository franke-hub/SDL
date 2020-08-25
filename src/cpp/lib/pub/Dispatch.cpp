//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
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
//       2020/08/24
//
//----------------------------------------------------------------------------
#include "Dispatch.h"

namespace _PUB_NAMESPACE::Dispatch {
//----------------------------------------------------------------------------
// Dispatch::Static attributes
//----------------------------------------------------------------------------
Latch                  Disp::mutex; // Termination mutex
Timers*                Disp::timers= nullptr;

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::Disp::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Disp::debug( void )              // Debugging display
{
   traceh("Dispatch::debug()\n");
   WorkerPool::debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::Disp::cancel
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
//       Dispatch::Disp::delay
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
//       Dispatch::Disp::wait
//
// Purpose-
//       Wait for all work to complete.
//
//----------------------------------------------------------------------------
void
   Disp::wait( void )               // Wait for all work to complete
{  IFHCDM( logf("Dispatch(%p)::wait()...\n", this); )

   if( timers != nullptr )
   {
     timers->stop();
     timers->join();
     delete timers;
     timers= nullptr;
   }

   IFHCDM( logf("...Dispatch(%p)::wait()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::Item::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   Item::debug( void ) const        // Debugging display
{
   tracef("Item(%p)::debug() fc(%d) cc(%d) done(%p) work(%p)\n",
        this, fc, cc, done, work);
}

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::Task::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   Task::debug( void ) const        // Debugging display
{
   tracef("Task(%p)::debug()\n", this);
   tracef("..itemList\n");
   Item* item= itemList.get_tail();
   while( item ) {
     item->debug();
     item= item->get_prev();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::Task::work
//
// Purpose-
//       Process all available Items
//
//----------------------------------------------------------------------------
void
   Task::work( void )               // Worker interface
{  IFHCDM( traceh("Task(%p):work()\n", this); )
   if( itemList.get_tail() == nullptr ) // If nothing to do (should not occur)
     return;                        // Do it quickly

   //-----------------------------------------------------------------------
   // In order to prevent the Task from being driven by multiple Threads,
   // we add a fake Item to the itemList. We use swap to replace the list
   // with the fake link.
   Item                only;        // Our fake work Item
   Item*               fake= &only;

   Item* list= itemList.swap(fake); // Replace List with fake element
   AU_FIFO<Item> fifo(list);        // Initialize the FIFO
   for(;;) {                        // Drain the itemList
     Item* item= fifo.remq();       // Get oldest link
     if( item == nullptr ) {        // If none remain
       list= itemList.swap(fake);   // Get new list
       if( list == nullptr )        // If none left
         return;

       fifo.reset(list);            // Re-initialize the fifo
       item= fifo.remq();           // Remove oldest link (the fake item)
       assert( item == fake );      // Verify what we think we know
       item->set_prev(nullptr);     // The fake item ends the AU_List

       item= fifo.remq();           // Remove oldest link
       assert( item != nullptr );   // Which should not be a nullptr
     }

     if( item->fc < 0 )
     {
       int cc= Item::CC_NORMAL;
       switch(item->fc)
       {
         case Item::FC_CHASE:
           break;

         case Item::FC_TRACE:
           traceh("Task(%p):trace(%p)\n", this, item);
           break;

         case Item::FC_RESET:
           itemList.reset();
           item->post();          // May delete Item and/or Task
           return;                // So don't look at them any more

         default:
           cc= Item::CC_INVALID_FC;
           break;
       }

       item->post(cc);
     }
     else
       work(item);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::Task::work
//
// Purpose-
//       Implement pure virtual method
//
//----------------------------------------------------------------------------
void
   Task::work(                      // Process
     Item*             item)        // This work Item
{  IFHCDM( logf("Task(%p)::work(%p)\n", this, item); )
   item->post();
}
}  // namespace _PUB_NAMESPACE::Dispatch
