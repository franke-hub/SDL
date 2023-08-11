<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/Worker.md
//
// Purpose-
//       Worker.h reference manual
//
// Last change date-
//       2023/07/28
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
