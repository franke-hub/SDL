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
//       All Latch instances implement Lockable and interoperate with
//       std::lock_guard. Latch does not inherit from Object.
//
//       The SharedLatch share counter is an unsigned 31 bit integer.
//       Counter overflow temporarily prevents additional sharing.
//
//----------------------------------------------------------------------------
#ifndef _PUB_LATCH_H_INCLUDED
#define _PUB_LATCH_H_INCLUDED

#include <atomic>                   // For std::atomic

#include "config.h"                 // For _PUB_NAMESPACE, ...
#include "Exception.h"              // For Exception
#include "Thread.h"                 // For std::thread::id, Thread::null_id

namespace _PUB_NAMESPACE {
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
   unlock( void )                   // Release the Latch
{  std::atomic<unsigned>* const atomic_latch= (std::atomic<unsigned>*)&latch;

   atomic_latch->store(RESET);
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the Latch
{  std::atomic<unsigned>* const atomic_latch= (std::atomic<unsigned>*)&latch;

   unsigned oldValue= RESET;
   return atomic_latch->compare_exchange_strong(oldValue, LOCKED);
}

void
   reset( void )                    // Initialize/Reset the Latch
{  std::atomic<unsigned>* const atomic_latch= (std::atomic<unsigned>*)&latch;

   atomic_latch->store(RESET);
}

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
}; // struct Latch

//----------------------------------------------------------------------------
//
// Struct-
//       NullLatch
//
// Purpose-
//       Define a Latch that does nothing (for single-threaded use.)
//
//----------------------------------------------------------------------------
struct NullLatch {                  // NullLatch descriptor
//----------------------------------------------------------------------------
// NullLatch::Methods
//----------------------------------------------------------------------------
void
   unlock( void ) {}                // Release the NullLatch

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the NullLatch
{  return true;                     // (Always succeeds)
}

void
   reset( void ) {}                 // Initialize/Reset the NullLatch

void
   lock( void ) {}                  // Obtain the NullLatch
}; // struct NullLatch

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
   unlock( void )                   // Release the RecursiveLatch
{
   if( --count )                    // If not the original latch holder
     return;                        // Return, latch still held

   std::atomic<std::thread::id>* const atomic_latch= (std::atomic<std::thread::id>*)&latch;
   atomic_latch->store(std::thread::id(0));
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain a RecursiveLatch
{
   std::atomic<std::thread::id>* const atomic_latch= (std::atomic<std::thread::id>*)&latch;
   std::thread::id oldValue= atomic_latch->load();
   std::thread::id newValue= std::this_thread::get_id();
   if( oldValue != newValue )
   {
     oldValue= Thread::null_id;
     if( !atomic_latch->compare_exchange_strong(oldValue, newValue) )
       return false;
   }

   count++;
   return true;
}

void                                // NOTE: This method is NOT thread-safe
   reset( void )                    // Initialize/Reset the RecursiveLatch
{  std::atomic<std::thread::id>* const atomic_latch= (std::atomic<std::thread::id>*)&latch;

   count= 0;
   atomic_latch->store(std::thread::id(0));
}

void
   lock( void )                     // Obtain the Latch
{
   while( !try_lock() )
     std::this_thread::sleep_for(std::chrono::nanoseconds(8));
}
}; // struct RecursiveLatch

//----------------------------------------------------------------------------
//
// Struct-
//       NonRecursiveLatch
//
// Purpose-
//       Primitive non-recursive latch.
//
// Implementation notes-
//       THe NonRecursiveLatch object is a Latch that explicitly does not
//       allow recursion. An Exception is thrown if a thread attempts to
//       obtain a NonRecursiveLatch while holding it. Should this occur,
//       THE LATCH IS RELEASED before the Exception is thrown.
//
//----------------------------------------------------------------------------
struct NonRecursiveLatch {          // NonRecursiveLatch descriptor
volatile std::thread::id
                       latch;       // The latch holder std::thread::id

//----------------------------------------------------------------------------
// NonRecursiveLatch::Methods
//----------------------------------------------------------------------------
void
   unlock( void )                   // Release the NonRecursiveLatch
{  std::atomic<std::thread::id>* const atomic_latch= (std::atomic<std::thread::id>*)&latch;

   atomic_latch->store(std::thread::id(0));
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain a NonRecursiveLatch
{  std::atomic<std::thread::id>* const atomic_latch= (std::atomic<std::thread::id>*)&latch;

   std::thread::id oldValue= atomic_latch->load();
   std::thread::id newValue= std::this_thread::get_id();
   if( oldValue == newValue )       // If recursive
   {
     atomic_latch->store(std::thread::id(0));
     throw Exception("NonRecursiveLatch");
   }

   return atomic_latch->compare_exchange_strong(oldValue, newValue);
}

void                                // NOTE: This method is NOT thread-safe
   reset( void )                    // Initialize/Reset the NonRecursiveLatch
{  std::atomic<std::thread::id>* const atomic_latch= (std::atomic<std::thread::id>*)&latch;

   atomic_latch->store(std::thread::id(0));
}

void
   lock( void )                     // Obtain the Latch
{
   while( !try_lock() )
     std::this_thread::sleep_for(std::chrono::nanoseconds(8));
}
}; // struct NonRecursiveLatch

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

//----------------------------------------------------------------------------
// SharedLatch::Methods
//----------------------------------------------------------------------------
void
   unlock( void )                   // Release the SharedLatch
{  std::atomic<unsigned>* const atomic_count= (std::atomic<unsigned>*)&count;

   atomic_count->operator--();
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the latch
{  std::atomic<unsigned>* atomic_count= (std::atomic<unsigned>*)&count;

   unsigned oldValue= atomic_count->load() & 0x7fffffff;
   unsigned newValue= oldValue + 1;
   return atomic_count->compare_exchange_strong(oldValue, newValue);
}

void
   reset( void )                    // Initialize/Reset the SharedLatch
{  count= 0;
}

void
   lock( void )                     // Obtain the SharedLatch
{
   while( !try_lock() )
     std::this_thread::yield();
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

//----------------------------------------------------------------------------
// ExclusiveLatch::Constructor
//----------------------------------------------------------------------------
   ExclusiveLatch(
     SharedLatch&      source)
:  shared(source) {}

//----------------------------------------------------------------------------
// ExclusiveLatch::Methods
//----------------------------------------------------------------------------
void
   unlock( void )                   // Release the ExclusivedLatch
{  std::atomic<unsigned>* atomic_count= (std::atomic<unsigned>*)&shared.count;

   atomic_count->store(0);
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the ExclusivedLatch
{  std::atomic<unsigned>* atomic_count= (std::atomic<unsigned>*)&shared.count;

   unsigned oldValue= 0;
   unsigned newValue= 0x80000000;
   return atomic_count->compare_exchange_strong(oldValue, newValue);
}

void
   lock( void )                     // Obtain the ExclusiveLatch
{  std::atomic<unsigned>* atomic_count= (std::atomic<unsigned>*)&shared.count;

   // First, reserve the Latch for exclusive use
   unsigned oldValue= atomic_count->load() & 0x7fffffff;
   unsigned newValue= oldValue | 0x80000000;
   for(unsigned spinCount= 1;;spinCount++)
   {
     if( atomic_count->compare_exchange_weak(oldValue, newValue) )
        break;

     oldValue &= 0x7fffffff;
     newValue= oldValue | 0x80000000;

     if( spinCount & 0x00000007 )
       std::this_thread::yield();
     else
       std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
   }

   // We have the exclusive reservation. Wait for shares to unlock.
   for(unsigned spinCount= 1; atomic_count->load() != 0x80000000; spinCount++)
   {
     if( spinCount & 0x00000007 )
       std::this_thread::yield();
     else
       std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
   }
}
}; // struct ExclusiveLatch
}  // namespace _PUB_NAMESPACE
#endif // _PUB_LATCH_H_INCLUDED
