<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
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
//       ~/doc/cpp/pub_thread.md
//
// Purpose-
//       Thread.h reference manual
//
// Last change date-
//       2026/02/20
//
// Documentation notes-
//       This is the fourth draft of this document. It needs more work.
//
-------------------------------------------------------------------------- -->
<!-- --------------------------------------------------------------------- -->
###### Defined in header <pub/Thread.h>
## `pub\::Thread`

Thread is the base threading class.
The (pure virtual) run method provides Thread operational logic which must be
implemented in a subclass.

#### Thread\::tlss_event
Fields:
  - drive_initialized: set by Thread\::drive when the tlss is initialized
  - start_completed: set by Thread\::start just before it exits

The tlss_event is constructed in-place in Thread\::tlss\::TES.

We (in-place) construct the tlss_event when the tlss is constructed and invoke
its destructor after waiting for drive_initialized and start_completed.
This somewhat convoluted mechanism allows us to release Event resources used
in initialization as soon as we're done using them.
(Each Event contains a condition variable and a std\::mutex.)
<!-- ---------------------------------------------------------------------
We don't know how useful this is.
- We don't know what system resources condition variables might require.
- A std\::mutex contains a waiting thread queue. The implementation is opaque.

Once we're done with the tlss_event, no threads will ever be waiting on them
again. That doesn't guarantee that system resources aren't still required.
---------------------------------------------------------------------- -->

#### Thread\::tlss
Thread\::tlss (Thread Local Storage Struct) is an internal class used to
maintain state information.

Fields:
- mutex: A RecursiveLatch serializing changes to the tlss itself
- fsm: A Finite State Machine
- thread: A pointer to the associated Thread (while it exists)
- handle: The associated pthread_t
- E: A pointer to the tlss_event (alway to TES)
- TES: (The storage area for the tlss_event structure)

#### Addressing Thread\::tlss
The Thread itself addresses the tlss using Thread\::tlss_, which is valid for
active Threads that have not been detached or joined.

An internal thread-local pointer, tl_tlss, locates the active Thread's tlss.

## Threading overview

- The Thread is constructed in an idle state.
- Thread\::start
  - Allocates the (internal) Thread\::tlss
  - Begins the Thread, starting Thread\::drive
- Thread\::drive
  - Sets tl_tlss, pointer to the (internal) Thread Local Storage Struct
  - Invokes Thread\::run (an application supplied virtual method that)
provides the Thread's execution logic.
    - Thread\::run completes, returning to Thread\::drive
  - For detached Threads, tlss storage is deleted
  - For other Threads, tlss storage ownership is passed to Thread. The tlss
will be deleted when Thread\::join is invoked.

## Implementation notes
Thread logic includes serialization logic not described here, but which is
well documented in the source code: `~/src/cpp/lib/pub/Thread.cpp`.

Of particular concern:
- Thread\::start and Thread\::drive synchronization.
- Thread\::detach, Thread\::join, and Thread\::~Thread synchronization.

Thread\::run is invoked using try..catch recovery.
If an exception occurs during run, an error message and a program backtrace
automatically occur but otherwise treated as if run returned normally.

#### <a id="constructor">Thread(void)</a>
Note: The copy constructor and assignment operator are deleted.
Threads cannot be copied or assigned.

Constructs the Thread, beginning in an idle state.
The Thread\::start method begins executing the Thread, eventually driving
the user-supplied virtual Thread\::run method.

#### <a id="destructor">~Thread(void)</a>
Deconstructs the Thread.

A Thread should not invoke the destructor without first invoking either
Thread\::detach or Thread\::join, making it inactive.

###### Destructor error recovery (active Thread deletion)
Active Thread deletion is an error, recorded in "~/.local/log/syslog.out".

If the running Thread invokes the destructor, it is automatically detached.
(Thread\::detach can only be invoked from a running Thread.)

If a completed Thread invokes the destructor, it is automatically joined.

Once an application invokes the destructor, if the application accesses it
later the storage is no longer a Thread.
Results will be unpredictable, unexpected, and generally bad.

#### <a id="debug">void debug(const char* info= "")</a>
Writes debugging messages displaying the Thread's content.

#### <a id="static_debug">void static_debug(const char* info= "")</a>
Writes debugging messages displaying Thread statistics.

<!-- Accessor methods                                                      -->

#### <a id="get_handle">handle_t get_handle(void)</a>
Returns the native Thread handle.
The current implementation uses a pthread_t.

The handle only applies when the Thread is running, i.e. in the run() method.

#### <a id="get_max">size_t get_max_threads(void)</a>
Returns the current max_threads counter.

A (static) Semaphore is used to limit the number of concurrently running
Threads. This value begins with get_max_threads().

Thread\::start invokes Semaphore\::wait, decrementing the count and continuing
or waiting (for a short time) and retrying.

When Thread\::run completes, Semaphore\::post allows another Thread to start.

#### <a id="set_max">void set_max_threads(size_t size)</a>
Updates the current max_threads counter and the Semaphore count.

Interaction with Thread\::start and Thread\::run completion may occur.
The Semaphore count will be precisely updated if Threads do not start or
complete while this method is running, but can be imprecise otherwise.

#### <a id="joinable">bool joinable(void)</a>
Returns `true` if and only if the Thread is in a joinable state.

<!-- Static public methods                                                 -->

#### <a id="current">Thread* current(void)</a>
Returns the current Thread.

Thread\::current only returns Thread* for threads created by a Thread\::start.
Thread\::current returns nullptr for threads created by any other mechanism.
(The main thread was not started by Thread\::start.)

#### <a id="sleep">void sleep(double delay)</a>
This delays the current execution thread (no matter how it was created) by
`delay` seconds.

#### <a id="yield">void yield(void)</a>
Causes the current execution thread (no matter how it was created) to yield
control to all other threads.

<!-- Methods                                                               -->

#### <a id="detach">void detach(void)</a>
Causes the current Thread to go from the active into the detached state.
Once detached, its state cannot change.

- Thread\::detach may only be invoked (once) and only from the running Thread.
- A detached Thread cannot be joined. Its system resources remain until it
completes, at which time they are automatically released.
- Thread\::current *is* valid and returns the Thread pointer for a detached
Thread.

#### <a id="join">void join(void)</a>
Wait for Thread to complete.

If invoked from the current Thread, `errno` is set to `EDEADLK` and no other
action is taken.
(The current Thread cannot complete while waiting for it to complete.)

Implementation note: join with timeout is not implemented.

#### <a id="run">virtual void run(void) = 0</a>
Subclasses *MUST* implement this pure virtual method.

This method provides application-defined execution logic.

#### <a id="start">void start(ITS its= ITS_JOINABLE)</a>
This begins Thread execution, invoking Thread\::run.

The `its` (Initial Thread State) parameter is set to Thread\::ITS_DETACHED
to start the Thread in the detached state.
This state is behaves as if `detach()` was the first statement in the
(subclass's) `Thread\::run` method.

<!-- Private methods                                                       -->

#### <a id="drive">private: void drive(void* _thread)</a>
This method:
- Completes Thread initialization
- Invokes Thread\::run
- Handles Thread termination

#### <a id="failure">private: void start_failure(void)</a>
Handle Thread\::start exceptions:
- Logging the failure in the Trace table
- Deleting the tlss (Thread Local Storage Struct)
- Logging the failure using System\::log.
- Invoking Debug\::throwf which:
  - Writes an error message to stdout and `./debug.out`.
  - Throws a std\::runtime_error (constructed using the error message as a
parameter.)

Implementation note: This method has never been tested.
