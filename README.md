#### Copyright (C) 2020-2021 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

Last change date: 2021/01/25

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

#### The installation subdirectories
- bat: The bash script library
- src: Source files
- src/cpp: C++ source files, including four libraries
- src/java: Java language source files, including a minimal library.
- src/mcs: C# source files, including a library
- src/py: Python source files, including a library

The bin subdirectory is a placeholder, currently unused.

The usr subdirectory contains files that are not distributed.
It's used for local machine to machine copying.

#### Implementation notes-
This distribution is kept locally in /home/data/home/.
In addition to this public distribution, the /home/data/. directory contains
information shared between multiple machines.

Links into /home/data that do not also link to /home/data/home
are used at times similarly to links to /home/data/usr/.
In particular, java web applications cannot use links when running
in Cygwin, so we use a direct path to /home/data/web/database/
instead.
