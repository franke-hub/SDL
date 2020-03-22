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
//       Event.h
//
// Purpose-
//       Event (wait/post) implementation.                                                   ts.
//
// Last change date-
//       2018/01/01
//
// Implementation note-
//       An Event is not (currently) an Object.
//
//----------------------------------------------------------------------------
#ifndef OBJ_EVENT_H_INCLUDED
#define OBJ_EVENT_H_INCLUDED

#include <condition_variable>
#include <mutex>
#include <stdint.h>

#include "define.h"                 // For _OBJ_NAMESPACE (and more)

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
//
// Class-
//       Event
//
// Purpose-
//       Event descriptor.
//
//----------------------------------------------------------------------------
class Event {                       // Event descriptor
//----------------------------------------------------------------------------
// Event::Attributes
//----------------------------------------------------------------------------
private:
uint32_t               code;        // (31 bit) Post code
std::condition_variable
                       cv;          // Event driver
std::mutex             mutex;       // Protects cv

//----------------------------------------------------------------------------
// Event::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
   ~Event( void ) {}                // Destructor

inline
   Event( void )                    // Default constructor
:  code(0), cv(), mutex() { }

// Disallowed: Copy constructor, assignment operator
   Event(const Event&) = delete;
Event& operator=(const Event&) = delete;

//----------------------------------------------------------------------------
// Event::Methods
//----------------------------------------------------------------------------
public:
void
   post(                            // Indicate event ready
     uint32_t          code= 0)     // (31 bit) completion code
{  std::unique_lock<decltype(mutex)> lock(mutex);

   this->code= code | 0x80000000;   // (High order bit indicates posted)
   cv.notify_all();
}

void
   reset( void )                    // Reset the Event
{  std::unique_lock<decltype(mutex)> lock(mutex);
   code= 0;
}

int32_t                             // The event code (Always positive)
   wait( void )                     // Wait for Event
{  std::unique_lock<decltype(mutex)> lock(mutex);

   while( !code )                   // Handle spurious wake-ups
     cv.wait(lock);

   return code & 0x7fffffff;        // 31-bit post code
}
}; // class Event
}  // namespace _OBJ_NAMESPACE
#endif // OBJ_EVENT_H_INCLUDED
