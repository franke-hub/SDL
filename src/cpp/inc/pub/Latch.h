//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2026 Frank Eskesen.
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
//       2026/03/03
//
// Implementation notes-
//       All Latch methods are duplicated in ~/src/cpp/lib/pub/Latch.cpp.
//       Latch.cpp methods are used when _PUBLIB_LATCH_INLINE is NOT defined.
//
//       Internal logic for these mechanisms are further described in
//         "~/src/cpp/lib/pub/.LOGICS.md".
//
//       All Latch instances implement *Lockable* and interoperate with the
//       STL (Standard Template Library.) They also implement the reset method,
//       which unconditionally resets the Latch to its initial available state.
//
//       Latch objects do not and never will contain virtual methods.
//       The XCL_latch requires construction, requiring caution when used in
//       static initialization.
//
//       Refer to the implemenation notes for each latch type.
//         Block_latch     Simple exclusive latch (no Thread checking)
//         Latch           Exclusive thread-specific Latch
//         RecursiveLatch  Recursive thread-specific Latch
//         SHR_latch       Shared/exclusive latch pair: shared access
//         XCL_latch       Shared/exclusive latch pair: exclusive access
//
//       Block_latch is the only Latch designed so that it can be locked in
//       one Thread and unlocked in a different one. Latch, RecursiveLatch,
//       and XCL_latch explicity disallow unlock from different Threads.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_LATCH_H_INCLUDED
#define _LIBPUB_LATCH_H_INCLUDED

#include <atomic>                   // For std::atomic
#include <stdexcept>                // For std::runtime_error
#include <thread>                   // For std::thread, std_this_thread, ...
#include <cstdint>                  // For uint32_t

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// MACRO _IF_PUBLIB_LATCH_INLINE, controlled by _PUBLIB_LATCH_INLINE
//----------------------------------------------------------------------------
#ifndef   _PUBLIB_LATCH_INLINE      // For production, use INLINE compilation
#  undef  _PUBLIB_LATCH_INLINE      // (Last for Latch.cpp debug compilation)
#  define _PUBLIB_LATCH_INLINE      // (Last for INLINE compilation)
#endif

#ifdef _PUBLIB_LATCH_INLINE
#  define _IF_PUBLIB_LATCH_INLINE(x) x
#else
#  define _IF_PUBLIB_LATCH_INLINE(x) ;
#endif

#define MAX_SPIN 10'000             // Maximim spin delay in nanoseconds
#define MIN_SPIN  5'000             // Minimum spin delay (after MAX_SPIN)

//============================================================================
//
// Struct-
//       pub::latch::Spin
//
// Purpose-
//       Common spin retry logic method
//
//----------------------------------------------------------------------------
namespace latch {
struct Spin {                       // Implement Spin method
inline void
   spin(                            // Spin delay logic
     int&              spin_count)  // The current spin count
{
   if( (spin_count & 0x0000000f) != 0 ) { // With 15/16 probability
     std::this_thread::yield();
   } else {
     std::this_thread::sleep_for(std::chrono::nanoseconds(spin_count));
     if( spin_count > MAX_SPIN )
       spin_count= MIN_SPIN;
   }
}
}; // struct Spin
}  // namespace latch

//============================================================================
//
// Struct-
//       Block_latch
//
// Purpose-
//       Simple blocking latch.
//
//----------------------------------------------------------------------------
struct Block_latch : protected latch::Spin { // Latch descriptor
//----------------------------------------------------------------------------
// Block_latch::Attributes
//----------------------------------------------------------------------------
typedef uint32_t       latch_t;     // The Block_latch latch type

std::atomic<latch_t>   latch{0};    // The BLOCK latch

//----------------------------------------------------------------------------
// Block_latch::Constructors/destructor
//----------------------------------------------------------------------------
   Block_latch( void ) = default;   // Default constructor, latch available

   Block_latch(latch_t init)        // Constructor, init usually true
{  latch.store(init); }

