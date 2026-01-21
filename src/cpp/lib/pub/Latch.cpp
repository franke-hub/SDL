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
//       2026/01/18
//
// Implementation notes-
//       Maintain implementation compatability with Latch.h
//       Only used if _PUBLIB_LATCH_DEBUG is defined in Latch.h
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

//----------------------------------------------------------------------------
#ifdef _PUBLIB_LATCH_DEBUG          // Conditionally compiled

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
{  if( HCDM || VERBOSE > 0 ) debugh("pub::Latch::StaticGlobal!\n"); }

   ~StaticGlobal( void )            // Initialize main()'s tl_tlss
{  if( HCDM || VERBOSE > 0 ) debugh("pub::Latch::StaticGlobal~\n"); }
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
static inline std::string           // The current std::thread::id string
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
bool                                // TRUE if latch is held
   pub::Basic_latch::is_held( void ) const // Is latch held?
{  return latch.load() != 0; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   pub::Basic_latch::lock( void )   // Obtain the Basic_latch
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
             debugh("BasicLatch(%p) SPIN\n", this);
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
   pub::Basic_latch::reset( void )  // Initialize/Reset the Basic_latch
{  latch.store(0); }                // Note: Unchecked

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   pub::Basic_latch::try_lock( void ) // Attempt to obtain the Basic_latch
{  latch_t oldValue= 0;
   latch_t newValue= 1;
   return latch.compare_exchange_strong(oldValue, newValue);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   pub::Basic_latch::unlock( void ) // Release the Basic_latch
{
   if( USE_ITRACE )
     Trace::trace(".LKB", "=REL", this);

   // Verify that the latch is held
   if( latch.load() == 0 )
     throw std::runtime_error("Basic_latch unlock error");

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
bool                                // TRUE if latch is held
   pub::Latch::is_held( void ) const // Is Latch held?
{  return latch.load() != std::thread::id(); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   pub::Latch::lock( void )         // Obtain the Latch
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
   pub::Latch::reset( void )        // Initialize/Reset the Latch
{  latch.store(std::thread::id()); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   pub::Latch::try_lock( void )     // Attempt to obtain the Latch
{
   std::thread::id oldValue= std::thread::id();
   std::thread::id newValue= std::this_thread::get_id();
   if( VERBOSE == 0 ) {
     if( latch.compare_exchange_strong(oldValue, newValue) )
       return true;
   } else {
     bool cc= latch.compare_exchange_strong(oldValue, newValue);
     traceh("%4d Latch: CPU[%2d] %s= compare_exchange(%s,%s)\n", __LINE__
           , sched_getcpu(), b2c(cc)
           , s2c(to_string(oldValue)), s2c(to_string(newValue)));
     if( cc )
       return true;
   }

   if( oldValue != newValue )       // If Latch is by a different Thread
     return false;

   //-------------------------------------------------------------------------
   // ERROR: Latch is aready held by this thread (Probably not recoverable)
   if( USE_ITRACE )
     Trace::trace(".LKL", "=LRE", this, tid(newValue)); // LatchRecursionError
   debugf("pub::Latch(%p) thread(%s) recursion error\n", this
         , s2c(id_string()));

   latch.store(std::thread::id());  // Release the Latch (held by this Thread)
   if( VERBOSE > 0 )
     throwf("pub::Latch recursion error"); // (This adds a backtrace)
   throw std::runtime_error("pub::Latch recursion error");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   pub::Latch::unlock( void )       // Release the Latch
{
   if( VERBOSE > 0 )
     traceh("%4d Latch: CPU[%2d] latch(%s) thread(%s)\n", __LINE__
           , sched_getcpu(), s2c(to_string(latch.load())), s2c(id_string()));
   else if( USE_ITRACE )
     Trace::trace(".LKL", "=REL", this, tid(latch));

   // Verify that the current thread holds the Latch
   if( latch.load() != std::this_thread::get_id() ) {
     if( VERBOSE > 0 )
       throwf("pub::Latch unlock error");
     throw std::runtime_error("pub::Latch unlock error");
   }

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
bool                                // TRUE if latch is held
   pub::RecursiveLatch::is_held( void ) const // Is Latch held?
{  return latch.load() != std::thread::id(); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   pub::RecursiveLatch::lock( void ) // Obtain the Latch
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
             debugh("RecursiveLatch(%p) spin loop: "
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
   pub::RecursiveLatch::reset( void ) // Initialize/Reset the RecursiveLatch
{
   count= 0;
   latch.store(std::thread::id());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   pub::RecursiveLatch::try_lock( void ) // Attempt to obtain a RecursiveLatch
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
   pub::RecursiveLatch::unlock( void ) // Release the RecursiveLatch
{
   if( USE_ITRACE )
     Trace::trace(".LKR", "=REL", this, i2v(count));

   // Verify that the current thread holds the RecursiveLatch
   if( latch.load() != std::this_thread::get_id() )
     throw std::runtime_error("RecursiveLatch unlock error");

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
bool                                // TRUE if latch is held
   pub::SHR_latch::is_held( void ) const // Is Latch held? (shared or exclusive)
{  return count.load() != 0; }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   pub::SHR_latch::lock( void )     // Obtain the SHR_latch
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
             debugh("SHR_latch(%p) spin loop: count(%zx) Current(%s)\n", this
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
   pub::SHR_latch::reset( void )    // Initialize/Reset the SHR_latch
{  count.store(0); }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   pub::SHR_latch::try_lock( void ) // Attempt to obtain the latch
{
   uintptr_t oldValue= count.load();
   if( oldValue & HBIT )            // (Disallow SHR_latch if XCL reservation)
     return false;

   uintptr_t newValue= oldValue + 1;
   return count.compare_exchange_strong(oldValue, newValue);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   pub::SHR_latch::unlock( void )   // Release the SHR_latch
{
   if( USE_ITRACE )
     Trace::trace(".LKS", "=REL", this);

   // Unlock, detecting unlock when not locked errors
   uintptr_t oldValue= count.load();
   for(;;) {
     if( oldValue == 0 || oldValue == HBIT ) // If unlock when not locked
       throw std::runtime_error("SHR_latch unlock error");

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
   pub::XCL_latch::XCL_latch(       // Constructor
     SHR_latch&        source)
:  share(source) {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   pub::XCL_latch::downgrade( void ) // Downgrade XCL_latch to SHR_latch
{
   if( thread != std::this_thread::get_id()
       || share.count.load() != HBIT )
     throw std::runtime_error("XCL_latch downgrade error");

   thread= std::thread::id();
   share.count.store(1);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE if latch is held
   pub::XCL_latch::is_held( void ) const // Is latch (exclusively) held
{  return share.count.load() & HBIT; }

void
   pub::XCL_latch::lock( void )     // Obtain the XCL_latch
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
             debugh("XCL_latch(%p) SPIN: count(%zx) thread(%s) owner(%s)\n"
                   , this, share.count.load()
                   , s2c(id_string()), s2c(to_string(thread)));
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
   pub::XCL_latch::reset( void )    // Reset the XCL_latch
{
   thread= std::thread::id();
   share.reset();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff successful
   pub::XCL_latch::try_lock( void ) // Attempt to obtain the XCL_latch
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
   for(uint32_t spin_count= 1; oldValue != HBIT; ++spin_count) {
     if( spin_count & 0x00000007 )
       std::this_thread::yield();
     else {
       std::this_thread::sleep_for(std::chrono::nanoseconds(spin_count));
       if( spin_count > MAX_SPIN ) {  // Maximum delay: 10 microseconds
         spin_count= MIN_SPIN;
         if( USE_IDEBUG )
           debugh("XCL_latch(%p) TRY SPIN: count(%zx) Current(%s)\n", this
                 , share.count.load(), s2c(id_string()));
         else if( USE_ITRACE )
           Trace::trace(".LKX", "WAIT", this, i2v(spin_count));
       }
     }
     oldValue= share.count.load();
   }

   return true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   pub::XCL_latch::unlock( void )   // Release the XCL_latch
{
   if( USE_ITRACE )
     Trace::trace(".LKX", "=REL", this);

   if( thread != std::this_thread::get_id() )
     throw std::runtime_error("XCL_latch unlock error");

   thread= std::thread::id();
   share.count.store(0);
}

//============================================================================
//
// Struct-
//       pub::NullLatch::methods
//
// Purpose-
//       Implement pub::NullLatch methods defined in Latch.h
//
//----------------------------------------------------------------------------
bool                                // TRUE if latch is held
   pub::NullLatch::is_held( void ) const // Is Latch held?
{  return false; }

void
   pub::NullLatch::lock( void )     // Obtain the NullLatch
{  }

void
   pub::NullLatch::reset( void )    // Initialize/Reset the NullLatch
{  }

bool                                // TRUE iff successful
   pub::NullLatch::try_lock( void ) // Attempt to obtain the NullLatch
{  return true; }

void
   pub::NullLatch::unlock( void )   // Release the NullLatch
{  }
#endif // #ifdef _PUBLIB_LATCH_DEBUG
