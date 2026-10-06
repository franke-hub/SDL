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
//       ~/doc/cpp/pub_disp-disp.md
//
// Purpose-
//       Dispatch.h reference manual: Disp
//
// Last change date-
//       2026/09/30
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Dispatch.h>
## pub::dispatch::Disp

Disp provides a set of Dispatcher utility functions.
All methods are static: there is no constructor and there are no
application-visible attributes.

<!-- ===================================================================== -->
---
#### <a id="cancel">static bool pub::dispatch::Disp::cancel(void* token);</a>

Return value: TRUE iff the delay was cancelled.

Cancels the delay specified by `token`, the return value from
[delay](#delay).
A cancelled delay's work Item completes with completion code
`Item::CC_PURGE`.

An application can determine whether a delay expired or was cancelled
using either the return value from cancel or the Item's completion code:
- `Item::CC_NORMAL`: The delay expired.
- `Item::CC_PURGE`: The delay was cancelled.

---
#### <a id="delay">static void* pub::dispatch::Disp::delay(double seconds, Item* item);</a>

Return value: A token that can be used by [cancel](#cancel) to cancel the
delay.

Schedules a delay. The delay is not associated with any Task.
After `seconds` seconds, the `item` completes (`item->post()` is invoked)
with completion code `Item::CC_NORMAL`.

---
#### <a id="enqueue">static void pub::dispatch::Disp::enqueue(Task* task, Item* item);</a>

Enqueues the work `item` onto the specified `task`.
This has the same effect as `task->enqueue(item)`, which applications should
generally use instead.

---
#### <a id="post">static void pub::dispatch::Disp::post(Item* item, int cc= 0);</a>

Completes the work `item` with completion code `cc`, the same as
`item->post(cc)` except that the post operation runs under control of an
internal Dispatch Task rather than the caller's.

This is needed when posting an Item could delete the running Task.
Consider an Item containing a shared_ptr reference to the running Task.
When the Task destructor discovers that the Task is running, it waits for all
enqueued Items to complete.
If posting the Item removes the last shared_ptr reference (deleting the
running Task), a livelock occurs: the destructor waits for the running Task,
which is running the destructor.

Using Disp::post moves the post operation to a different Task, preventing
this livelock.

---
#### <a id="shutdown">static void pub::dispatch::Disp::shutdown();</a>

Terminates Dispatcher delay processing.
All incomplete [delay](#delay) operations complete with completion code
`Item::CC_PURGE`.

A subsequent delay operation restarts delay processing.

Applications are not required to invoke shutdown.
However, no delay cleanup occurs when a process completes:
pending delay Items are not posted.
