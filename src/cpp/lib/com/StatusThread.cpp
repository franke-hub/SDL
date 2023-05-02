//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       StatusThread.cpp
//
// Purpose-
//       StatusThread object methods.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <com/Debug.h>

#include "com/StatusThread.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Method-
//       StatusThread::~StatusThread
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   StatusThread::~StatusThread( void ) // Destructor
{
   IFHCDM( debugf("StatusThread(%p)::~StatusThread()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       StatusThread::StatusThread
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   StatusThread::StatusThread( void ) // Constructor
:  Thread()
,  event(0)
,  fsm(FSM_INUSE)
{
   IFHCDM( debugf("StatusThread(%p)::StatusThread()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       StatusThread::post
//
// Purpose-
//       Send "work available" signal.
//
//----------------------------------------------------------------------------
void
   StatusThread::post( void )       // Signal work available
{
   IFHCDM( debugf("StatusThread(%p)::post()\n", this); )

   event.post();
}

//----------------------------------------------------------------------------
//
// Method-
//       StatusThread::run
//
// Purpose-
//       Operate the Thread, looking for work units
//
//----------------------------------------------------------------------------
long
   StatusThread::run( void )        // Operate the Thread
{
   IFHCDM( debugf("StatusThread(%p)::run()\n", this); )

   while( fsm == FSM_INUSE )
   {
     //-----------------------------------------------------------------------
     // Make transition from FSM_INUSE to FSM_READY
     int32_t oldValue= fsm;
     int cc= csw(&fsm, FSM_INUSE, FSM_READY);
     if( cc != 0 )
     {
       if( oldValue != FSM_CLOSE )
         debugf("%4d %s FSM(%d)\n", __LINE__, __FILE__, oldValue);

       do
       {
         oldValue= fsm;
         cc= csw(&fsm, oldValue, FSM_RESET);
       } while( cc != 0 );

       break;
     }

     //-----------------------------------------------------------------------
     // Wait for work
     event.wait();

     //-----------------------------------------------------------------------
     // Make transition from FSM_READY to FSM_INUSE
     oldValue= fsm;
     cc= csw(&fsm, FSM_READY, FSM_INUSE);
     if( cc != 0 )
     {
       if( oldValue != FSM_CLOSE )
         debugf("%4d %s FSM(%d)\n", __LINE__, __FILE__, oldValue);

       do
       {
         oldValue= fsm;
         cc= csw(&fsm, oldValue, FSM_RESET);
       } while( cc != 0 );

       break;
     }

     //-----------------------------------------------------------------------
     // Process one work unit
     work();
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       StatusThread::stop
//
// Purpose-
//       Terminate processing.
//
//----------------------------------------------------------------------------
void
   StatusThread::stop( void )       // Terminate processing
{
   IFHCDM( debugf("StatusThread(%p)::stop() fsm(%d)\n", this, fsm); )

   int cc;
   int32_t oldValue;
   do
   {
     oldValue= fsm;
     cc= csw(&fsm, oldValue, FSM_CLOSE);
   } while( cc != 0 );

   switch(oldValue)
   {
     case FSM_RESET:
       fsm= FSM_RESET;
       break;

     case FSM_READY:
       event.post();
       break;

     default:
       break;
   }
}

