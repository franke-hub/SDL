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
//       Latch.cpp
//
// Purpose-
//       Primitive mechanisms for granting access to a resource.
//
// Last change date-
//       2025/10/25
//
// Implementation notes-
//       Maintain implementation compatability with Latch.h
//       Only used if _PUBLIB_LATCH_DEBUG is defined in Latch.h
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE                 // For sched_getcpu
#define _GNU_SOURCE
#endif

#ifndef _PUBLIB_LATCH_DEBUG
#define _PUBLIB_LATCH_DEBUG         // (Always use outline methods)
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
using namespace PUB;                // For pub::Trace methods
using namespace PUB::debugging;     // For debugging subroutines
using namespace PUB::utility;       // For pub::utility::to_string

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

// Production mode settings: USE_TRACEF= false
,  USE_TRACEF= false                // Use internal trace?
}; // generic enum

//----------------------------------------------------------------------------
// Static constructor/destructor
//----------------------------------------------------------------------------
// Global initialization/termination
// We don't want to accidentally leave debugging active
namespace {                         // Anonymous namespace
static struct StaticGlobal {
   StaticGlobal( void )             // Initialize main()'s tl_tlss
{  if( HCDM || USE_TRACEF ) debugh("Latch::StaticGlobal!\n");
}

   ~StaticGlobal( void )            // Initialize main()'s tl_tlss
{  if( HCDM || USE_TRACEF ) debugh("Latch::StaticGlobal~\n");
}
}  static_global;
}  // Anonymous namespace

//----------------------------------------------------------------------------
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

void
   pub::Basic_latch::lock( void )   // Obtain the Basic_latch
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
}

void
   pub::Basic_latch::reset( void )  // Initialize/Reset the Basic_latch
{  latch.store(0); }                // Note: Unchecked

bool                                // TRUE iff successful
   pub::Basic_latch::try_lock( void ) // Attempt to obtain the Basic_latch
{  latch_t oldValue= 0;
   latch_t newValue= 1;
   return latch.compare_exchange_strong(oldValue, newValue);
}

void
   pub::Basic_latch::unlock( void ) // Release the Basic_latch
{
   // Verify that the latch is held
   if( latch.load() == 0 )
     throw std::runtime_error("Basic_latch unlock error");

   latch.store(0);                  // Release the Basic_latch
}

//----------------------------------------------------------------------------
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

void
   pub::Latch::lock( void )         // Obtain the Latch
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
}

void
   pub::Latch::reset( void )        // Initialize/Reset the Latch
{  latch.store(std::thread::id()); }

bool                                // TRUE iff successful
   pub::Latch::try_lock( void )     // Attempt to obtain the Latch
{
   std::thread::id oldValue= std::thread::id();
   std::thread::id newValue= std::this_thread::get_id();
   if( latch.load() == newValue ) { // If already held
     latch.store(std::thread::id());

     if( USE_TRACEF )
       throwf("Latch recursion error");
     throw std::runtime_error("Latch recursion error");
   }

   if( USE_TRACEF ) {
     std::thread::id wasValue= oldValue;
     bool cc= latch.compare_exchange_strong(oldValue, newValue);
     traceh("Latch:%.4d [%2d] %s= compare_exchange(%s,%s)\n", __LINE__
           , sched_getcpu()
           , b2c(cc), s2c(to_string(wasValue)), s2c(to_string(newValue)));
     return cc;
   } else {
     return latch.compare_exchange_strong(oldValue, newValue);
   }
}

void
   pub::Latch::unlock( void )       // Release the Latch
{
   // Verify that the current thread holds the Latch
   if( USE_TRACEF )
     traceh("Latch:%.4d [%2d] latch(%s) thread(%s)\n", __LINE__
           , sched_getcpu()
           , s2c(to_string(latch.load()))
           , s2c(to_string(std::this_thread::get_id())));

   if( latch.load() != std::this_thread::get_id() ) {
     if( USE_TRACEF )
       throwf("Latch unlock error");
     throw std::runtime_error("Latch unlock error");
   }

   latch.store(std::thread::id());  // Release the Latch
}

//----------------------------------------------------------------------------
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

void
   pub::RecursiveLatch::lock( void ) // Obtain the Latch
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
   pub::RecursiveLatch::reset( void ) // Initialize/Reset the RecursiveLatch
{
   count= 0;
   latch.store(std::thread::id());
}

bool                                // TRUE iff successful
   pub::RecursiveLatch::try_lock( void ) // Attempt to obtain a RecursiveLatch
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
   pub::RecursiveLatch::unlock( void ) // Release the RecursiveLatch
{
   // Verify that the current thread holds the RecursiveLatch
   if( latch.load() != std::this_thread::get_id() )
     throw std::runtime_error("RecursiveLatch unlock error");

   // We have the latch (so we own both the count and the latch)
   --count;                         // Decrement the recursion count
   if( count == 0 )                 // If we're releasing the latch
     latch.store(std::thread::id());
}

//----------------------------------------------------------------------------
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

void
   pub::SHR_latch::lock( void )     // Obtain the SHR_latch
{
   while( !try_lock() )
     std::this_thread::yield();
}

void
   pub::SHR_latch::reset( void )    // Initialize/Reset the SHR_latch
{  count.store(0); }

bool                                // TRUE iff successful
   pub::SHR_latch::try_lock( void ) // Attempt to obtain the latch
{
   uintptr_t oldValue= count.load();
   if( oldValue & HBIT )            // (Disallow SHR_latch if XCL reservation)
     return false;

   uintptr_t newValue= oldValue + 1;
   return count.compare_exchange_strong(oldValue, newValue);
}

void
   pub::SHR_latch::unlock( void )   // Release the SHR_latch
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
}

//----------------------------------------------------------------------------
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

void
   pub::XCL_latch::downgrade( void ) // Downgrade XCL_latch to SHR_latch
{
   if( thread != std::this_thread::get_id()
       || share.count.load() != HBIT )
     throw std::runtime_error("XCL_latch downgrade error");

   thread= std::thread::id();
   share.count.store(1);
}

bool                                // TRUE if latch is held
   pub::XCL_latch::is_held( void ) const // Is latch (exclusively) held
{  return share.count.load() & HBIT; }

void
   pub::XCL_latch::lock( void )     // Obtain the XCL_latch
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
   pub::XCL_latch::reset( void )    // Reset the XCL_latch
{
   thread= std::thread::id();
   share.reset();
}

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
   for(uint32_t spinCount= 1; oldValue != HBIT; spinCount++) {
     if( spinCount & 0x00000007 )
       std::this_thread::yield();
     else
       std::this_thread::sleep_for(std::chrono::nanoseconds(spinCount));
     oldValue= share.count.load();
   }

   return true;
}

void
   pub::XCL_latch::unlock( void )   // Release the XCL_latch
{
   if( thread != std::this_thread::get_id() )
     throw std::runtime_error("XCL_latch unlock error");

   thread= std::thread::id();
   share.count.store(0);
}

//----------------------------------------------------------------------------
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
