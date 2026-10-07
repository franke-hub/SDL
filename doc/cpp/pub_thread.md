<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
//
//       This file is free content, distributed under cc by-sa version 4.0
//       with attribution required.
//       (See accompanying file LICENSE.BY_SA-4.0 or the original contained
//       within https://creativecommons.org/licenses/by-sa/4.0/us/legalcode)
//
// SPDX-License-Identifier: CC-BY-SA-4.0
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/pub_thread.md
//
// Purpose-
//       Thread.h reference manual
//
// Last change date-
//       2026/10/07
//
-------------------------------------------------------------------------- -->
###### Defined in header <pub/Thread.h>
## pub::Thread

Thread is the base threading class.
Its pure virtual `run` method provides the Thread's operational logic,
which a subclass implements.

Types and constants:
- `typedef pthread_t handle_t;` The native thread handle type.
- `static const handle_t null_handle;` The handle of a Thread that isn't
active.
- `enum ITS { ITS_JOINABLE, ITS_DETACHED };` The initial Thread state,
specified when the Thread is [started](#start).

Thread::tlss (Thread Local Storage Struct) is an internal structure, not
for application use.

<!-- ===================================================================== -->
---
#### <a id="constructor">pub::Thread::Thread();</a>

Creates the Thread in an idle state.
The [start](#start) method begins Thread execution.

---
#### <a id="destructor">virtual pub::Thread::~Thread();</a>

Deletes the Thread.

A started Thread should be [joined](#join) or [detached](#detach) before
it's deleted. Deleting one that isn't is an error, which is recorded in the
system log. The destructor then attempts recovery:
- If the Thread deletes itself while running, it's automatically detached.
- Otherwise, the destructor waits for the Thread to complete, joining it.

Do not use a Thread after deleting it. In particular, if another thread is
joining the Thread, results are unpredictable.

---
#### <a id="current">static Thread* pub::Thread::current();</a>

Return value: The current Thread, or nullptr.

Only threads started by [start](#start) are Threads.
For any other thread, including the main thread, nullptr is returned.

---
#### <a id="debug">virtual void pub::Thread::debug(const char* info= "") const;</a>

Displays the Thread's state.
`info` is an optional caller-supplied identifying string included in the
display.

---
#### <a id="detach">void pub::Thread::detach();</a>

Detaches the Thread.
A detached Thread cannot be joined. Its system resources are released
automatically when it completes.
[current](#current) still returns the Thread while a detached Thread runs.

Detach may only be invoked once, and only from the running Thread itself.
Otherwise, a std::runtime_error exception is thrown.

---
#### <a id="get_handle">handle_t pub::Thread::get_handle() const;</a>

Return value: The native thread handle, or null_handle.

The handle is available from the time the Thread is started until it's
joined.
For a detached Thread, null_handle is returned.

---
#### <a id="get_max_threads">static size_t pub::Thread::get_max_threads();</a>

Return value: The maximum number of concurrently running Threads.
The default is 8192.

When this many Threads are running, [start](#start) waits until another
Thread completes.

---
#### <a id="join">void pub::Thread::join();</a>

Waits for the Thread to complete, then releases its system resources.
After a join completes, the Thread may be started again.

If invoked from the Thread itself, `errno` is set to `EDEADLK` and the
method returns without waiting. (The Thread could never complete.)

If the Thread was never started, was detached, or was already joined, a
std::runtime_error exception is thrown.

---
#### <a id="joinable">bool pub::Thread::joinable() const;</a>

Return value: TRUE iff the Thread can be [joined](#join).

---
#### <a id="run">virtual void pub::Thread::run() = 0;</a>

Operates the Thread. **Override this method.**

If `run` throws an exception, the exception is caught, a message and a
backtrace are written, and the Thread otherwise completes normally.

---
#### <a id="set_max_threads">static void pub::Thread::set_max_threads(size_t count);</a>

Sets the maximum number of concurrently running Threads.

If Threads start or complete while this method runs, the limit may
temporarily be inexact.

---
#### <a id="sleep">static void pub::Thread::sleep(double seconds);</a>

Delays the current thread, whether or not it's a Thread, for `seconds`
seconds.
The resolution is one microsecond. Shorter delays return immediately.

---
#### <a id="start">void pub::Thread::start(ITS its= ITS_JOINABLE);</a>

Starts the Thread, which then invokes [run](#run).

If `its` is `ITS_DETACHED`, the Thread starts in the detached state, as if
[detach](#detach) were the first statement in `run`.

If the maximum number of Threads is already running, start waits until
another Thread completes. (See [get_max_threads](#get_max_threads).)

A std::runtime_error exception is thrown if the Thread is already active or
if the system cannot create the thread.

__Note__ The Thread may complete, and could even be deleted, before start
returns. Applications needing to reference the Thread after start must
provide their own synchronization.

---
#### <a id="static_debug">static void pub::Thread::static_debug(const char* info= "");</a>

Displays Thread statistics, such as the number of Threads started, running,
and completed.
`info` is an optional caller-supplied identifying string included in the
display.

---
#### <a id="yield">static void pub::Thread::yield();</a>

Gives up the current thread's time slice, whether or not it's a Thread.

---
[Back](./Thread.md) | [Reference manual](./REFERENCE.md)
