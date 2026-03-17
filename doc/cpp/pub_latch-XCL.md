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
//       ~/doc/cpp/pub_latch-XCL.md
//
// Purpose-
//       Latch.h reference manual: XCL_latch
//
// Last change date-
//       2026/03/04
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>

## <a id=shr_latch>pub::XCL_latch</a>


The XCL_latch is an exclusive spin Latch with thread affinity. Method lock and
unlock must be invoked by the same thread.

The XCL_latch references and shares state with a SHR_latch. Multple
XCL_latches may reference the same SHR_latch.

XCL_latches control the `reserved` state of a SHR_latch, which includes one
bit in the SHR_latch counter and the thread id of the thread holding the
reservation.

Restrictions:
- Application threads *MUST NOT* recursively hold the SHR_latch. Doing so
*might* create a livelock.
- Application threads *MUST NOT* hold the SHR_latch while invoking XCL_latch's
lock method. Doing so *will always* create a livelock.

Implementation notes:
- Method lock uses spin logic waiting for try_lock to return true
- Method try_lock returns false unless try_reserve returns true
- Method try_reserve
  - Returns false if the latch is already reserved (but throws an exception if
the running thread holds the latch)
  - Returns false if the latch cannot be atomically reserved
  - Sets the thread owner to the current thread and returns true
- In try_lock, once try_reserve return true, it spins waiting until the share
lock count is zero

Usage note: Applications *MUST NOT* hold a corresponding SHR_latch when
invoking XCL_Latch's lock method. There's no way for that SHR_latch to be
released and there's no way to release the reservation.

An exception is thrown if:
- Method lock or try_lock is invoked while a reservation is held by the same
thread
- Method unlock is invoked from a thread that does not hold the reservation

<!-- ===================================================================== -->
---
#### <a id=constructor>void pub::XCL_latch::XCL_latch(SHR_latch& shr)</a>

Constructs a Latch used to control exclusive access to a SHR_latch.

---
#### <a id=destructor>void pub::XCL_latch::~XCL_latch()</a>

The destructor *DOES NOTHING*. It neither downgrades nor unlocks the Latch.

---
#### <a id=downgrade>void pub::XCL_latch::downgrade</a>

The caller *MUST ALREADY HOLD* XCL_latch.

Releases the exclusive Latch, leaving the SHR_latch singly held.
The SHR_latch's unlock method must be invoked to release shared access.

---
#### <a id=is_held>void pub::XCL_latch::is_held</a>

Returns TRUE if the SHR_latch is already exclusively held, without regard to
which Thread holds the Latch.

---
#### <a id=lock>void pub::XCL_latch::lock</a>

(Compare to XCL_latch::upgrade.)

Obtains the latch reservation, then (spins) waiting for all SHR_latches to be
unlocked.

An exception is thrown if the reservation is already held by the same thread.

---
#### <a id=reset>void pub::XCL_latch::reset</a>

Unconditionally resets the Latch to its initial (non-held) state.
This should only be used for error recovery.

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
#### <a id=try_reserve>void pub::XCL_latch::try_reserve</a>

Obtains the Latch reservation if it's available.

An exception is thrown if the reservation is already held by the same thread.

---
#### <a id=unlock>void pub::XCL_latch::unlock</a>

Releases the (exclusive) Latch, leaving no SHR_latch held.

An exception is thrown if the current thread didn't lock the Latch.

---
#### <a id=upgrade>void pub::XCL_latch::upgrade</a>

(Compare to XCL_latch::lock.)

Requirements:
- The SHR_latch *MUST BE* singly held.
- The XCL_latch *MUST NOT* be held by the invoking thread

Upgrade invokes try_reserve, and returns false when try_reserve returns false.
When this occurs, the application *MUST INVOKE* SHR_latch::unlock. This allows
the thread holding the reservation to continue. (It will generally in a spin
loop until SHR_latches are unlocked.)

Other than the SHR_latch unlock recovery is application-dependent, usually
requiring some form of transaction recovery.

Generally, the next application step after invoking SHR_latch\::unlock would
be to invoke XCL_latch::lock, but that's not an upgrade  might not be appropriate

If the upgrade succeeds the latch is in the locked state, and remains locked
until invoking either downgrade or unlock.
