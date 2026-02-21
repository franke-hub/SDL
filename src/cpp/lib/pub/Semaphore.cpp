//----------------------------------------------------------------------------
//
//       Copyright (C) 2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Semaphore.cpp
//
// Purpose-
//       Semaphore object methods.
//
// Last change date-
//       2026/02/09
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For namespace pub::debugging
#include "pub/Semaphore.h"          // For pub::Semaphore

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (Generic) enum

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Method-
//       Semaphore::Semaphore
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Semaphore::Semaphore(
     size_t            _count)      // Initial count
:  count(_count), cv(), mutex()
{  }

//----------------------------------------------------------------------------
//
// Method-
//       Semaphore::~Semaphore
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Semaphore::~Semaphore( void )
{
   // This pre-delete test attempts to insure that no waiters exist.
   // If no other waiters exist, the timed wait returns immediately reducing
   // the count from 2 to 1.
   // If other waiters do exist, we might wait up to 1/8th second before
   // retrying the sequence.
   // Since we're in the Semaphore's destructor, no one else should be able to
   // find it. All bets are off if that's not true.
   int retry_count= 0;
   for(; retry_count < 16; ++retry_count) { // Wake up any and all waiters
     reset(2);                      // Possible wake up one waiter
     bool locked= wait(0.125);      // Wait (up to 1/8th second)
     if( locked && count == 1 )     // If we were the only new waiter
       break;                       // We're done

     if( retry_count ) {            // Should not occur
       debugh("pub::Semaphore(%p)::~Semaphore while in use(%d), count(%zd)\n"
             , this, retry_count, count);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Semaphore::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Semaphore::debug(const char* info) const // Debugging display
{
   if( info == nullptr )
     info= "";

   debugf("Semaphore(%p)::debug(%s) count(%#zx,%zd)\n", this, info
         , count, count);
}

//----------------------------------------------------------------------------
//
// Method-
//       Semaphore::post
//
// Purpose-
//       Post the Semaphore, possibly completing a wait
//
//----------------------------------------------------------------------------
void
   Semaphore::post( void )          // Post the Semaphore
{  std::unique_lock<decltype(mutex)> lock(mutex);

   ++count;
   cv.notify_one();
}

//----------------------------------------------------------------------------
//
// Method-
//       Semaphore::reset
//
// Purpose-
//       Reset the post count
//
//----------------------------------------------------------------------------
void
   Semaphore::reset(                // Reset the post count
     size_t            _count)      // To this value
{  std::unique_lock<decltype(mutex)> lock(mutex);

   if( _count > count ) {           // If increased count
     _count -= count;               // The increase
     while( _count > 0 ) {          // For each increase
       ++count;                     // Simulate post
       cv.notify_one();             // "

       --_count;
     }
   } else if( _count < count ) {    // If decreased count
     count= _count;                 // Just update it
   }                                // (If unchanged, no action required)
}

//----------------------------------------------------------------------------
//
// Method-
//       Semaphore::wait
//       Semaphore::wait(double)
//
// Purpose-
//       Untimed wait
//       Timed wait
//
//----------------------------------------------------------------------------
void
   Semaphore::wait( void )          // Untimed wait
{  std::unique_lock<decltype(mutex)> lock(mutex);

   while( count == 0 )              // Wait, handling spurious wake-ups
     cv.wait(lock);

   --count;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool                                // TRUE iff wait completed (not timeout)
   Semaphore::wait(                 // Timed wait
     double            seconds)     // Maximum wait time, in seconds
{  std::unique_lock<decltype(mutex)> lock(mutex);

   if( seconds > 0.0 ) {            // If delayed operation
     std::chrono::microseconds delta(uint64_t(seconds * 1'000'000.0));
     std::chrono::high_resolution_clock::time_point now=
         std::chrono::high_resolution_clock::now();
     std::chrono::high_resolution_clock::time_point timeout= now + delta;
     while( count == 0 ) {          // Wait, handling spurious wake-ups
       if( cv.wait_until(lock, timeout) == std::cv_status::timeout )
         break;
     }
   }

   if( count == 0 )
     return false;

   --count;
   return true;
}
}  // namespace _LIBPUB_NAMESPACE
