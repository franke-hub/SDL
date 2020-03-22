//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ThreadLock.h
//
// Purpose-
//       Define the ThreadLock object.
//
// Last change date-
//       2014/01/01
//
// Usage notes-
//       See NamedLock.h (and Software.h for obtaining a thread id.)
//
// Additional locking rules-
//       A thread may hold multiple SHR locks on the same name.
//       It is possible for a thread to deadlock with itself.
//
//       Holding an XCL lock, modify to SHR always succeeds.
//       Holding a SHR lock, modify to XCL succeeds only if there are no
//       other holders of the lock.
//
//----------------------------------------------------------------------------
#ifndef THREADLOCK_H_INCLUDED
#define THREADLOCK_H_INCLUDED

#include "com/NamedLock.h"

//----------------------------------------------------------------------------
//
// Class-
//       ThreadLock
//
// Purpose-
//       ThreadLock descriptor.
//
//----------------------------------------------------------------------------
class ThreadLock : public NamedLock  { // ThreadLock descriptor
//----------------------------------------------------------------------------
// ThreadLock::Attributes
//----------------------------------------------------------------------------
protected:
void*                  object;      // The hidden ThreadLock object

//----------------------------------------------------------------------------
// ThreadLock::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ThreadLock( void );             // Destructor
   ThreadLock( void );              // Constructor

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::debug
//
// Purpose-
//       Debugging display of internal locking tables.
//
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Debugging display

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::attempt
//
// Purpose-
//       Attempt to obtain a Lock, returning immediately.
//
//----------------------------------------------------------------------------
public:
virtual Token                       // The lock token, or NULL
   attemptSHR(                      // Attempt to obtain a SHR lock
     const char*       name);       // For this lock name

virtual Token                       // The lock token, or NULL
   attemptXCL(                      // Attempt to obtain an XCL lock
     const char*       name);       // For this lock name

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::modify
//
// Purpose-
//       Attempt to modify the lock Mode, returning immmediately.
//
//----------------------------------------------------------------------------
public:
virtual Token                       // Replacement token, NULL if failure
   modifySHR(                       // Modify (downgrade)
     Token             token);      // This ThreadLock

virtual Token                       // Replacement token, NULL if failure
   modifyXCL(                       // Modify (upgrade)
     Token             token);      // This ThreadLock

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::obtain
//
// Purpose-
//       Obtain a Lock, waiting if necessary until the lock becomes available.
//
//----------------------------------------------------------------------------
public:
virtual Token                       // The lock token
   obtainSHR(                       // Obtain a SHR lock
     const char*       name);       // For this lock name

virtual Token                       // The lock token
   obtainXCL(                       // Obtain an XCL lock
     const char*       name);       // For this lock name

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::release
//
// Purpose-
//       Release a ThreadLock.
//
//----------------------------------------------------------------------------
public:
virtual void
   release(                         // Release
     Token             token);      // This ThreadLock

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::threadAbort
//
// Purpose-
//       Release all locks held by the specified Thread.
//
//----------------------------------------------------------------------------
public:
void
   threadAbort(                     // Release all held locks
     unsigned long     tid);        // Held with this thread id

//----------------------------------------------------------------------------
//
// Method-
//       ThreadLock::threadExit
//
// Purpose-
//       Release all locks held by the current Thread.
//
//----------------------------------------------------------------------------
public:
void
   threadExit( void );              // Release all held locks
}; // class ThreadLock

#endif // THREADLOCK_H_INCLUDED
