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
//       ~/doc/cpp/pub_disp-item.md
//
// Purpose-
//       Dispatch.h reference manual: Item
//
// Last change date-
//       2026/10/07
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Dispatch.h>
## pub::dispatch::Item

An Item describes a unit of work, the operation to process and its
associated data.
Applications usually extend Item, adding the data the operation needs.

Items are enqueued onto a [Task](./pub_disp-task.md), which processes them.
An Item completes when its [post](#post) method is invoked.

<!-- ===================================================================== -->
### Constants

Completion codes (`enum CC`):

| Name | Value | Meaning |
|------|-------|---------|
| CC_NORMAL | 0 | Normal (OK) |
| CC_PURGE | -1 | Function purged (for example, a cancelled delay) |
| CC_ERROR | -2 | Generic error |
| CC_ERROR_FC | -3 | Invalid function code |
| CC_ERROR_IT | -4 | Invalid Item type |

Function codes (`enum FC`):

| Name | Value | Meaning |
|------|-------|---------|
| FC_VALID | 0 | Default. All application function codes are >= 0. |
| FC_CHASE | -1 | Chase (handled internally by the Dispatcher) |
| FC_UNDEF | -2 | Undefined/invalid function code |

### Attributes

#### <a id="fc">int fc= FC_VALID;</a>

The function code.
Applications may use any non-negative value.
Negative function codes are reserved: they are handled by the Dispatcher
and are not passed to `Task::work(Item*)`.
An Item with an unrecognized negative function code completes with
completion code `CC_ERROR_FC`.

#### <a id="cc">int cc= CC_NORMAL;</a>

The completion code, set by [post](#post).
Negative values are pre-defined (see above.)

#### <a id="done">Done* done= nullptr;</a>

The [completion handler](./pub_disp-done.md#Done).
If `nullptr`, the Item is deleted when it's posted.

<!-- ===================================================================== -->
---
#### <a id="constructor">pub::dispatch::Item::Item();</a>
#### pub::dispatch::Item::Item(Done* done);
#### pub::dispatch::Item::Item(int fc, Done* done= nullptr);

Creates a work Item, initializing the specified fields.
Unspecified fields have their default values.

---
#### <a id="post">void pub::dispatch::Item::post(int cc= 0);</a>

Completes the work Item.

If `done` is not `nullptr`, sets the completion code to `cc` and invokes
`done->done(this)`.
Otherwise, the Item is deleted.
(Items without a Done object must therefore be allocated using `new`.)

---
[Back](./Dispatch.md) | [Reference manual](./REFERENCE.md)
