<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2023-2024 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/COMMIT.md
//
// Purpose-
//       Contains brief descriptions of project commits.
//
// Last change date-
//       2024/03/26
//
//------------------------------------------------------------------------ -->

# ~/COMMIT.md

Copyright (C) 2023-2024 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See the accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

Minor changes are not documented in this change log, but since the distribution
is maintained in git, changes are always recorded.

----

#### 03/26/2024 maint

This is a maintenance commit.

Existing known anomalies:
- Running make compile, ~/obj/cpp/Stock once attempted to link before the
object files were compiled. (Not reproducible.)
- ~/obj/cpp/HTTP/SampleSSL loops. (Not diagnosed.)

Changes:
- The C++ library build has been restructured.
  - Static libraries are built in ~/obj/cpp/lib/static
  - Shared libraries are built in ~/obj/cpp/lib/shared
  - In CYGWIN, the shared libraries are DLLs rather than unix libraries
- Documentation has been updated. This work is incomplete.
- ~/bat/.home contains recommended bash initialization scripts.

The C++ library build needs more testing, especially in regard to dependency
resolution.
Perhaps the ${module} variable can be used to make the Makefile.BSD more
common.

----

#### 03/05/2024 maint

- Partially updated Debug.h documentation, correcting links and missing
information.
- Removed pub::Debug::clr_head and changed pub::Debug::set_head to set
complete heading rather than set heading bit.
- Fixed ~/src/cpp/lib/dev/Test/T_Stream.hpp, adding a short delay to allow
threads to complete. (Otherwise Stream object count error could occur.)
- Fixed ~/src/cpp/lib/dev/Test/script/regression.d/.prereq.sh and
~/src/cpp/lib/pub/Test/script/regression.d/.prereq.sh, making ~/obj/cpp/lib
rather than just the associated library.
- Fixed static library build, removing include of dependent libraries. This
was just wasted filler.
- Corrected some static library build dependencies.

----

#### 03/02/2024 maint/trunk
- Fixed one minor problem in Makefile (Remove operations are not displayed.)

The C++ library build has been modified so that the shared and static libraries
use the same set of object files.
The static libraries are copied to ~/obj/cpp/lib/static/.
The shared libraries are copied to ~/obj/cpp/lib/shared/.

When building, your L link that points to the library should point to the
static or shared library.

To use the C++ shared library:
Your "L" library link should link to ~/.local/lib/sdlc++ (which is a link to
~/obj/cpp/lib/shared created during shared library installation.)

On Cygwin, add ~/.local/lib/sdlc++ to your PATH
On Linux,  add ~/.local/lib/sdlc++ to your LD_LIBRARY_PATH

To use the C++ static library:
Your "L" (library link) should link to either ~/obj/cpp/L, ~/obj/cpp/lib/L,
or ~/obj/cpp/lib/static/. It SHOULD NOT link to ~/obj/cpp/lib, which contains
both the static and (incomplete) shared library objects.

----

#### 03/01/2024 maint
Note: This is a test commit.

- Updated makefiles
  - C++ libraries now use the same object files, building static libraries in
~/obj/cpp/lib/static and shared libraries in ~/obj/cpp/lib/shared/.
  - Use "@rm -f ..." instead of "-@rm ... >/dev/null 2>/dev/null" (Even
updating the now unsupported Windows makefiles.)
  - Empty last makefile lines have been removed.
  - The python library is now built using standard python mechanisms rather
than creating a link to the library source.

----

#### 12/03/2023 maint
- Added "make update" feature.
  - File ~/bat/sys/_want.version contains a list of desired versions
  - Various libraries and components contain _have.version version files
  - We update the _want.version file when a recompile is needed even when
include dependencies are met.
  - Make update recompiles the updated components.
- Doxygen now built from ~/doc/obj/Doxygen
  - This is the only documentation for the COM, DEV, GUI, and OBJ libraries
  - Only the PUB library has any Doxygen markup, and that is minimal.
Githup markup documentation in progress for the PUB library.

----

#### 11/16/2023 maint (test)
- Documentation update test (Verify .md files on github.)
- Removed Debug::errorp method. (Moved to ~/src/cpp/lib/pub/Fileman.cpp)

----

#### 10/13/2023 maint/trunk commit
- XCB editor (~/src/cpp/Edit/Xcb)
  - New feature: ctrl-F9 copies current line to history line. (F9 without ctrl
copies the filename to the history line.)
- src/cpp/RFC: RFC7541.*
  - Almost all functions fully operational. (Pack::resize does not yet transfer
the resize request via an Ioda Writer/Reader.)
  - Added Ioda::dump function. It displays the entire Ioda buffer sequence.

----

#### 09/22/2023 maint/trunk commit
- XCB editor (~/src/cpp/Edit/Xcb)
  - New feature: If not an editor command, run as a system command and display
