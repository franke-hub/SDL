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
//       ~/doc/cpp/pub_disp-done.md
//
// Purpose-
//       Dispatch.h reference manual: Done, Wait
//
// Last change date-
//       2026/09/30
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Dispatch.h>
## <a id="Done">pub::dispatch::Done</a>

Done is the work Item completion handler.
When a work Item whose `done` field points to a Done object is
[posted](./pub_disp-item.md#post), that Done object's `done` method is
invoked.

A Done object may be shared: any number of work Items may use the same
Done object simultaneously.
Since those Items may complete on different threads, a shared Done object's
`done` method must allow concurrent invocation.

Done is an abstract class with no application-visible attributes.
Applications override its `done` method, either in a Done subclass or by
using a [LambdaDone](./pub_disp-lambda.md#LambdaDone).

<!-- ===================================================================== -->
---
#### <a id="done">virtual void pub::dispatch::Done::done(Item* item) = 0;</a>

Handles the completion of the work `item`. **Override this method.**

The Item's completion code, `item->cc`, has already been set.

<!-- ===================================================================== -->
## <a id="Wait">pub::dispatch::Wait</a>

Wait extends Done, allowing an application to wait for a work Item to
complete.

Unlike other Done objects, a Wait object **cannot be shared**.
It can be used by only one work Item at a time, because it contains a
single Event that can be posted only once.
Posting it again before it's [reset](#reset) is an (unchecked) error.

A Wait object can be reused by calling its [reset](#reset) method after the
wait has been satisfied.
Wait objects cannot be copied.

<!-- ===================================================================== -->
---
#### <a id="reset">void pub::dispatch::Wait::reset();</a>

Resets the Wait object for reuse.
Once a work Item using this Wait object completes, [wait](#wait) returns
immediately until reset is invoked.

---
#### <a id="wait">void pub::dispatch::Wait::wait();</a>

Waits for work Item completion.

The Item's completion code is available in its `cc` field when wait
returns.
