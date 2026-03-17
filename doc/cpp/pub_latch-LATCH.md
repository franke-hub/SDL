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
//       ~/doc/cpp/pub_latch-LATCH.md
//
// Purpose-
//       Latch.h reference manual: Latch
//
// Last change date-
//       2026/03/04
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>

## <a id=latch>pub::Latch</a>

The Latch is an exclusive spin Latch with thread affinity. Method lock and
unlock must be invoked by the same thread.

An exception is thrown if:
- Method lock or try_lock is invoked while the lock is held by the same thread
- Method unlock is invoked from a thread that does not hold the lock

<!-- ===================================================================== -->
---
#### <a id=is_held>void pub::Latch::is_held</a>

Returns TRUE if the latch is held, without regard to which Thread holds
the Latch.

---
#### <a id=lock>void pub::Latch::lock</a>

Obtains the (exclusive) Latch.

---
#### <a id=reset>void pub::Latch::reset</a>

Unconditionally resets the Latch to its initial (non-held) state.

---
#### <a id=try_lock>void pub::Latch::try_lock</a>

Obtains the (exclusive) Latch if it's available.

---
#### <a id=unlock>void pub::Latch::unlock</a>

Releases the (exclusive) Latch.
