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
//       ~/src/cpp/inc/pub/experimental/README.md
//
// Purpose-
//       SDL: Experimental PUB library information
//
// Last change date-
//       2026/04/26
//
-------------------------------------------------------------------------- -->

Copyright (c) 2026 Frank Eskesen.

This file is free content, distributed under cc by-sa version 4.0
with attribution required.
(See accompanying file LICENSE.BY_SA-4.0 or the original contained
within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)

----

# ~/src/cpp/inc/pub/experimental/README.md

This README describes the PUB (public) library experimental includes.

----

The experimental library extention include features that are not currently
ready for general distribution. Although implemented, they are not included
in the PUB library documentation.

- Allocator.h/SubAllocator.h
  - The Allocator interface has not been reconciled with std::allocator.
  - Current performance measurements shows no improvement over malloc, so
there's no reason for it to be used.
- Interface.h
  - Defines interfaces used in multiple places. There's no compelling reason
to use this. Either an Interface is implemented or it isn't.
