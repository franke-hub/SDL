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
//       ~/doc/cpp/pub_latch-NULL.md
//
// Purpose-
//       Latch.h reference manual: NullLatch
//
// Last change date-
//       2025/11/04
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Latch.h>

## <a id=null_latch>pub::NullLatch</a>

The NullLatch does nothing.
A NullLatch may be appropriate within a conditional compile where a Latch is
required in a multiprocessor environment but not in a uniprocessor
environment.

<!-- ===================================================================== -->
---
#### <a id=is_held>void pub::NullLatch::is_held</a>

Always returns FALSE.

---
#### <a id=lock>void pub::NullLatch::lock</a>

Does nothing.

---
#### <a id=reset>void pub::NullLatch::reset</a>

Does nothing.

---
#### <a id=try_lock>void pub::NullLatch::try_lock</a>

Always returns TRUE.

---
#### <a id=unlock>void pub::NullLatch::unlock</a>

Does nothing.
