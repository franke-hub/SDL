//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2025 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Latch.h
//
// Purpose-
//       Primitive mechanisms for granting access to a resource.
//
// Last change date-
//       2025/10/25
//
// Implementation notes-
//       All Latch methods are duplicated in ~/src/cpp/lib/pub/Latch.cpp.
//       Latch.cpp methods are only used when _PUBLIB_LATCH_DEBUG is defined.
//
//       Internal logic for these mechanisms are further described in
//         "~/src/cpp/lib/pub/.LOGICS.md".
//
//       All Latch instances implement *Lockable* and interoperate with the
//       STL (Standard Template Library.) They also implement the reset method,
//       which unconditionally resets the Latch to its initial available state.
//
//       Latch objects do not inherit from Object. They do not and never will
//       contain virtual methods. Only the XCL_latch requires construction.
//       All other latch types may be used during static initialization.
//
//       Refer to the implemenation notes for each latch type.
//         Basic_latch     Primitive exclusive latch (no Thread checking)
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
#include <cstdint>                  // For uint32_t

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// MACRO _IF_PUBLIB_LATCH_INLINE, controlled by _PUBLIB_LATCH_DEBUG
//----------------------------------------------------------------------------
#ifndef   _PUBLIB_LATCH_DEBUG
#  define _PUBLIB_LATCH_DEBUG       // (Last for OUTLINE compilation)
#  undef  _PUBLIB_LATCH_DEBUG       // (Last for INLINE compilation)
#endif

#ifndef _PUBLIB_LATCH_DEBUG
#  define _IF_PUBLIB_LATCH_INLINE(x) x
#else
#  define _IF_PUBLIB_LATCH_INLINE(x) ;
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       Basic_latch
//
// Purpose-
//       Primitive (exclusive) spin latch.
//
// Implementation notes-
//       (Explicitly) does not check thread::id in lock or unlock.
//
//----------------------------------------------------------------------------
struct Basic_latch {                // Latch descriptor
//----------------------------------------------------------------------------
// Basic_latch::Attributes
//----------------------------------------------------------------------------
typedef size_t         latch_t;     // The Basic_latch spin latch type

std::atomic<latch_t>   latch{0};    // The BASIC spin latch

//----------------------------------------------------------------------------
// Basic_latch::Methods
//----------------------------------------------------------------------------
bool                                // TRUE if latch is held
   is_held( void ) const            // Is Latch held?
_IF_PUBLIB_LATCH_INLINE(
{  return latch.load() != 0; })

void
   lock( void )                     // Obtain the Basic_latch
_IF_PUBLIB_LATCH_INLINE(
{
   for(uint32_t spinCount= 0;;++spinCount) {
     if( try_lock() )
       return;

     if( spinCount & 0x00000008 ) {
       std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
       if( spinCount >= 0x00008000 )
         spinCount <<= 1;
     } else {
       std::this_thread::yield();
     }
   }
})

void
   reset( void )                    // Initialize/Reset the Basic_latch
_IF_PUBLIB_LATCH_INLINE(
{  latch.store(0); })               // Note: Unchecked

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the Basic_latch
_IF_PUBLIB_LATCH_INLINE(
{  latch_t oldValue= 0;
   latch_t newValue= 1;
   return latch.compare_exchange_strong(oldValue, newValue);
})

void
   unlock( void )                   // Release the Basic_latch
_IF_PUBLIB_LATCH_INLINE(
{
   // Verify that the latch is held
   if( latch.load() == 0 )
     throw std::runtime_error("Basic_latch unlock error");

   latch.store(0);                  // Release the Basic_latch
})
}; // struct Basic_latch

//----------------------------------------------------------------------------
//
// Struct-
//       Latch
//
// Purpose-
//       Primitive (exclusive) spin latch.
//
// Implementation notes-
//       Error checks: (std::runtime_error thrown if detected)
//       - In try_lock, the Latch must NOT be held by this Thread. If this
//         check fails, the Latch is released before an exception is thrown.
//       - In unlock, the Latch must be held by this Thread.
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
bool                                // TRUE if latch is held
   is_held( void ) const            // Is Latch held?
