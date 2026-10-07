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
//       ~/doc/cpp/pub_worker.md
//
// Purpose-
//       Worker.h reference manual
//
// Last change date-
//       2026/10/07
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Worker.h>
## <a id="Worker">pub::Worker</a>

Worker is an interface class with no application-visible attributes.
Applications implement its `work` method in a subclass, then use
[WorkerPool::work](#WorkerPool-work) to drive it.

---
#### <a id="work">virtual void pub::Worker::work() = 0;</a>

Processes work. **Override this method.**

This method runs on a WorkerPool thread.
If it throws an exception, the exception is caught and reported, and is
otherwise treated as if `work` returned normally.

The Worker must remain valid until `work` returns.

<!-- ===================================================================== -->
## <a id="WorkerPool">pub::WorkerPool</a>

WorkerPool manages a process-wide thread pool.
All methods are static and there are no application-visible attributes.

When [work](#WorkerPool-work) is invoked, a thread is removed from the pool
or, if the pool is empty, a new thread is created.
That thread invokes the Worker's `work` method.
When `work` returns, the thread is added to the pool if there's room.
Otherwise, the thread terminates.

The pool initially contains no threads, and its default size is 16.

The statistics returned by the `get_` methods are instantaneous values:
they may change before the method returns.

<!-- ===================================================================== -->
---
#### <a id="debug">static void pub::WorkerPool::debug(const char* info= "", bool detail= false);</a>

Displays WorkerPool statistics.
`info` is an optional caller-supplied identifying string included in the
display.
If `detail` is TRUE, information about each pooled thread is also
displayed.

---
#### <a id="get_max_running">static size_t pub::WorkerPool::get_max_running();</a>

Return value: The maximum number of concurrently running Workers.

---
#### <a id="get_max_threads">static size_t pub::WorkerPool::get_max_threads();</a>

Return value: The maximum number of concurrently existing Worker threads,
both running and pooled.

---
#### <a id="get_max_used">static size_t pub::WorkerPool::get_max_used();</a>

Return value: The maximum number of threads that have been in the pool at
the same time.

---
#### <a id="get_running">static size_t pub::WorkerPool::get_running();</a>

Return value: The number of currently running Workers.

---
#### <a id="get_size">static size_t pub::WorkerPool::get_size();</a>

Return value: The thread pool size, the maximum number of idle threads kept
for reuse.

---
#### <a id="get_threads">static size_t pub::WorkerPool::get_threads();</a>

Return value: The current number of Worker threads, both running and
pooled.

---
#### <a id="get_used">static size_t pub::WorkerPool::get_used();</a>

Return value: The number of threads currently in the pool.

---
#### <a id="get_worker_creates">static size_t pub::WorkerPool::get_worker_creates();</a>

Return value: The number of Worker threads created.

---
#### <a id="get_worker_deletes">static size_t pub::WorkerPool::get_worker_deletes();</a>

Return value: The number of Worker threads deleted.

---
#### <a id="get_workers">static size_t pub::WorkerPool::get_workers();</a>

Return value: The number of [work](#WorkerPool-work) invocations.

---
#### <a id="reset">static void pub::WorkerPool::reset();</a>

Empties the thread pool, terminating the pooled threads, and resets all
statistics to zero.

Currently running Workers are not affected.
When they complete, their threads are again added to the pool.

---
#### <a id="set_size">static void pub::WorkerPool::set_size(size_t size);</a>

Empties the thread pool, terminating the pooled threads, then sets the pool
size to `size`.

`set_size(0)` is valid. Threads then terminate when their Workers complete,
rather than being pooled.

This method is thread safe and can be invoked at any time.

---
#### <a id="WorkerPool-work">static void pub::WorkerPool::work(Worker* worker);</a>

Invokes `worker->work()` using a pooled thread, or using a new thread if
the pool is empty.

This method returns immediately, without waiting for `work` to complete.

---
[Back](./Worker.md) | [Reference manual](./REFERENCE.md)
