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
//       ~/doc/cpp/pub_disp-task.md
//
// Purpose-
//       Dispatch.h reference manual: Task
//
// Last change date-
//       2026/10/07
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Dispatch.h>
## pub::dispatch::Task

A Task processes the work Items on its work queue one at a time, in the
order they were enqueued.
Any number of threads may enqueue work Items simultaneously, without
locking.

A Task runs single threaded but without thread affinity: `work(Item*)` is
never invoked again until it returns, but the thread that invokes it may
differ from one invocation to the next.

Task extends [pub::Worker](./Worker.md). It has no application-visible
attributes, and Tasks cannot be copied.

<!-- ===================================================================== -->
---
#### <a id="destructor">virtual pub::dispatch::Task::~Task();</a>

If the Task has enqueued work Items, the destructor waits until they
complete. A Task is never deleted while it has outstanding work Items.

__Note__ Do not delete a Task from within its own `work(Item*)` method.
The destructor would wait for the running `work(Item*)` method to complete,
which it never does.
This can occur unexpectedly, for example when a work Item contains the last
shared_ptr reference to the Task (or to an object containing the Task) and
posting it deletes that reference.
Use [Disp::post](./pub_disp-disp.md#post) rather than
[Item::post](./pub_disp-item.md#post) to complete such work Items.

---
#### <a id="enqueue">void pub::dispatch::Task::enqueue(Item* item);</a>

Enqueues the work `item` onto this Task's work queue, scheduling the Task
if it isn't already running.
This operation is thread safe and lock free.
Any number of threads may simultaneously enqueue work Items.

---
#### <a id="is_running">bool pub::dispatch::Task::is_running();</a>

Return value: TRUE if the Task has work Items enqueued or in process.

This method has limited utility: the status may change before the method
returns.

---
#### <a id="work">protected: virtual void pub::dispatch::Task::work(Item* item);</a>

Processes the work `item`. **Override this method.**
(A [LambdaTask](./pub_disp-lambda.md#LambdaTask) provides an alternative
to overriding it.)

When it's finished with the work Item, `work` either completes it by
invoking `item->post()` or enqueues it onto another (or even the same)
Task.

Only work Items with non-negative function codes are passed to `work`.

The default implementation simply completes the Item, invoking
`item->post()`.

---
[Back](./Dispatch.md) | [Reference manual](./REFERENCE.md)
