<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023-2026 Frank Eskesen.
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
//       ~/doc/cpp/pub_disp-disp.md
//
// Purpose-
//       Dispatch.h reference manual: Disp
//
// Last change date-
//       2026/04/25
//
-------------------------------------------------------------------------- -->
## pub::dispatch::Disp
###### Defined in header <pub/Dispatch.h>

<!-- ===================================================================== -->
## <a id="class-disp">class pub\::dispatch\::Disp</a>

The Disp class provides a set of utility functions implemented as static
methods.

### *Attributes*

(None defined.)

### *Methods*

#### <a id="construct">void pub::dispatch::Disp::Disp(void) = delete;</a>

There is no constructor. All methods are static.

#### <a id="delay">void* pub::dispatch::Disp::delay(double seconds, Item* item);</a>

Return value: A token that can be used by cancel() to cancel the delay.

Schedules a delay. The delay is not associated with any Task.
When the delay expires or is cancelled, Dispatch invokes item->post(), which
then invokes item->done->done(Item*).

#### <a id="cancel">bool pub::dispatch::Disp::cancel(void* token);</a>

Return value: TRUE iff the delay was cancelled.

Cancels the delay specified by token, the return value from delay().

An application using cancel can also differentiate whether or not the delay
completed or was cancelled using the Item condition code (Item.cc):
- Timeout delays are posted with CC_NORMAL.
- Canceled delays are posted with CC_PURGED.

#### <a id="shutdown">void pub::dispatch::Disp::shutdown(void);</a>

This terminates the Dispatcher's (internal) delay notification thread.
All incomplete delay operations complete with item.cc= CC_PURGE.

Any subsequent delay operation restarts the delay notification thread.

#### <a id="post">void pub::dispatch::Disp::post(Item* item, int cc);</a>

The Item is posted under control of a (hidden) Dispatch Task.

This feature is useful under certain circumstances.

Consider an Item containing a shared_ptr reference to the running Task.
When the Task destructor discovers that the Task is running, it issues a
CHASE operation and waits for it to complete, insuring that all queued Items
have completed.
If posting the Item removes the last shared_ptr (deleting the running Task,)
a livelock occurs because the CHASE is blocked by the running Task.

Moving the post operation to a different task prevents this livelock.

#### <a id="enqueue">void pub::dispatch::Disp::enqueue(Task* task, Item* item);</a>

Enqueue the specified work item onto the specified Task.
This has the same effect as `task->enqueue(item)`.
