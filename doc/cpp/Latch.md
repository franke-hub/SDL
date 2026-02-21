<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
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
//       2026/02/08
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>
## `pub::Latch`

| Class             | Description
| :---------------- | :--------------------------------------------------- |
| [Basic_latch](#Basic_latch) | Primitive exclusive spin Latch |
| [Latch](#Latch) | Exclusive spin Latch |
| [RecursiveLatch](#RecursiveLatch) | Exclusive recursive spin Latch |
| [SHR_latch](#SHR_latch) | Shared spin Latch (shared mode) |
| [XCL_latch](#XCL_latch) | Shared spin Latch (exclusive mode) |

** <a id="Latch_classes">Table 1: Latch Classes</a> **

#### <a id="Basic_latch">[Basic_latch](./pub_latch-BASIC.md)</a>
| Method | Purpose |
|--------|---------|
| [is_held](./pub_latch-BASIC.md#is_held) | Test: is the Latch held? |
| [lock](./pub_latch-BASIC.md#lock) | Obtain the (exclusive) Latch. |
| [reset](./pub_latch-BASIC.md#reset) | Unconditionally reset the Latch. |
| [try_lock](./pub_latch-BASIC.md#try_lock) | Try to obtain the Latch. |
| [unlock](./pub_latch-BASIC.md#unlock) | Release the Latch. |

#### <a id="Latch">[Latch](./pub_latch-LATCH.md)</a>
| Method | Purpose |
|--------|---------|
| [is_held](./pub_latch-LATCH.md#is_held) | Test: is the Latch held? |
| [lock](./pub_latch-LATCH.md#lock) | Obtain the (exclusive) Latch. |
| [reset](./pub_latch-LATCH.md#reset) | Unconditionally reset the Latch. |
| [try_lock](./pub_latch-LATCH.md#try_lock) | Try to obtain the Latch. |
| [unlock](./pub_latch-LATCH.md#unlock) | Release the Latch. |

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
| [try_lock](./pub_latch-XCL.md#try_lock) | Try to reserve exclusive mode. |
| [try_wait](./pub_latch-XCL.md#try_wait) | Wait for exclusive mode access. |
| [unlock](./pub_latch-XCL.md#unlock) | Completely release the Latch. |
| [upgrade](./pub_latch-XCL.md#upgrade) | Change mode: shared to exclusive. |
