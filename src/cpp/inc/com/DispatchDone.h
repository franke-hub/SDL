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
//       DispatchDone.h
//
// Purpose-
//       Standard Dispatch Done callback objects. (Included from Dispatch.h)
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef DISPATCHDONE_H_INCLUDED
#define DISPATCHDONE_H_INCLUDED

#include <com/Status.h>             // For class DispatchWait

#ifndef DISPATCH_H_INCLUDED
#include "Dispatch.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       DispatchDone
//
// Purpose-
//       The Dispatcher Done callback Object
//
//----------------------------------------------------------------------------
class DispatchDone {                // The dispatcher Done callback Object
//----------------------------------------------------------------------------
// DispatchDone::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DispatchDone( void );           // Destructor
   DispatchDone( void );            // Constructor

private:                            // Bitwise copy is prohibited
   DispatchDone(const DispatchDone&); // Disallowed copy constructor
   DispatchDone& operator=(const DispatchDone&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// DispatchDone::Methods
//----------------------------------------------------------------------------
public:
virtual void                        // OVERRIDE this method
   done(                            // Work Item completion
     DispatchItem*     item) = 0;   // With this Item
}; // class DispatchDone

//----------------------------------------------------------------------------
//
// Class-
//       DispatchWait
//
// Purpose-
//       The Dispatcher wait until Done Object
//
// Notes-
//       This Object can be used for a single work Item. It cannot be shared,
//       but it can be reused by calling the reset method after the wait
//       has been satisfied.
//
//----------------------------------------------------------------------------
class DispatchWait : public DispatchDone { // The dispatcher Done wait Object
//----------------------------------------------------------------------------
// DispatchWait::Attributes
//----------------------------------------------------------------------------
private:
Status                 status;      // For wait/post

//----------------------------------------------------------------------------
// DispatchWait::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~DispatchWait( void );           // Destructor
   DispatchWait( void );            // Constructor

private:                            // Bitwise copy is prohibited
   DispatchWait(const DispatchWait&); // Disallowed copy constructor
   DispatchWait& operator=(const DispatchWait&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// DispatchWait::Methods
//----------------------------------------------------------------------------
public:
virtual void                        // Invokes status.post()
   done(                            // Work Item completion
     DispatchItem*     item);       // With this Item

void
   reset( void );                   // Reset for re-use

int                                 // The completion code
   wait( void );                    // Wait for Item completion
}; // class DispatchWait

#endif // DISPATCHDONE_H_INCLUDED
