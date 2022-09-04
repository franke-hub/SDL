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
//       Semaphore.h
//
// Purpose-
//       Semaphore implemenentation using condition variable.
//
// Last change date-
//       2022/09/02
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_SEMAPHORE_H_INCLUDED
#define _LIBPUB_SEMAPHORE_H_INCLUDED

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdint.h>                 // For uint64_t

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

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
unsigned               count;       // Post counter
std::condition_variable
                       cv;          // Event driver
std::mutex             mutex;       // Protects cv

//----------------------------------------------------------------------------
// Semaphore::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
inline
   ~Semaphore( void ) {}            // Destructor

   Semaphore(                       // Default constructor
     unsigned          count= 0)    // Default, count= 0
:  count(count), cv(), mutex() {}

// Disallowed: Copy constructor, assignment operator
   Semaphore(const Semaphore&) = delete;
Semaphore& operator=(const Semaphore&) = delete;

//----------------------------------------------------------------------------
// Semaphore::Methods
//----------------------------------------------------------------------------
public:
unsigned
   get_count( void ) const          // Get current count
{  return count; }

void
   post( void )                     // Indicate resource available
{  std::unique_lock<decltype(mutex)> lock(mutex);

   count++;
   cv.notify_one();
}

void
   reset( void )                    // Reset the Semaphore
{  count= 0; }

void
   wait( void )                     // Wait for resource
{  std::unique_lock<decltype(mutex)> lock(mutex);

   while( !count )                  // Handle spurious wake-ups
     cv.wait(lock);

   count--;
}

bool                                // TRUE iff semaphore available
   wait(                            // Wait for resource
     double            seconds)     // Timeout delay, in seconds
{  std::unique_lock<decltype(mutex)> lock(mutex);

   if( seconds > 0.0 ) {            // If delayed operation
     std::chrono::microseconds delta(uint64_t(seconds * 1000000.0));
     std::chrono::high_resolution_clock::time_point now=
         std::chrono::high_resolution_clock::now();
     std::chrono::high_resolution_clock::time_point timeout= now + delta;
     while( !count ) {              // Handle spurious wake-ups
       if( cv.wait_until(lock, timeout) == std::cv_status::timeout )
         break;
     }
   }

   if( count == 0 )
     return false;

   count--;
   return true;
}
}; // class Semaphore
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_SEMAPHORE_H_INCLUDED
