<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2022-2024 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/REFEFENCE.md
//
// Purpose-
//       SDL Distribution reference manual
//
// Last change date-
//       2024/08/30
//
-------------------------------------------------------------------------- -->

# Software Development Lab (SDL) C++ Library reference manual

This reference manual describes SDL C++ interfaces provided by the
distribution.

### The COM Library

The COM (common) library uses namespace `com`.
It defines general-purpose utility objects.

While this library is still maintined, much of its functionality has been
moved to the PUB library.

The source code provides the only documentation.

### The DEV Library

The DEV (development) library uses namespace `pub`.
It defines HTTP (HTTP/1 only) control utility objects.

When ready, this library will be moved to the PUB library.
It is currently experimental.
(Currently ~/src/cpp/inc/pub/http links to ~/src/cpp/inc/dev/.)

The source code currently provides the only documentation.
When ready, documentation will be added here.

### The GUI Library

The GUI (Graphical User Interface) library uses namespace `gui`.
It defines GUI objects used by the XCB editor (~/src/cpp/Edit/Xcb/*,)
replacing the library now in ~/src/cpp/inc/.OBSOLETE/gui.

The source code provides the only documentation.

### The OBJ Library

The OBJ (object) library uses namespace `obj`.
It defines automatic thread-safe storage management capabilities.

The source code provides the only documentation.

### The PUB Library

The PUB (public) library uses namespace `pub`.
It defines general-purpose utility objects.

While a smattering of Doxygen source documentation exists, it will be removed
after completing this documentation.

__TODO__ Update in progress.

- [Allocator.h:](./Allocator.md) An experimental storage allocator.
(It's use is not currently recommended.)
- [ASCII.h:](../../src/cpp/inc/pub/ASCII.h) A list of ASCII characters.
This is intended as a reference rather than an include file.
- [Clock.h:](./Clock.md) An epoch-offset clock, represented as a double.
- [config.h:](./config.md) Provides a (minimal) set of user macros.
The SDL libraries avoid using macros, preferring `enum` where possible.
(Since there is no macro namespace, avoiding macros helps avoid name
collisions.)
- [Console.h:](./Console.md) Wrappers for console I/O functions.
- [Debug.h:](./Debug.md) Debugging tools
- [diag-shared_ptr.h:](./diag-shared_ptr.md) A diagnostic tool for debugging
std::shared_ptr and std::weak_ptr usage problems.
- [diag-pristine.h:](./diag-pristine.md) A diagnostic tool for debugging
"wild store" problems.
- [Dispatch.h:](./Dispatch.md) Provides lock-free multi-threading control
mechanisms.
- Event.h: Provides a wait/post event handling mechanism.
- Exception.h: (to be removed?) Provides a base Exception.
- Fileman.h: Provides file handling utility functions.
- Hardware.h: Mechanisms for reading the link pointer, stack pointer, and
timestamp counter.
- http Subdirectory containing HTTP Client/Server includes.
(This is a work in progress, and currently only supports HTTP/1.)
- ifmacro.h: Provides a set of compilation controls.
(deprecated. To be removed.)
If really wanted, use ~/src/cpp/inc/com/ifmacro.h instead.
- Interval.h: Provides an interval timer.
- Latch.h: Provides (lockable) spin latches, which can sometimes be more
useful than a mutex. There is also a latch type that can be accessed either
in shared or exclusive mode.
- List.h: Provides AI_List, DHDL_list, DHSL_list, and SHSL_list: respectively
Atomic Insert list,
Doubly Headed Doubly Linked list,
Doubly Headed Singly Linked list, and
Singly Headed Singly Linked list.
It also provides List, an alias for DHDL_list.
- [List.h:](./List.md) (Currently only documents AI_list.)
- Lock.h: Provides a process-wide named lock.
- memory.h: Implements atomic_shared_ptr<class T>, currently via boost.
- Must.h: Provides utility routines that throw std::bad_alloc instead of
returning nullptr.
- Mutex.h: (This is simply a std::mutex and an Object.)
- Named.h: Adds the capability for a class to have a name.
- Number.h: Number holds a numeric integer value of any (byte) length > 8.
- Object.h: A base class for Objects providing utility functions.
- Parser.h: Provides a simple parameter file parser.
- Properties.h: Provides a name/value string pair mapping.
- Random.h: Provides a simple thread-safe random number generator.
- Reporter.h: Provides a generic mechanism for statistical event recording and
reporting. __TODO__ Event status checking.
- Select.h: Provides a socket polling controller and selector.
- Semaphore.h: Provides wait/post semaphore functions.
- [Signals.h:](./Signals.md) Provides a "signals and slots" mechanism.
- Socket.h: Provides socket interfaces.
- Statistic.h: Provides statistical measurement object.
- SubAllocator.h: (Placeholder: not implemented)
- TEST.H: (Note all caps name) Provides test case error checking tools.
- Thread.h: Provides a thread representaion.
- Tokenizer.h: A string tokenizer
- Trace.h: Provides circular trace table controls. When used in conjunction
with a memory-mapped file, handy for debugging multi-threading problems.
- [Utf.h:](./Utf.md) Provides UTF decoding and encoding functions.
- utility.h: A set of utility functions
- [Worker.h:](./Worker.md) Worker is an interface class that provides thread
scheduling via WorkerPool, a thread pool manager.
- Wrapper.h: A generic program wrapper with user lambda functions exits.

---
[Back](../index.md)
