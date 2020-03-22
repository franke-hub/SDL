//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       NoisyLockable.h
//
// Purpose-
//       BasicLockable object with debugf statements.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       Implements BasicLockable. (Works with std::lock_guard)
//
//----------------------------------------------------------------------------
#ifndef OBJ_NOISYLOCKABLE_H_INCLUDED
#define OBJ_NOISYLOCKABLE_H_INCLUDED

#include "com/Debug.h"              // For debugf()
#include "obj/Latch.h"              // For SharedLatch

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
//
// Struct-
//       template<class T> NoisyLockable_t
//
// Purpose-
//       Wrapper for BasicLockable object with debug statements.
//
//----------------------------------------------------------------------------
template<class T>
struct NoisyLockable_t {            // Noisy Lockable
//----------------------------------------------------------------------------
// NoisyLockable_t::Attributes
//----------------------------------------------------------------------------
T&                     lockable;    // The BasicLockable class

//----------------------------------------------------------------------------
// NoisyLockable_t::Methods
//----------------------------------------------------------------------------
void
   method(                          // Method call
     const char*       name)        // Method name
{  debugf("NoisyLockable_t(%p)::%s()\n", this, name); }

   ~NoisyLockable_t( void )         // Destructor
{  method("~NoisyLockable_t"); }

   NoisyLockable_t(                 // Constructor
     T&                item)        // The Lockable item
:  lockable(item)
{  method("NoisyLockable_t"); }

void
   lock( void )                     // Obtain the Latch
{  method("lock");
   lockable.lock();
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the Latch
{  method("try_lock");
   return lockable.try_lock();
}

void
   unlock( void )                   // Release the Latch
{  method("unlock");
   lockable.unlock();
}
}; // struct NoisyLockable_t

typedef NoisyLockable_t<SharedLatch>
                       NoisyLockable; // The default NoisyLockable
}  // namespace _OBJ_NAMESPACE

#endif // OBJ_NOISYLOCKABLE_H_INCLUDED
