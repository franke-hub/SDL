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
//       ~/doc/cpp/pub_latch-SHR.md
//
// Purpose-
//       Latch.h reference manual: SHR_latch
//
// Last change date-
//       2026/03/04
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>

## <a id=shr_latch>pub::SHR_latch</a>

The SHR_latch is a spin Latch counter. Method lock increments the counter and
method unlock decrements it.

The counter's high-order bit indicates exclusive latch reservation. While
reserved, the SHR_latch's try_lock returns failure and method lock spins,
waiting for the reserve to be released.

Applications *MUST NOT* lock the SHR_latch recursively. Doing so can result
in a livelock between the SHR_latch and an XCL_latch.

<!-- ===================================================================== -->
---
#### <a id=is_held>void pub::SHR_latch::is_held</a>

Returns TRUE if the latch is held, without regard to which Thread holds
the Latch or whether the latch is held in shared or exclusive mode.

---
#### <a id=lock>void pub::SHR_latch::lock</a>

Obtains the shared Latch.

Note: it is an error to try to obtain exclusive access to a SHR_latch that's
held by the same Thread. A livelock always occurs.

---
#### <a id=reset>void pub::SHR_latch::reset</a>

Unconditionally resets the Latch to its initial (non-held) state.

---
#### <a id=try_lock>void pub::SHR_latch::try_lock</a>

Obtains the shared Latch if it's available.

---
#### <a id=unlock>void pub::SHR_latch::unlock</a>

Releases the shared Latch.

---
#### <a id=upgrade>void pub::SHR_latch::upgrade</a>

There is no upgrade method. Exclusive mode access cannot be granted when a
share latch is held. (A livelock always occurs.)
Instead:
- Release the SHR_latch
- Obtain the XCL_latch
