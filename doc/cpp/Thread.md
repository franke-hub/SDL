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
//       ~/doc/cpp/Thread.md
//
// Purpose-
//       Thread.h reference manual
//
// Last change date-
//       2026/10/07
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Thread.h>
## pub::Thread

A Thread is a separate unit of execution.
All Threads in a process share the same address space.
Applications implement the Thread's `run` method in a subclass, then
[start](./pub_thread.md#start) the Thread.

Usage notes:
- A started Thread must be either [joined](./pub_thread.md#join) or
[detached](./pub_thread.md#detach). A joined Thread may be started again.
- Only the running Thread itself may detach, and a Thread cannot join
itself.
- If `run` throws an exception, the exception is caught, a message and a
backtrace are written, and the Thread otherwise completes normally.
- Thread::tlss is an internal structure, not for application use.
- Threads cannot be copied.

<!-- ===================================================================== -->
| Method | Purpose |
|--------|---------|
| [constructor](./pub_thread.md#constructor) | Create an idle Thread. |
| [destructor](./pub_thread.md#destructor) | Delete the Thread. |
| [current](./pub_thread.md#current) | Get the current Thread. |
| [debug](./pub_thread.md#debug) | Display the Thread's state. |
| [detach](./pub_thread.md#detach) | Detach the running Thread. |
| [get_handle](./pub_thread.md#get_handle) | Get the native thread handle. |
| [get_max_threads](./pub_thread.md#get_max_threads) | Get the maximum number of running Threads. |
| [join](./pub_thread.md#join) | Wait for the Thread to complete. |
| [joinable](./pub_thread.md#joinable) | Test: Is the Thread joinable? |
| [run](./pub_thread.md#run) | Operate the Thread. **Override this method.** |
| [set_max_threads](./pub_thread.md#set_max_threads) | Set the maximum number of running Threads. |
| [sleep](./pub_thread.md#sleep) | Delay the current thread. |
| [start](./pub_thread.md#start) | Start the Thread. |
| [static_debug](./pub_thread.md#static_debug) | Display Thread statistics. |
| [yield](./pub_thread.md#yield) | Give up the current time slice. |

<!-- ===================================================================== -->
#### Example

```cpp
#include <cstdio>
#include <pub/Thread.h>

class MyThread : public pub::Thread {
public:
virtual void
   run( void ) override
{  printf("Hello from thread\n"); }
}; // class MyThread

int main() {
   MyThread t;
   t.start();                       // (Returns immediately)
   t.join();                        // Wait until run() returns
}
```

---
[Back](./REFERENCE.md)
