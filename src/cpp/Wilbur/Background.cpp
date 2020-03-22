//----------------------------------------------------------------------------
//
//       Copyright (c) 2011 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Background.cpp
//
// Purpose-
//       Background implementation methods.
//
// Last change date-
//       2011/01/01
//
//----------------------------------------------------------------------------
#include <new>
#include <stdio.h>
#include <com/Thread.h>

#include "Common.h"
#include "Background.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BACKGROUND_DELAY 3600.0     // Background delay interval, seconds

//----------------------------------------------------------------------------
//
// Subroutine-
//       schedule
//
// Purpose-
//       Schedule a work Item.
//
//----------------------------------------------------------------------------
static inline void
   schedule(                        // Schedule a work Item
     DispatchTask*     task,        // -> Task
     DispatchItem*     item)        // -> Item
{
   Common::get()->dispatcher.enqueue(task, item);
}

//----------------------------------------------------------------------------
//
// Method-
//       BackgroundTask::~BackgroundTask
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   BackgroundTask::~BackgroundTask( void ) // Destructor
{
   IFHCDM( logf("BackgroundTask(%p)::~BackgroundTask()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       BackgroundTask::BackgroundTask
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   BackgroundTask::BackgroundTask( void ) // Constructor
:  DispatchTask()
,  cleanCache()
{
   IFHCDM( logf("BackgroundTask(%p)::BackgroundTask()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       BackgroundTask::close
//
// Purpose-
//       Reset background Tasks
//
// Implementation notes-
//       This method is only called from Background::done when terminating.
//
//----------------------------------------------------------------------------
void
   BackgroundTask::close( void )    // Termination event
{
   cleanCache.reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       BackgroundTask::work
//
// Purpose-
//       Handle Background time slice.
//
//----------------------------------------------------------------------------
void
   BackgroundTask::work(            // Background event
     DispatchItem*     inp)         // Associated Item
{
   IFHCDM( logf("BackgroundTask(%p)::work(%p)...\n", this, inp); )

   DispatchWait        wait;        // Wait object
   DispatchItem        item(DispatchItem::FC_VALID, &wait); // Work Item

   schedule(&cleanCache, &item);    // Drive background
   wait.wait();
   wait.reset();

   Common& common= *Common::get();  // Our Common
   common.dispatcher.delay(BACKGROUND_DELAY, inp);

   IFHCDM( logf("BackgroundTask(%p)::work(%p) EXIT\n", this, inp); )
}

//----------------------------------------------------------------------------
//
// Method-
//       Background::~Background
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Background::~Background( void )  // Destructor
{
   IFHCDM( logf("Background(%p)::~Background()\n", this); )

   //-------------------------------------------------------------------------
   // We may get here before the timer cleanup processing completes.
   // We need to wait for that to occur.
   wait.wait();
}

//----------------------------------------------------------------------------
//
// Method-
//       Background::Background
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Background::Background( void )   // Constructor
:  DispatchWait()
,  item(DispatchItem::FC_VALID, this)
{
   IFHCDM( logf("Background(%p)::Background()\n", this); )

   // Schedule initial background event
   Common& common= *Common::get();  // Our Common
   common.dispatcher.delay(120.0, &item); // Startup delay
}

//----------------------------------------------------------------------------
//
// Method-
//       Background::done
//
// Purpose-
//       Handle Background timer event.
//
//----------------------------------------------------------------------------
void
   Background::done(                // Background timer completion
     DispatchItem*     item)        // Associated Item
{
   IFHCDM( logf("Background(%p)::done(%p)\n", this, item); )

   Common& common= *Common::get();  // Our Common
   if( common.fsm != Common::FSM_READY ) // If not in READY state
   {
     IFHCDM( logf("Background(%p) terminating\n", this); )

     // RESET the BackgroundTask object
     task.close();                  // BackgroundTask internal reset

     item->setDone(&wait);          // Ready to terminate
     item->setFC(DispatchItem::FC_RESET); // Reset the background Task
     schedule(&task, item);         // Reset, post Wait on completion
   }
   else
     schedule(&task, item);         // Defer processing
}

