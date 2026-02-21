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
//       Semaphore.h
//
// Purpose-
//       Semaphore implementation using condition variable.
//
// Last change date-
//       2026/02/09
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_SEMAPHORE_H_INCLUDED
#define _LIBPUB_SEMAPHORE_H_INCLUDED

#include <chrono>                   // For std::chrono
#include <condition_variable>       // For std::condition_variable
#include <mutex>                    // For std::mutex
#include <cstdint>                  // For uint64_t

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Semaphore
//
// Purpose-
//       Semaphore descriptor.
//
// Implementation notes-
//       A Semaphore is not (currently) an Object.
//       Semaphore::post is known as P in the literature
//       Semaphore::wait is known as V in the literature
//
//----------------------------------------------------------------------------
class Semaphore {                   // Semaphore descriptor
//----------------------------------------------------------------------------
// Semaphore::Attributes
//----------------------------------------------------------------------------
private:
size_t                 count;       // Post counter
std::condition_variable
                       cv;          // Event driver
mutable std::mutex     mutex;       // Protects cv and count

//----------------------------------------------------------------------------
// Semaphore::Constructors/asssignment/destructor
//----------------------------------------------------------------------------
public:
   Semaphore(                       // Default constructor
     size_t            _count= 0);  // Default, count= 0

   Semaphore(const Semaphore&) = delete; // Disallowed copy constructor

Semaphore&
   operator=(const Semaphore&) = delete; // Disallowed assignment operator

   ~Semaphore( void );              // Destructor

//----------------------------------------------------------------------------
// Semaphore::debug
//----------------------------------------------------------------------------
void
   debug(const char* info= nullptr) const; // Debugging display

//----------------------------------------------------------------------------
// Semaphore::Methods
//----------------------------------------------------------------------------
size_t
   get_count( void ) const          // Get current count
{  return count; }

void
   post( void );                    // Indicate resource available

void
   reset(                           // Reset the Semaphore
     size_t            _count= 0);  // To this post count

void
   wait( void );                    // Wait for resource

bool                                // TRUE iff semaphore available
   wait(                            // Wait for resource
     double            seconds);    // Timeout delay, in seconds
}; // class Semaphore
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_SEMAPHORE_H_INCLUDED
