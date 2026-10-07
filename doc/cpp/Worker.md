<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023-2026 Frank Eskesen.
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
//       ~/doc/cpp/Worker.md
//
// Purpose-
//       Worker.h reference manual
//
// Last change date-
//       2026/10/07
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Worker.h>
## pub::Worker

Worker.h provides a thread pool.
An application implements a Worker, and the WorkerPool drives its
`work` method using a pooled thread.

Usage notes:
- [pub::dispatch::Task](./pub_disp-task.md) is a Worker.
- WorkerPool does not limit the number of concurrently running Workers.
It only limits the number of idle threads kept in the pool for reuse.
- WorkerPool does not prevent a Worker from being scheduled while it's
already running. If it is, its `work` method runs concurrently on two
threads.

** <a id="Worker_classes">Worker Classes</a> **
| Class             | Description
| :---------------- | :--------------------------------------------------- |
| [Worker](#Worker) | The Worker interface |
| [WorkerPool](#WorkerPool) | The thread pool that drives Workers |

<!-- ===================================================================== -->
#### <a id="Worker">[Worker](./pub_worker.md#Worker)</a>
Worker is an interface class with no application-visible attributes.

| Method | Purpose |
|--------|---------|
| [work](./pub_worker.md#work) | Process work. **Override this method.** |

<!-- ===================================================================== -->
#### <a id="WorkerPool">[WorkerPool](./pub_worker.md#WorkerPool)</a>
All WorkerPool methods are static. There is one process-wide WorkerPool.

| Method | Purpose |
|--------|---------|
| [debug](./pub_worker.md#debug) | Display WorkerPool statistics. |
| [get_max_running](./pub_worker.md#get_max_running) | Get the maximum number of concurrently running Workers. |
| [get_max_threads](./pub_worker.md#get_max_threads) | Get the maximum number of concurrent Worker threads. |
| [get_max_used](./pub_worker.md#get_max_used) | Get the maximum number of pooled threads. |
| [get_running](./pub_worker.md#get_running) | Get the number of running Workers. |
| [get_size](./pub_worker.md#get_size) | Get the thread pool size. |
| [get_threads](./pub_worker.md#get_threads) | Get the number of Worker threads. |
| [get_used](./pub_worker.md#get_used) | Get the number of pooled threads. |
| [get_worker_creates](./pub_worker.md#get_worker_creates) | Get the number of Worker threads created. |
| [get_worker_deletes](./pub_worker.md#get_worker_deletes) | Get the number of Worker threads deleted. |
| [get_workers](./pub_worker.md#get_workers) | Get the number of work invocations. |
| [reset](./pub_worker.md#reset) | Empty the thread pool and reset the statistics. |
| [set_size](./pub_worker.md#set_size) | Set the thread pool size. |
| [work](./pub_worker.md#WorkerPool-work) | Drive a Worker's work method. |

<!-- ===================================================================== -->
#### Example

```cpp
#include <cstdio>
#include <pub/Event.h>
#include <pub/Worker.h>

class MyWorker : public pub::Worker {
public:
pub::Event             done;        // Posted when work completes

virtual void
   work( void ) override
{
   printf("Hello from worker thread\n");
   done.post();
}
}; // class MyWorker

int main() {
   MyWorker w;
   pub::WorkerPool::work(&w);       // (Returns immediately)
   w.done.wait();                   // Wait for completion
   w.done.wait();                   // (Already posted: does not wait)

   w.done.reset();                  // (Allows reuse)
   pub::WorkerPool::work(&w);
   w.done.wait();                   // Wait for completion
}
```

See [Event](./Event.md) for the Event wait/post/reset rules.

---
[Back](./REFERENCE.md)
