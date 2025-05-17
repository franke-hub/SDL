<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2020-2025 Frank Eskesen.
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
//       ~/README.md
//
// Purpose-
//       SDL Project overview information.
//
// Last change date-
//       2025/05/17
//
//------------------------------------------------------------------------ -->

Copyright (C) 2020-2025 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See the accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

## Table of Contents

- [About](#about)
- [Quick Start](#quick-start)
- [Documentation](#documentation)
- [Contributing](./CONTRIBUTORS.md)
- [Copying](#copying)
- [License](#license)
- [Overview](#overview)

- [SDL C++ Library Reference manual](doc/cpp/REFERENCE.md)
- [Development detail](doc/index.md)
- [Change log](./COMMIT.md)

### <a id="overview">Content overview</a>
This is a Software Development Laboratory (SDL) distribution package,
sometimes referred to as "The Distribution".

This package contains multiple programming examples in multiple languages.
C++, Python, and Java examples also include libraries.
A (mostly) bash script library is also provided.

Compiled libraries, sample programs and utilities are built using GNU make
from the associated object subdirectory.

Programs are compiled and tested on Cygwin, Fedora, and Ubuntu.
While the COM library and some sample programs once ran on Windows,
Windows support is deprecated and is no longer tested.

All content is distributed AS-IS, without purpose or warranty of any kind;
not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

### <a id="copying">Copying</a>
This is an open source project, consisting of free content distributed under
an open source license.

While the distribution author retains the open source copyright,
no additional restrictive rights are or will be claimed.
In particular but not exclusively, the author warrants that no trade mark or
patent rights exist and none will be generated for any content in this
distribution. This distribution is and will remain open source.

**ALL** content in this distribution may be copied, but
- Your license is not exclusive. No person or entity can restrict any other
person or entity's usage of *any* content.
- Imported content retains the copyright, copying, and licensing restrictions
of the original author (or source, if no copyright information was included
in the imported content.)
Every SDL distribution file containing imported content contains
originating copyright and licensing information

Small content segments may be copied under the public domain license
without attribution or licensing requirements[^1].
The GNU GPL applies to any significant content aggregation.

### <a id="license">Licensing</a>
All imported content retains the copyright and licensing restrictions of
the original authors.

Most original C++ source code is distributed under the GNU Public License.
Most original include headers are distributed under the Lesser GPL.

Control files (e.g. make files) and most documentation (such as this file) are
distributed using the MIT license.

The Creative Commons license is used for Lilypond (music) files.

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
  - All control Makefile segments (in ~/src/cpp/ctl/.)
  - All example code.
  - All BASH control files in ~/bat/.home/ except for .bash_logout.
(.bash_logout contains no original content.)

### <a id="quick-start">Quick Start Guide</a>
Prerequisite packages:
- [Cygwin](#prereq-cygwin)
- [Fedora](#prereq-fedora)
- [Ubuntu](#prereq-ubuntu)

(The prequisites are the same for each environment, but they don't use the
same naming conventions.)

These instructions use the SDL package as delivered to install the C++ and

```bash
git clone https://github.com/franke-hub/SDL.git
cd SDL
. setupSDL
make install
```

#### Other make options
- `make` (without options) lists available options
- `make install` Installs the C++ and Python libraries
- `make reinstall` Uninstalls and then installs. (Use this after installing
a new Linux version.)
- `make uninstall` Removes all files created by make install.
- `make check` Runs all available test suites.
- `make compile` Compiles (but does not test) all sample programs.
- `make clean` Removes all files created by make check and make compile.
- `make pristine` Essentially the same as running make clean followed by
make uninstall.

__TODO__ Implement C# and Java library installation Makefiles.
(Only C++ and Python library Makefiles are currently implemented.)

<!-- --------------------------------------------------------------------- -->
### <a id="prereq-cygwin">Cygwin prerequisite packages</a>
These packages are Cygwin packages, not Windows packages.

Build environment packages:
- autoconf
- automake
- binutils
- gcc-core
- gcc-g++
- gdb
- git
- libtool
- make
- patch
- pkgconf

Build library packages:
- bzip2
- glm-devel
- ImageMagick           (Version >= 7.0)
- libboost-devel
- curl,         libcurl-devel
- libdb-devel
- libjpeg-devel
- libhunspell-devel
- libncurses-devel
- libX11-devel
- libX11-xcb-devel
- libX11-xcb1
- libxcb-devel
- libxcb-util-devel
- libxcb-xfixes-devel
- libxcb1
- libXft-devel
- mariadb-common
- ncurses
- openssl
- zlib-devel

### <a id="prereq-fedora">Fedora prerequisite packages</a>
Build environment packages:
- autoconf
- automake
- binutils
- gcc
- gcc-c++
- gdb
- git
- libtool
- make
- patch
- pkgconf

Build library packages:
- boost-devel
- bzip2-devel
- glm-devel
- ImageMagick-c++-devel (Version >= 7.0)
- ImageMagick-devel     (Version >= 7.0)
- libcurl-devel
- libdb-devel, libdb-cxx-devel
- libhunspell-devel
- libjpeg-turbo-devel
- libX11-devel
- libxcb-devel
- libXft-devel
- mariadb
- ncurses-devel
- openssl-devel
- xcb-util-cursor-devel
- zlib-ng

<!-- --------------------------------------------------------------------- -->
### <a id="prereq-ubuntu">Ubuntu prerequisite packages</a>
Build environment packages:
- autoconf
- automake
- binutils
- gcc
- g++
- gdb
- git
- libtool
- make
- patch
- pkgconf

Build library packages:
- ImageMagick-c++-devel (Version >= 7.0)
- ImageMagick-devel     (Version >= 7.0)
- libboost-all-dev
- libbz2-dev
- libcurl4-gnutls-dev
- libdb-dev,  libdb++-dev
- libglm-dev
- libhunspell-dev
- libjpeg-dev
- libncurses-dev
- libssl-dev
- libx11-dev
- libxcb-image0-dev
- libxcb-xfixes0-dev
- libxcb1-dev
- mariadb
- zlib1g-dev

### <a id="about">The Distribution</a>
The distribution primarily uses two git branches:
- The trunk branch, relatively well tested.
- The maint branch, more current but more unstable.
This branch may contain known errors when used for distribution testing.

The distribution is kept locally in /home/data/SDL on all local machines,
physical or virtual.
In addition to this public distribution, the /home/data/ subdirectory contains
private information shared via rsync between machines.
One of these machines (NFS) exports /home/data in read-only mode and some
local virtual machines access that.

An Ubuntu build test (virtual) machine updates only via github.
It does not access the exported /home/data Network File System.
It's used to verify maint and trunk releases operate properly as distributed,
and does not access the NFS file system.

#### Subdirectory structure:

```
(Root) The distribution root directory, *documented as ~* in this distribution
   ~
   |
   [-- bat ((Mostly bash) scripts)
   |
   [-- doc (Documentation)
   |
   [-- obj (Object files)
   |    |
   |    [-- cpp  (C++)
   |    |
   |    [-- java (Java)
   |    |
   |    [-- mcs  (C#)
   |    |
   |    [-- py   (Python)
   |
   [-- src (Source files)
   |    |
   |    [-- cpp
   |    |
   |    [-- java
   |    |
   |    [-- mcs
   |    |
   |    [-- py
   |
   [-- usr (Not distributed. Used for machine to machine backup.)
```

Multiple libraries are also provided:

```
   ~
   |
   [-- obj
   |    |
   |    [-- cpp/lib (The library subdirectory)
   |    |      |
   |    |      [-- com (The common library: oldest)
   |    |      |
   |    |      [-- dev (The development library: experimental)
   |    |      |
   |    |      [-- gui (The Graphical User Interface library)
   |    |      |
   |    |      [-- obj (The object library)
   |    |      |       (Objects are reference count deleted)
   |    |      |
   |    |      [-- pub (The public library: in active development)
   |    |      |
   |    |   cpp/lib/*/Test (Library test object/executables)
   |    |
   |    [-- [java,js,mcs]/lib
   |    |
   |    [-- py/site-packages (The python library subdirectory)
   |    |      |
   |    |   The python library
   |
   [-- src
   |    |
   |    [-- cpp/inc (Header files)
   |    [-- cpp/lib (Source files)
   |    |      |
   |    |      [-- com
   |    |      |
   |    |      [-- dev
   |    |      |
   |    |      [-- gui
   |    |      |
   |    |      [-- obj
   |    |      |
   |    |      [-- pub
   |    |      |
   |    |   cpp/lib/*/Test (Library test source)
   |    |
   |    [-- [java,mcs]/lib
   |    |
   |    [-- py/site-packages
```

All C++ libraries have an associated Test subdirectory:
`~/src/cpp/lib/*/Test`
The dev and pub libraries also have a regression test script, `regression`,
used for distribution testing.
These can be invoked from the root directory using `make check`.

For C++ programs, two build helper programs are required.
These programs are built in the `~/obj/cpp/sys/` subdirectory and installed
into $HOME/bin using `make install`.
(The $HOME/bin directory location is not currrently configurable.)

- makeproj: Used to create or update dependencies.
- filecomp: Used in regression testing to (partially) compare files.

In addition to the libraries, (many) sample programs are provided.

__TODO__ Document the sample programs.

The project Wiki contains complete build instructions starting from an
"Ubuntu Desktop for developers" package.
