<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/README.md
//
// Purpose-
//       SDL Project overview information.
//
// Last change date-
//       2024/03/03
//
//------------------------------------------------------------------------ -->

# ~/README.md

Copyright (C) 2020-2023 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See the accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

#### Content overview
This is a Software Development Laboratory (SDL) distribution package,
sometimes referred to as "The Distribution".

This package contains multiple programming examples in multiple languages.
C++, Python, and Java examples also include libraries.
A (mostly) bash script library is also provided.

Compiled libraries, sample programs and utilities are built using GNU make
from the associated object subdirectory.

Programs are compiled and tested on CYGWIN and Linux (Fedora and Ubuntu.)
While there are some sample programs that once ran on Windows,
Windows support is deprecated and is no longer tested.

All content is distributed AS-IS, without purpose or warranty of any kind;
not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

#### Copying
This is an open source project, consisting of free content distributed under
an open source license.

While the distribution author retains the open source copyright and licensing
rights, no additional restrictive rights are or will be claimed.
In particular but not exclusively, the author warrants that no trade mark or
patent rights exist and none will be generated for any content in this
distribution. This distribution is and will remain open source.

**ALL** content in this distribution may be copied, but
- Your license is not exclusive. No person or entity can restrict any other
person or entity's usage of *any* content.
- Imported content retains the copyright, copying and licensing restrictions
of the original author (or source, if no copyright information was included
in the imported content.)
Every SDL distribution file containing imported content contains
originating copyright and licensing information

Small content segments may be copied under the public domain license
without attribution or licensing requirements[^1].
Any significant aggregation of content segments requires the GNU GPL
including its attribution and licensing requirements.

[Contributors](./CONTRIBUTORS.md)

#### Licensing
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
  - ~/src/cpp/inc/pub/memory.h (UNUSED/UNTESTED)
  - ~/src/cpp/lib/pub/Debug.cpp (Includes: <boost/stacktrace.hpp>)
- [BSD](.licenses/LICENSE.BSD-3)
  - ~/src/py/Sample/GUI/Qt/widgets-example.py
- [Creative Commons V3.0](.licenses/LICENSE.BY_SA-3.0)
  - ~/src/lily/Eskesen/* (Cheesy, but original, music)
  - ~/src/lily/Public/* (Lily representations of out of copyright music.)
  - ~/src/lily/Sample/* (Incomplete sample Lily layouts.
    Probably should be MIT or public domain license.)
- [Creative Commons V4.0](.licenses/LICENSE.BY_SA-4.0)
  - ~/src/java/Sample/Swing/Main.java
- [GNU GPL (General Public License)](.licenses/LICENSE.GPL-3.0)
  - Almost all source code.
- [GNU LGPL (Lesser General Public License)](.licenses/LICENSE.LGPL-3.0)
  - Almost all include files.
- [MIT License](.licenses/LICENSE.MIT)
  - Almost all Makefile segments, control and documentation files.
- [Public domain license](.licenses/LICENSE.ZERO) (Creative Commons CC0)
  - All control Makefile segments (in ~/src/cpp/ctl/.)
  - All BASH control files in ~/bat/.home/ except for .bash_logout.
    (.bash_logout contains no original content.)

#### Installation and running
These instructions assume that you'll be using the SDL package as delivered.

Use `. setupSDL` to set up the `$SDL_ROOT` environment variable and update
your `$PATH` environment variable.
(You might want to look at and modify setupSDL first.)

Now, from the installation root subdirectory,
use `make` for configuration control.
- `make` (without options) lists available options
- `make install` Creates or updates:
  - bin: Installs library and regression test prerequisite files.
  - obj/cpp: Installs lib (an include library) and dll (a dll library.)
- `make reinstall` Uninstalls and then installs. Use this after installing
a new Linux version.
- `make uninstall` Removes all files created by make install.
- `make check` Runs all available test suites.
- `make compile` Compiles (but does not test) all sample programs.
- `make clean` Removes all files created by make check and make compile.
- `make pristine` Essentially the same as running make clean followed by
make uninstall.

__TODO__ Implement C# and Java library installation Makefiles.
(Only C++ and Python library Makefiles are currently implemented.)

##### Virtualbox prerequisite packages:
(We recommend downloading and installing Virtualbox rather than using any
distribution provided packages.
When updating Virtualbox, always also update the extension pack.)
- bzip2 gcc make perl

<!-- --------------------------------------------------------------------- -->
##### Fedora prerequisite packages:
Bringup packages:
- firewall-config
- openssh-server openssh-client
  - (sudo systemctl enable sshd.service)
  - (sudo systemctl start  sshd.service)
- xterm
- xorg-x11-fonts-misc

Build environment packages:
- git
- autoconf
- automake
- binutils
- gcc
- gcc-c++
- gdb
- libtool
- make
- patch
- pkgconf

Build library packages:
- boost-devel
- bzip2-devel
- ImageMagick-c++-devel
- ImageMagick-devel
- libcurl-devel
- ncurses-devel
- glm-devel
- openssl-devel
- libdb-devel libdb-cxx-devel
- libX11-devel
- libxcb-devel
- libxcb-util-devel
- libxcb-util-image-devel
- libxcb-util-keysyms-devel
- libxcb-util-renderutil-devel
- libxcb-util-wm-devel
- libxcb-util-xrm-devel
- libXft-devel
- mariadb-server
  - (sudo systemctl enable mariadb)
- xcb-util-cursor-devel
- zlib-devel

Other (optional) packages:
- cvs
- gimp
- gpg
- yum-cron

<!-- --------------------------------------------------------------------- -->
##### Ubuntu prerequisite packages:
Bringup packages:
- firewall-config
- openssh-server openssh-client
  - sudo systemctl enable ssh.service
  - sudo systemctl start  ssh.service
- xterm

Build environment packages:
- git
- autoconf
- automake
- binutils
- gcc
- g++
- gdb
- libtool
- make
- patch
- pkgconf

Build library packages:
- ImageMagick-c++-devel
- ImageMagick-devel
- libboost-all-dev
- libbz2-dev
- libcurl4-openssl-dev
- libglm-dev
- libdb-dev   libdb++-dev
- libncurses-dev
- libssl-dev
- libx11-dev
- libxcb-dev
- libxcb-image0-dev
- libxcb-xfixes0-dev
- mariadb-server
- zlib1g-dev

Other (optional) packages:
- cvs
- gimp
- gpg
- yum-cron

#### The Distribution
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

#### Further reading
- [SDL C++ Reference manual](doc/cpp/REFERENCE.md)
(This is a work in progress.)
- [Development detail](doc/index.md)
- [Change log](./COMMIT.md)
