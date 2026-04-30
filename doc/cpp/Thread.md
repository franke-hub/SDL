<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
//
//       This file is free content, distributed under cc by-sa version 4.0
//       with attribution required.
//       (See accompanying file LICENSE.BY_SA-4.0 or the original contained
//       within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)
//
// SPDX-License-Identifier: CC-BY-4.0
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/Thread.md
//
// Purpose-
//       Thread.h reference manual
//
// Last change date-
//       2026/04/17
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Thread.h>
## `pub::Thread`

<!-- ===================================================================== -->
---
#### Thread
A Thread is a separate unit of execution.
All Threads in a process share the same address space.

##### Thread::tlss (Thread Local Storage Struct)
The Thread Local Storage Struct is an internal structure that maintains
Thread state information independently from the Thread.

A thread-local pointer to it is defined in Thread.cpp, tl_tlss.
(This pointer is not available for application use.)
Method Thread::current uses this to quickly locate the current Thread.

##### Member functions

| Method               | Purpose                                             |
| :------------------- | :-------------------------------------------------- |
| [constructor](./pub_thread.md#constructor) | Create a Thread               |
| [destructor](./pub_thread.md#destructor)   | Delete the Thread             |
| [debug](./pub_thread.md#debug)             | Display the Thread state      |
| [static_debug](./pub_thread.md#static_debug) | Display global Thread state |
| [get_handle](./pub_thread.md#get_handle)   | Get the Thread handle         |
| [get_max_threads](./pub_thread.md#get_max) | Get the maximum allowed running Thread count |
| [set_max_threads](./pub_thread.md#set_max) | Set the maximum allowed running Thread count |
| [joinable](./pub_thread.md#joinable)       | Test: Is the Thread joinable? |
| [current](./pub_thread.md#current)         | Get the current Thread*       |
| [sleep](./pub_thread.md#sleep)             | Delay the current Thread      |
| [yield](./pub_thread.md#yield)             | Give up current time slice    |
| [detach](./pub_thread.md#detach)           | Detach execution thread       |
| [join](./pub_thread.md#join)               | Wait for Thread completion    |
| [run](./pub_thread.md#run)                 | Operate the Thread            |
| [start](./pub_thread.md#start)             | Start the Thread              |
| [drive](./pub_thread.md#drive)             | Drive the Thread              |
| [start_failure](./pub_thread.md#failure)   | Handle start failure          |

#### Example

```cpp
#include <cstdio>
#include <pub/Thread.h>

class MyThread : public pub::Thread {
public:
    void run() override {
        printf("Hello from thread\n");
    }
};

int main() {
    MyThread t;
    t.start();
    t.join();
}
```

`start()` launches the thread and returns immediately.
`join()` blocks until `run()` returns.
