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
//       ~/doc/cpp/Debug.md
//
// Purpose-
//       Debug.h reference manual
//
// Last change date-
//       2023/11/26
//
-------------------------------------------------------------------------- -->
## pub::Debug
\#include <pub/Debug.h>

#### Methods

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [attributes](./pub_debug.md#attributes) | Attributes |
| [constructor](./pub_debug.md#construct) | Constructors |

------------------------------------------------------------------------------
Write methods:

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [debugf](./pub_debug.md#debugf) | Write to stdout and the trace file. |
| [debugh](./pub_debug.md#debugh) | Write to stdout and the trace file, including heading information. |
| [errorf](./pub_debug.md#errorf) | Write to stderr and the trace file. |
| [errorh](./pub_debug.md#errorh) | Write to stderr and the trace file, including heading information. |
| [tracef](./pub_debug.md#tracef) | Write (only) to the trace file |
| [traceh](./pub_debug.md#traceh) | Write (only) to the trace file, including heading information. |
| [vdebugf](./pub_debug.md#vdebugf) | Write to stdout and the trace file. |
| [vdebugh](./pub_debug.md#vdebugh) | Write to stdout and the trace file, including heading information. |
| [verrorf](./pub_debug.md#verrorf) | Write to stderr and the trace file. |
| [verrorh](./pub_debug.md#verrorh) | Write to stderr and the trace file, including heading information. |
| [vtracef](./pub_debug.md#vtracef) | Write (only) to the trace file. |
| [vtraceh](./pub_debug.md#vtraceh) | Write (only) to the trace file, including heading information. |

------------------------------------------------------------------------------
Exception generator methods:

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [throwf](./pub_debug.md#throwf) | Write to stderr and the trace file, then throw an exception. |
| [vthrowf](./pub_debug.md#vthrowf) | Write to stderr and the trace file, then throw an exception |

------------------------------------------------------------------------------
Accessor and control methods:

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [backtrace](./pub_debug.md#backtrace) | Write debugging backtrace. |
| [clr_head](./pub_debug.md#clr_head) | Clear heading options. |
| [flush](./pub_debug.md#flush) | Flush the trace file. |
| [get_FILE](./pub_debug.md#get_file) | Get the trace FILE (pointer). |
| [get_file_mode](./pub_debug.md#get_file_mode) | Get the file mode. |
| [get_file_name](./pub_debug.md#get_file_name) | Get the file name. |
| [set_file_mode](./pub_debug.md#set_file_mode) | Set the file mode. |
| [set_file_name](./pub_debug.md#set_file_name) | Set the file name. |
| [set_head](./pub_debug.md#set_head) | Set heading options. |
| [set_mode](./pub_debug.md#set_mode) | Set debugging mode. |

------------------------------------------------------------------------------
[Default Debug object access:](./pub_debug.md#default-debug-object)

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [get](./pub_debug.md#get) | Access the global debug object. |
| [set](./pub_debug.md#set) | Set the global debug object. |
| [show](./pub_debug.md#show) | Conditionally access the global debug object. |

The default Debug object:
 The default Debug object is either set using Debug::set or, if not
initialized, created and set when a pub::debugging write method is used.

------------------------------------------------------------------------------
[*Lockable* methods:](./pub_debug.md#lockable)

| <div style="width:10%">Method</div> | <div style="width:90%">Purpose<div> |
|--------|---------|
| [lock](./pub_debug.md#lock) | Obtain the Debug recursive lock. |
| [try_lock](./pub_debug.md#try_lock) | Conditionally obtain the Debug recursive lock. |
| [unlock](./pub_debug.md#unlock) | Release the Debug recursive lock. |

------------------------------------------------------------------------------
#### [Namespace pub::debugging](./pub_debug.md#namespace-debugging)

Namespace pub::debugging subroutines duplicate pub::Debug methods, implicitly
creating a default Debug object it if needed.

| Subroutine | Purpose |
|------------|---------|
| [debug_backtrace](./pub_debug.md#debug_backtrace) | Write debugging backtrace. |
| [debug_clr_head](./pub_debug.md#debug_clr_head) | Clear Heading control options. |
| [debug_flush](./pub_debug.md#debug_flush) | Flush the trace file. |
| [debug_get_file_mode](./pub_debug.md#debug_get_file_mode) | Get the file mode. |
| [debug_get_file_name](./pub_debug.md#debug_get_file_name) | Get the file name. |
| [debug_set_file_mode](./pub_debug.md#debug_set_file_mode) | Set the file mode. |
| [debug_set_file_name](./pub_debug.md#debug_set_file_name) | Set the file name. |
| [debug_set_head](./pub_debug.md#debug_set_head) | Set Heading control options. |
| [debug_set_mode](./pub_debug.md#debug_set_mode) | Set the debugging mode. |

These subroutines have the same names and interface as their Debug method
counterparts:

| Subroutine | Purpose |
|------------|---------|
| [debugf](./pub_debug.md#debugf) | Write to stdout and the trace file. |
| [debugh](./pub_debug.md#debugh) | Write to stdout and the trace file, including heading information. |
| [errorf](./pub_debug.md#errorf) | Write to stderr and the trace file. |
| [errorh](./pub_debug.md#errorh) | Write to stderr and the trace file, including heading information. |
| [tracef](./pub_debug.md#tracef) | Write (only) to the trace file. |
| [traceh](./pub_debug.md#traceh) | Write (only) to the trace file, including heading information. |
| [vdebugf](./pub_debug.md#vdebugf) | Write to stdout and the trace file. |
| [vdebugh](./pub_debug.md#vdebugh) | Write to stdout and the trace file, including heading information. |
| [verrorf](./pub_debug.md#verrorf) | Write to stderr and the trace file. |
| [verrorh](./pub_debug.md#verrorh) | Write to stderr and the trace file, including heading information. |
| [vtracef](./pub_debug.md#vtracef) | Write (only) to the trace file. |
| [vtraceh](./pub_debug.md#vtraceh) | Write (only) to the trace file, including heading information. |
| [throwf](./pub_debug.md#throwf) | Write to stderr and the trace file, then throw an exception. |
| [vthrowf](./pub_debug.md#vthrowf) | Write to stderr and the trace file, then throw an exception. |
