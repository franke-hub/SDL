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
//       RecursiveBarrier.h
//
// Purpose-
//       Primitive mechanism for granting exclusive access to a resource.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef RECURSIVEBARRIER_H_INCLUDED
#define RECURSIVEBARRIER_H_INCLUDED

#ifndef ATOMIC_H_INCLUDED
#include "Atomic.h"
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define RECURSIVEBARRIER_INIT   {0} // Initial value for a Barrier

//----------------------------------------------------------------------------
//
// Struct-
//       RecursiveBarrier
//
// Purpose-
//       This is similar to Barrier, except that the same thread may obtain
//       the associated latch recursively.
//
// Implementation notes-
//       Latches should be released in the reverse order that they are obtained.
//
//       The attempt method returns 0 iff the latch was available and not
//       already held by the same thread.
//
//----------------------------------------------------------------------------
struct RecursiveBarrier {           // RecursiveBarrier descriptor
//----------------------------------------------------------------------------
// RecursiveBarrier::Attributes
//----------------------------------------------------------------------------
public:                             // Required for RECURSIVEBARRIER_INIT
ATOMICP*               barrier;     // The barrier

//----------------------------------------------------------------------------
// RecursiveBarrier::Methods
//----------------------------------------------------------------------------
public:
int                                 // Return code, 0 iff successful
   attempt( void );                 // Attempt to obtain the RecursiveBarrier latch

int                                 // Return code, 0 iff first holder
   obtain( void );                  // Obtain the RecursiveBarrier latch

void
   release( void );                 // Release the RecursiveBarrier latch

void
   reset( void );                   // Initialize the RecursiveBarrier
}; // struct RecursiveBarrier

//----------------------------------------------------------------------------
//
// Class-
//       AutoRecursiveBarrier
//
// Purpose-
//       Automatic RecursiveBarrier Object
//
// Usage-
//       Given RecursiveBarrier barrier;
//       :
//       {{{{
//         AutoRecursiveBarrier lock(barrier); // Obtain the lock
//         :                        // Any exit from scope releases RecursiveBarrier
//       }}}}
//
//----------------------------------------------------------------------------
class AutoRecursiveBarrier {        // Automatic recursive mutual exclusion object
protected:
int                    cc;          // TRUE iff lock held recursively
RecursiveBarrier&      barrier;     // The RecursiveBarrier

public:
inline
   ~AutoRecursiveBarrier( void )    // Destructor
{
   if( cc == 0 )
     barrier.release();
}

inline
   AutoRecursiveBarrier(            // Constructor
     RecursiveBarrier& barrier)     // Associated RecursiveBarrier
:  barrier(barrier), cc(0)
{
   cc= barrier.obtain();
}
}; // class AutoRecursiveBarrier

#endif // RECURSIVEBARRIER_H_INCLUDED