output in a new pseudo file.
  - New features: 'help' command and 'set help' command.
  - New features: Added common ctrl- keys: ctrl-S, ctrl-Q, ctrl-Y, and ctrl-Z.
  - Added 'TIO' misspelling of 'TOP' command.
- List.h (~/src/cpp/inc/pub/List.h, ~/src/cpp/inc/pub/bits/List.h
  - Fixed SHSL_list iterator. It shouldn't be destructive.
  - Added '= delete' constructors and assignment operators. Since all lists use
link references rather than copies, none of the list objects can be copied.
List objects can be moved, but that functionality isn't needed (yet.)
- src/cpp/RFC: RFC7541.*
  - Huffman decoding and encoding are fully operational. (Timing tests and
production tests have not been run.)
  - HPACK compression has a preliminary interface defined. The interface
implementation is scaffolded and compile tested. There's a lot left to do.
- src/cpp/RFC: other content, not ready for distribution.
  - README.md: Documents the RFC7541 implementation process. At this point
it's poorly written and contains a lot of muddled thinking. Once the writing
is satisfactory it can be added with muddled thinking included.
  - RFC7540.h/cpp: A placeholder for HTTP/2 functionality. It's currently only
scaffolding and might not ever be used.

----

#### 08/23/2023 maint/trunk commit
Make now also controls Python installation.
Python 3.8 implemented PEP 517 and 518, changing the install mechanism.
This updated mechanism is now used.

PyQt5, which is required by the Golf program, is not installed by default.
PyQt5 requires qt5, which installs separately.

The stdio (Python C extention) installs using setuptools.
The local Python library currently installs using sudo to install a link to
the library subdirectory.

----

#### 08/07/2023 maint/trunk commit
Fixed problems found in full recompile on Ubuntu test build machine running
GCC 11.4.0 and ImageMagick-6. With ImageMagick-6, src/cpp/NN2 does not
compile, so we changed its Makefile.BSD to reflect that fact but *ONLY*
on that build machine.
(Cygwin and Fedora's ImageMagick is version 7.)
Fixed ~/src/cpp/Clone/FSlist.cpp, so it's no longer deprecated.

----

#### 08/06/2023 maint commit
Removed links that cannot be resolved in the distributed content, retaining
the associated "how to obtain it" information.

----

#### 08/05/2023 maint commit
Created configure/build Makefiles and fixed most compile problems found in full
recompile. (~/src/cpp/Clone/FSlist.cpp temporarily deprecated.)

----

#### 07/30/2023 trunk/maint commit
Promoted Ioda from dev library into pub library.

----

#### 07/28/2023 trunk/maint commit
Fixed documentation problems, primarily with regard to using characters
"<" and ">".

#### 07/24/2023 maint commit
(Primarily a documentation test release.)

This is a preliminary documentation release needed to verify that all the
updated documentation is actually provided and not, for example, hidden by
.gitignore.

The public domain license was changed from the "un-license" to the
Creative Commons CC0 public domain license.
This seems to be more worldwide in scope.

It does have a trade mark and patent restriction clause that might give
pause to copiers, but the README.md "Copying" section essentially asserts
that trade mark or patent restrictions do not and will not apply.

----

#### 07/16/2023 maint/trunk commit
(Primarily a documentation release.)

On Windows/Cygwin, when running certain DEV library stress tests the Windows
resource manager shows continual but small virtual storage growth, which
could/should indicate a memory leak.
The equivalent Linux top command does not show any storage growth.
Storage allocation and release trace hooks were added, but the memory leak
is yet to be found.

#### 05/25/2023 maint commit
(A synchronization commit with debugging hooks left in the code.
Most of these hooks need to be removed before a trunk commit.)

Issue #2 has been corrected, but dispatch::Task::~Task still has issues.
The FC_UNDEF wait request doesn't complete, so the destructor doesn't complete
and the Client and Server destructors don't complete. (Client and Server
objects contain dispatch::Task objects.)

Isolating the the problem to the dispatch::Task destructor proved exceedingly
difficult and compounded by the FC_UNDEF wait request (lack of) completion
issue. Minor ancillary code base modifications were made while waiting for
some flash of inspiration for ways to debug the problem.

This commit synchronizes the local and remote maint branch. This makes it
easier to focus on the FC_UNDEF processing issue.

#### 05/19/2023 maint commit
Distribution test: Moved prerequisite build programs to ~/src/cpp/sys/.

#### 02/01/2023 trunk commit
Branch trunk merged with maint.

Includes updated java golfer files and miscellaneous copyright updates,
correcting either typos, formatting, or missing information.

#### 02/01/2023 maint commit
Distribution test: ~/src/java/Webapp/*

#### 01/23/2023 maint commit
Distribution test: ~/src/java/Webapp/*
