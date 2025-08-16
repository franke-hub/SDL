<!-- -------------------------------------------------------------------------
//
//       Copyright (C) 2023-2025 Frank Eskesen.
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
//       ~/COMMIT.md
//
// Purpose-
//       Contains brief descriptions of project commits.
//
// Last change date-
//       2025/08/16
//
//------------------------------------------------------------------------ -->
<!-- --------------------------------------------------------------------- -->

Copyright (C) 2023-2025 Frank Eskesen.

This file is free content, distributed under cc by-sa version 4.0
with attribution required.
(See accompanying file LICENSE.BY_SA-4.0 or the original contained
within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)

Minor changes are generally not documented here.
This distribution is maintained using git, which records every change.
The gitk viewer provides a line by line change view.

----

Commits are now sorted in date order rather than reverse date order.
(Oldest first, newest last)

[\[Last change date:\]](#most-recent-change) 2025/05/17

----

<!-- --------------------------------------------------------------------- -->
### 01/23/2023 maint commit
Distribution test: ~/src/java/Webapp/*

### 02/01/2023 maint commit
Distribution test: ~/src/java/Webapp/*

### 02/01/2023 trunk commit
Branch trunk merged with maint.

Includes updated java golfer files and miscellaneous copyright updates,
correcting either typos, formatting, or missing information.

### 05/19/2023 maint commit
Distribution test: Moved prerequisite build programs to ~/src/cpp/sys/.

### 05/25/2023 maint commit
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

### 07/16/2023 maint/trunk commit
(Primarily a documentation release.)

On Windows/Cygwin, when running certain DEV library stress tests the Windows
resource manager shows continual but small virtual storage growth, which
could/should indicate a memory leak.
The equivalent Linux top command does not show any storage growth.
Storage allocation and release trace hooks were added, but the memory leak
is yet to be found.

### 07/24/2023 maint commit
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

### 07/28/2023 trunk/maint commit
Fixed documentation problems, primarily with regard to using characters
"<" and ">".

### 07/30/2023 trunk/maint commit
Promoted Ioda from dev library into pub library.

### 08/05/2023 maint commit
Created configure/build Makefiles and fixed most compile problems found in full
recompile. (~/src/cpp/Clone/FSlist.cpp temporarily deprecated.)

### 08/06/2023 maint commit
Removed links that cannot be resolved in the distributed content, retaining
the associated "how to obtain it" information.

### 08/07/2023 maint/trunk commit
Fixed problems found in full recompile on Ubuntu test build machine running
GCC 11.4.0 and ImageMagick-6. With ImageMagick-6, src/cpp/NN2 does not
compile, so we changed its Makefile.BSD to reflect that fact but *ONLY*
on that build machine.
(Cygwin and Fedora's ImageMagick is version 7.)
Fixed ~/src/cpp/Clone/FSlist.cpp, so it's no longer deprecated.

### 08/23/2023 maint/trunk commit
Make now also controls Python installation.
Python 3.8 implemented PEP 517 and 518, changing the install mechanism.
This updated mechanism is now used.

PyQt5, which is required by the Golf program, is not installed by default.
PyQt5 requires qt5, which installs separately.

The stdio (Python C extention) installs using setuptools.
The local Python library currently installs using sudo to install a link to
the library subdirectory.

### 09/22/2023 maint/trunk commit
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

### 10/13/2023 maint/trunk commit
- XCB editor (~/src/cpp/Edit/Xcb)
  - New feature: ctrl-F9 copies current line to history line. (F9 without ctrl
copies the filename to the history line.)
- src/cpp/RFC: RFC7541.*
  - Almost all functions fully operational. (Pack::resize does not yet transfer
the resize request via an Ioda Writer/Reader.)
  - Added Ioda::dump function. It displays the entire Ioda buffer sequence.

### 11/16/2023 maint (test)
- Documentation update test (Verify .md files on github.)
- Removed Debug::errorp method. (Moved to ~/src/cpp/lib/pub/Fileman.cpp)

### 12/03/2023 maint
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

<!-- --------------------------------------------------------------------- -->
### 03/01/2024 maint
Note: This is a test commit.

- Updated makefiles
  - C++ libraries now use the same object files, building static libraries in
~/obj/cpp/lib/static and shared libraries in ~/obj/cpp/lib/shared/.
  - Use "@rm -f ..." instead of "-@rm ... >/dev/null 2>/dev/null" (Even
updating the now unsupported Windows makefiles.)
  - Empty last makefile lines have been removed.
  - The python library is now built using standard python mechanisms rather
than creating a link to the library source.

### 03/02/2024 maint/trunk
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

### 03/05/2024 maint

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

### 03/26/2024 maint

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

### 03/27/2024 maint

- The ~/obj/cpp/NN2 make process now writes an error message (but completes
normally) if ImageMagick-7 is not installed. User modification of the makefile
is not required.

### 04/02/2024 maint/trunk

Library restructure stable and ready for trunk merge.

Updated editxcb, restructuring and adding function.
See: [.README](src/cpp/Edit/Xcb/.README)

### 05/11/2024 maint

The ncurses and xcb editors are now nominally operational.

Too much is working not to share this version, but too much is not working
well enough for this to be a trunk commit.

The xcb editor has been refactored so that almost all functionality is
shared with the ncurses editor.

The EdInps and EdOuts modules are not shared. They provide the input and
output function for each of the editors.

The ncurses editor is compiled with `CDEFS  += -DUSE_NCURSES_VERSION` added
to the makefile.
This is only used in one place: In Edit.cpp function main(), the `opt_bg`
option is ignored in the ncurses version.
The curses editor always runs in the main thread.

What's not working:

- UTF-8 support.

When running an xterm a Fedora machine (locally or remotely,) a UTF-8
character sequence isn't recognized as UTF at all.
The easiest way to explain the result is that it's not usable.
You get jibberish on lines and the next line's display can be corrupted.
This certainly has to be fixed.

On Cygwin, UTF-8 sequences are displayed correctly, even including combined
character sequences.
Unfortunately, code that we rely on to position the cursor doesn't recognize
these sequences.
Since our cursor is mis-positioned at every point on the line after a
combined sequence, editing becomes problematic.

Neither the xcb editor nor the pub library handle combined characters, so
at least they agree on character positioning and editing works.
However, they both need to be fixed to handle these combined characters.
This looks like a difficult job.

- Cursor support

The ncurses editor currently lets ncurses take care of cursor positioning,
so the cursors look different.
We'll probably need to turn off ncurses cursor positioning and just use the
hide and show_cursor methods.
This should be but hasn't been relatively easy.

- Mouse cursor hiding

The xcb editor appears to have regressed. Hiding no longer works.
The ncurses editor doesn't have this function.

### 06/07/2024 maint

Updated ~/src/cpp/lib/pub/Utf.cpp and ~/src/cpp/inc/pub/Utf.h.
Added ~/src/cpp/lib/pub/Test/Test_utf.cpp and ~/src/cpp/inc/pub/bits/Utf.i.

The Utf.h updates use user allocated buffers for encoding and decoding rather
than the current automatically allocated buffers.
Test_utf.cpp partially tests the encoding and decoding portions of the
newly implemented code.

~/src/cpp/Edit/Xcb/EdOuts.cpp tested the new implementation in production
code. (The testing code was included but not activated in this commit.)

### 06/21/2024 maint

Completely tested all portions of all the utf*_decoder and utf*_encoder
implementations.

Fixed a minor library makefile glitch: Each library also had a build
dependency on its Test subdirectory, causing unnessary make operations
when that subdirectory changed.

### 06/21/2024 NOT DONE

~/src/cpp/Edit/Xcb/* and ~/src/cpp/Edit/Term/* modules need to use the updated
decoders and encoders.

This has no visible effect for Xcb/Editor modules but does require changes.
This editor does not support Utf combining characters.
It writes them as separate characters.

The (ncurses based) Term/Editor output already combined Utf combining
characters, but did not properly position the cursor.
Since the cursor wasn't accurate for combining characters, as a safety measure
we partially disabled Utf support.
Files containing non-ASCII Unicode characters were marked as damaged so they
could not be overwritten by the editor.

### 07/25/2024 maint

Revised utf*_decoder and utf*_encoder interfaces so that the current
column always matches the column as set by set_column().
This is a cleaner and more logical interface.

Since method decode() updates the offset and column number,
buffer[offset] applies to the column number of the next codepoint.
Method set_column() updates both the column and offset fields.
With this change buffer[offset] is the first character of a column, not
one past that character.

The editor modules have not been updated yet.
They (still) do not use the updated decoders and encoders.

### 08/14/2024 maint

Removed utf*_decoder set_offset and added set_cpoint method.
This removes the need for OFFSET_COLUMN, and is a cleaner and more logical
interface.

The editor modules have been updated to use the updated decoders and encoders.
These modules are now in beta test mode.
A complete function test is still required.

### 08/23/2024 maint

The combined editor is now in production test, with no known new errors.
Known error: include command after a line with a null terminator does not
build the undo/redo object correctly.

Added fixes: src/cpp/Edit/Xcb, src/cpp/Edit/Term

Renamed ~/src/cpp/Edit/Xcb/editxcb to xcbedit and
~/src/cpp/Edit/Term/editerm to xtmedit.
This also required changes to some scripts in ~/bat/.

Removed obsolete Utf objects.
Moved ~/src/cpp/inc/pub/Utf.i to ~/src/cpp/inc/pub/bits/Utf.i.
Moved ~/src/cpp/inc/pub/bits/Utf_types.i to ~/src/cpp/inc/pub/Utf.i.
~/pub/Utf.i is now the include used to define the Utf.h types in the
application's namespace.

### 08/27/2024 maint

The combined editor is now in production test, with no known new errors.

We spent a lot of time trying to implement common REDO functions in
EdRedo.hpp. This was wasted effort: callers still needed to test whether
or not an insert after a line with a binary delimiter occurred and handle it
differently in each case.

The include command (in EdBifs.cpp) was too difficult to fix. Rather than
botching the file when including a file after a binary line, now it's simply
disallowed. The line insert command (Alt-I) can be used to repair the
null-delimited binary line, then a file insert after it works properly.

Object counter tests were added to detect memory leaks. So far, so good.

We'll use the updated editor to restructure it. Redo, line, and message
functions shouldn't be in EdFile. While this restructure isn't strictly
necessary, it will be a good production test.
The Utf documentation has been in progress for a while. This and a full
editor function test need to be completed before a trunk release.

### 09/11/2024 preview

The combined editors (xcbedit and xtmedit) are now ready.

The Utf.h documentation is ready for github markdown verification.

The documentation updates and most of the code changes in this commit are
interrelated. The Symbol type was particularly helpful in simplifying the
documentation while also clarifying the interface definitions.

### 09/12/2024 trunk

Merged maint/preview into trunk.

### 09/18/2024 trunk

Removed boost::core::demangle.hpp dependency.

### 10/05/2024 maint/trunk

Added auto-recompile trigger for dev and pub libraries to account for a
pub::Object::~Object linkage change.

Restructured ~/src/cpp/Brian/.
- Updated the Command and Service objects.
- Added USE_STATIC environmental control to select whether to build a STATIC
or a SHARED (dynamic) library.

Regardless of whether static or shared library linkage is chosen, simply
adding an object module containing a static Command or Service object
automatically adds that Command or Service to its map.

### 10/22/2024 maint

~/src/cpp/Brian updated.
- Synchronized Brian/Console and ~/lib/cpp/pub/Console
   ~/lib/cpp/pub/Console decodes escape sequences, but doesn't do anything with
them.
- Added some type include files that might be useful. They might also be
discarded. (They bring some std and pub library includes into the current
namespace.)
- src/cpp/HTTP/* moved to src/cpp/HTTP/openssl
- src/cpp/HTTP/socket added
  - It contains an *extremely simple* HTTP server and a Curl-based HTTP client.
(This is something simple to use as a starting point for an HttpServer in
Brian.)
- The dispatch Wait object now returns the post code. Object destructors
were moved after the constructors and deleted assignment operators.

### 10/24/2024 maint

Cygwin on Windows 11 can sometimes modify a symbolic link so that it links
before the origin, e.g. "/../../var" becomes "/var" . This is actually what
Cygwin and Linux do natively, so ~/src/cpp/lib/pub/Fileman.cpp filename
resolver needs to accept this rather than reject it. This commit corrects it
along with the associated regression test.

### 11/25/2024 maint (With extensive diagnostics)

Looking at a Brian termination problem. Sometimes it completes and sometimes
it doesn't.
When it doesn't complete pub::Console appears to be in some sort
of loop.
(The loop is known, but exactly what's looping hasn't been determined yet.)

### 12/06/2024 maint (With extensive diagnostics, commented out)

The Brian termination problem hasn't been debugged.

Extensive diagnostics remain in the code, commented out so they don't affect
regression test output. (This applies to both the dev and the pub libraries.)

### 12/11/2024 maint

The 12/06/2024 maint comments still apply.

Added common inline conversion routines to ~/src/cpp/inc/pub/utility.i.

### 12/20/2024 maint/trunk

The Brian termination problem is Cygwin-only. It does not occur on Linux.
The diagnostics associated with this have been removed from the code and,
since it doesn't occur on Linux, we are leaving this as-is.

Additional Cygwin diagnostic information:
When running Brian under gdb and issuing two curl operations, a SEGFAULT
occurs with a garbage stack. The SEGFAULT does not appear unless running
under gdb.
Brian usually hangs waiting for a join to complete, even though the waiting
thread exited after the join request was made.

This commit prepares the trunk commit. If no problems are found, the maint
branch will be merged and the trunk committed.

### 12/24/2024 maint/trunk

Lots of physical changes but no logical changes.

All "include <assert.h>" statements changed to "include <cassert>".
All "include <ctype.h>" statements changed to "include <cctype>".
All "include <errno.h>" statements changed to "include <cerror>".
All "include <limits.h>" statements changed to "include <climits>".
and so on, in *all* files for all applicable include files.

The "~/obj/cpp/lib/pub/Test/regression" and "~/obj/cpp/lib/dev/Test/regression"
tests were successfully run, all cpp source files were compiled (including
files that aren't distributed,) and all object files were verified that they
ran, but not that they actually did anything useful since input files were
missing.

(We should, but don't, have regression scripts in all the object files.)

----

<!-- --------------------------------------------------------------------- -->
### 01/21/2025 maint

~/src/cpp/inc/pub/Signals.h was restructured to aid in debugging.

The test build machine fails to run ~/src/cpp/Brian, getting an exception
because of a string constructed from nullptr. This error does not occur in
other environments.

Debugging this problem hasn't been possible. It appears to be occuring during
startup complete Signal handling, but the event handler doesn't appear to be
invoked. The Signals restructuring requires all Events to be derived from a
new Signals::Event struct which is basically just a tag. Event handlers now
need to dynamically cast the provided basic Event type into the expected
derived type rather than simply rely on template type matching.

All current Signals usage has been revised to use the changed interface.
This updated interface should allow additional debugging code to be added more
easily.

Brian modules now also test for failure to delete shared_ptr objects.
Interestingly, adding debugging statements seems to affect whether or not the
shared_ptr's are left in limbo.

### 01/31/2025 maint

This commit was contains a back port of fixes so that it uses a host address
as specified in /etc/hosts, if one is present.

(Multiple local commits are included. It wasn't fixed exactly right the first
time.)

### 02/21/2025 maint/trunk

Fix: ~/bat/dnf.status updated for new interface.

~/src/cpp/Clone rdclient/rdserver testing was delayed due to travel plans, a
week-long illness, and a misconfiguration of the build test machine's
firewall. (TCP and UDP ports 1024-65535 were not enabled.)

These programs did not operate correctly on Cygwin since a change to
VirtualBox's virtual adapter code resulted in the virtual adapter's selection
as the host address. Since this address is unknown in the local lan, other
machines on the lan were unable to connect to it. Since programs in the
~/src/cpp/Clone subdirectory are based on the COM library, fixes to adjust to
the new VirtualBox adapter code needed to be back-ported.

### 03/01/2025 maint/trunk

Brian regression operational. Tweaking ~/src/cpp/lib/pub/Console.cpp was
required.

### 03/04/2025 maint/trunk

~/src/cpp/Edit/Term editor operational in all environments.

The actual problem was in ~/src/cpp/inc/com/Makefile.BSD, which caused the
ncurses library to be used instead of ncursesw.

### 04/02/2025 maint/trunk

Modified:
- ~/src/cpp/lib/pub/Clock.cpp and ~/src/cpp/inc/pub/Clock.h
  - (No significant changes. Added comments but no algorithmic changes.)

- ~/src/cpp/lib/pub/Test/Makefile.BSD
  - Changed default test object to build or test TestTime
  - Removed some meaningless comments

- ~/src/cpp/lib/pub/Test/script/regression.d/test_base.sh
  - Added regression test: TestTime

Added:
- ~/src/cpp/inc/pub/Calendar.h   (The Calendar interface)
- ~/src/cpp/inc/pub/Julian.h     (The Julian interface)
- ~/src/cpp/lib/pub/Calendar.cpp (Most of the Calendar.h implementation)
- ~/src/cpp/lib/pub/Cal400.cpp   (Calendar::day2ymd table implementation)
- ~/src/cpp/lib/pub/Julian.cpp   (Julian.h implementation)
- ~/src/cpp/lib/pub/Test/.H      (A link ~/src/cpp/inc/pub)
- ~/src/cpp/lib/pub/Test/TestTime.cpp (The Calendar, Clock and Julian
regression test)

The Calendar and Julian modules are loosely based on the COM library versions.
(This is part of the work needed to convert ~/src/cpp/Clone to the PUB
library.)

This is likely to be the last change to the SDL project for quite a while as
focus shifts to a new project that documents algorithms.

There is, however, some known work left to do in the SDL project:
- Convert ~/src/cpp/Clone to use the PUB library rather than the COM library.
- One algorithm (yet to be documented) does not have a PUB library reference
implementation.

### 2025/04/24 maint/trunk (edited)

Added:
- ~/bat/restore
Similar to ~/bat/backup, but using rsync fetch rather than store.

Modified:
- ~/doc
Added description of an obscure problem along with its solution.
Other sections were rewritten for clarity.

- ~/src/cpp/Edit/Xcb/.README, ~/src/cpp/Edit/Term/.README
Reformatted .README information for clarity.

- ~/bat/editgpg, ~/bat/fromgpg, ~/bat/intogpg, and ~/bat/viewgpg

These scripts now allow selection of the public key to be used for encryption.
(The key to be used is specified in your "~/.gnupg/gpg.conf" file.)
- Modify the file specifying:
  - "default-recipient-self"
  - "default-key=" (the ID or fingerprint the public key you want to use for
encryption.)

***MAKE SURE*** that you specify an ID of a key that you can decrypt, i.e. the
ID of a key that's on your secret key list.

----

### 2025/04/25 trunk/maint

Commits are now sorted in date order rather than reverse date order.

Removed unnecessary error checking in the Select::~Select (destructor) method
implemented in ~/src/cpp/lib/pub/Select.cpp.
Now that Sockets don't reference Select objects, it's not an issue whether or
not Socket objects appear in a Select object's database.

----

### 2025/05/03 trunk/maint

This commit is "much ado about nothing."

I'm having trouble getting a Windows share working. As part of my attempted
repair, I removed and restored the share. Well, not such a good idea.

Among other things, the share contains the SDL repository. Windows thought it
would be a good idea to reset all the Cygwin settings to the default, which
made all the files executable. GIT thought those files whose permissions had
changed were modifiable, so I had a boatload of file mode restorations to do.
Too many to do by hand.

So I heavily modified ~/src/cpp/Fileman/Scanner.cpp, which I use to verify
the distribution (when I run it.) As part of the change I also modified the
SA30 and SA40 copyright, removing a gratuitous comma. The html copyright
format was changed to have leading '//' in each line.

But the big problem was determining whether or not a file should have execute
permissions. The huristic chosen was to check whether or not a file began with
a "#!" heading. If so, it was an executable script. If not it was a plain
file. This was mostly OK, but some files had the "#!" heading and weren't
really meant to be called directly: They were sourced bash scripts or imported
python files. Now that the headers have been corrected, automatic permission
setting is just a matter of running the scanner program (in ~/obj/cpp/Fileman)

I have two updates in progress that are not ready to go yet. At least they
can be worked on without a zillion other updates pending.

So, lots of updates are included but there's not a lot of real changes.
- ~/src/cpp/Fileman/Scanner.cpp is a real change. These options are now
completely independent:
  - copyright verification (including last change date)
  - file permission mode checking using '#!' header for scripts
  - unix file format checking (including trailing blanks in lines)
- SA30 and SA40 copyrights removed an unneeded comma, and are now more
consistent with other copyright formats.
- Script v. source file is determined by '#!' beginning the data.
- ~/src/cpp/lib/pub/Fileman was updated to make its error messages more
consistent with other such messages in the library.

----

### 2025/05/06 trunk/maint

Change log:
- ~/src/cpp/Edit/Xcb/EdInps.*, ~/src/cpp/Edit/Term/EdInps.*:
Fixed an annoying Editor interaction between backspace and F10, F11, and F12.
- ~/src/cpp/inc/pub/List.h: Moved internal documentation out of List module.
Added some error checking for "should not occur" conditions.
- ~/src/cpp/Fileman/Scanner.cpp: Added some file types to the list of binary
types.
- Exposed some methods that were accidently hidden. These errors were
detected by gcc version 15.1.1.

----

### <a id=most-recent-change>2025/05/17 trunk/maint</a>

The appropriate SPDX-License-Identifier was added to each copyright header.

~/src/cpp/Fileman/Scanner.cpp automated this task.

A simple check was added to pub::List::insert to detect inverted after and
link parameters.

----

### 2025/08/16 trunk/maint

Change log:
- ~/bat/make-temp, ~/bat/*gpg:
Now save stderr output in a /tmp file for use in case there's an error.
Script make-temp creates the temporary file name.
(The /tmp file is removed before the script exits.)
- ~/src/cpp/Edit/Xcb/EdFile.cpp:
Did not handle no-delimiter line properly. This caused an error message when
writing files with no final '\n' delimiter even though the file was written
without error.
- ~/src/cpp/Edit/Xcb, ~/src/cpp/Edit/Temp:
This is only a small structure change. No function was added or removed.
- ~/src/cpp/EiDB/data:
A (737M) data source file was recovered from an old CD. Since it's so large,
the change only documents that the file is available upon request.
