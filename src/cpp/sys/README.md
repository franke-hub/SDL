<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/src/cpp/sys/README.md
//
// Purpose-
//       Subdirector description and change log
//
// Last change date-
//       2023/05/19
//
-------------------------------------------------------------------------- -->
# ~/src/cpp/sys/README.md

Copyright (C) 2023 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See the accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

----

This README describes the `~/src/cpp/sys/.` subdirectory.

This subdirectory is intended to contain programs that need to be in the
user's `~/bin/.` subdirectory.
These programs are generally prequisites for building and testing other .cpp
programs included in the distribution.
As such, they must not utilize any of the distribution libraries.
(This also implies that they usually won't need to be recompiled when the
operating system is upgraded.)

----

## CHANGE LOG
### 05/19/2023 Initial version
- Moved ~/src/cpp/Tools/Makeproj/* here, renaming Makeproj to makeproj.
- Moved ~/src/cpp/Tools/Compare/* here, removing pub dependencies.
- Created scaffolded versions of rdconfig.cpp and wrconfig.cpp.
These files are non-functional and not distributed.

----
