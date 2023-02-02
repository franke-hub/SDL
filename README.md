<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2020-2023 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See the accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/README.md
//
// Purpose-
//       Project overview information.
//
// Last change date-
//       2023/02/01
//
//------------------------------------------------------------------------ -->

# ~/README.md

Copyright (C) 2020-2023 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See the accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

#### Content overview
This package contains multiple language examples.
The C++, Python, and Java examples also include libraries as well as working
example programs.
A bash script library is also provided.

All libraries, sample programs and utilities are built using make from an
object library, with one exception: makeproj.
This program examines C/C++ programs looking for #include files, creating
a makefile dependency list.

The project Wiki contains complete build instructions.

#### Licensing
This is an open source project, consisting of free content distributed under
multiple open source licenses.

Most source code is distributed under the GNU Public License.
Most include headers are distributed under the Lesser GPL.
Some exceptions exist.
For example, ~/src/cpp/inc/pub/memory.h essentially uses and adds nothing to
the Boost atomic_shared_ptr and is distributed under the BOOST license.

Documentation, such as this file, and control files (e.g. make files) are
normally distributed using the MIT license.
The Creative Commons license is used (at least) for Lilypond (music) files.
Code intended to be used as skeleton code samples or cut and paste fragments
generally use the un-license, given to the public domain.

All content is distributed AS-IS, without purpose or warranty of any kind;
not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

#### The installation subdirectories
- bat: The bash script library
- src: Source files
- src/cpp: C++ source files, including five libraries
- src/java: Java language source files, including a minimal library.
- src/mcs: C# source files, including a library
- src/py: Python source files, including a library

The bin subdirectory is a placeholder, currently unused.

The usr subdirectory contains files that are not distributed.
It's used for local machine to machine copying.

#### Implementation notes-
The trunk branch is relatively well tested, but the maint branch is more up to
date.
The maint branch is occasionally used for distribution testing, checking that
required files are present.

This distribution is kept locally in /home/data/home/.
In addition to this public distribution, the /home/data/. directory contains
information shared between multiple machines.

Links into /home/data that do not also link to /home/data/home
are used at times similarly to links to /home/data/usr/.
In particular, java web applications cannot use links when running
in Cygwin, so we use a direct path to /home/data/web/database/
instead.

#### Further reading-

[Development Journal](/src/doc/Journal/Journal.md)

[DEV library](/src/cpp/lib/dev/README.md)

[PUB library](/src/cpp/lib/pub/README.md)

