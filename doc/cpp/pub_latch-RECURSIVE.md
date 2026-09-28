<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2025-2026 Frank Eskesen.
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
//       ~/doc/cpp/pub_latch-RECURSIVE.md
//
// Purpose-
//       Latch.h reference manual: RecursiveLatch
//
// Last change date-
//       2026/09/09
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>
## pub::RecursiveLatch

The RecursiveLatch is an exclusive spin Latch with thread affinity: methods
lock and unlock must be invoked from the same thread.

Unlike Latch, a thread already holding the RecursiveLatch may lock it again:
each such lock increments a use count, and each unlock decrements it. The
Latch becomes available again only when the use count returns to zero.

<!-- ===================================================================== -->
---
#### <a id="is_held">bool pub::RecursiveLatch::is_held() const;</a>

Return value: TRUE if the Latch is held, without regard to which thread
holds it.

---
#### <a id="lock">void pub::RecursiveLatch::lock();</a>

Obtains the Latch, spinning until it's available. If the calling thread
already holds the Latch, increments the use count instead of spinning.

---
#### <a id="reset">void pub::RecursiveLatch::reset();</a>

Unconditionally resets the Latch to its initial (unheld) state, regardless
of the current use count.

---
#### <a id="try_lock">bool pub::RecursiveLatch::try_lock();</a>

Return value: TRUE iff successful.

Obtains the Latch if it's immediately available, without spinning, or if
the calling thread already holds it (incrementing the use count).

---
#### <a id="unlock">void pub::RecursiveLatch::unlock();</a>

Decrements the use count. Releases the Latch when the use count reaches
zero.

An exception is thrown if invoked from a thread that does not hold the
Latch.