   ~Block_latch( void ) = default;  // Destructor, does nothing

//----------------------------------------------------------------------------
// Block_latch::Methods
//----------------------------------------------------------------------------
bool                                // TRUE if latch is held by anyone
   is_held( void ) const            // Is Latch held?
_IF_PUBLIB_LATCH_INLINE(
{  return latch.load() != 0; })

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   lock( void )                     // Obtain the Block_latch
_IF_PUBLIB_LATCH_INLINE(
{
   for(int spin_count= 1;;++spin_count) {
     if( try_lock() )
       break;

     spin(spin_count);
   }
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   reset( void )                    // Initialize/Reset the Block_latch
_IF_PUBLIB_LATCH_INLINE(
{  latch.store(0); })               // Note: Unchecked

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   set( void )                      // Set the Block_latch (HELD)
_IF_PUBLIB_LATCH_INLINE(
{  latch.store(1); })               // Note: Unchecked

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the Block_latch
_IF_PUBLIB_LATCH_INLINE(
{  latch_t oldValue= 0;
   latch_t newValue= 1;
   return latch.compare_exchange_strong(oldValue, newValue);
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   unlock( void )                   // Release the Block_latch
_IF_PUBLIB_LATCH_INLINE(
{
   // Verify that the latch is held
   if( latch.load() == 0 )
     throw std::runtime_error("pub::Block_latch unlock when not locked");

   latch.store(0);                  // Release the Block_latch
})
}; // struct Block_latch

//============================================================================
//
// Struct-
//       Latch
//
// Purpose-
//       (Exclusive) spin latch.
//
//----------------------------------------------------------------------------
struct Latch : protected latch::Spin { // Latch descriptor
//----------------------------------------------------------------------------
// Latch::Attributes
//----------------------------------------------------------------------------
std::atomic<std::thread::id>
                       owner{std::thread::id()}; // The spin latch owner

//----------------------------------------------------------------------------
// Latch::Methods
//----------------------------------------------------------------------------
bool                                // TRUE if latch is held by anyone
   is_held( void ) const            // Is Latch held?
_IF_PUBLIB_LATCH_INLINE(
{  return owner.load() != std::thread::id(); })

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   lock( void )                     // Obtain the Latch
_IF_PUBLIB_LATCH_INLINE(
{
   for(int spin_count= 1;;++spin_count) {
     if( try_lock() )
       break;

     spin(spin_count);
   }
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   reset( void )                    // Initialize/Reset the Latch
_IF_PUBLIB_LATCH_INLINE(
{  owner.store(std::thread::id()); })

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the Latch
_IF_PUBLIB_LATCH_INLINE(
{
   std::thread::id oldValue= std::thread::id();
   std::thread::id newValue= std::this_thread::get_id();
   if( owner.compare_exchange_strong(oldValue, newValue) )
     return true;

   if( oldValue == newValue ) {
     //-----------------------------------------------------------------------
     // ERROR: Latch is aready held by this thread
     owner.store(std::thread::id());  // (We release the Latch)
     throw std::runtime_error("pub::Latch try_lock but already locked");
   }

   return false;
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   unlock( void )                   // Release the Latch
_IF_PUBLIB_LATCH_INLINE(
{
   // Verify that the Latch is held by this Thread
   if( owner.load() != std::this_thread::get_id() )
     throw std::runtime_error("pub::Latch unlock when not locked");

   owner.store(std::thread::id());  // Release the Latch
})

//----------------------------------------------------------------------------
// Latch::Static methods
//----------------------------------------------------------------------------
static void
   statistics(const char* info="")  // Display statistics (if activated)
_IF_PUBLIB_LATCH_INLINE(
{  (void)info; })                   // (Not recorded inline)
}; // struct Latch

//============================================================================
//
// Struct-
//       RecursiveLatch
//
// Purpose-
//       Primitive recursive latch.
//
//----------------------------------------------------------------------------
struct RecursiveLatch : protected latch::Spin { // RecursiveLatch descriptor
std::atomic<std::thread::id>
                       owner{std::thread::id()}; // The RecursiveLatch owner
uintptr_t              count{};     // Share count

//----------------------------------------------------------------------------
// RecursiveLatch::Methods
//----------------------------------------------------------------------------
bool                                // TRUE if latch is held by anyone
   is_held( void ) const            // Is Latch held?
_IF_PUBLIB_LATCH_INLINE(
{  return owner.load() != std::thread::id(); })

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   lock( void )                     // Obtain the Latch
_IF_PUBLIB_LATCH_INLINE(
{
   for(int spin_count= 1;;++spin_count) {
     if( try_lock() )
       break;

     spin(spin_count);
   }
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   reset( void )                    // Initialize/Reset the RecursiveLatch
_IF_PUBLIB_LATCH_INLINE(
{
   count= 0;
   owner.store(std::thread::id());
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain a RecursiveLatch
_IF_PUBLIB_LATCH_INLINE(
{
   std::thread::id oldValue= std::thread::id();
   std::thread::id newValue= std::this_thread::get_id();
   if( owner.compare_exchange_strong(oldValue, newValue)
       || oldValue == newValue ) {
     ++count;
     return true;
   }

   return false;
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   unlock( void )                   // Release the RecursiveLatch
_IF_PUBLIB_LATCH_INLINE(
{
   // Verify that the current thread holds the RecursiveLatch
   if( owner.load() != std::this_thread::get_id() )
     throw std::runtime_error("pub::RecursiveLatch unlock when not locked");

   // We have the latch (so we own both the count and the latch)
   --count;                         // Decrement the recursion count
   if( count == 0 )                 // If we're releasing the latch
     owner.store(std::thread::id());
})
}; // struct RecursiveLatch

//============================================================================
//
// Struct-
//       SHR_latch
//
// Purpose-
//       Primitive shared/exclusive latch, held shared.
//
// Implementation notes-
//       A Thread may hold either a SHR_latch or an XCL_latch, but not both.
//       Upgrade must be used to convert share to exclusive mode.
//
//----------------------------------------------------------------------------
static_assert( sizeof(uintptr_t) == 8 || sizeof(uintptr_t) == 4
             , "Unexpected sizeof(uintptr_t) [code update required]" );

struct SHR_latch : protected latch::Spin { // SHR_latch descriptor
//----------------------------------------------------------------------------
// SHR_latch::Attributes
//----------------------------------------------------------------------------
std::atomic<uintptr_t> count{};     // The number of shared users
std::thread::id        owner{std::thread::id()}; // The XCL owner

static constexpr const uintptr_t
       HBIT= sizeof(uintptr_t) == 8 ? 0x8000'0000'0000'0000L : 0x8000'0000;

//----------------------------------------------------------------------------
// SHR_latch::Methods
//----------------------------------------------------------------------------
bool                                // TRUE if latch is held by anyone
   is_held( void ) const            // Is Latch held or reserved?
_IF_PUBLIB_LATCH_INLINE(
{  return count.load() != 0; })

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   lock( void )                     // Obtain the SHR_latch
_IF_PUBLIB_LATCH_INLINE(
{
   for(int spin_count= 1;;++spin_count) {
     if( try_lock() )
       break;

     spin(spin_count);
   }
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   reset( void )                    // Initialize/reset the SHR/XCL_latch
_IF_PUBLIB_LATCH_INLINE(
{  owner= std::thread::id(); count.store(0); })

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the Latch
_IF_PUBLIB_LATCH_INLINE(
{
   uintptr_t oldValue= count.load();
   if( oldValue & HBIT )            // (Disallow SHR_latch if XCL reservation)
     return false;

   uintptr_t newValue= oldValue + 1;
   return count.compare_exchange_strong(oldValue, newValue);
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   unlock( void )                   // Release the SHR_latch
_IF_PUBLIB_LATCH_INLINE(
{
   // Unlock, detecting unlock when not locked errors
   uintptr_t oldValue= count.load();
   for(;;) {
     if( oldValue == 0 || oldValue == HBIT ) // If unlock when not locked
       throw std::runtime_error("pub::SHR_latch unlock when not locked");

     uintptr_t newValue= oldValue - 1;
     if( count.compare_exchange_strong(oldValue, newValue) )
       break;
   }
})
}; // struct SHR_latch

//============================================================================
//
// Struct-
//       XCL_latch
//
// Purpose-
//       Extends a SHR_latch to allow exclusive use.
//
//----------------------------------------------------------------------------
struct XCL_latch : protected latch::Spin { // XCL_latch descriptor
SHR_latch&             share;       // The associated SHR_latch

static constexpr const uintptr_t
       HBIT= sizeof(uintptr_t) == 8 ? 0x8000'0000'0000'0000L : 0x8000'0000;

static constexpr const uintptr_t
       HONE= sizeof(uintptr_t) == 8 ? 0x8000'0000'0000'0001L : 0x8000'0001;

//----------------------------------------------------------------------------
// XCL_latch::Constructor/destructor
//----------------------------------------------------------------------------
   XCL_latch(                       // Constructor
     SHR_latch&        source)
_IF_PUBLIB_LATCH_INLINE(
:  share(source) {})

   ~XCL_latch( void )               // Destructor
_IF_PUBLIB_LATCH_INLINE(= default;) // Explicitly *DOES NOTHING*

//----------------------------------------------------------------------------
// XCL_latch::Methods
//----------------------------------------------------------------------------
/***
   Downgrade from exclusive to shared mode.

   Preconditions:
     The exclusive latch *MUST* be held by the currently running Thread.
***/
// (Compare to unlock)
void
   downgrade( void )                // Downgrade XCL_latch to SHR_latch
_IF_PUBLIB_LATCH_INLINE(
{
   if( share.owner != std::this_thread::get_id()
       || (share.count.load() & HBIT) == 0 )
     throw std::runtime_error("pub::XCL_latch downgrade when not locked");

   share.owner= std::thread::id();
   share.count.store(1);
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE if latch is reserved by anyone
   is_held( void ) const            // Is latch (exclusively) held
_IF_PUBLIB_LATCH_INLINE(
{  return share.count.load() & HBIT; }) // True even when held by this thread

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   lock( void )                     // Obtain the XCL_latch
_IF_PUBLIB_LATCH_INLINE(
{
   for(int spin_count= 1;;++spin_count) {
     if( try_lock() )
       break;

     spin(spin_count);
   }
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   reset( void )                    // Unconditionally reset the XCL_latch
_IF_PUBLIB_LATCH_INLINE(
{
   share.owner= std::thread::id();
   share.reset();
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   try_lock( void )                 // Attempt to obtain the XCL_latch
_IF_PUBLIB_LATCH_INLINE(
{
   if( !try_reserve() )             // If we can't reserve
     return false;                  // (We can't continue)

   // SUCCESS: Wait for all shares to unlock.
   uintptr_t oldValue= share.count.load();
   for(uint32_t spin_count= 1; oldValue != HBIT; ++spin_count) {
     if( spin_count & 0x00000007 )
       std::this_thread::yield();
     else {
       std::this_thread::sleep_for(std::chrono::nanoseconds(spin_count));
       if( spin_count > MAX_SPIN ) {
         spin_count= MIN_SPIN;
       }
     }
     oldValue= share.count.load();
   }

   return true;
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff reserved
   try_reserve( void )              // Try to reserve the XCL_latch
_IF_PUBLIB_LATCH_INLINE(
{
   uintptr_t oldValue= share.count.load();
   for(;;) {
     if( oldValue & HBIT ) {        // If already reserved
       if( share.owner == std::this_thread::get_id() ) { // If by this Thread
         unlock();
         throw std::runtime_error("pub::XCL_latch try_reserve "
                                  "when already reserved");
       }

       return false;
     }

     uintptr_t newValue= oldValue | HBIT;
     if( share.count.compare_exchange_strong(oldValue, newValue) )
       break;
   }

   share.owner= std::this_thread::get_id();
   return true;
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// (Compare to downgrade)
void
   unlock( void )                   // Release the XCL_latch (and SHR_latch)
_IF_PUBLIB_LATCH_INLINE(
{
   if( share.owner != std::this_thread::get_id() )
     throw std::runtime_error("pub::XCL_latch unlock when not locked");

   share.owner= std::thread::id();
   share.count.store(0);
})

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE if sucessful
   upgrade( void )                  // Upgrade a SHR_latch into an XCL_Latch
/***
   Upgrade from shared to exclusive mode.

   Preconditions:
     The exclusive latch *MUST NOT* be held by the currently running Thread.
     The shared latch *MUST* be *SINGLY* held by the currently running Thread.
***/
_IF_PUBLIB_LATCH_INLINE(
{
   if( !try_reserve() )             // If we can't reserve
     return false;                  // (We can't continue)

   if( share.count.load() == HBIT ) { // If invoked without a SHR_latch held
     unlock();                      // (Restore the pre-reserve state)
     throw std::runtime_error("pub::XCL_latch upgrade without SHR_latch held");
   }

   // Wait for all shares but one to unlock.
   uintptr_t oldValue= share.count.load();
   for(uint32_t spin_count= 1; oldValue != HONE; ++spin_count) {
     if( spin_count & 0x00000007 )
       std::this_thread::yield();
     else {
       std::this_thread::sleep_for(std::chrono::nanoseconds(spin_count));
       if( spin_count > MAX_SPIN ) {
         spin_count= MIN_SPIN;
       }
     }
     oldValue= share.count.load();
   }

   return true;
})
}; // struct XCL_latch

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// (Macro removal)
#undef _IF_PUBLIB_LATCH_INLINE
#undef MAX_SPIN
#undef MIN_SPIN

_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_LATCH_H_INCLUDED
