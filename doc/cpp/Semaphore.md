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
//       ~/doc/cpp/Semaphore.md
//
// Purpose-
//       Semaphore.h reference manual
//
// Last change date-
//       2026/10/07
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Semaphore.h>
## pub::Semaphore

A Semaphore is a counting semaphore.
It controls access to a set of resources: [post](#post) makes a resource
available, and [wait](#wait) obtains one, waiting if none is available.

Usage notes:
- The Semaphore uses a condition variable for waiting, so a waiting thread
does not use processor time.
- Each post satisfies at most one wait. Posts made when no thread is waiting
are counted, and satisfy later waits.
- Do not delete a Semaphore while any thread is waiting for it.
- The Semaphore has no application-visible attributes.
Semaphores cannot be copied.

| Method | Purpose |
|--------|---------|
| [constructor](#constructor) | Create the Semaphore, specifying its initial count. |
| [get_count](#get_count) | Get the (instantaneous) available count. |
| [post](#post) | Make a resource available. |
| [reset](#reset) | Set the available count. |
| [wait](#wait) | Obtain a resource, waiting until one is available. |
| [wait(double)](#wait-timed) | Obtain a resource, waiting up to a timeout. |

<!-- ===================================================================== -->
---
#### <a id="constructor">pub::Semaphore::Semaphore(size_t count= 0);</a>

Creates the Semaphore with `count` resources available.

---
#### <a id="get_count">size_t pub::Semaphore::get_count() const;</a>

Return value: The number of resources currently available.

This method has limited utility: the count may change before the method
returns.

---
#### <a id="post">void pub::Semaphore::post();</a>

Makes one resource available, completing one waiting [wait](#wait) if any.

(In the literature, post is known as V.)

---
#### <a id="reset">void pub::Semaphore::reset(size_t count= 0);</a>

Sets the number of available resources to `count`.
If `count` is not zero, waiting threads may then obtain resources.

---
#### <a id="wait">void pub::Semaphore::wait();</a>

Obtains one resource, waiting until one is available.

(In the literature, wait is known as P.)

---
#### <a id="wait-timed">bool pub::Semaphore::wait(double seconds);</a>

Return value: TRUE iff a resource was obtained.

Obtains one resource, waiting at most `seconds` seconds for one to become
available.
If `seconds` is zero or negative, does not wait: a resource is obtained
only if one is immediately available.

<!-- ===================================================================== -->
#### Example

```cpp
#include <cstdio>
#include <thread>
#include <pub/Semaphore.h>

int main() {
   pub::Semaphore ready;            // (Initial count 0)

   std::thread worker([&ready]() {
     printf("worker running\n");
     ready.post();                  // Signal main
   });

   if( !ready.wait(5.0) )           // Wait up to 5 seconds
     printf("worker timed out\n");
   else
     printf("worker signaled\n");
   worker.join();
}
```

---
[Back](./REFERENCE.md)
