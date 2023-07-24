<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/src/doc/cpp/Debug.md
//
// Purpose-
//       Debug.h reference manual
//
// Last change date-
//       2023/07/24
//
-------------------------------------------------------------------------- -->
## pub::Debug
\#include &lt;pub/Debug.h&gt;

(Partially documented)

#### Member functions

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [debugf](./pub_debug.md) | Write to stdout and the trace file |
| [debugh](./pub_debug.md) | Write to stdout and the trace file, including heading information |
| [errorf](./pub_debug.md) | Write to stderr and the trace file |
| [errorh](./pub_debug.md) | Write to stderr and the trace file, including heading information |
| [throwf](./pub_debug.md) | Write to stderr and the trace file, then throw an exception |
| throwh | (NOT IMPLEMENTED) Write to stderr and the trace file including heading information, then throw an exception |
| [tracef](./pub_debug.md) | Write (only) to the trace file |
| [traceh](./pub_debug.md) | Write (only) to the trace file, including heading information |
| [vdebugf](./pub_debug.md) | Write to stdout and the trace file |
| [vdebugh](./pub_debug.md) | Write to stdout and the trace file, including heading information |
| [verrorf](./pub_debug.md) | Write to stderr and the trace file |
| [verrorh](./pub_debug.md) | Write to stderr and the trace file, including heading information |
| [vthrowf](./pub_debug.md) | Write to stderr and the trace file, then throw an exception |
| [vtracef](./pub_debug.md) | Write (only) to the trace file |
| [vtraceh](./pub_debug.md) | Write (only) to the trace file, including heading information |

#### Namespace pub::debugging
This namespace provides subroutines that duplicate many of the pub::Debug
member functions.
These write to the default Debug object.

| Subroutine | Purpose |
|------------|---------|
| [debugf](./pub_debug.md) | Write to stdout and the trace file |
| [debugh](./pub_debug.md) | Write to stdout and the trace file, including heading information |
| [errorf](./pub_debug.md) | Write to stderr and the trace file |
| [errorh](./pub_debug.md) | Write to stderr and the trace file, including heading information |
| [throwf](./pub_debug.md) | Write to stderr and the trace file, then throw an exception |
| [tracef](./pub_debug.md) | Write (only) to the trace file |
| [traceh](./pub_debug.md) | Write (only) to the trace file, including heading information |
| [vdebugf](./pub_debug.md) | Write to stdout and the trace file |
| [vdebugh](./pub_debug.md) | Write to stdout and the trace file, including heading information |
| [verrorf](./pub_debug.md) | Write to stderr and the trace file |
| [verrorh](./pub_debug.md) | Write to stderr and the trace file, including heading information |
| [vthrowf](./pub_debug.md) | Write to stderr and the trace file, then throw an exception |
| [vtracef](./pub_debug.md) | Write (only) to the trace file |
| [vtraceh](./pub_debug.md) | Write (only) to the trace file, including heading information |

#### The default Debug object.
If undefined, the default Debug object is created upon a namespace subroutine
first use. The default debug object trace file name is "debug.out".

An application may replace the default object using pub::Debug::set, currently
documented only in the Debug.h header file.

#### The Debug *Lockable* implementation.

Debug uses a static internal RecursiveLatch or recursive_mutex for locking,
and to allow sequential writing functions in a multithreading environment.

Usage:
```
   {{{{
     std::lock_guard&lt;decltype(*pub::Debug::get())&gt; lock(*pub::Debug::get());
     debugf("The value is: ");
     debugf("42 (of course)\n");
   }}}}
```

Without the lock_guard, another thread might write another Debug message
between the two debugf statements.
Note that nothing prevents another thread from using printf which could then
break up the debugf statements.

__TODO__ Document: When DLLs are used, applications must use the DLL library
rather than the include library.

#### Example
```
#include "pub/Debug.h"
#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;

int main() {
   debugf("This appears in %s and %s\n", "TRACE", "STDOUT");
   errorf("This appears in %s and %s\n", "TRACE", "STDERR");
   tracef("This ONLY appears in %s\n",   "TRACE");
   debugh("This appears in %s and %s\n", "TRACE", "STDOUT");
   errorh("This appears in %s and %s\n", "TRACE", "STDERR");
   traceh("This ONLY appears in %s\n",   "TRACE");
}
```
