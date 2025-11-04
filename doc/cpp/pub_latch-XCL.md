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
//       ~/doc/cpp/pub_latch-XCL.md
//
// Purpose-
//       Latch.h reference manual: XCL_latch
//
// Last change date-
//       2025/11/04
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>

## <a id=shr_latch>pub::XCL_latch</a>

Use the XCL_latch to obtain exclusive access to a SHR_latch.
(This is the only Latch that provides for and requires construction.)

<!-- ===================================================================== -->
---
#### <a id=constructor>void pub::XCL_latch::XCL_latch(SHR_latch& shr)</a>

Constructs an XCL_latch to be used to obtain exclusive access to a SHR_latch.

---
#### <a id=downgrade>void pub::XCL_latch::downgrade</a>

This method must only be invoked with the XCL_latch held.

This releases exclusive access to the Latch, setting the Latch share count
to one.
The SHR_latch's unlock method must be invoked to release shared access.

---
#### <a id=is_held>void pub::XCL_latch::is_held</a>

Returns TRUE if the SHR_latch is already exclusively held, without regard to
which Thread holds the Latch.

---
#### <a id=lock>void pub::XCL_latch::lock</a>

Obtains the (exclusive) Latch.

---
#### <a id=reset>void pub::XCL_latch::reset</a>

Unconditionally resets the Latch to its initial (non-held) state.

---
#### <a id=try_lock>void pub::XCL_latch::try_lock</a>

Obtains the (exclusive) Latch if it's available.

If the SHR_latch is currently held in shared mode, the Latch is (atomically)
converted into reserved mode, reserved for the current Thread.
Only one Thread can reserve the SHR_latch.
Once reserved, no other Thread can obtain a SHR_latch or XCL_latch.
Exclusive access is not granted until all SHR_latch holders have invoked
unlock.

Don't try to recursively obtain a SHR_latch. It can result in livelock.

Consider this sequence:
- Thread one obtains SHR_latch access.
- Thread two requests XLC_latch access, and converting Latch mode to reserved.
Thread two spins, waiting for all SHR_latch locks to be released.
- Thread one requests an additional SHR_latch access. In reserved state,
SHR_latch access it not available and Thread one spins, waiting for the
latch to exit the reserved state.

This results in Thread one spinning waiting for Thread two and Thread two
spinning waiting for Thread one.

---
#### <a id=unlock>void pub::XCL_latch::unlock</a>

Releases the (exclusive) Latch.
