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
//       2026/09/09
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>
## pub::XCL_latch

The XCL_latch is an exclusive spin Latch with thread affinity: methods lock
and unlock must be invoked from the same thread.

An XCL_latch references, and shares state with, a SHR_latch (see
[SHR_latch](./pub_latch-SHR.md)). Multiple XCL_latches may reference the
same SHR_latch. An XCL_latch controls that SHR_latch's *reserved* state: the
SHR_latch tracks which thread, if any, currently holds the reservation.

Restrictions:
- Application threads *MUST NOT* recursively hold the SHR_latch. Doing so
*might* create a livelock.
- Application threads *MUST NOT* hold the SHR_latch while invoking
XCL_latch's lock method. Doing so *will always* create a livelock: there
would be no way for that SHR_latch to be released, and so no way for the
XCL_latch reservation to ever complete.

An exception is thrown if:
- Method lock or try_lock is invoked while the reservation is already held
by the same thread.
- Method unlock is invoked from a thread that does not hold the reservation.

<!-- ===================================================================== -->
---
#### <a id="constructor">pub::XCL_latch::XCL_latch(SHR_latch& shr);</a>

Constructs an XCL_latch used to control exclusive access to the SHR_latch
`shr`.

---
#### <a id="destructor">pub::XCL_latch::~XCL_latch();</a>

The destructor *does nothing*. It neither downgrades nor unlocks the Latch.

---
#### <a id="downgrade">void pub::XCL_latch::downgrade();</a>

The caller *MUST ALREADY HOLD* the XCL_latch.

Releases the exclusive reservation, leaving the SHR_latch singly held in
shared mode. The SHR_latch's own unlock method must still be invoked to
release that shared hold.

---
#### <a id="is_held">bool pub::XCL_latch::is_held() const;</a>

Return value: TRUE if the SHR_latch is exclusively reserved, without regard
to which thread holds the reservation.

---
#### <a id="lock">void pub::XCL_latch::lock();</a>

(Compare to [upgrade](#upgrade).)

Obtains the reservation, then waits for every existing SHR_latch holder to
unlock.

An exception is thrown if the reservation is already held by the same
thread.

---
#### <a id="reset">void pub::XCL_latch::reset();</a>

Unconditionally resets the associated SHR_latch to its initial (unheld,
unreserved) state. This should only be used for error recovery.

---
#### <a id="try_lock">bool pub::XCL_latch::try_lock();</a>

Return value: TRUE iff successful.

Obtains the reservation if it's immediately available, then waits for
every existing SHR_latch holder to unlock. Only one thread can hold the
reservation; once held, no other thread can obtain either the SHR_latch or
the XCL_latch until it's released.

Consider this sequence, which livelocks:
- Thread one obtains the SHR_latch.
- Thread two calls XCL_latch::try_lock, obtaining the reservation, then
spins waiting for thread one to release the SHR_latch.
- Thread one, still holding the SHR_latch, tries to obtain it again
(recursively). Because the reservation is held, that attempt spins too.

Thread one is now waiting on thread two, and thread two is waiting on
thread one — hence the restriction against recursively holding a SHR_latch.

---
#### <a id="try_reserve">bool pub::XCL_latch::try_reserve();</a>

Return value: TRUE iff successful.

Obtains the reservation if it's immediately available, without waiting for
existing SHR_latch holders to unlock.

An exception is thrown if the reservation is already held by the same
thread.

---
#### <a id="unlock">void pub::XCL_latch::unlock();</a>

Releases the reservation, leaving the SHR_latch unheld.

An exception is thrown if the calling thread doesn't hold the reservation.

---
#### <a id="upgrade">bool pub::XCL_latch::upgrade();</a>

(Compare to [lock](#lock).)

Return value: TRUE iff successful.

Requirements:
- The calling thread *MUST* currently hold the SHR_latch.
- The XCL_latch *MUST NOT* already be held by the calling thread.

Attempts to reserve the XCL_latch (as try_reserve does) and, on success,
waits for every *other* SHR_latch holder to unlock (the calling thread's
own shared hold is not released), then returns TRUE with the Latch held
exclusively.

If the reservation can't be obtained, upgrade returns FALSE and takes no
other action. In that case the application *MUST* still invoke
SHR_latch::unlock to release its shared hold — recovery beyond that is
application-dependent, typically some form of transaction retry. (A
subsequent XCL_latch::lock is a separate operation, not a continuation of
the upgrade, and may or may not be the appropriate next step.)

An exception is thrown if the reservation is obtained but the calling
thread turns out not to have held the SHR_latch at all.

If upgrade succeeds, the Latch remains held exclusively until downgrade or
unlock is invoked.
