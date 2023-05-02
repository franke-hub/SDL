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
//       Dispatch.cpp
//
// Purpose-
//       Implement Dispatch object methods
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include "Dispatch.h"

//----------------------------------------------------------------------------
//
// Class-
//       DispatchDone_REDO
//
// Purpose-
//       This special internal class is only used in Dispatch::drain to
//       requeue the special "fake" link back onto the Task.
//
//----------------------------------------------------------------------------
class DispatchDone_REDO : public DispatchDone {
Dispatch*              dispatch;
DispatchTask*          task;

public:
inline
   DispatchDone_REDO(
     Dispatch*         dispatch,
     DispatchTask*     task)
:  DispatchDone()
,  dispatch(dispatch), task(task)
{
   IFHCDM( logf("DispatchDone_REDO(%p)::DispatchDone_REDO(%p)\n", this, task); )
}

virtual void
   done(
     DispatchItem*     item)
{
   IFHCDM( logf("DispatchDone_REDO(%p)::done(%p) Task(%p)\n", this, item, task); )

   dispatch->enqueue(task, item);
}
}; // class DispatchDone_REDO

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::~Dispatch
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Dispatch::~Dispatch( void )      // Destructor
{
   IFHCDM( logf("Dispatch(%p)::~Dispatch()\n", this); )

   wait();
}

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::Dispatch
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Dispatch::Dispatch( void )       // Default constructor
:  master(NULL)
,  timers(NULL)
{
   IFHCDM( logf("Dispatch(%p)::Dispatch()\n", this); )

   barrier.reset();
   master= new DispatchMaster(this);
   master->start();

   timers= new DispatchTimers(this);
   timers->start();
}

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Dispatch::debug( void ) const    // Debugging display
{
   logf("Dispatch(%p)::debug()\n", this);
   if( master != NULL )
     master->debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::cancel
//
// Purpose-
//       Cancel delay event.
//
//----------------------------------------------------------------------------
void
   Dispatch::cancel(                // Cancel
     void*             token)       // This timer event
{
   AutoBarrier lock(barrier);

   if( timers != NULL )
     timers->cancel(token);
}

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::delay
//
// Purpose-
//       Handle delay event request.
//
//----------------------------------------------------------------------------
void*                               // Cancellation token
   Dispatch::delay(                 // Delay for
     double            seconds,     // This many seconds, then
     DispatchItem*     item)        // Complete this work Item
{
   AutoBarrier lock(barrier);

   if( timers == NULL )
   {
     item->post(0);
     return NULL;
   }

   return timers->delay(seconds, item);
}

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::enqueue
//
// Purpose-
//       Schedule work
//
//----------------------------------------------------------------------------
void
   Dispatch::enqueue(               // Schedule work
     DispatchTask*     task,        // -> Task
     DispatchItem*     item)        // -> Item
{
   IFHCDM( logf("Dispatch(%p)::enqueue(%p,%p)\n", this, task, item); )

   if( task->dispatch == NULL )
   {
     task->dispatch= this;
     task->fsm= task->FSM_ACTIVE;
   }

   if( task->dispatch != this )
     throw "Dispatch::enqueue WRONG DISPATCH object";

   DispatchItem* tail= task->itemList.fifo(item); // Insert Item
   if( tail == NULL )
   {
     DispatchMaster* master= this->master;
     if( master == NULL )
     {
       logf("Dispatch(%p)::enqueue(%p,%p) MASTER == NULL\n", this, task, item);
       task->drain();
     }
     else
       master->enqueue(task);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Dispatch::wait
//
// Purpose-
//       Wait for all work to complete.
//
//----------------------------------------------------------------------------
void
   Dispatch::wait( void )           // Wait for all work to complete
{
   IFHCDM( logf("Dispatch(%p)::wait()...\n", this); )

   DispatchTimers* timers;          // Set under Barrier protection
   {{{{
     AutoBarrier lock(barrier);
     timers= this->timers;
     this->timers= NULL;
   }}}}
   if( timers != NULL )
   {
     timers->notify(0);
     timers->wait();
     delete timers;
   }

   DispatchMaster* master;          // Set under Barrier protection
   {{{{
     AutoBarrier lock(barrier);
     master= this->master;
     this->master= NULL;
   }}}}
   if( master != NULL )
   {
     master->stop();
     master->wait();
     delete master;
   }

   IFHCDM( logf("...Dispatch(%p)::wait()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTask::~DispatchTask
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DispatchTask::~DispatchTask( void )// Destructor
{
   IFHCDM( logf("DispatchTask(%p)::~DispatchTask() fsm(%d)\n", this, fsm); )

   if( fsm != FSM_RESET )
   {
     logf("%d %s Task(%p) fsm(%d) not reset\n", __LINE__, __FILE__, this, fsm);
     reset();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTask::DispatchTask
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DispatchTask::DispatchTask( void ) // Constructor
:  AU_List<DispatchTask>::Link()
,  dispatch(NULL)
,  itemList()
,  fsm(FSM_RESET)
{
   IFHCDM( logf("DispatchTask(%p)::DispatchTask()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTask::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   DispatchTask::debug( void ) const // Debugging display
{
   logf("DispatchTask(%p)::debug()\n", this);
   tracef("..dispatch(%p) fsm(%d)\n", dispatch, fsm);
   tracef("..itemList\n");
   DispatchItem* item= itemList.getTail();
   while( item != NULL )
   {
     item->debug();
     item= item->getPrev();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTask::drain
//
// Purpose-
//       Drain work
//
//----------------------------------------------------------------------------
void
   DispatchTask::drain( void )       // Drain work
{
   IFHCDM( logf("DispatchTask(%p):drain()\n", this); )

   if( itemList.getTail() == NULL ) // If nothing to do
     return;                        // Do it quickly

   //-----------------------------------------------------------------------
   // In order to prevent the Task from being driven by multiple Threads,
   // we add a fake Item to task->itemList. We use the special version of
   // remq to determine the state when we remove the fake link.
   //
   // Note that this fake link is specially crafted so that it can be
   // handled without any special handling within the loop. If it is found
   // it just adds itself back onto the list.
   DispatchDone_REDO   redo(dispatch, this);  // Our Done redo object
   DispatchItem        last(DispatchItem::FC_CHASE, &redo);
   DispatchItem*       fake= &last;
   itemList.fifo(fake);

   for(;;)                          // Drain the itemList
   {
     DispatchItem* item= itemList.remq(fake); // Remove oldest link
     if( item == NULL )
       return;

     if( item->getFC() < 0 )
     {
       int cc= DispatchItem::CC_NORMAL;
       switch(item->getFC())
       {
         case DispatchItem::FC_CHASE:
           break;

         case DispatchItem::FC_TRACE:
           logf("Dispatch(%p):trace(%p)\n", this, item);
           break;

         case DispatchItem::FC_RESET:
           reset();
           item->post(item->CC_NORMAL); // May delete Item and/or Task
           return;                // So don't look at them any more

         default:
           cc= DispatchItem::CC_INVALID_FC;
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
//       DispatchTask::reset
//
// Purpose-
//       Immediately reset the Task
//
//----------------------------------------------------------------------------
void
   DispatchTask::reset( void )      // Close (reset) the Task
{
   IFHCDM( logf("DispatchTask(%p)::reset() fsm(%d)\n", this, fsm); )

   dispatch= NULL;
   itemList.reset();
   fsm= FSM_RESET;
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTask::work
//
// Purpose-
//       Operate on a work Item
//
//----------------------------------------------------------------------------
void
   DispatchTask::work(              // Operate on
     DispatchItem*     item)        // This work Item
{
   IFHCDM( logf("DispatchTask(%p)::work() fsm(%d)\n", this, fsm); )

   item->post(DispatchItem::CC_NORMAL);
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchItem::~DispatchItem
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DispatchItem::~DispatchItem( void ) // Destructor
{
   IFHCDM( logf("DispatchItem(%p)::~DispatchItem()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchItem::DispatchItem
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DispatchItem::DispatchItem( void ) // Default constructor
:  AU_List<DispatchItem>::Link()
,  fc(0)
,  cc(0)
,  done(NULL)
{
   IFHCDM( logf("DispatchItem(%p)::DispatchItem()\n", this); )
}

   DispatchItem::DispatchItem(      // Constructor
     int               fc,          // Function code
     DispatchDone*     done)        // -> Done callback Object
:  AU_List<DispatchItem>::Link()
,  fc(fc)
,  cc(0)
,  done(done)
{
   IFHCDM( logf("DispatchItem(%p)::DispatchItem(%d,%p)\n", this, fc, done); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchItem::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   DispatchItem::debug( void ) const // Debugging display
{
   logf("DispatchItem(%p)::debug() fc(%d) cc(%d) Done(%p)\n",
        this, fc, cc, done);
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchItem::post
//
// Purpose-
//       Indicate work Item complete
//
//----------------------------------------------------------------------------
void
   DispatchItem::post(              // Complete a work Item
     int               cc)          // Completion code
{
   IFHCDM(
     logf("DispatchItem(%p)::post(%d) Done(%p)\n", this, cc, done);
   )

   this->cc= cc;                    // Set the completion code
   if( done == NULL )               // If no Done callback object exists
     delete this;                   // Delete the work Item
   else                             // If a Done callback object exists
     done->done(this);              // Invoke it
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchDone::~DispatchDone
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DispatchDone::~DispatchDone( void ) // Destructor
{
   IFHCDM( logf("DispatchDone(%p)::~DispatchDone()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchDone::DispatchDone
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DispatchDone::DispatchDone( void ) // Constructor
{
   IFHCDM( logf("DispatchDone(%p)::DispatchDone()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchDone::done
//
// Purpose-
//       Handle work Item completion (DOES NOTHING, SHOULD NOT OCCUR)
//
//----------------------------------------------------------------------------
void
   DispatchDone::done(              // Handle work Item completion
     DispatchItem*)                 // The Item that completed
{
   IFHCDM( logf("DispatchDone(%p)::done() SHOULD NOT OCCUR\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchWait::~DispatchWait
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DispatchWait::~DispatchWait( void ) // Destructor
{
   IFHCDM( logf("DispatchWait(%p)::~DispatchWait()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchWait::DispatchWait
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DispatchWait::DispatchWait( void ) // Constructor
:  DispatchDone()
,  status()
{
   IFHCDM( logf("DispatchWait(%p)::DispatchWait()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchWait::done
//
// Purpose-
//       Handle work Item completion
//
//----------------------------------------------------------------------------
void
   DispatchWait::done(              // Handle Item completion
     DispatchItem*     item)        // The Item that completed
{
   IFHCDM( logf("DispatchWait(%p)::done()\n", this); )

   status.post(item->getCC());
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchWait::reset
//
// Purpose-
//       Reset this object, allowing re-use
//
//----------------------------------------------------------------------------
void
   DispatchWait::reset( void )      // Reset the DispatchWait
{
   IFHCDM( logf("DispatchWait(%p)::reset()\n", this); )

   status.reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchWait::wait
//
// Purpose-
//       Wait for work Item completion
//
//----------------------------------------------------------------------------
int                                 // The completion code
   DispatchWait::wait( void )       // Wait for Item completion
{
   IFHCDM( logf("DispatchWait(%p)::wait()...\n", this); )

   int result= status.wait();

   IFHCDM( logf("%d= DispatchWait(%p)::wait()\n", result, this); )

   return result;
}

