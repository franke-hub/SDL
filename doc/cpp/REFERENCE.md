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
//       ~/doc/cpp/REFEFENCE.md
//
// Purpose-
//       SDL Distribution reference manual
//
// Last change date-
//       2023/11/09
//
-------------------------------------------------------------------------- -->

# Software Development Lab (SDL) C++ Library reference manual

__Note__
TODO: This manual is incomplete.
It currently only documents Dispatch.h and associated objects.
- Debug.h: (partially documented since it's used by Dispatch.h's example.)
- Dispatch.h: The lock free multi-threading dispatcher
- List.h: The AI_list is a key to the Dispatch scheduling mechanism.
- Worker.h: provides the base class for pub::dispatch::Task.

This reference manual describes SDL C++ interfaces provided by the distribution.

- [Allocator.h:](./Allocator.md) An experimental storage allocator.
(It's use is not currently recommended.)
- [ASCII.h:](../../src/cpp/inc/pub/ASCII.h) A list of ASCII characters.
This is intended as a reference rather than an include file.
- [Clock.h:](./Clock.md) An epoch-offset clock, represented as a double.
- [config.h:](./config.md) Provides a (minimal) set of user macros.
- [Console.h:](./Console.md) Wrappers for console I/O functions.
- [Debug.h:](./Debug.md) Debugging tools
- Diagnostic.h: (to be renamed) Provides a mechanism for diagnosing shared_ptr
and weak_ptr usage problems.
- [Dispatch.h:](./Dispatch.md)
Dispatch provides lock-free multi-threading control mechanisms.
- Event.h: Provides a wait/post event handling mechanism.
- Exception.h: (to be removed?) Provides a base Exception.
- Fileman.h: Provides file handling utility functions.
- Hardware.h: Mechanisms for reading the link pointer, stack pointer, and
timestamp counter.
- http Subdirectory containing HTTP Client/Server objects. (This is a work
in progress, and currently only supports HTTP/1.)
- ifmacro.h: (deprecated) A set of compilation controls.
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
- [List.h:](./List.md) The AI_list.
- Lock.h: Provides a process named lock.
- memory.h: Implements atomic_shared_ptr<class T>, currently via boost.
- Must.h: Provides utility routines that throw std::bad_alloc instead of
returning nullptr.
- Mutex.h: (This is simply a std::mutex and an Object.)
- Named.h: Adds the capability for a class to have a name.
- Number.h: Number holds a numeric integer value of any (byte) length > 8.
- Object.h: A base class for Objects providing utility functions.
- Parser.h: Provides a simple parameter file parser.
- Properties.h: Provides a name/value string pair mapping.
- Reporter.h: Provides a generic mechanism for statistical event recording and
reporting. __TODO__ Event status checking.
- Select.h: Provides a socket polling controller and selector.
- Semaphore.h: Provides wait/post semaphore functions.
- Signals.h: Provides slots and signals mechanisms.
- Socket.h: Provides socket interfaces.
- Statistic.h: Provides statistical measurement object.
- SubAllocator.h: (Placeholder: not implemented)
- TEST.h: (Note all caps name) Provides test case error checking tools.
- Thread.h: Provides a thread representaion.
- Tokenizer.h: A string tokenizer
- Trace.h: Provides circular trace table controls. When used in conjunction
with a memory-mapped file, handy for debugging multi-threading problems.
- Utf.h: UTF encoder/decoder mechanisms
- utility.h: A set of utility functions
- [Worker.h:](./Worker.md) Worker is an interface class that provides thread
scheduling via WorkerPool, a thread pool manager.
- Wrapper.h: A generic program wrapper with user lambda functions exits.

---
[Back](../index.md)
