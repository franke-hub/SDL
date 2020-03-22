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
// Implementation note-
//       A Semaphore is not (currently) an Object.
//
//----------------------------------------------------------------------------
#ifndef OBJ_SEMAPHORE_H_INCLUDED
#define OBJ_SEMAPHORE_H_INCLUDED

#include <condition_variable>
#include <mutex>

#include "define.h"                 // For _OBJ_NAMESPACE (and more)

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Semaphore
//
// Purpose-
//       Semaphore descriptor.
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

inline
   Semaphore( void )                // Default constructor
:  count(0), cv(), mutex() {}

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
}; // class Semaphore
}  // namespace _OBJ_NAMESPACE
#endif // OBJ_SEMAPHORE_H_INCLUDED
