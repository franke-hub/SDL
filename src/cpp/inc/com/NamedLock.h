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
//       NamedLock.h
//
// Purpose-
//       Define the NamedLock interface.
//
// Last change date-
//       2014/01/01
//
// Usage notes-
//       Locking is a voluntary protocol used by cooperating entities.
//       Lock names have no intrinsic meaning other than the same name
//       represents the same lock.
//
//       The NamedLock interface defines the mechanisms for obtaining and
//       releasing a lock. The lock name represents the lock, and the
//       Lock::Token represents the held lock. The scope of the name is
//       the NamedLock object. Names are not shared across objects.
//
//       SHR mode indicates that the lock is shared. The resource represented
//       by the lock is available in read-only mode.
//       XCL mode indicates that the lock is exclusive. The resource
//       represented by the lock is available in read-write mode.
//
// Locking rules-
//       An exclusive mode lock request takes precedence over share mode
//       requests. Exclusive mode lock requests are granted in the order
//       received after all other lock requests for the same lock have
//       been released.
//
// Known implementations-
//       ThreadLock.h
//
//----------------------------------------------------------------------------
#ifndef NAMEDLOCK_H_INCLUDED
#define NAMEDLOCK_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       NamedLock
//
// Purpose-
//       NamedLock descriptor.
//
//----------------------------------------------------------------------------
class NamedLock  {                  // NamedLock descriptor
//----------------------------------------------------------------------------
// NamedLock::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef void*          Token;       // NamedLock Token

//----------------------------------------------------------------------------
// NamedLock::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~NamedLock( void ) = 0;          // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       NamedLock::attempt
//
// Purpose-
//       Attempt to obtain a NamedLock, returning immediately.
//
//----------------------------------------------------------------------------
public:
virtual Token                       // The lock token, or NULL
   attemptSHR(                      // Attempt to obtain a SHR lock
     const char*       name) = 0;   // For this lock name

virtual Token                       // The lock token, or NULL
   attemptXCL(                      // Attempt to obtain an XCL lock
     const char*       name) = 0;   // For this lock name

//----------------------------------------------------------------------------
//
// Method-
//       NamedLock::modify
//
// Purpose-
//       Attempt to modify the lock Mode, returning immmediately.
//
// Usage notes-
//       For downgrades (from XCL to SHR,) the function always succeeds.
//       For upgrades (from SHR to XCL,) the function succeeds only if no
//       other thread holds a SHR lock.
//
//       If a lock mode cannot be modified, the lock remains unchanged.
//
//----------------------------------------------------------------------------
public:
virtual Token                       // Replacement token, NULL if failure
   modifySHR(                       // Modify (downgrade)
     Token             token) = 0;  // This NamedLock

virtual Token                       // Replacement token, NULL if failure
   modifyXCL(                       // Modify (upgrade)
     Token             token) = 0;  // This NamedLock

//----------------------------------------------------------------------------
//
// Method-
//       NamedLock::obtain
//
// Purpose-
//       Obtain a Lock, waiting if necessary until the lock becomes available.
//
//----------------------------------------------------------------------------
public:
virtual Token                       // The lock token
   obtainSHR(                       // Obtain a SHR lock
     const char*       name) = 0;   // For this lock name

virtual Token                       // The lock token
   obtainXCL(                       // Obtain an XCL lock
     const char*       name) = 0;   // For this lock name

//----------------------------------------------------------------------------
//
// Method-
//       NamedLock::release
//
// Purpose-
//       Release a NamedLock.
//
//----------------------------------------------------------------------------
public:
virtual void
   release(                         // Release
     Token             token) = 0;  // This NamedLock
}; // class NamedLock

#endif // NAMEDLOCK_H_INCLUDED
