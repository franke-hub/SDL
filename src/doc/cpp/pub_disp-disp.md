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
//       ~/src/doc/cpp/pub_disp-disp.md
//
// Purpose-
//       Dispatch.h reference manual: Disp
//
// Last change date-
//       2023/06/21
//
-------------------------------------------------------------------------- -->
## (pub::dispatch::Disp::)cancel, delay, enqueue, post, shutdown

###### Defined in header &lt;pub/Dispatch.h&gt

<!-- ===================================================================== -->
---
#### void pub::dispatch::Disp::Disp(void) = delete;

There is no constructor. All methods are static.

---
#### void pub::dispatch::Disp::cancel(void* token);

Cancels the delay specified by token, the return value from delay().

__Note__: An application must be prepared to handle a cancelled delay.
(The associated delay Item may have already been posted when the cancel
operation is processed.)

Delays completing normally are posted with CC_NORMAL. Those that are
cancelled get CC_PURGED.

---
#### void* pub::dispatch::Disp::delay(double seconds, Item* item);

Schedules a delay. Note that the delay is not associated with any Task.
When the delay expires, Dispatch invokes item->post(), which then invokes
item->done->done(Item*).

Return value: The return value is a token that can be used by cancel()
to delete the delay.

---
#### void pub::dispatch::Disp::post(Item* item, int cc);

The Item is posted under control of an internal Dispatch Task.

An application may require this feature under certain circumstances.
For example, if posting an Item might cause the running Task to be deleted,
a livelock can occur when that Item is posted.
Moving the post operation to a different task prevents this livelock.

---
#### void pub::dispatch::Disp::enqueue(Task* task, Item* item);

Enqueue the specified work item onto the specified Task.
(This has the same effect as `task->enqueue(item)`.)

---
#### void pub::dispatch::Disp::shutdown(void);

Terminate the Dispatcher's (internal) delay notification thread.
All currently scheduled delay operations are posted with the CC_PURGE
completion code.

Any subsequent delay operation restarts the delay notification thread.
