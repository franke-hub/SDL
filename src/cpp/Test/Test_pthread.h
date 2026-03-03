//----------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
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
//       Test_pthread.h
//
// Purpose-
//       Include file for Test_pthread.cpp
//
// Last change date-
//       2026/03/03
//
//----------------------------------------------------------------------------
#ifndef TEST_PTHREAD_H_INCLUDED
#define TEST_PTHREAD_H_INCLUDED

#include <atomic>                   // For std::atomic_int32_t
#include <condition_variable>       // For std::condition_variable
#include <mutex>                    // For std::mutex
#include <thread>                   // For std::this_thread::yield

#include <cstdint>                  // For uint32_t

//----------------------------------------------------------------------------
//
// Struct-
//       atomic_double
//
// Purpose-
//       Extend std::atomic_double with needed arithmetic operators.
//
//----------------------------------------------------------------------------
struct atomic_double : std::atomic<double> { // Atomic double value
//----------------------------------------------------------------------------
// atomic_double::Constructors/destructor
//----------------------------------------------------------------------------
public:
   atomic_double( void ) noexcept   // Default constructor
:  std::atomic<double>()
{  }

   atomic_double(double D) noexcept // Value constructor
:  std::atomic<double>(D)
{  }

// Destructor declaration not required

//----------------------------------------------------------------------------
// atomic_double::Operators
//----------------------------------------------------------------------------
// A complete implementation would need all arithmetic operators, but we only
// need the += operator
atomic_double&
   operator+=(double rhs) noexcept
{
   double _old= load();             // (The current value)
   for(;;) {                        // Atomic add
     double _new= _old + rhs;
     int cc= compare_exchange_strong(_old, _new);
     if( cc )
       break;
   }

   return *this;
}
}; // struct atomic_double

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
// Class-
//       Event
//
// Purpose-
//       Event descriptor.
//
// Implementation note-
//       Duplicated for performance testing.
//
//----------------------------------------------------------------------------
class Event {                       // Event descriptor
//----------------------------------------------------------------------------
// Event::Attributes
//----------------------------------------------------------------------------
private:
volatile uint32_t      code;        // (31 bit) Post code
std::condition_variable
                       cv;          // Event driver
std::mutex             mutex;       // Protects cv

//----------------------------------------------------------------------------
// Event::Constructors/Assignment/Destructor
//----------------------------------------------------------------------------
public:
   Event( void )                    // Default constructor
:  code(0), cv(), mutex() { }

   Event(const Event&) = delete;    // Disallowed copy constructor
Event& operator=(const Event&) = delete; // Disallowed assignment operator

   ~Event( void ) = default;        // Destructor

//----------------------------------------------------------------------------
// Event::Accessor methods
//----------------------------------------------------------------------------
bool                                // TRUE iff posted
   has_posted( void ) const         // Is Event in posted state?
{  return code & 0x8000'0000; }     // (High order bit indicates posted)

//----------------------------------------------------------------------------
// Event::Methods
//----------------------------------------------------------------------------
void
   post(                            // Post event completion
     uint32_t          code= 0)     // (31 bit) completion code
{  std::unique_lock<decltype(mutex)> lock(mutex);

   this->code= code | 0x8000'0000;   // (High order bit indicates posted)
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

   while( !has_posted() )           // Handle spurious wake-ups
     cv.wait(lock);

   return code & 0x7fff'ffff;       // 31-bit post code
}
}; // class Event

//----------------------------------------------------------------------------
//
// Struct-
//       Event_yield
//
// Purpose-
//       Event_yield descriptor.
//
// Implementation note-
//       The Event_yield is suitable for synchonizing events expected to
//       happen very close in time.
//       Until posted, Event_yield::wait() yields its time slice, giving
//       posting threads a chance to run.
//
//----------------------------------------------------------------------------
struct Event_yield {                // Event_yield descriptor
//----------------------------------------------------------------------------
// Event_yield::Attributes
//----------------------------------------------------------------------------
std::atomic_int32_t    latch= 0;    // Initial value: NOT POSTED

//----------------------------------------------------------------------------
// Event_yield::Methods
//----------------------------------------------------------------------------
void
   post( void )                     // Post event completion
{  latch.store(1); }

void
   reset( void )                    // Reset the Event_yield
{  latch.store(0); }

void
   wait( void )                     // Wait for Event
{  while( latch.load() == 0 ) std::this_thread::yield(); }
}; // struct Event_yield
#endif // TEST_PTHREAD_H_INCLUDED
