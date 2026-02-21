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
//       Latch.cpp
//
// Purpose-
//       Primitive mechanisms for granting access to a resource.
//
// Last change date-
//       2026/02/20
//
// Implementation notes-
//       Maintain implementation compatability with Latch.h
//       Only used if _PUBLIB_LATCH_INLINE is left undefined by Latch.h
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE                 // For sched_getcpu
#define _GNU_SOURCE
#endif

#include <atomic>                   // For std::atomic
#include <stdexcept>                // For std::runtime_error
#include <thread>                   // For std::thread::id
#include <cstdint>                  // For uint32_t
#include <sched.h>                  // For sched_getcpu

#include <pub/Debug.h>              // For debugging
#include "pub/Latch.h"              // For pub::Latch definitions
#include <pub/Trace.h>              // For pub::Trace methods
#include <pub/utility.h>            // For pub::utility::to_string
#include <pub/utility.i>            // For pub::b2c, pub::s2c

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging subroutines
using namespace PUB::utility;       // For pub::utility::to_string
using std::string;

//----------------------------------------------------------------------------
#ifndef _PUBLIB_LATCH_INLINE        // Conditionally compiled

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

// Production mode settings: all false
,  USE_IDEBUG= true                 // Use internal debugging checks?
,  USE_ITRACE= false                // Use internal trace?
}; // generic enum

#define MAX_SPIN 10'000             // Maximim spin delay in nanoseconds
#define MIN_SPIN  5'000             // Minimum spin delay (after MAX_SPIN)

//----------------------------------------------------------------------------
// Static constructor/destructor
//----------------------------------------------------------------------------
// Global initialization/termination
// We don't want to accidentally leave debugging active
namespace {                         // Anonymous namespace
static struct StaticGlobal {
   StaticGlobal( void )             // Initialize main()'s tl_tlss
{  if( HCDM ) debugf("pub::Latch::StaticGlobal!\n"); }

