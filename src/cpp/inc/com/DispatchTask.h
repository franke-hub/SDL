//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DispatchTask.h
//
// Purpose-
//       Standard Dispatch Task block. (Included from Dispatch.h)
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef DISPATCHTASK_H_INCLUDED
#define DISPATCHTASK_H_INCLUDED

#ifndef DISPATCH_H_INCLUDED
#include "Dispatch.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       DispatchTask
//
// Purpose-
//       The dispatcher Task Block.
//
// Notes-
//       Each Task Block handles one work item at a time.
//
//       The reset method is used to disassociate the Task from the Dispatch
//       object. This method must not be used from within the work method.
//       The user must insure that no new request will occur between the
//       invocation of the reset method and its return.
//
//----------------------------------------------------------------------------
class DispatchTask : public AU_List<DispatchTask>::Link { // Dispatcher Task Block
friend class Dispatch;              // Only Dispatch invokes work()
friend class DispatchMaster;        // Only DispatchMaster accesses AU_Link
friend class DispatchThread;        // Only DispatchThread invokes drain()

//----------------------------------------------------------------------------
// DispatchTask::Enumerations and typedefs
//----------------------------------------------------------------------------
protected:
enum FSM                            // Finite State Machine
{  FSM_RESET= 0                     // Reset (idle)
,  FSM_ACTIVE                       // Active
}; // enum FSM

//----------------------------------------------------------------------------
// DispatchTask::Attributes
//----------------------------------------------------------------------------
protected:
Dispatch*              dispatch;    // The associated Dispatch object
AU_List<DispatchItem>  itemList;    // The Item List
int                    fsm;         // Finite State Machine

//----------------------------------------------------------------------------
// DispatchTask::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DispatchTask( void );           // Destructor
   DispatchTask( void );            // Constructor

private:                            // Bitwise copy is prohibited
   DispatchTask(const DispatchTask&); // Disallowed copy constructor
   DispatchTask& operator=(const DispatchTask&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// DispatchTask::Methods
//----------------------------------------------------------------------------
public:
inline int                          // TRUE iff Task has no enqueued work
   isIdle( void ) const             // Is this Task idle?
{
   return (itemList.getTail() == NULL);
}

inline int                          // TRUE iff Task has enqueued work
   isBusy( void ) const             // Is this Task busy?
{
   return (itemList.getTail() != NULL);
}

void
   debug( void ) const;             // Debugging display

void
   reset( void );                   // Reset the Task

protected:
void
   drain( void );                   // Drain work from Task

virtual void                        // (OVERRIDE this method)
   work(                            // Operate on a work Item
     DispatchItem*     item);       // The work Item
}; // class DispatchTask

#endif // DISPATCHTASK_H_INCLUDED
