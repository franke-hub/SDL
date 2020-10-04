//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DispatchTHR.cpp
//
// Purpose-
//       Implement Dispatch internal Threads
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <com/Interval.h>

#include "Dispatch.h"

//----------------------------------------------------------------------------
//
// Method-
//       DispatchMaster::~DispatchMaster
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DispatchMaster::~DispatchMaster( void )
{
   IFHCDM(
     logf("DispatchMaster(%p)::~DispatchMaster() count(%u)\n", this, count);
   )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchMaster::DispatchMaster
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DispatchMaster::DispatchMaster(
     Dispatch*         owner)       // The associated Dispatch object
:  StatusThread()
,  owner(owner)
,  dtlList()
,  taskList()
,  count(0)
{
   IFHCDM( logf("DispatchMaster(%p)::DispatchMaster(%p)\n", this, owner); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchMaster::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   DispatchMaster::debug( void ) const // Debugging display
{
   logf("DispatchMaster(%p)::debug()\n", this);
   tracef("..count(%d) owner(%p)\n", count, owner);
   tracef("..taskList\n");
   DispatchTask* task= taskList.getTail();
   while( task != NULL )
   {
     task->debug();
     task= task->getPrev();
   }

   tracef("..dtlList\n");
   DispatchDTL* dtl= dtlList.getTail();
   while( dtl != NULL )
   {
     tracef("DTL(%p) thread(%p)\n", dtl, dtl->getThread());
     dtl= dtl->getPrev();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchMaster::done
//
// Purpose-
//       Reuse a DispatchThread
//
//----------------------------------------------------------------------------
void
   DispatchMaster::done(            // A DispatchThread completed
     DispatchThread*    thread)     // -> DispatchThread
{
   IFHCDM( logf("DispatchMaster(%p)::done(%p)\n", this, thread); )

   dtlList.fifo(thread->getDispatchDTL());
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchMaster::stop
//
// Purpose-
//       Terminate processing.
//
//----------------------------------------------------------------------------
void
   DispatchMaster::stop( void )     // Terminate processing
{
   IFHCDM( logf("DispatchMaster(%p)::stop()...\n", this); )

   Interval interval;
   for(;;)
   {
     DispatchDTL* link= dtlList.reset();
     while( link != NULL )
     {
       DispatchDTL* prev= link->getPrev();

       DispatchThread* thread= link->getThread();
       thread->stop();
       thread->wait();
       delete thread;               // (Which contains the link)
       count--;

       link= prev;
     }

     if( count == 0 )               // If no remaining threads
       break;

     logf("DispatchMaster.stop() Waiting for open threads\n");
     if( interval.stop() > 30.0 )
     {
       printf("DispatchMaster.stop() Waiting for open threads\n");
       interval.start();
     }

     Thread::sleep(1);
   }

   StatusThread::stop();
   IFHCDM( logf("...DispatchMaster(%p)::stop()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchMaster::work
//
// Purpose-
//       Process one work Item.
//
//----------------------------------------------------------------------------
void
   DispatchMaster::work( void )     // Operate the Thread
{
   IFHCDM( logf("DispatchMaster(%p)::work()\n", this); )

   //-----------------------------------------------------------------------
   // In order to prevent being driven multiple times for a single post,
   // we add a fake Link to taskList. We use the special version of
   // remq to determine the state when we remove the fake link.
   DispatchTask        last;
   DispatchTask*       fake= &last;
   taskList.fifo(fake);

   for(;;)
   {
     DispatchTask* task= taskList.remq(fake);
     if( task == NULL )
       break;
     else if( task == fake )
     {
       taskList.fifo(fake);
       continue;
     }

     DispatchThread* thread= NULL;
     DispatchDTL* dispatchDTL= dtlList.remq();
     if( dispatchDTL == NULL )
     {
       thread= new DispatchThread(owner, this);
       thread->start();
       count++;
     }
     else
       thread= dispatchDTL->getThread();

     IFHCDM( logf("DispatchMaster(%p)::work() thread(%p)\n", this, thread); )
     thread->enqueue(task);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchThread::~DispatchThread
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DispatchThread::~DispatchThread( void )
{
   IFHCDM( logf("DispatchThread(%p)::~DispatchThread()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchThread::DispatchThread
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
#ifdef _OS_WIN
#pragma warning( push )             // Microsoft
#pragma warning( disable: 4355 )    // Microsoft
#endif
   DispatchThread::DispatchThread(
     Dispatch*         dispatch,    // The associated Dispatch
     DispatchMaster*   owner)       // The associated DispatchMaster
:  StatusThread()
,  link(this)
,  dispatch(dispatch)
,  owner(owner)
,  task(NULL)
{
   IFHCDM( logf("DispatchThread(%p)::DispatchThread()\n", this); )
}
#ifdef _OS_WIN
#pragma warning( pop )              // Microsoft
#endif

//----------------------------------------------------------------------------
//
// Method-
//       DispatchThread::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   DispatchThread::debug( void ) const // Debugging display
{
   logf("DispatchThread(%p)::debug() dispatch(%p) owner(%p) task(%p)\n",
        this, dispatch, owner, task);
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchThread::post
//
// Purpose-
//       Signal work available. (Adds debugging)
//
//----------------------------------------------------------------------------
void
   DispatchThread::post( void )     // Signal work available
{
   IFHCDM( logf("DispatchThread(%p)::post()\n", this); )

   StatusThread::post();
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchThread::run
//
// Purpose-
//       Operate the thread. (Adds debugging)
//
//----------------------------------------------------------------------------
long
   DispatchThread::run( void )      // Operate the thread
{
   IFHCDM( logf("DispatchThread(%p)::run()...\n", this); )

   int result= StatusThread::run();

   IFHCDM( logf("%d= ...DispatchThread(%p)::run()\n", result, this); )
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchThread::stop
//
// Purpose-
//       Signal termination. (Adds debugging)
//
//----------------------------------------------------------------------------
void
   DispatchThread::stop( void )     // Signal termination
{
   IFHCDM( logf("DispatchThread(%p)::stop()\n", this); )

   StatusThread::stop();
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchThread::wait
//
// Purpose-
//       Wait for thread to complete. (Adds debugging and termination)
//
//----------------------------------------------------------------------------
long
   DispatchThread::wait( void )     // Wait for completion
{
   IFHCDM( logf("DispatchThread(%p)::wait()...\n", this); )

   stop();                          // Terminate the StatusThread
   int result= StatusThread::wait();

   IFHCDM( logf("%d= DispatchThread(%p)::wait()\n", result, this); )
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchThread::work
//
// Purpose-
//       Process work unit.
//
//----------------------------------------------------------------------------
void
   DispatchThread::work( void )     // Process work unit
{
   IFHCDM( logf("DispatchThread(%p)::work()\n", this); )

   task->drain();
   task= NULL;
   owner->done(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTimers::~DispatchTimers
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   DispatchTimers::~DispatchTimers( void )
{
   IFHCDM( logf("DispatchTimers(%p)::~DispatchTimers()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTimers::DispatchTimers
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   DispatchTimers::DispatchTimers(
     Dispatch*         owner)       // The associated Dispatch object
:  NamedThread("DispatchTime")
,  owner(owner)
,  fsm(FSM_RESET)
,  event()
,  list()
,  pend()
{
   IFHCDM( logf("DispatchTimers(%p)::DispatchTimers()\n", this); )

   fsm= FSM_START;
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTimers::cancel
//
// Purpose-
//       Cancel delay event.
//
// Implementation notes-
//       The actual cancel function must be done in DispatchTimers::run().
//       We create a dummy work Item, add it to the delay list, and when the
//       dummy work Item completes, actually process the cancel process.
//
//----------------------------------------------------------------------------
void
   DispatchTimers::cancel(          // Cancel
     void*             token)       // This timer event
{
   IFHCDM( logf("DispatchTimers(%p)::cancel(%p)\n", this, token); )

   DispatchTTL* link= (DispatchTTL*)token;
   if( link != NULL && fsm == FSM_READY )
   {
     class DispatchTimersWait : public DispatchWait {
       // Attributes
       DispatchTimers* timer;
       DispatchTTL*    token;

       // Constructor
       public:
       DispatchTimersWait( DispatchTimers* timers, DispatchTTL* token )
       : DispatchWait(), timer(timers), token(token) {}

       // Method: DispatchTimersWait::done()
       // Called indirectly from DispatchTimers::run(), thus protecting
       // the ordered list of pending events.
       //
       // At this point, all prior events have been moved from the atomic
       // pending list to the sorted pending list. We need only check the
       // sorted pending list to find the DispatchTTL element to cancel.
       // Note that this element may not exist; It could already be posted.
       void done( DispatchItem* item ) {
         IFHCDM( logf("DispatchTimers(%p)::cancelWait.done(%p)\n", timer, token); )

         if( timer->getList().isOnList(token) ) // If element is still on the list
         {
           timer->getList().remove(token, token); // Remove it
           token->item->post(item->CC_ERROR); // Complete the work item w/error

           delete token;            // Delete the timer event
         }

         DispatchWait::done( item ); // Complete the cancel Item
       }
     } wait(this, link);

     DispatchItem item(DispatchItem::FC_VALID, &wait);
     delay(0.0, &item);
     wait.wait();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTimers::delay
//
// Purpose-
//       Handle delay event request.
//
//----------------------------------------------------------------------------
void*                               // Cancellation token
   DispatchTimers::delay(           // Delay for
     double            seconds,     // This many seconds, then
     DispatchItem*     workItem)    // Complete this work Item
{
   IFHCDM( logf("DispatchTimers(%p)::delay(%.3f,%p)\n",
                this, seconds, workItem); )

   Clock time;                      // Get current time
   time += seconds;                 // Set termination time

   //-------------------------------------------------------------------------
   // If ready, add the event to the event queue (otherwise immediate post)
   DispatchTTL* link= NULL;         // Default, already posted

   if( fsm != FSM_READY )
     workItem->post(0);
   else
   {
     link= new DispatchTTL(time, workItem);

     if( pend.fifo(link) == NULL )  // If the list was created
       event.post();                // Drive the thread
   }

   return link;
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTimers::notify
//
// Purpose-
//       Termination notification.
//
//----------------------------------------------------------------------------
int                                 // Return code
   DispatchTimers::notify(          // Notify DispatchTimers
     int               code)        // With this (unused) code
{
   IFHCDM( logf("DispatchTimers(%p)::notify(%d)\n", this, code); )
   (void)code;                      // code is ignored

   AutoBarrier lock(owner->barrier);
   if( fsm != FSM_CLOSE )           // If not already notified
   {
     fsm= FSM_CLOSE;
     event.post();
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTimers::run
//
// Purpose-
//       Timers driver.
//
//----------------------------------------------------------------------------
long                                // Return code
   DispatchTimers::run( void )      // DispatchTimers driver
{
   DispatchTTL*        link= NULL;  // Current work Item

   IFHCDM( logf("DispatchTimers(%p)::run\n", this); )

   {{{{
     AutoBarrier lock(owner->barrier);

     if( fsm != FSM_START )        // If sequence error
       return 1;

     fsm= FSM_READY;
   }}}}

   for(;;)
   {
     //-----------------------------------------------------------------------
     // Hard Core Debug Mode
     IFHCDM( logf("DispatchTimers(%p)::run (drain)\n", this); )

     //-----------------------------------------------------------------------
     // Drain the pend list, adding to the ordered list
     AU_List<DispatchTTL>::Link* tail= pend.reset();
     while( tail != NULL )
     {
       link= (DispatchTTL*)tail;
       tail= tail->getPrev();

       DispatchTTL* item= list.getHead();
       DispatchTTL* prior= NULL;
       while( item != NULL )
       {
         if( item->time > link->time)
           break;

         prior= item;
         item= item->getNext();
       }

       list.insert(prior, link, link);
     }

     //-----------------------------------------------------------------------
     // Drive events that have exired
     Clock now;                     // Get current time
     for(;;)
     {
       link= list.getHead();
       if( link == NULL || link->time > now )
         break;

       list.remq();
       link->item->post(0);

       delete link;
       now= Clock::current();
     }

     //-----------------------------------------------------------------------
     // State check
     //
     // If the state changes (to FSM_RESET) after this check, the Semaphore
     // will be posted. The lists will be drained the next time through.
     if( fsm != FSM_READY )
       break;

     //-----------------------------------------------------------------------
     // If required, update waiter
     double delay= 60.0;            // Maximum wait time
     link= list.getHead();
     if( link != NULL )
     {
       double waitTime= link->time - now; // Event delay
       if( delay > waitTime )       // If less than maximum wait time
         delay= waitTime;           // Set event delay
     }

     //-----------------------------------------------------------------------
     // Wait for event (with timeout)
     IFHCDM( logf("DispatchTimers(%p)::wait(%8.3f)\n", this, delay); )
     event.wait(delay);
   }

   //-------------------------------------------------------------------------
   // Hard Core Debug Mode
   IFHCDM( logf("DispatchTimers(%p)::run (terminating)\n", this); )

   //-------------------------------------------------------------------------
   // Terminate communication
   AutoBarrier lock(owner->barrier);
   fsm= FSM_RESET;

   //-------------------------------------------------------------------------
   // Drive any remaining events
   link= list.remq();
   while( link != NULL )
   {
     link->item->post(0);
     delete link;

     link= list.remq();
   }

   link= pend.remq();
   while( link != NULL )
   {
     link->item->post(0);
     delete link;

     link= pend.remq();
   }

   IFHCDM( logf("DispatchTimers(%p)::terminated\n", this); )
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTTL::~DispatchTTL
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   DispatchTTL::~DispatchTTL( void ) // Destructor
{
   IFHCDM( logf("DispatchTTL(%p)::~DispatchTTL()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       DispatchTTL::DispatchTTL
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   DispatchTTL::DispatchTTL(        // Constructor
     const Clock&      time,        // Completion time
     DispatchItem*     item)        // Work Item
:  List<DispatchTTL>::Link(), AU_List<DispatchTTL>::Link()
,  time(time)
,  item(item)
{
   IFHCDM( logf("DispatchTTL(%p)::DispatchTTL(%12.3f,%p)\n",
                this, (double)time, item); )
}

