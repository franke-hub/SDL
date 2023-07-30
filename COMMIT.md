<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2023 Frank Eskesen.
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
//       2023/07/30
//
//------------------------------------------------------------------------ -->

# ~/COMMIT.md

Copyright (C) 2023 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See the accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

Minor changes are not documented in this change log, but since the distribution
is maintained in git, changes are always recorded.

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
