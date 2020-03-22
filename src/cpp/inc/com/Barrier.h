//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Barrier.h
//
// Purpose-
//       Primitive mechanism for granting exclusive access to a resource.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef BARRIER_H_INCLUDED
#define BARRIER_H_INCLUDED

#ifndef ATOMIC_H_INCLUDED
#include "Atomic.h"                 // For ATOMIC32 typedef
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define BARRIER_INIT            {0} // Initial value for a Barrier

//----------------------------------------------------------------------------
//
// Struct-
//       Barrier
//
// Purpose-
//       Primitive exclusive latch.
//
// Implementation notes-
//       This is implemented as a structure to allow static initialization,
//       i.e. static Barrier barrier= BARRIER_INIT. This allows static Barrier
//       objects to be used safely within constructors.
//
//       The Barrier object does not and never will contain virtual methods
//       or require class construction before it is used.
//
//       The Barrier is implemented as a spin latch. That is, if the latch is
//       not immediately available the implementation spins attempting over and
//       over trying to obtain the latch.
//
//       The reset method does not check the state of the latch; the latch
//       is unconditionally reset to its available state. The release method
//       MAY check the state of the latch. It is an error to release a latch
//       that has not been obtained.
//
//----------------------------------------------------------------------------
struct Barrier {                    // Barrier descriptor
//----------------------------------------------------------------------------
// Barrier::Attributes
//----------------------------------------------------------------------------
public:                             // Required for BARRIER_INIT
ATOMIC32               barrier;     // The barrier

//----------------------------------------------------------------------------
// Barrier::Methods
//----------------------------------------------------------------------------
public:
int                                 // Return code, 0 iff successful
   attempt( void );                 // Attempt to obtain the Barrier latch

void
   obtain( void );                  // Obtain the Barrier latch

void
   release( void );                 // Release the Barrier latch

void
   reset( void );                   // Initialize the Barrier
}; // struct Barrier

//----------------------------------------------------------------------------
//
// Class-
//       AutoBarrier
//
// Purpose-
//       Automatic Barrier Object
//
// Usage-
//       Given Barrier barrier;
//       :
//       {{{{
//         AutoBarrier lock(barrier); // Obtain the lock, released by destructor
//         :                        // Any exit from scope releases the Barrier
//       }}}}
//
//----------------------------------------------------------------------------
class AutoBarrier {                 // Automatic mutual exclusion object
protected:
Barrier&               barrier;     // The Barrier

public:
inline
   ~AutoBarrier( void )             // Destructor
{
   barrier.release();
}

inline
   AutoBarrier(                     // Constructor
     Barrier&          barrier)     // Associated Barrier
:  barrier(barrier)
{
   barrier.obtain();
}
}; // class AutoBarrier

#endif // BARRIER_H_INCLUDED