   ~StaticGlobal( void )            // Initialize main()'s tl_tlss
{  if( HCDM ) debugf("pub::Latch::StaticGlobal~\n"); }
}  static_global;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Subroutine-
//       id_string
//
// Purpose-
//       Return the current std::thread::id string
//
//----------------------------------------------------------------------------
static inline string                // The current std::thread::id string
   id_string( void )                // Get current std::thread::id string
{  return to_string(std::this_thread::get_id()); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       id_thread
//
// Purpose-
//       Return the current std::thread::id
//
//----------------------------------------------------------------------------
static inline std::thread::id       // The current std::thread::id
   id_thread( void )                // Get current std::thread::id
{  return std::this_thread::get_id(); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       tid(const std::thread::id&)
//       tid( void )
//
// Purpose-
//       Convert a std::thread::id& to a void*
//       Return the current std::thread::id as a void*
//
// Implemenation note-
//       Only used when USE_ITRACE == true
//
//----------------------------------------------------------------------------
static inline const void*
   vv2v(const void** t)             // Only invoked from tid()
{  return *t; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static inline const void*           // The associated std::thread::id (void*)
   tid(const std::thread::id& t)    // Get associated std::thread::id (void*)
{  return vv2v((const void**)&t); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static inline const void*           // The current std::thread::id (void*)
   tid( void )                      // Get current std::thread::id (void*)
{  return tid(id_thread()); }

//============================================================================
//
// Macro-
//       _LOCK
//
// Purpose-
//       Implement lock method
//
//----------------------------------------------------------------------------
#define _LOCK(type, debug)                                                   \
{  if( USE_ITRACE )                                                          \
     Trace::trace(type, "=TRY", this);                                       \
                                                                             \
   for(uint32_t spin_count= 1;;++spin_count) {                               \
     if( try_lock() )                                                        \
       break;                                                                \
                                                                             \
     if( (spin_count & 0x0000000f) == 0 ) {                                  \
       if( (spin_count & 0x00000010) != 0 )                                  \
         std::this_thread::yield();                                          \
       else {                                                                \
         std::this_thread::sleep_for(std::chrono::nanoseconds(spin_count));  \
         if( spin_count > MAX_SPIN ) {                                       \
           debug                                                             \
           spin_count= MIN_SPIN;                                             \
   } } } }                                                                   \
                                                                             \
   if( USE_ITRACE )                                                          \
     Trace::trace(type, "=OWN", this);                                       \
}

//============================================================================
//
// Struct-
//       pub::Basic_latch::methods
//
// Purpose-
//       Implement pub::Basic_latch methods defined in Latch.h
//
// Implementation notes-
//       (Explicitly) does not check thread::id in lock or unlock.
//
//----------------------------------------------------------------------------
bool                                // TRUE if latch is held by anyone
   Basic_latch::is_held( void ) const // Is latch held?
{  return latch.load() != 0; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   Basic_latch::lock( void )        // Obtain the Basic_latch
{
   if( USE_ITRACE )
     Trace::trace(".LKB", "=TRY", this);

   for(uint32_t spin_count= 1;;++spin_count) {
     if( try_lock() )
       break;

     if( (spin_count & 0x0000000f) == 0 ) {
       if( (spin_count & 0x00000010) != 0 )
         std::this_thread::yield();
       else {
         std::this_thread::sleep_for(std::chrono::nanoseconds(spin_count));
         if( spin_count > MAX_SPIN ) {
           if( USE_IDEBUG )
             debugh("pub::BasicLatch(%p) SPIN\n", this);
           else if( USE_ITRACE )
             Trace::trace(".LKB", "SPIN", this, i2v(spin_count));
           spin_count= MIN_SPIN;
         }
       }
     }
   }

   if( USE_ITRACE )
     Trace::trace(".LKB", "=OWN", this);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   Basic_latch::reset( void )       // Initialize/Reset the Basic_latch
{  latch.store(0); }                // Note: Unchecked

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   Basic_latch::try_lock( void )    // Attempt to obtain the Basic_latch
{  latch_t oldValue= 0;
   latch_t newValue= 1;
   return latch.compare_exchange_strong(oldValue, newValue);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   Basic_latch::unlock( void )      // Release the Basic_latch
{
   if( USE_ITRACE )
     Trace::trace(".LKB", "=REL", this);

   // Verify that the latch is held
   if( latch.load() == 0 )
     throwf("pub::Basic_latch(%p)::unlock when not locked\n", this);

   latch.store(0);                  // Release the Basic_latch
}

//============================================================================
//
// Struct-
//       pub::Latch::methods
//
// Purpose-
//       Implement pub::Latch methods defined in Latch.h
//
//----------------------------------------------------------------------------
bool                                // TRUE if latch is held by anyone
   Latch::is_held( void ) const     // Is Latch held?
{  return latch.load() != std::thread::id(); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   Latch::lock( void )              // Obtain the Latch
{
   if( USE_ITRACE )
     Trace::trace(".LKL", "=TRY", this);

   for(uint32_t spin_count= 1;;++spin_count) {
     if( try_lock() )
       break;

     if( (spin_count & 0x0000000f) == 0 ) {
       if( (spin_count & 0x00000010) != 0 )
         std::this_thread::yield();
       else {
         std::this_thread::sleep_for(std::chrono::nanoseconds(spin_count));
         if( spin_count > MAX_SPIN ) {
           if( USE_IDEBUG ) {
             string s_thread= id_string();
             string s_latch= to_string(latch);
             debugh("pub::Latch(%p): SPIN Current(%s) Latch(%s)\n", this
                   , s2c(s_thread), s2c(s_latch));
           } else if( USE_ITRACE ) {
             Trace::trace(".LKL", "SPIN", this, i2v(spin_count)
                         , tid(), tid(latch));
           }
           spin_count= MIN_SPIN;
         }
       }
     }
   }

   if( USE_ITRACE )
     Trace::trace(".LKL", "=OWN", this);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   Latch::reset( void )             // Initialize/Reset the Latch
{  latch.store(std::thread::id()); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   Latch::try_lock( void )          // Attempt to obtain the Latch
{
   std::thread::id oldValue= std::thread::id();
   std::thread::id newValue= std::this_thread::get_id();
   if( HCDM || VERBOSE > 0 ) {
     bool cc= latch.compare_exchange_strong(oldValue, newValue);
     traceh("%4d pub::Latch: CPU[%2d] %s= compare_exchange(%s,%s)\n", __LINE__
           , sched_getcpu(), b2c(cc)
           , s2c(to_string(oldValue)), s2c(to_string(newValue)));
     if( cc )
       return true;
   } else {
     if( latch.compare_exchange_strong(oldValue, newValue) )
       return true;
   }

   if( oldValue != newValue )       // If Latch is by a different Thread
     return false;

   //-------------------------------------------------------------------------
   // ERROR: Latch is aready held by this thread (Probably not recoverable)
   if( USE_ITRACE )
     Trace::trace(".LKL", "=LRE", this, tid(newValue)); // LatchRecursionError

   latch.store(std::thread::id());  // Release the Latch (held by this Thread)
   throwf("pub::Latch(%p)::try_lock thread(%s) when already locked\n", this
         , s2c(id_string()));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   Latch::unlock( void )            // Release the Latch
{
   if( HCDM || VERBOSE > 0 )
     traceh("%4d pub::Latch: CPU[%2d] latch(%s) thread(%s)\n", __LINE__
           , sched_getcpu(), s2c(to_string(latch.load())), s2c(id_string()));
   else if( USE_ITRACE )
     Trace::trace(".LKL", "=REL", this, tid(latch));

   // Verify that the current thread holds the Latch
   if( latch.load() != std::this_thread::get_id() )
     throwf("pub::Latch(%p)::unlock when not locked\n", this);

   latch.store(std::thread::id());  // Release the Latch
}

//============================================================================
//
// Struct-
//       pub::RecursiveLatch::methods
//
// Purpose-
//       Implement pub::RecursiveLatch methods defined in Latch.h
//
//----------------------------------------------------------------------------
bool                                // TRUE if latch is held by anyone
   RecursiveLatch::is_held( void ) const // Is Latch held?
{  return latch.load() != std::thread::id(); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   RecursiveLatch::lock( void )     // Obtain the Latch
{
   if( USE_ITRACE )
     Trace::trace(".LKR", "=TRY", this, tid());

   for(uint32_t spin_count= 1;;++spin_count) {
     if( try_lock() )
       break;

     if( (spin_count & 0x0000000f) == 0 ) {
       if( (spin_count & 0x00000010) != 0 )
         std::this_thread::yield();
       else {
         std::this_thread::sleep_for(std::chrono::nanoseconds(spin_count));
         if( spin_count > MAX_SPIN ) {
           if( USE_IDEBUG )
             debugh("pub::RecursiveLatch(%p) SPIN: "
                    "count(%zd) Current(%s) Owner(%s)\n", this
                   , count, s2c(id_string()), s2c(to_string(latch)) );
           else if( USE_ITRACE )
             Trace::trace(".LKR", "SPIN", this, i2v(spin_count)
                         , tid(), tid(latch));
           spin_count= MIN_SPIN;
         }
       }
     }
   }

   if( USE_ITRACE )
     Trace::trace(".LKR", "=OWN", this, i2v(count), tid());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   RecursiveLatch::reset( void )    // Initialize/Reset the RecursiveLatch
{
   count= 0;
   latch.store(std::thread::id());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   RecursiveLatch::try_lock( void ) // Attempt to obtain a RecursiveLatch
{
   std::thread::id oldValue= std::thread::id();
   std::thread::id newValue= std::this_thread::get_id();
   if( latch.compare_exchange_strong(oldValue, newValue)
       || oldValue == newValue ) {
     ++count;
     return true;
   }

   return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   RecursiveLatch::unlock( void )   // Release the RecursiveLatch
{
   if( USE_ITRACE )
     Trace::trace(".LKR", "=REL", this, i2v(count));

   // Verify that the current thread holds the RecursiveLatch
   if( latch.load() != std::this_thread::get_id() )
     throwf("pub::RecursiveLatch(%p)::unlock when not locked\n", this);

   // We have the latch (so we own both the count and the latch)
   --count;                         // Decrement the recursion count
   if( count == 0 )                 // If we're releasing the latch
     latch.store(std::thread::id());
}

//============================================================================
//
// Struct-
//       pub::SHR_latch::methods
//
// Purpose-
//       Implement pub::SHR_latch methods defined in Latch.h
//
//----------------------------------------------------------------------------
bool                                // TRUE if latch is held by anyone
   SHR_latch::is_held( void ) const // Is Latch held or reserved?
{  return count.load() != 0; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   SHR_latch::lock( void )          // Obtain the SHR_latch
{
   if( USE_ITRACE )
     Trace::trace(".LKS", "=TRY", this);

   for(uint32_t spin_count= 1;;++spin_count) {
     if( try_lock() )
       break;

     if( (spin_count & 0x0000000f) == 0 ) {
       if( (spin_count & 0x00000010) != 0 )
         std::this_thread::yield();
       else {
         std::this_thread::sleep_for(std::chrono::nanoseconds(spin_count));
         if( spin_count > MAX_SPIN ) {
           if( USE_IDEBUG )
             debugh("pub::SHR_latch(%p) SPIN: count(%zx) Current(%s)\n", this
                   , count.load(), s2c(id_string()));
           else if( USE_ITRACE )
             Trace::trace(".LKS", "SPIN", this, i2v(spin_count));
           spin_count= MIN_SPIN;
         }
       }
     }
   }

   if( USE_ITRACE )
     Trace::trace(".LKS", "=OWN", this);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   SHR_latch::reset( void )         // Initialize/reset the SHR/XCL_latch
{  thread= std::thread::id(); count.store(0); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   SHR_latch::try_lock( void )      // Attempt to obtain the Latch
{
   uintptr_t oldValue= count.load();
   if( oldValue & HBIT )            // (Disallow SHR_latch if XCL reservation)
     return false;

   uintptr_t newValue= oldValue + 1;
   return count.compare_exchange_strong(oldValue, newValue);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   SHR_latch::unlock( void )        // Release the SHR_latch
{
   if( USE_ITRACE )
     Trace::trace(".LKS", "=REL", this);

   // Unlock, detecting unlock when not locked errors
   uintptr_t oldValue= count.load();
   for(;;) {
     if( oldValue == 0 || oldValue == HBIT ) // If unlock when not locked
       throwf("pub::SHR_latch(%p)::unlock when not locked\n", this);

     uintptr_t newValue= oldValue - 1;
     if( count.compare_exchange_strong(oldValue, newValue) )
       break;
   }
}

//============================================================================
//
// Struct-
//       pub::XCL_latch::methods
//
// Purpose-
//       Implement pub::XCL_latch methods defined in Latch.h
//
//----------------------------------------------------------------------------
   XCL_latch::XCL_latch(            // Constructor
     SHR_latch&        source)
:  share(source)
{  if( HCDM ) debugf("pub::XCL_latch!(%p,%p)\n", this, &source); }

   XCL_latch::~XCL_latch( void )    // Destructor
{  if( HCDM ) debugf("pub::XCL_latch~(%p)\n", this); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   XCL_latch::downgrade( void )     // Downgrade XCL_latch to SHR_latch
{
   if( share.thread != std::this_thread::get_id()
       || (share.count.load() & HBIT) == 0 )
     throwf("pub::XCL_latch(%p)::downgrade when not locked\n", this);

   share.thread= std::thread::id();
   share.count.store(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE if latch is reserved by anyone
   XCL_latch::is_held( void ) const // Is latch reserved?
{  return share.count.load() & HBIT; } // True even when held by this thread

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   XCL_latch::lock( void )          // Obtain the XCL_latch
{
   if( USE_ITRACE )
     Trace::trace(".LKX", "=TRY", this);

   for(uint32_t spin_count= 1;;++spin_count) {
     if( try_lock() )
       break;

     if( (spin_count & 0x0000000f) == 0 ) {
       if( (spin_count & 0x00000010) != 0 )
         std::this_thread::yield();
       else {
         std::this_thread::sleep_for(std::chrono::nanoseconds(spin_count));
         if( spin_count > MAX_SPIN ) {
           if( USE_IDEBUG )
             debugh("pub::XCL_latch(%p) SPIN: shr_count(%zx) "
                    "thread(%s) owner(%s)\n"
                   , this, share.count.load()
                   , s2c(id_string()), s2c(to_string(share.thread)));
           else if( USE_ITRACE )
             Trace::trace(".LKX", "SPIN", this, i2v(spin_count));
           spin_count= MIN_SPIN;
         }
       }
     }
   }

   if( USE_ITRACE )
     Trace::trace(".LKX", "=OWN", this);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   XCL_latch::reset( void )         // Reset the XCL_latch
{  share.reset(); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   XCL_latch::try_lock( void )      // Attempt to obtain the XCL_latch
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

// share.count= HBIT;
// share.thread= std::this_thread::get_id();
   return true;
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Note: invoked from try_lock, upgrade, and/or application
bool                                // TRUE iff reserved
   XCL_latch::try_reserve( void )   // Try to reserve the XCL_latch
{
   uintptr_t oldValue= share.count.load();
   for(;;) {
     if( oldValue & HBIT ) {        // If already reserved
       if( share.thread == std::this_thread::get_id() ) { // If by this Thread
         unlock();
         throwf("pub::XCL_latch(%p)::try_reserve thread(%s) "
                "when already reserved\n", this, s2c(id_string()));
       }

       return false;
     }

     uintptr_t newValue= oldValue | HBIT;
     if( share.count.compare_exchange_strong(oldValue, newValue) )
       break;
   }

// share.count= HBIT | (current share count);
   share.thread= std::this_thread::get_id();
   return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   XCL_latch::unlock( void )        // Release the XCL_latch
{
   if( USE_ITRACE )
     Trace::trace(".LKX", "=REL", this);

   if( share.thread != std::this_thread::get_id() )
     throwf("pub::XCL_latch(%p)::unlock when not locked\n", this);

   share.thread= std::thread::id();
   share.count.store(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool
    XCL_latch::upgrade( void )      // Upgrade a SHR_latch into an XCL_Latch
{
   if( !try_reserve() )             // If we can't reserve
     return false;                  // (We can't continue)

   if( share.count.load() == HBIT ) { // If invoked without a SHR_latch held
     unlock();                      // (Restore the pre-reserve state)
     throwf("pub::XCL_latch(%p)::upgrade without SHR_latch held\n", this);
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

// share.count= HONE);
// share.thread= std::this_thread::get_id();
   return true;
}
} // namespace _LIBPUB_NAMESPACE
#endif // #ifndef _PUBLIB_LATCH_INLINE
