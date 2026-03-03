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
//       Event.h
//
// Purpose-
//       Event (wait/post) implementations.
//
// Last change date-
//       2026/03/01
//
// Implementation notes-
//       Use of the wait method during static initialization is discouraged,
//       since that customarily runs in a single thread.
//
//       Invoking method post when already posted is an (unchecked) error.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_EVENT_H_INCLUDED
#define _LIBPUB_EVENT_H_INCLUDED

#include <atomic>                   // For std::atomic_int32_t
#include <condition_variable>       // For std::condition_variable
#include <mutex>                    // For std::mutex
#include <thread>                   // For std::this_thread::yield

#include <cstdint>                  // For uint32_t

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
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
//       Yield_event
//
// Purpose-
//       Yield_event descriptor.
//
// Implementation note-
//       The Yield_event is suitable for synchonizing events expected to
//       happen very close in time.
//       Until posted, Yield_event::wait() yields its time slice, giving
//       posting threads a chance to run.
//
//----------------------------------------------------------------------------
struct Yield_event {                // Yield_event descriptor
//----------------------------------------------------------------------------
// Yield_event::Attributes
//----------------------------------------------------------------------------
std::atomic_int32_t    latch= 0;    // Initial value: NOT POSTED

//----------------------------------------------------------------------------
// Yield_event::Methods
//----------------------------------------------------------------------------
void
   post( void )                     // Post event completion
{  latch.store(1); }

void
   reset( void )                    // Reset the Yield_event
{  latch.store(0); }

void
   wait( void )                     // Wait for Event
{  while( latch.load() == 0 ) std::this_thread::yield(); }
}; // struct Yield_event
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_EVENT_H_INCLUDED