_IF_PUBLIB_LATCH_INLINE(
{  return latch.load() != std::thread::id(); })

void
   lock( void )                     // Obtain the Latch
_IF_PUBLIB_LATCH_INLINE(
{
   for(uint32_t spinCount= 1;;spinCount++) {
     if( try_lock() )
       return;

     if( (spinCount & 0x0000000f) == 0 ) {
       if( (spinCount & 0x00000010) != 0 )
         std::this_thread::yield();
       else {
         std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
         if( spinCount > 10'000 )   // Maximum delay: 10 microseconds
           spinCount= 0;
       }
     }
   }
})

void
   reset( void )                    // Initialize/Reset the Latch
_IF_PUBLIB_LATCH_INLINE(
{  latch.store(std::thread::id()); })

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the Latch
_IF_PUBLIB_LATCH_INLINE(
{
   std::thread::id oldValue= std::thread::id();
   std::thread::id newValue= std::this_thread::get_id();
   if( latch.load() == newValue ) { // If already held
     latch.store(std::thread::id());
     throw std::runtime_error("Latch recursion error");
   }

   return latch.compare_exchange_strong(oldValue, newValue);
})

void
   unlock( void )                   // Release the Latch
_IF_PUBLIB_LATCH_INLINE(
{
   // Verify that the current thread holds the Latch
   if( latch.load() != std::this_thread::get_id() )
     throw std::runtime_error("Latch unlock error");

   latch.store(std::thread::id()); // Release the Latch
})
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
bool                                // TRUE if latch is held
   is_held( void ) const            // Is Latch held?
_IF_PUBLIB_LATCH_INLINE(
{  return latch.load() != std::thread::id(); })

void
   lock( void )                     // Obtain the Latch
_IF_PUBLIB_LATCH_INLINE(
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
})

void
   reset( void )                    // Initialize/Reset the RecursiveLatch
_IF_PUBLIB_LATCH_INLINE(
{
   count= 0;
   latch.store(std::thread::id());
})

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain a RecursiveLatch
_IF_PUBLIB_LATCH_INLINE(
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
})

void
   unlock( void )                   // Release the RecursiveLatch
_IF_PUBLIB_LATCH_INLINE(
{
   // Verify that the current thread holds the RecursiveLatch
   if( latch.load() != std::this_thread::get_id() )
     throw std::runtime_error("RecursiveLatch unlock error");

   // We have the latch (so we own both the count and the latch)
   --count;                         // Decrement the recursion count
   if( count == 0 )                 // If we're releasing the latch
     latch.store(std::thread::id());
})
}; // struct RecursiveLatch

//----------------------------------------------------------------------------
//
// Struct-
//       SHR_latch
//
// Purpose-
//       Primitive shared/exclusive latch, held shared.
//
// Implementation notes-
//       A Thread may hold either a SHR_latch or an XCL_latch, but not both.
//       The implementation deadlocks during an attempt to hold both latches.
//
//----------------------------------------------------------------------------
static_assert( sizeof(uintptr_t) == 8 || sizeof(uintptr_t) == 4
             , "Unexpected sizeof(uintptr_t) [code update required]" );

