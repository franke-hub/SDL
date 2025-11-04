<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2025 Frank Eskesen.
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
//       2025/11/04
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>

## <a id=recursive_latch>pub::RecursiveLatch</a>

The RecursiveLatch is a spin Latch that may be obtained recursively by the
same Thread.
The Latch must be unlocked for each time that it's locked.

<!-- ===================================================================== -->
---
#### <a id=is_held>void pub::RecursiveLatch::is_held</a>

Returns TRUE if the latch is held, without regard to which Thread holds
the Latch.

---
#### <a id=lock>void pub::RecursiveLatch::lock</a>

Obtains the (exclusive) Latch.

---
#### <a id=reset>void pub::RecursiveLatch::reset</a>

Unconditionally resets the Latch to its initial (non-held) state.

---
#### <a id=try_lock>void pub::RecursiveLatch::try_lock</a>

Obtains the (exclusive) Latch if it's available.

---
#### <a id=unlock>void pub::RecursiveLatch::unlock</a>

Releases the (exclusive) Latch.
