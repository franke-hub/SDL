<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023-2024 Frank Eskesen.
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
//       ~/doc/cpp/Worker.md
//
// Purpose-
//       Worker.h reference manual
//
// Last change date-
//       2024/04/17
//
-------------------------------------------------------------------------- -->
## pub::Worker
\#include <pub/Worker.h>

Worker is the base class for (pub::)dispatch::Task.

<!-- ===================================================================== -->
---
#### Worker
Worker is an interface class.
Workers are scheduled using the WorkerPool

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_worker.md) | None. This is an interface class. |
| [work](./pub_worker.md) | Process work |

<!-- ===================================================================== -->
---
#### WorkerPool
All WorkerPool methods are static. There is one process-wide WorkerPool.

WorkerPool manages a thread pool.
When work is scheduled (by WorkerPool::work) a thread is taken from the pool.
If the thread pool is empty, a new thread is created.
When a Worker completes (i.e. Worker::work returns,) its thread is added
to the available thread pool.
The maximum number of pooled threads is implementation defined.

An application can empty the Worker thread pool at any time.
Workers that were running repopulate the thread pool when they complete.

##### Member functions

| Method | Purpose |
|--------|---------|
| [(constructor)](./pub_worker.md) | None. All methods are static. |
| [get_running](./pub_worker.md) | Get the number of running Worker threads. |
| [reset](./pub_worker.md) | Reset (empty) the Worker thread pool. |
| [work](./pub_worker.md) | Drive a Worker's work method. |

#### Example

```cpp
#include <cstdio>
#include <pub/Event.h>
#include <pub/Worker.h>

class MyWorker : public pub::Worker {
public:
    pub::Event done;

    void work() override {
        printf("Hello from worker thread\n");
        done.post();
    }
};

int main() {
    MyWorker w;
    pub::WorkerPool::work(&w);
    w.done.wait();
    w.done.wait();  // (Does not block)

    w.done.reset(); // (Allows reuse)
    pub::WorkerPool::work(&w);
    w.done.wait();  // (Blocks until complete)
}
```

WorkerPool::work() schedules the worker on a pooled thread and returns
immediately.
The pub::Event blocks until the worker signals completion.

Event semantics: Once post() is called, all pending wait() calls unblock and
any subsequent wait() calls return immediately.
The Event stays in the posted state until its reset method is invoked.
The reset() method allows the Event to be reused. (Method wait() blocks.)
