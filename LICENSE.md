<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2025 Frank Eskesen.
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
//       ~/LICENSE.md
//
// Purpose-
//       SDL (Software Development Laboratory) lisensing information
//
// Last change date-
//       2025/08/27
//
//------------------------------------------------------------------------ -->

# ~/LICENSE.md

Copyright (C) 2025 Frank Eskesen.

This file is free content, distributed under cc by-sa version 4.0
with attribution required.
(See accompanying file LICENSE.BY_SA-4.0 or the original contained
within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)

----

### Licensing
Almost all files contain an SPDX header containing the license information for
that file.

Exceptions:
- ~/src/py/aiml/std-startup.xml
(Copyright incompatible with AIML parser.)
- Testing data (input and verification output)

We use an automated tool source file to verify distribution files.
This scans the distibution files, checking:
- The file's mode
- That the file is in unix format (not DOS format)
- That the file has no lines with trailing blanks
- That the copyright header matches a predefined header (by file type.)
This is run using ~/obj/cpp/Scanner/regression (in ~/obj/cpp/Scanner)

All imported content retains the copyright and licensing restrictions of
the original authors.

Most original C++ and python source modules are distributed under the GNU
Public License.
Most original C++ include headers are distributed under the Lesser GPL.

Control files (e.g. make and .gitignore files,) bash scripts, and certain
html files are distributed under the MIT license.

Lilypond (music) files, markdown files, html, and xml files are distributed
under a Creative Commons license.

Some sample content explictly uses the public domain license.
This content does not require attribution or licensing.

Any and all "look and feel" content in this entire distribution is explicitly
licensed under the public domain license.

License detail:
- [Boost](.licenses/LICENSE.BOOST-1.0)
  - SPDX-License-Identifier: BSL-1.0
  - ~/src/cpp/inc/pub/memory.h (UNUSED/UNTESTED)
  - ~/src/cpp/lib/pub/Debug.cpp (Includes: <boost/stacktrace.hpp>)
- [BSD](.licenses/LICENSE.BSD-3)
  - SPDX-License-Identifier: BSD-3-Clause
  - ~/src/py/Sample/GUI/Qt/widgets-example.py
- [Creative Commons V4.0](.licenses/LICENSE.BY_SA-4.0)
  - SPDX-License-Identifier: CC-BY-4.0
  - ~/src/lily/Eskesen/* (Cheesy, but original, music)
  - ~/src/lily/Public/* (Lily representations of out of copyright music.)
  - ~/src/lily/Sample/* (Incomplete sample Lily layouts.
    Probably should be MIT or public domain license.)
  - ~/src/java/Sample/Swing/Main.java
- [GNU GPL (General Public License)](.licenses/LICENSE.GPL-3.0)
  - SPDX-License-Identifier: GPL-3.0-only
  - Almost all source code.
- [GNU LGPL (Lesser General Public License)](.licenses/LICENSE.LGPL-3.0)
  - SPDX-License-Identifier: LGPL-3.0-only
  - Almost all library include files.
- [MIT License](.licenses/LICENSE.MIT)
  - SPDX-License-Identifier: MIT
  - Almost all Makefile segments, control and documentation files.
- [Public domain license](.licenses/LICENSE.ZERO) (Creative Commons CC0)
  - SPDX-License-Identifier: CC0-1.0
  - All common control Makefile segments in ~/src/cpp/ctl/.
  - All example code.
  - All BASH control files in ~/bat/.home/ except for .bash_logout.
(.bash_logout is comment-only and contains no original content.)
