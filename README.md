<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2020-2023 Frank Eskesen.
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
//       2023/02/01
//
//------------------------------------------------------------------------ -->

# ~/README.md

Copyright (C) 2020-2023 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See the accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

#### Content overview
This is the Software Development Laboratory (SDL) distribution package,
sometimes referred to as "The Distribution".

This package contains multiple language examples:
- C++, Python, and Java examples also include libraries and working example programs.
- A (mostly) bash script library.

Compiled libraries, sample programs and utilities are generally built using
make from the associated object subdirectory.

Subdirectory structure:

```
(Root) The distribution root directory, designated as ~
   ~
   |
   [-- bat ((Mostly bash) scripts)
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
   |    [--  [java,js,mcs,py]/lib
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
   |    [--  [java,mcs,py]/lib
```

Libraries are built using make from the ~/obj/*/lib subdirectory.

All C++ libraries have an associated Test subdirectory:
`~/src/cpp/lib/*/Test`
The dev and pub libraries also have a regression test script, `regression`,
used for distribution testing.

For C++ programs, two build helper programs are required.
These programs are built in the `~/obj/cpp/sys/` subdirectory and installed
into $(HOME)/bin using `make install`.
(The bin directory location is not currrently configurable.)

- makeproj: Used to create or update dependencies.
- filecomp: Used in regression testing to (partially) compare files.

In addition to the libraries, (many) sample programs are provided.

__TODO__ Document the sample programs.

The project Wiki contains complete build instructions starting from an
"Ubuntu Desktop for developers" package.

#### Licensing
All content is distributed AS-IS, without purpose or warranty of any kind;
not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

This is an open source project, consisting of free content distributed under
open source licenses.

Most source code is distributed under the GNU Public License.
Most include headers are distributed under the Lesser GPL.

Exceptions exist for imported code.
For example, ~/src/cpp/inc/pub/memory.h essentially uses and adds nothing to
the Boost atomic_shared_ptr and is distributed under the BOOST license.

Documentation, such as this file, and control files (e.g. make files) are
normally distributed using the MIT license. (No attribution is required.)

The Creative Commons license is used (at least) for Lilypond (music) files.
Code intended to be used as skeleton code samples or cut and paste fragments
generally use the un-license, given to the public domain.

#### Usage notes-
The trunk branch is relatively well tested;
The maint branch is more current but is sometimes used for distribution
testing.

The distribution is kept locally in /home/data/home/.
In addition to this public distribution, the /home/data/. directory contains
information shared between multiple machines.
A build test (virtual) machine only updates source files from github.

Links into /home/data that do not also link to /home/data/home
are used at times similarly to links to /home/data/usr/.
In particular, java web applications cannot use links when running
in Cygwin, so we use a direct path to /home/data/web/database/ instead.

#### Further reading-

- [Documentation](src/doc/index.md)
- [Latest Commit](./COMMIT.md)
