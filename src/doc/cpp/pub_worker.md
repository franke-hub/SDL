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
//       ~/src/doc/cpp/pub_worker.md
//
// Purpose-
//       Worker.h reference manual
//
// Last change date-
//       2023/06/30
//
-------------------------------------------------------------------------- -->
## pub::Worker::work; pub::WorkerPool:: get_running, reset, work

###### Defined in header &lt;pub/Worker.h&gt

---
#### virtual void pub::Worker::work(void);
The Worker interface.
`WorkerPool::work(Worker* worker)` invokes `worker->work()`.

When Worker::work returns:
- if there is room in the (static internal) thread pool,
the associated thread is added to the thread pool.
- otherwise the thread completes.

---
#### static unsigned pub::WorkerPool::get_running(void);
Returns the (instantaneous) count of running Worker threads.

---
#### static void pub::WorkerPool::reset(void);
Resets (empties) the WorkerPool thread pool.

Currently running Worker tasks are not affected.

---
#### static void pub::WorkerPool::work(Worker* worker);
If there is a thread available in the (static internal) thread pool,
that thread is removed from the pool and selected.
Otherwise a new thread is created and selected.

The selected thread is then used to invoke `worker->work()`.

Implementation note: WorkerPool does not limit the number of concurrent
threads. It only limits the number of threads that can be reused.
