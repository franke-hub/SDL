//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       StatusThread.h
//
// Purpose-
//       A Thread that handles discrete units of work.
//
// Last change date-
//       2010/01/01
//
// Notes-
//       The run() method for StatusThread is the final version. It must not
//       be overridden. The work() method user replaceable.
//
//       The constructor of a StatusThread should call start() immediately
//       after construction. (This cannot be done in this constructor because
//       the StatusThread cannot be the final class. The wait method could
//       be called before construction was complete and would certainly fail.)
//
//       When a unit of work has been selected for the StatusThread, the
//       post() method is called. This in turn drives the work() method
//       which then processes the work unit.
//
//       The StatusThread maintains its internal state in a Finite State
//       Machine, which must be atomically updated. While this can be
//       extended, the FSM state must be FSM_INUSE when work completes
//       or the StatusThread will terminate.
//
//       To complete Thread operation, invoke stop() and then wait().
//
//----------------------------------------------------------------------------
#ifndef STATUSTHREAD_H_INCLUDED
#define STATUSTHREAD_H_INCLUDED

#ifndef ATOMIC_H_INCLUDED
#include "Atomic.h"
#endif

#ifndef SEMAPHORE_H_INCLUDED
#include "Semaphore.h"
#endif

#ifndef THREAD_H_INCLUDED
#include "Thread.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       StatusThread
//
// Purpose-
//       A Thread that handles discrete units of work.
//
//----------------------------------------------------------------------------
class StatusThread : public Thread { // The StatusThread Thread
//----------------------------------------------------------------------------
// StatusThread::Attributes
//----------------------------------------------------------------------------
protected:
Semaphore              event;       // State switch event Semaphore
ATOMIC32               fsm;         // Finite State Machine

//----------------------------------------------------------------------------
// StatusThread::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum FSM                            // Finite State Machine states
{  FSM_RESET= 0                     // Reset
,  FSM_READY                        // Ready, waiting
,  FSM_INUSE                        // Ready, operating
,  FSM_CLOSE                        // Halted
}; // enum FSM

//----------------------------------------------------------------------------
// StatusThread::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~StatusThread( void );           // Destructor
   StatusThread( void );            // Constructor

//----------------------------------------------------------------------------
// StatusThread::Accessor methods
//----------------------------------------------------------------------------
public:
inline int32_t                      // The current state
   getFSM( void )                   // Get current state
{  return fsm;
}

//----------------------------------------------------------------------------
// StatusThread::post()
//
// Call this function once for each unit of work to be processed.
//----------------------------------------------------------------------------
virtual void
   post( void );                    // Signal work available

//----------------------------------------------------------------------------
// StatusThread::stop()
//
// Terminate Thread processing.
//----------------------------------------------------------------------------
virtual void
   stop( void );                    // Terminate processing

protected:
//----------------------------------------------------------------------------
// StatusThread::run()
//
// This method performs the StatusThread function.
// Override it only to add entry and/or exit functionality, e.g.
//   myEntryFunction();
//   StatusThread::run;
//   myExitFunction();
//----------------------------------------------------------------------------
virtual long                        // !!DO NOT OVERRIDE!!
   run( void );                     // Operate the Thread

//----------------------------------------------------------------------------
// StatusThread::work()
//
// This user-provided method processes one unit of work.
// It is called once for each invocation of post().
//----------------------------------------------------------------------------
virtual void
   work( void ) = 0;                // Process unit of work
}; // class StatusThread

#endif // STATUSTHREAD_H_INCLUDED
