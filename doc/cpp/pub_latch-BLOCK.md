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
//       ~/doc/cpp/pub_latch-BLOCK.md
//
// Purpose-
//       Latch.h reference manual: block_latch
//
// Last change date-
//       2026/03/04
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>

## <a id=block_latch>pub::Block_latch</a>

The Block_latch is an exclusive spin Latch without thread checking, i.e.
Latch::lock may be used in one thread and Latch::unlock in a different one.

Usage notes:
- The lock method attempts to obtain the lock even if the same thread holds
the lock. This is allowed but can result in a spinlock.
- The unlock method throws an exception if invoked while unlocked.

<!-- ===================================================================== -->
---
#### <a id=is_held>void pub::Block_latch::is_held</a>

Returns TRUE if the latch is held, without regard to which Thread holds
the Latch.

---
#### <a id=lock>void pub::Block_latch::lock</a>

Obtains the (exclusive) Latch.

---
#### <a id=reset>void pub::Block_latch::reset</a>

Unconditionally resets the Latch to its initial (non-held) state.

---
#### <a id=try_lock>void pub::Block_latch::try_lock</a>

Obtains the (exclusive) Latch if it's available.

---
#### <a id=unlock>void pub::Block_latch::unlock</a>

Releases the (exclusive) Latch.
