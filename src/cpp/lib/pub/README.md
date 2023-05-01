<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/src/lib/pub/README.md
//
// Purpose-
//       PUB library description
//
// Last change date-
//       2023/04/05
//
-------------------------------------------------------------------------- -->

# ~/src/lib/pub/README.md

Copyright (C) 2022-2023 Frank Eskesen.

This file is free content, distributed under the MIT license.
(See the accompanying file LICENSE.MIT or the original contained
within https://opensource.org/licenses/MIT)

----

This README describes the PUB (public) library.
It is a work in progress, as is this documentation which currently contains
a brief overview of featured functions.

----

PUB (public) is the currently preferred include library, found in
../../inc/pub.
This subdirectory contains the implementation code.
It contains updated versions of certain COM library functions with improved
performance and usability characteristics as well as additional functions.

## USAGE NOTE
There is no attempt to keep interfaces consistent across time, but we do
attempt to keep the interfaces consistent across the distribution.
When an interface changes, we attempt to also modify every usage within the
distribution to match the updated interface.
We do not recompile every distributed source module whenever a new trunk
is distributed. We do recompile every library source module.

----

## 2023/04/05
The ~/src/cpp/lib/pub/Test/TestDisp.cpp timing testcase has an unexpected and
large throughput performance improvement. No pub library code change explains
this improvement, so it's assumed that Linux kernel/library changes are
responsible. (This is good, since there was also no explanation of the
Linux throughput decrease between 3/2020 and 3/2023.)

See: ~/src/cpp/lib/pub/Test/.TIMING

----

### Design philosophy
- "It seemed like a good idea at the time"
Most code in this distribution was written to solve a particular problem,
either directly or indirectly in library code where a problem was likely
to arise again and again.
There is no separation of design and development.
Code is just written, designed on the fly.
Library code is written with the philosophy of get it working first, then get
error paths working, then (if needed) optimize.

- Static objects
The library contains static objects, and we have run into some of the known
problems that causes.
Some problems remain, and are next on the project "to do" list.
While static object construction is a well recognized problem, there is also an
analogous problem with static object deconstruction, especially with library
functions.
The basic question that needs to be answered is: When do we stop servicing
library functions?
The only good answer is, when we stop getting library function calls.
Since we can't tell when that might be, we have to carry on as best we can
even after our static object destructors are invoked.

Consider the Debug functions.
It's critical to keep these functions operational so that problems that occur
during static object deconstruction can be properly recorded.
This code needed to be and has been hardened.

Other library functions need to be kept at least semi-operational.
They should work as well as possible as long as possible, but never segfault.
(I'm talking about you, Trace.cpp, who used a map after it was deconstructed.
The segfault was in std::map code but the problem was in Trace.cpp.)

However, once we reach static object deconstruction there's no point in
worrying about memory leaks any more.
We can rely on process termination cleanup to clean up what we can't.

----

### Library includes

#### Debug.h<sup>1</sup>
This was one of the earliest library functions created.
Its iterfaces are more stable than most.

A debugging control object, writing to either stderr or stdout, and/or a
trace file. Both object and static function call methods are provided.

The debugf, debugh, errorf, errorh, tracef, traceh, functions all use the
printf interface.
Subroutine debugf writes to stdout and the trace file, errorf writes to stderr
and the trace file, and tracef writes only to the trace file. The debugh,
errorh, and traceh do the same but also prepend optional header information:
a time stamp and the current Thread.

A pointer to the first Debug object created is saved in Debug::common. This
object is automatically created if needed when any of the static debugging
functions are used. The default file name is "debug.out".

Using the static function calls:<br>

```
include <pub/Debug.h>
:
using namespace pub::debugging;     // For static pub debugging functions
```

The debugging write functions all use the standard printf and vprintf
interfaces.

- debugf(const char*, ...) writes to stdout and the trace file.

- debugh(const char*, ...) also writes to stdout and the trace file and
includes a heading, described later.

- errorf(const char*, ...) writes to stderr and the trace file.

- errorh(const char*, ...) also writes to stderr and the trace file and
includes a heading.

- throwf(const char*, ...) writes to stderr and the trace file, then throws
an exception. (The current implementation uses `throw "ShouldNotOccur";`, but
this is subject to change.)

- tracef(const char*, ...) writes (only) to the trace file.

- traceh(const char*, ...) also writes the trace file and includes a heading.

- vdebugf(const char*, va_list) writes to stdout and the trace file.

- vdebugh(const char*, va_list) writes to stdout and the trace file, including
a heading.

- verrorf(const char*, va_list) writes to stderr and the trace file.

- verrorh(const char*, va_list) writes to stderr and the trace file, including
a heading.

- vthrowf(const char*, va_list) writes to stderr and the trace file, then
throws an an exception.

- vtracef(const char*, va_list) writes to the trace file.

- vtraceh(const char*, va_list) writes to the trace file, including a heading.

The heading optionally includes the current time and current thread.
These use heading control interfaces, called separately.

The trace mode can be set to DEFAULT, IGNORE, or INTENSIVE.
When set to IGNORE, no writing to the trace file occurs.
When set to INTENSIVE, the trace file is closed and re-opened (append) after
each write.
(This was found necessary in certain environments to insure write completion.
A simple flush operation sometimes left the trace file incomplete.)

This is not a complete description of the Debug object or the pub::debugging
static methods. Please refer to the include file for additional information.

#### Diagnostic.h
This recent addition to the pub library was added to help locate problems
found in C++ version migration and the current dev library development.

Pristine.h defines a simple data block with content checking. You place one
before and after an area suspected of being clobbered by wild stores.

Aside:
Thread.cpp contains a map of managed Threads and started getting segfaults
in map code in Fedora Linux running dev library tests. Placing a Pristine
object before and after Thread's static map, no wild stores were found.
However, the map error actually occurred after Thread's static map object's
(and the Pristine object's) destructor were called. No wild stores involved,
just a misuse of static storage.

Namespace std::pub_diag was created in order to find shared_ptr<Stream>
objects that prevented Streams from being deleted.
Besides Diagnostic.h, a control file ~/src/cpp/inc/pub/bits/Diagnostic.i
needs to be included that (conditionally) defines make_shared as
pub_diag::make_debug, shared_ptr as pub_diag::debug_ptr, and weak_ptr as
pub_diag::dweak_ptr.
It also defines macros to be used in constructors and destructors of
objects that contain shared_ptrs.
This control file must be included *after* system includes but *before* any
headers that contain suspect shared_ptr objects.

Aside:
It turned out to be easier to write (and debug) this debugging code than to
track down the rogue shared_ptr<Stream> usage. The biggest problem was coding
the (global) dynamic_pointer_cast function correctly.
Cygwin's slow recompile time compounded the problem.
The dynamic_pointer_cast code was temporarily moved into a separate file that
wasn't dependency tracked so that it could be modified without requiring
recompiling the entire dev library.

#### Dispatch.h<sup>1</sup>
The atomic work dispatcher allows multiple threads to schedule work requests
while processing these requests under control of individual threads.

This dispatcher reduces the effort needed to create and control a
multi-threaded program.
It's one of the distribution highlights.

There are four dispatch control objects:

- pub::dispatch::Task (A work processor)

- pub::dispatch::Item (A work item)

- pub::dispatch::Done (A work completion processor)

- pub::dispatch::Disp (Controls timer work items and shutdown)

A Task runs under control of a single thread, so it doesn't have multi-thread
considerations for its own processing. More precisely, each time it is driven
it is single threaded. The actual thread can and will generally differ.
Each Task processes pub::dispatch::Items, one at a time, from virtual method
`work(pub::dispatch::Item*)`.
It can either enqueue the Item to another Task or, if work for the Item is
complete, post the Item.

An Item defines a work unit.
Items are (asychronously and atomically) enqueued to a Task using
Task::enqueue(Item*).
Each Task contains an AI_list (Atomic Insert List) of work items.
When enqueued, if the Task was idle (nothing already enqueued,) the Task is
scheduled using a WorkerPool thread.

The Done object describes what to do when an Item is posted. The Done object
pointer may be a nullptr, indicating that the Item can simply be deleted.
A Wait object is a subclass of the Done object.
Wait objects can be used to simply wait for an Item to be posted.

The (static) Disp methods allow timer work Items to be scheduled and driven
when the associated real-time delay has expired or the delay cancelled.
The Disp::wait method disallows new work Items to be scheduled and waits for
all outstanding work Items to complete.

The dispatcher relies upon other pub mechanisms.
It uses the AI_list object to provide a queue that can be used by any number
of threads for adding Items.
Only one thread (at a time) can remove items from that queue.
It uses the WorkerPool object to schedule Worker threads to drive Task
processing.

#### Select.h
Select.h was added to the library to support the experimental dev library.
While operational, it's fragile.

It contains a mechanism for inserting, modifying, and removing Socket polling
controls.
It also contains a select method, which performs the actual polling operation.

#### Socket.h
Socket.h wraps socket control into a C++ class.
Function names used in socket.h are preserved in the Socket class, so users
familiar with socket programming should find the Socket class relatively
easy to understand.
SSL_socket is derived from Socket, supporting encryption.

#### Thread.h
Thread.h expands std::thread, making it more similar to a Java Thread,
and allows a Thread to contain data it uses.

#### Trace.h
Trace.h defines memory trace functions.
A storage buffer contains a circular trace buffer for operation tracing.
Tracing operations have low overhead, are multi-thread capable and lock free.
This functionality is sometimes needed for debugging multi-threading programs,
and can be useful in other environments.

The xcb editor (~/src/cpp/Edit/Xcb) used this functionality during bringup
and been left enabled and active in the production version used for this
distribution's development.

----

[1] Migrated from the COM (common) library and improved by the PUB (public)
implementation.

----
