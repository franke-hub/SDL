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
//       ~/doc/cpp/pub_latch-BASIC.md
//
// Purpose-
//       Latch.h reference manual: Basic_latch
//
// Last change date-
//       2025/11/04
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>

## <a id=basic_latch>pub::Basic_latch</a>

The Basic_latch is a spin Latch with limited error checking.

The only error check is in unlock.
We verify that the Latch is held, but don't verify that it's held by the
current Thread.

<!-- ===================================================================== -->
---
#### <a id=is_held>void pub::Basic_latch::is_held</a>

Returns TRUE if the latch is held, without regard to which Thread holds
the Latch.

---
#### <a id=lock>void pub::Basic_latch::lock</a>

Obtains the (exclusive) Latch.

---
#### <a id=reset>void pub::Basic_latch::reset</a>

Unconditionally resets the Latch to its initial (non-held) state.

---
#### <a id=try_lock>void pub::Basic_latch::try_lock</a>

Obtains the (exclusive) Latch if it's available.

---
#### <a id=unlock>void pub::Basic_latch::unlock</a>

Releases the (exclusive) Latch.
