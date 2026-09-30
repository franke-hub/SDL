<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2025-2026 Frank Eskesen.
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
//       ~/doc/cpp/pub_latch-BLOCK.md
//
// Purpose-
//       Latch.h reference manual: Block_latch
//
// Last change date-
//       2026/09/28
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>
## pub::Block_latch

The Block_latch is an exclusive spin Latch with no thread affinity: unlike
every other Latch type, it may be locked on one thread and unlocked on a
different one.

Usage notes:
- The lock method attempts to obtain the Latch even if it's already held by
the calling thread. This is allowed, but results in the calling thread
spinning against itself.
- The unlock method throws an exception if invoked while the Latch isn't
held.

<!-- ===================================================================== -->
---
#### <a id="constructor">pub::Block_latch::Block_latch();</a>
#### <a id="constructor2">pub::Block_latch::Block_latch(latch_t init);</a>

The default constructor creates the Block_latch in the available (unheld)
state.

The second constructor creates the Block_latch already in the state given by
`init`: a non-zero value creates it already held.

---
#### <a id="is_held">bool pub::Block_latch::is_held() const;</a>

Return value: TRUE if the Latch is held, without regard to which thread
holds it.

---
#### <a id="lock">void pub::Block_latch::lock();</a>

Obtains the (exclusive) Latch, spinning until it's available.

---
#### <a id="reset">void pub::Block_latch::reset();</a>

Unconditionally resets the Latch to its initial (unheld) state.

---
#### <a id="set">void pub::Block_latch::set();</a>

Unconditionally sets the Latch to its held state.

---
#### <a id="try_lock">bool pub::Block_latch::try_lock();</a>

Return value: TRUE iff successful.

Obtains the (exclusive) Latch if it's immediately available, without
spinning.

---
#### <a id="unlock">void pub::Block_latch::unlock();</a>

Releases the (exclusive) Latch.