struct SHR_latch {                  // SHR_latch descriptor
//----------------------------------------------------------------------------
// SHR_latch::Attributes
//----------------------------------------------------------------------------
std::atomic<uintptr_t> count{};     // The number of shared users

static constexpr const uintptr_t
       HBIT= sizeof(uintptr_t) == 8 ? 0x8000'0000'0000'0000L : 0x8000'0000;

//----------------------------------------------------------------------------
// SHR_latch::Methods
//----------------------------------------------------------------------------
bool                                // TRUE if latch is held
   is_held( void ) const            // Is Latch held? (shared or exclusive)
_IF_PUBLIB_LATCH_INLINE(
{  return count.load() != 0; })

void
   lock( void )                     // Obtain the SHR_latch
_IF_PUBLIB_LATCH_INLINE(
{
   while( !try_lock() )
     std::this_thread::yield();
})

void
   reset( void )                    // Initialize/Reset the SHR_latch
_IF_PUBLIB_LATCH_INLINE(
{  count.store(0); })

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the latch
_IF_PUBLIB_LATCH_INLINE(
{
   uintptr_t oldValue= count.load();
   if( oldValue & HBIT )            // (Disallow SHR_latch if XCL reservation)
     return false;

   uintptr_t newValue= oldValue + 1;
   return count.compare_exchange_strong(oldValue, newValue);
})

void
   unlock( void )                   // Release the SHR_latch
_IF_PUBLIB_LATCH_INLINE(
{
   // Unlock, detecting unlock when not locked errors
   uintptr_t oldValue= count.load();
   for(;;) {
     if( oldValue == 0 || oldValue == HBIT ) // If unlock when not locked
       throw std::runtime_error("SHR_latch unlock error");

     uintptr_t newValue= oldValue - 1;
     if( count.compare_exchange_strong(oldValue, newValue) )
       return;
   }
})
}; // struct SHR_latch

//----------------------------------------------------------------------------
//
// Struct-
//       XCL_latch
//
// Purpose-
//       Primitive shared/exclusive latch, held exclusively.
//
// Implementation notes-
//       A Thread may hold either a SHR_latch or an XCL_latch, but not both.
//       The implementation deadlocks during an attempt to hold both latches.
//
//----------------------------------------------------------------------------
struct XCL_latch {                  // XCL_latch descriptor
SHR_latch&             share;       // The associated SHR_latch
std::thread::id        thread{std::thread::id()}; // The owning XCL thread

static constexpr const uintptr_t
       HBIT= sizeof(uintptr_t) == 8 ? 0x8000'0000'0000'0000L : 0x8000'0000;

//----------------------------------------------------------------------------
// XCL_latch::Constructor
//----------------------------------------------------------------------------
   XCL_latch(
     SHR_latch&        source)
_IF_PUBLIB_LATCH_INLINE(
:  share(source) {})

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
_IF_PUBLIB_LATCH_INLINE(
{
   if( thread != std::this_thread::get_id()
       || share.count.load() != HBIT )
     throw std::runtime_error("XCL_latch downgrade error");

   thread= std::thread::id();
   share.count.store(1);
})

bool                                // TRUE if latch is held
   is_held( void ) const            // Is latch (exclusively) held
_IF_PUBLIB_LATCH_INLINE(
{  return share.count.load() & HBIT; })

void
   lock( void )                     // Obtain the XCL_latch
_IF_PUBLIB_LATCH_INLINE(
{
   for(uint32_t spinCount= 1;;spinCount++) {
     if( try_lock() )
        break;

     if( spinCount & 0x00000007 )
       std::this_thread::yield();
     else
       std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
   }
})

void
   reset( void )                    // Reset the XCL_latch
_IF_PUBLIB_LATCH_INLINE(
{
   thread= std::thread::id();
   share.reset();
})

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the XCL_latch
_IF_PUBLIB_LATCH_INLINE(
{
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
})

void
   unlock( void )                   // Release the XCL_latch
_IF_PUBLIB_LATCH_INLINE(
{
   if( thread != std::this_thread::get_id() )
     throw std::runtime_error("XCL_latch unlock error");

   thread= std::thread::id();
   share.count.store(0);
})
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
bool                                // TRUE if latch is held
   is_held( void ) const            // Is Latch held?
_IF_PUBLIB_LATCH_INLINE(
{  return false; })

void
   lock( void )                     // Obtain the NullLatch
_IF_PUBLIB_LATCH_INLINE(
{  })

void
   reset( void )                    // Initialize/Reset the NullLatch
_IF_PUBLIB_LATCH_INLINE(
{  })

bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the NullLatch
_IF_PUBLIB_LATCH_INLINE(
{  return true; })

void
   unlock( void )                   // Release the NullLatch
_IF_PUBLIB_LATCH_INLINE(
{  })
}; // struct NullLatch
#undef _IF_PUBLIB_LATCH_INLINE

_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_LATCH_H_INCLUDED
