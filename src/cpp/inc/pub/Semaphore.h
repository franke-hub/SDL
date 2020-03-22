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
//       Semaphore.h
//
// Purpose-
//       Semaphore implemenentation using condition variable.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef _PUB_SEMAPHORE_H_INCLUDED
#define _PUB_SEMAPHORE_H_INCLUDED

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <stdint.h>                 // For uint64_t

#include "config.h"                 // For _PUB_NAMESPACE

namespace _PUB_NAMESPACE {
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
void
   post( void )                     // Indicate resource available
{  std::unique_lock<decltype(mutex)> lock(mutex);

   count++;
   cv.notify_one();
}

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

   if( seconds > 0.0 )              // If delayed operation
   {
     std::chrono::microseconds delta(uint64_t(seconds * 1000000.0));
     std::chrono::high_resolution_clock::time_point now=
         std::chrono::high_resolution_clock::now();
     std::chrono::high_resolution_clock::time_point timeout= now + delta;
     while( !count )                // Handle spurious wake-ups
     {
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
}  // namespace _PUB_NAMESPACE
#endif // _PUB_SEMAPHORE_H_INCLUDED
