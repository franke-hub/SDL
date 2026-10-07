<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
//
//       This file is free content, distributed under cc by-sa version 4.0
//       with attribution required.
//       (See accompanying file LICENSE.BY_SA-4.0 or the original contained
//       within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)
//
// SPDX-License-Identifier: CC-BY-SA-4.0
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/Event.md
//
// Purpose-
//       Event.h reference manual
//
// Last change date-
//       2026/10/07
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Event.h>
## pub::Event

Event.h provides wait/post event objects.
A thread waits for an event, which another thread posts.

Usage notes:
- Once an event is posted, every pending and every future wait completes
immediately until the event is reset.
- Posting an event that's already posted is an (unchecked) error.
- Waiting during static initialization is discouraged, since that
customarily runs in a single thread.
- Event objects have no application-visible attributes.

** <a id="Event_classes">Event Classes</a> **
| Class             | Description
| :---------------- | :--------------------------------------------------- |
| [Event](#Event) | Wait/post event, with a completion code |
| [Yield_event](#Yield_event) | Wait/post event for events expected very soon |

<!-- ===================================================================== -->
## <a id="Event">pub::Event</a>

An Event uses a condition variable for waiting, so a waiting thread does
not use processor time.
When posted, an Event carries a 31-bit completion code, which is returned
by [wait](#wait).

Event objects cannot be copied.

| Method | Purpose |
|--------|---------|
| [has_posted](#has_posted) | Test: has the Event been posted? |
| [post](#post) | Post the Event. |
| [reset](#reset) | Reset the Event for reuse. |
| [wait](#wait) | Wait for the Event to be posted. |

---
#### <a id="has_posted">bool pub::Event::has_posted() const;</a>

Return value: TRUE iff the Event is in the posted state.

---
#### <a id="post">void pub::Event::post(uint32_t code= 0);</a>

Posts the Event with the specified completion `code`, completing all
pending waits.

Only the low-order 31 bits of `code` are kept.

---
#### <a id="reset">void pub::Event::reset();</a>

Resets the Event to its initial (not posted) state.
Subsequent waits wait until the Event is posted again.

---
#### <a id="wait">int32_t pub::Event::wait();</a>

Return value: The (31-bit, never negative) completion code specified when
the Event was posted.

Waits until the Event is posted.
If the Event is already posted, returns immediately.

<!-- ===================================================================== -->
## <a id="Yield_event">pub::Yield_event</a>

A Yield_event is suitable for synchronizing events that are expected to
happen very soon.
Until the Yield_event is posted, a waiting thread repeatedly yields its
time slice, giving posting threads a chance to run.
Unlike an [Event](#Event), a waiting thread therefore uses processor time,
and no completion code is passed.

| Method | Purpose |
|--------|---------|
| [post](#Yield_event-post) | Post the Yield_event. |
| [reset](#Yield_event-reset) | Reset the Yield_event for reuse. |
| [wait](#Yield_event-wait) | Wait for the Yield_event to be posted. |

---
#### <a id="Yield_event-post">void pub::Yield_event::post();</a>

Posts the Yield_event, completing all pending waits.

---
#### <a id="Yield_event-reset">void pub::Yield_event::reset();</a>

Resets the Yield_event to its initial (not posted) state.

---
#### <a id="Yield_event-wait">void pub::Yield_event::wait();</a>

Waits until the Yield_event is posted, yielding the processor while
waiting.
If the Yield_event is already posted, returns immediately.

<!-- ===================================================================== -->
#### Example

```cpp
#include <cstdio>
#include <thread>
#include <pub/Event.h>

int main() {
   pub::Event event;

   std::thread worker([&event]() {
     printf("worker running\n");
     event.post(42);
   });

   int32_t code= event.wait();      // Wait for the worker
   printf("worker posted %d\n", code);
   worker.join();

   event.reset();                   // (Ready for reuse)
}
```

---
[Back](./REFERENCE.md)
