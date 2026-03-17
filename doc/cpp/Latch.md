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
//       ~/doc/cpp/Latch.md
//
// Purpose-
//       Latch.h reference manual
//
// Last change date-
//       2026/03/04
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>
## `pub::Latch`

Usage notes:
- All Latch types implement Lockable and support use with std::lock_guard.
- All Latch types except Block_latch are thread-specific: Methods lock and
unlock must be invoked from the same thread.
- Latches are intended to be locked for short code sequences. All Latch type
lock implementations use spin locking, using yield and/or timed wait when the
Latch isn't immediately available.

** <a id="Latch_classes">Latch Classes</a> **
| Class             | Description
| :---------------- | :--------------------------------------------------- |
| [Latch](#Latch) | Exclusive spin Latch |
| [Block_latch](#Block_latch) | Primitive exclusive spin Latch |
| [RecursiveLatch](#RecursiveLatch) | Exclusive recursive spin Latch |
| [SHR_latch](#SHR_latch) | Shared spin Latch (shared mode) |
| [XCL_latch](#XCL_latch) | Shared spin Latch (exclusive mode) |

#### <a id="Latch">[Latch](./pub_latch-LATCH.md)</a>
| Method | Purpose |
|--------|---------|
| [is_held](./pub_latch-LATCH.md#is_held) | Test: is the Latch held? |
| [lock](./pub_latch-LATCH.md#lock) | Obtain the (exclusive) Latch. |
| [reset](./pub_latch-LATCH.md#reset) | Unconditionally reset the Latch. |
| [try_lock](./pub_latch-LATCH.md#try_lock) | Try to obtain the Latch. |
| [unlock](./pub_latch-LATCH.md#unlock) | Release the Latch. |

#### <a id="Block_latch">[Block_latch](./pub_latch-BLOCK.md)</a>
| Method | Purpose |
|--------|---------|
| [is_held](./pub_latch-BLOCK.md#is_held) | Test: is the Latch held? |
| [lock](./pub_latch-BLOCK.md#lock) | Obtain the (exclusive) Latch. |
| [reset](./pub_latch-BLOCK.md#reset) | Unconditionally reset the Latch. |
| [try_lock](./pub_latch-BLOCK.md#try_lock) | Try to obtain the Latch. |
| [unlock](./pub_latch-BLOCK.md#unlock) | Release the Latch. |

#### <a id="RecursiveLatch">[RecursiveLatch](./pub_latch-RECURSIVE.md)</a>
| Method | Purpose |
|--------|---------|
| [is_held](./pub_latch-RECURSIVE.md#is_held) | Test: is the Latch held? |
| [lock](./pub_latch-RECURSIVE.md#lock) | Obtain the (exclusive) Latch. |
| [reset](./pub_latch-RECURSIVE.md#reset) | Unconditionally reset the Latch. |
| [try_lock](./pub_latch-RECURSIVE.md#try_lock) | Try to obtain the Latch. |
| [unlock](./pub_latch-RECURSIVE.md#unlock) | Release the Latch. |

#### <a id="SHR_latch">[SHR_latch](./pub_latch-SHR.md)</a>
| Method | Purpose |
|--------|---------|
| [is_held](./pub_latch-SHR.md#is_held) | Test: is the Latch held? |
| [lock](./pub_latch-SHR.md#lock) | Obtain the (exclusive) Latch. |
| [reset](./pub_latch-SHR.md#reset) | Unconditionally reset the Latch. |
| [try_lock](./pub_latch-SHR.md#try_lock) | Try to obtain the Latch. |
| [unlock](./pub_latch-SHR.md#unlock) | Release the Latch. |

#### <a id="XCL_latch">[XCL_latch](./pub_latch-XCL.md)</a>
| Method | Purpose |
|--------|---------|
| [constructor](./pub_latch-XCL.md#constructor) | Create the XCL Latch. |
| [downgrade](./pub_latch-XCL.md#downgrade) | Change mode: exclusive to shared. |
| [is_held](./pub_latch-XCL.md#is_held) | Test: is the Latch held? |
| [lock](./pub_latch-XCL.md#lock) | Obtain exclusive mode access. |
| [reset](./pub_latch-XCL.md#reset) | Unconditionally reset the Latch. |
| [try_lock](./pub_latch-XCL.md#try_lock) | Try to obtain exclusive mode access. |
| [try_reserve](./pub_latch-XCL.md#try_reserve) | Try to reserve exclusive mode access. |
| [unlock](./pub_latch-XCL.md#unlock) | Completely release the Latch. |
| [upgrade](./pub_latch-XCL.md#upgrade) | Change mode: shared to exclusive. |
