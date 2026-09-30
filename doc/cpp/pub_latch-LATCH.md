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
//       ~/doc/cpp/pub_latch-LATCH.md
//
// Purpose-
//       Latch.h reference manual: Latch
//
// Last change date-
//       2026/09/28
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>
## pub::Latch

The Latch is an exclusive spin Latch with thread affinity: methods lock and
unlock must be invoked from the same thread.

An exception is thrown if:
- Method lock or try_lock is invoked while the Latch is held by the same
thread.
- Method unlock is invoked from a thread that does not hold the Latch.

<!-- ===================================================================== -->
---
#### <a id="is_held">bool pub::Latch::is_held() const;</a>

Return value: TRUE if the Latch is held, without regard to which thread
holds it.

---
#### <a id="lock">void pub::Latch::lock();</a>

Obtains the (exclusive) Latch, spinning until it's available.

---
#### <a id="reset">void pub::Latch::reset();</a>

Unconditionally resets the Latch to its initial (unheld) state.

---
#### <a id="statistics">static void pub::Latch::statistics(const char* info="");</a>

Displays Latch usage statistics, when statistics recording is enabled.
`info` is an optional caller-supplied identifying string included in the
display.

---
#### <a id="try_lock">bool pub::Latch::try_lock();</a>

Return value: TRUE iff successful.

Obtains the (exclusive) Latch if it's immediately available, without
spinning.

---
#### <a id="unlock">void pub::Latch::unlock();</a>

Releases the (exclusive) Latch.
