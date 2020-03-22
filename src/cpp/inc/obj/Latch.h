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
//       Latch.h
//
// Purpose-
//       Primitive mechanism for granting access to a resource.
//
// Last change date-
//       2018/01/01
//
// Implementation notes-
//       Implements BasicLockable. (Works with std::lock_guard)
//
//----------------------------------------------------------------------------
#ifndef OBJ_LATCH_H_INCLUDED
#define OBJ_LATCH_H_INCLUDED

#include <thread>

#include "Object.h"                 // For std::atomic, Exception
#include "Thread.h"                 // For Thread::null_id

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
//
// Struct-
//       Latch
//
// Purpose-
//       Primitive (exclusive) latch.
//
// Implementation notes-
//       The Latch object does not and never will contain virtual methods
//       or require class construction before it is used.
//
//       The Latch is implemented as a spin latch. That is, if the Latch is
//       not immediately available the implementation loops attempting over
//       and over trying to obtain the Latch.
//
//       The reset method does not check the state of the Latch; the Latch
//       is unconditionally reset to its available state. The release method
//       MAY check the state of the Latch. It is an error to release a Latch
//       that has not been obtained.
//
//       Use SharedLatch to obtain shared access to a resource,
//       Use ExclusiveLatch to obtain exclusive access to a shared resource.
//
//----------------------------------------------------------------------------
struct Latch {                      // Latch descriptor
//----------------------------------------------------------------------------
// Latch::Attributes
//----------------------------------------------------------------------------
enum
{  RESET= 0                         // Available
,  LOCKED= -1                       // Locked
}; // (generic) enum

volatile unsigned      latch= 0;    // The physical spin latch

//----------------------------------------------------------------------------
// Latch::Methods
//----------------------------------------------------------------------------
void
   lock( void )                     // Obtain the Latch
{  // 0f/10 31.5 @ 70% [w/ browser] 35.9 @ 40% [no browser]
   for(unsigned spinCount= 1;;spinCount++)
   {
     if( try_lock() )
       return;

     if( (spinCount & 0x0000000f) == 0 )
     {
       if( (spinCount & 0x00000010) != 0 )
         std::this_thread::yield();
       else
         std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
     }
   }
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the Latch
{  std::atomic<unsigned>* const atomic_latch= (std::atomic<unsigned>*)&latch;

   unsigned oldValue= RESET;
   return atomic_latch->compare_exchange_strong(oldValue, LOCKED);
}

void
   unlock( void )                   // Release the Latch
{  std::atomic<unsigned>* const atomic_latch= (std::atomic<unsigned>*)&latch;

   atomic_latch->store(RESET);
}

void
   reset( void )                    // Initialize/Reset the Latch
{  std::atomic<unsigned>* const atomic_latch= (std::atomic<unsigned>*)&latch;

   atomic_latch->store(RESET);
}
}; // struct Latch

//----------------------------------------------------------------------------
//
// Struct-
//       RecursiveLatch
//
// Purpose-
//       Primitive recursive latch.
//
// Implementation notes-
//       The RecursiveLatch object does not and never will contain virtual
//       methods or require class construction before it is used.
//
//----------------------------------------------------------------------------
struct RecursiveLatch {             // RecursiveLatch descriptor
volatile std::thread::id
                       latch;       // The latch holder std::thread::id
unsigned               count= 0;    // Share count

//----------------------------------------------------------------------------
// RecursiveLatch::Methods
//----------------------------------------------------------------------------
void
   lock( void )                     // Obtain the Latch
{
   while( !try_lock() )
     std::this_thread::sleep_for(std::chrono::nanoseconds(8));
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain a RecursiveLatch
{  std::atomic<std::thread::id>* const atomic_latch= (std::atomic<std::thread::id>*)&latch;

   std::thread::id oldValue= atomic_latch->load();
   std::thread::id newValue= std::this_thread::get_id();
   if( oldValue != newValue )
   {
     if( !atomic_latch->compare_exchange_strong(oldValue, newValue) )
       return false;
   }

   count++;
   return true;
}

void
   unlock( void )                   // Release the RecursiveLatch
{  std::atomic<std::thread::id>* const atomic_latch= (std::atomic<std::thread::id>*)&latch;

   if( --count )                    // If not the original latch holder
     return;                        // Return, latch still held

   atomic_latch->store(Thread::null_id);
}

void                                // NOTE: This method is NOT thread-safe
   reset( void )                    // Initialize/Reset the RecursiveLatch
{  std::atomic<std::thread::id>* const atomic_latch= (std::atomic<std::thread::id>*)&latch;

   count= 0;
   atomic_latch->store(Thread::null_id);
}
}; // struct RecursiveLatch

//----------------------------------------------------------------------------
//
// Struct-
//       SharedLatch
//
// Purpose-
//       Primitive shared latch.
//
// Implementation notes-
//       The SharedLatch object does not and never will contain virtual
//       methods or require class construction before it is used.
//
//----------------------------------------------------------------------------
struct SharedLatch {                // SharedLatch descriptor
//----------------------------------------------------------------------------
// SharedLatch::Attributes
//----------------------------------------------------------------------------
volatile unsigned      count= 0;    // The number of shared users

void
   lock( void )                     // Obtain the SharedLatch
{
   while( !try_lock() )
     std::this_thread::yield();
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the latch
{  std::atomic<unsigned>* atomic_count= (std::atomic<unsigned>*)&count;

   unsigned oldValue= atomic_count->load() & 0x7fffffff;
   unsigned newValue= oldValue + 1;
   return atomic_count->compare_exchange_strong(oldValue, newValue);
}

void
   unlock( void )                   // Release the SharedLatch
{  std::atomic<unsigned>* const atomic_count= (std::atomic<unsigned>*)&count;
   atomic_count->operator--();
}

void
   reset( void )                    // Initialize/Reset the SharedLatch
{  count= 0;
}
}; // struct SharedLatch

//----------------------------------------------------------------------------
//
// Struct-
//       ExclusiveLatch
//
// Purpose-
//       Obtain exclusive access to a SharedLatch
//
// Usage-
//       SharedLatch     shared;
//       ExclusiverLatch exclusive(shared);
//
//       {{{{ std::lock_guard<decltype(exclusive)> lock(exclusive);
//         // exclusive access operations
//       }}}}
//
// Implementation notes-
//       Construction required. However, different ExclusiveLatch instances
//       referencing the same SharedLock instance operate properly.
//
//----------------------------------------------------------------------------
struct ExclusiveLatch {             // ExclusiveLatch descriptor
SharedLatch&           shared;      // The associated SharedLatch

   ExclusiveLatch(
     SharedLatch&      source)
:  shared(source) {}

void
   lock( void )                     // Obtain the ExclusiveLatch
{
   while( !try_lock() )
     std::this_thread::yield();
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the ExclusivedLatch
{  std::atomic<unsigned>* atomic_count= (std::atomic<unsigned>*)&shared.count;

   unsigned oldValue= 0;
   unsigned newValue= 0x80000000;
   return atomic_count->compare_exchange_strong(oldValue, newValue);
}

void
   unlock( void )                   // Release the ExclusivedLatch
{  shared.count= 0; }
}; // struct ExclusiveLatch
}  // namespace _OBJ_NAMESPACE

#endif // OBJ_LATCH_H_INCLUDED
