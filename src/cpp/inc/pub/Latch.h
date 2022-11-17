//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2022 Frank Eskesen.
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
//       Primitive mechanisms for granting access to a resource.
//
// Last change date-
//       2022/11/11
//
// Implementation notes-
//       Internal logic for these mechanisms are further described in
//         "~/src/cpp/lib/pub/.LOGICS.md".
//
//       All Latch instances implement *Lockable* and interoperate with the
//       STL (Standard Template Library.) They also implement the reset method,
//       which unconditionally resets the Latch to its initial available state.
//
//       Latch objects do not inherit from Object. They do not and never will
//       contain virtual methods. Only the XCL_latch requires construction.
//       All other latch types may be used within constructors.
//
//       Refer to the implemenation notes for each latch type.
//         Latch           Primitive exclusive latch
//         RecursiveLatch  Primitive recursive exclusive latch
//         SHR_latch       The shared part of a shared/exclusive latch pair
//         XCL_latch       The exclusive reference to a SHR_latch
//         NullLatch       A latch that does nothing
//         TestLatch       Primitive exclusive latch that disallows recursion
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_LATCH_H_INCLUDED
#define _LIBPUB_LATCH_H_INCLUDED

#include <atomic>                   // For std::atomic
#include <stdexcept>                // For std::runtime_error
#include <thread>                   // For std::thread::id
#include <stdint.h>                 // For uint32_t

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Struct-
//       Latch
//
// Purpose-
//       Primitive (exclusive) spin latch.
//
// Implementation notes-
//       We improve error checking by using the thread id as the latch.
//
//----------------------------------------------------------------------------
struct Latch {                      // Latch descriptor
//----------------------------------------------------------------------------
// Latch::Attributes
//----------------------------------------------------------------------------
std::atomic<std::thread::id>
                       latch{std::thread::id()}; // The spin latch

//----------------------------------------------------------------------------
// Latch::Methods
//----------------------------------------------------------------------------
void
   lock( void )                     // Obtain the Latch
{
   for(uint32_t spinCount= 1;;spinCount++) {
     if( try_lock() )
       return;

     if( (spinCount & 0x0000000f) == 0 ) {
       if( (spinCount & 0x00000010) != 0 )
         std::this_thread::yield();
       else
         std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
     }
   }
}

void
   reset( void )                    // Initialize/Reset the Latch
{  latch.store(std::thread::id()); }

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the Latch
{  std::thread::id oldValue= std::thread::id();
   std::thread::id newValue= std::this_thread::get_id();
   return latch.compare_exchange_strong(oldValue, newValue);
}

void
   unlock( void )                   // Release the Latch
{
   // Verify that the current thread holds the Latch
   if( latch.load() != std::this_thread::get_id() )
     throw std::runtime_error("Latch unlock error");

   latch.store(std::thread::id()); // Release the Latch
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
//----------------------------------------------------------------------------
struct RecursiveLatch {             // RecursiveLatch descriptor
std::atomic<std::thread::id>
                       latch{std::thread::id()}; // The RecursiveLatch Thread
uintptr_t              count{};     // Share count

//----------------------------------------------------------------------------
// RecursiveLatch::Methods
//----------------------------------------------------------------------------
void
   lock( void )                     // Obtain the Latch
{
   for(uint32_t spinCount= 1;;spinCount++) {
     if( try_lock() )
       return;

     if( (spinCount & 0x0000000f) == 0 ) {
       if( (spinCount & 0x00000010) != 0 )
         std::this_thread::yield();
       else
         std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
     }
   }
}

void
   reset( void )                    // Initialize/Reset the RecursiveLatch
{
   count= 0;
   latch.store(std::thread::id());
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain a RecursiveLatch
{
   std::thread::id oldValue= latch.load();
   std::thread::id newValue= std::this_thread::get_id();
   if( oldValue != newValue ) {
     oldValue= std::thread::id();
     if( !latch.compare_exchange_strong(oldValue, newValue) )
       return false;
   }

   count++;
   return true;
}

void
   unlock( void )                   // Release the RecursiveLatch
{
   // Verify that the current thread holds the RecursiveLatch
   if( latch.load() != std::this_thread::get_id() )
     throw std::runtime_error("RecursiveLatch unlock error");

   // We have the latch (so we own both the count and the latch)
   --count;                         // Decrement the recursion count
   if( count == 0 )                 // If we're releasing the latch
     latch.store(std::thread::id());
}
}; // struct RecursiveLatch

//----------------------------------------------------------------------------
//
// Struct-
//       SHR_latch
//       XCL_latch
//
// Purpose-
//       Primitive shared/exclusive latch, held shared.
//       Primitive shared/exclusive latch, held exclusively.
//
// Usage-
//       SHR_latch shr;             // (Used for shared access)
//       XCL_latch xcl(shr);        // (Used for exclusive access)
//
//       In multiple threads:
//       {{{{ std::lock_guard<decltype(shr)> s_lock(shr);
//         // shared access to protected resources
//       }}}}
//
//       In another thread:
//       {{{{ std::lock_guard<decltype(xcl)> x_lock(xcl);
//         // exclusive access to protected resources
//       }}}}
//
// Implementation notes-
//       A Thread may hold either a SHR_latch or an XCL_latch, but not both.
//       The implementation deadlocks during an attempt to hold both latches.
//
//============================================================================
static_assert( sizeof(uintptr_t) == 8 || sizeof(uintptr_t) == 4
             , "Unexpected sizeof(uintptr_t) [code update required]" );

struct SHR_latch {                  // SHR_latch descriptor
//----------------------------------------------------------------------------
// SHR_latch::Attributes
//----------------------------------------------------------------------------
std::atomic<uintptr_t> count{};     // The number of shared users

//----------------------------------------------------------------------------
// SHR_latch::Methods
//----------------------------------------------------------------------------
void
   lock( void )                     // Obtain the SHR_latch
{
   while( !try_lock() )
     std::this_thread::yield();
}

void
   reset( void )                    // Initialize/Reset the SHR_latch
{  count.store(0); }

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the latch
{
   static constexpr const uintptr_t
       HBIT= sizeof(uintptr_t) == 8 ? 0x8000000000000000L : 0x80000000;

   uintptr_t oldValue= count.load();
   if( oldValue & HBIT )            // (Disallow SHR_latch if XCL reservation)
     return false;

   uintptr_t newValue= oldValue + 1;
   return count.compare_exchange_strong(oldValue, newValue);
}

void
   unlock( void )                   // Release the SHR_latch
{
   static constexpr const uintptr_t
       HBIT= sizeof(uintptr_t) == 8 ? 0x8000000000000000L : 0x80000000;

   // Unlock, detecting unlock when not locked errors
   uintptr_t oldValue= count.load();
   for(;;) {
     if( oldValue == 0 || oldValue == HBIT ) // If unlock when not locked
       throw std::runtime_error("SHR_latch unlock error");

     uintptr_t newValue= oldValue - 1;
     if( count.compare_exchange_strong(oldValue, newValue) )
       return;
   }
}
}; // struct SHR_latch

//============================================================================
struct XCL_latch {                  // XCL_latch descriptor
SHR_latch&             share;       // The associated SHR_latch
std::thread::id        thread{std::thread::id()}; // The owning XCL thread

//----------------------------------------------------------------------------
// XCL_latch::Constructor
//----------------------------------------------------------------------------
   XCL_latch(
     SHR_latch&        source)
:  share(source) {}

//----------------------------------------------------------------------------
// XCL_latch::Methods
//----------------------------------------------------------------------------
/***
   Downgrade from exclusive to shared mode.

   Preconditions:
     The exclusive latch *MUST* be held by the currently running Thread.
***/
void
   downgrade( void )                // Downgrade XCL_latch to SHR_latch
{
   static constexpr const uintptr_t
       HBIT= sizeof(uintptr_t) == 8 ? 0x8000'0000'0000'0000L : 0x8000'0000;

   if( thread != std::this_thread::get_id()
       || share.count.load() != HBIT )
     throw std::runtime_error("XCL_latch downgrade error");

   thread= std::thread::id();
   share.count.store(1);
}

void
   lock( void )                     // Obtain the XCL_latch
{
   for(uint32_t spinCount= 1;;spinCount++) {
     if( try_lock() )
        break;

     if( spinCount & 0x00000007 )
       std::this_thread::yield();
     else
       std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
   }
}

void
   reset( void )                    // Reset the XCL_latch
{
   thread= std::thread::id();
   share.reset();
}

void
   unlock( void )                   // Release the XCL_latch
{
   if( thread != std::this_thread::get_id() )
     throw std::runtime_error("XCL_latch unlock error");

   thread= std::thread::id();
   share.count.store(0);
}

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the XCL_latch
{
   static constexpr const uintptr_t
       HBIT= sizeof(uintptr_t) == 8 ? 0x8000'0000'0000'0000L : 0x8000'0000;

   // Reserve the Latch for exclusive use
   uintptr_t oldValue= share.count.load();
   for(;;) {
     if( oldValue & HBIT )          // If already reserved
       return false;                // (Only one reservation allowed)

     uintptr_t newValue= oldValue | HBIT;
     if( share.count.compare_exchange_strong(oldValue, newValue) )
       break;
   }

   thread= std::this_thread::get_id(); // We have the reservation

   // Wait for all shares to unlock.
   for(uint32_t spinCount= 1; oldValue != HBIT; spinCount++) {
     if( spinCount & 0x00000007 )
       std::this_thread::yield();
     else
       std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
     oldValue= share.count.load();
   }

   return true;
}
}; // struct XCL_latch

//----------------------------------------------------------------------------
//
// Struct-
//       NullLatch
//
// Purpose-
//       Define a (limited purpose) Latch that does nothing.
//
// Implementation notes-
//       In code that may be conditionally compiled for single-threaded or
//       multi-threaded operation, a NullLatch can be used instead of a
//       Latch when compiled in single-threaded mode.
//
//----------------------------------------------------------------------------
struct NullLatch {                  // NullLatch descriptor
//----------------------------------------------------------------------------
// NullLatch::Methods
//----------------------------------------------------------------------------
void
   lock( void ) {}                  // Obtain the NullLatch

void
   reset( void ) {}                 // Initialize/Reset the NullLatch

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the NullLatch
{  return true; }

void
   unlock( void ) {}                // Release the NullLatch
}; // struct NullLatch

//----------------------------------------------------------------------------
//
// Struct-
//       TestLatch
//
// Purpose-
//       Primitive non-recursive debugging latch.
//
// Implementation notes-
//       The TestLatch object is a special purpose (debugging) Latch that
//       explicitly disallows recursion, thus detecting Latch self-deadlocks.
//       If a thread attempts to obtain a TestLatch while holding it, a
//       std::runtime_error is thrown. *THE LATCH IS RELEASED* before the
//       exception is thrown.
//
//----------------------------------------------------------------------------
struct TestLatch {                  // TestLatch descriptor
std::atomic<std::thread::id>
                       latch;       // The latch holder

//----------------------------------------------------------------------------
// TestLatch::Methods
//----------------------------------------------------------------------------
void
   lock( void )                     // Obtain the Latch
{
   while( !try_lock() )
     std::this_thread::sleep_for(std::chrono::nanoseconds(8));
}

void                                // NOTE: This method is NOT thread-safe
   reset( void )                    // Initialize/Reset the TestLatch
{  latch.store(std::thread::id()); }

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the TestLatch
{
   std::thread::id oldValue= latch.load();
   std::thread::id newValue= std::this_thread::get_id();
   if( oldValue == newValue ) {     // If recursive
     latch.store(std::thread::id());
     throw std::runtime_error("TestLatch recursion error");
   }

   return latch.compare_exchange_strong(oldValue, newValue);
}

void
   unlock( void )                   // Release the TestLatch
{  latch.store(std::thread::id()); }
}; // struct TestLatch
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_LATCH_H_INCLUDED
