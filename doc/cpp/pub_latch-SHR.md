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
//       ~/doc/cpp/pub_latch-SHR.md
//
// Purpose-
//       Latch.h reference manual: SHR_latch
//
// Last change date-
//       2026/10/07
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>
## pub::SHR_latch

The SHR_latch is a shared-mode spin Latch: any number of threads may hold it
at once. Method lock adds a shared holder; method unlock removes one.

An XCL_latch (see [XCL_latch](./pub_latch-XCL.md)) associated with the same
SHR_latch can reserve exclusive access. While that reservation is held,
SHR_latch::try_lock fails and SHR_latch::lock spins, waiting for the
reservation to be released.

Applications *MUST NOT* lock the SHR_latch recursively (i.e. lock it a
second time on a thread that already holds it). Doing so can livelock
against a pending XCL_latch reservation.

<!-- ===================================================================== -->
---
#### <a id="is_held">bool pub::SHR_latch::is_held() const;</a>

Return value: TRUE if the Latch is held or reserved, in either shared or
exclusive mode, without regard to which thread holds it.

---
#### <a id="lock">void pub::SHR_latch::lock();</a>

Obtains the Latch in shared mode, spinning until it's available.

Note: it is an error for a thread already holding the SHR_latch to try to
obtain exclusive access (via an associated XCL_latch). A livelock always
occurs.

---
#### <a id="reset">void pub::SHR_latch::reset();</a>

Unconditionally resets the Latch to its initial (unheld) state.

---
#### <a id="try_lock">bool pub::SHR_latch::try_lock();</a>

Return value: TRUE iff successful.

Obtains the Latch in shared mode if it's immediately available (not
exclusively reserved), without spinning.

---
#### <a id="unlock">void pub::SHR_latch::unlock();</a>

Releases one shared hold on the Latch.

---
#### <a id="upgrade">(No upgrade method)</a>

SHR_latch has no upgrade method: exclusive access cannot be granted while
any shared holder remains, including the calling thread, so upgrading a
held SHR_latch in place would always livelock. Instead:
- Release the SHR_latch, then
- Obtain the associated XCL_latch (see [XCL_latch::lock](./pub_latch-XCL.md#lock)).

---
[Back](./Latch.md) | [Reference manual](./REFERENCE.md)
