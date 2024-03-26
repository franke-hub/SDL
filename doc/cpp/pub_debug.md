<!-- -------------------------------------------------------------------------
//
//       Copyright (c) 2023-2024 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/doc/cpp/pub_debug.md
//
// Purpose-
//       Debug.h reference manual
//
// Last change date-
//       2024/03/04
//
-------------------------------------------------------------------------- -->
## Defined in header <pub/Debug.h>

##### pub::Debug::
debugf, errorf, throwf, tracef, vdebugf, verrorf, vtracef, debugh, errorh, traceh, vdebugh, verrorh, vthrowf, vtraceh

####
------------------------------------------------------------------------------
### <a id="attributes">Attributes:</a>

#### public:
- enum Mode (the debugging control mode)
  - MODE_DEFAULT (Default: debugging active, auto-flush disabled)
  - MODE_IGNORE (Debugging inactive)
  - MODE_INTENSIVE (Debugging active, auto-flush enabled)

- enum Heading (Heading bit field masks)
  - HEAD_TIME (Include current time in heading)
  - HEAD_THREAD (Include current thread in heading)
  - HEAD_DEFAULT (Default heading: currently HEAD_TIME)

#### protected:
- FILE*       handle= nullptr;
 The file handle, returned by method get_FILE.
- std::string file_mode= "wb";
 The file mode, accessible using methods get/set_file_mode.
- std::string file_name= "debug.out";
 The file name, accessible using methods get/set_file_name.
- int         head= HEAD_DEFAULT;
 The Heading bit fields, accessible using methods get/set_head.
- int         mode= MODE_DEFAULT;
 The file mode, accessible using methods get/set_mode.
- static Debug* common;
 The default Debug object, accessible using methods get, set, and show.

------------------------------------------------------------------------------
### <a id="construct">Constructor:</a>

#### Debug(const char* name= nullptr);
The constructor sets the file_name attribute from the specified name.
If name is not specified, the default "debug.out" is used.

The constructor does *NOT* open the trace file.
(The first write operation opens it.)
This allows the file_name, file_mode, Heading, and Mode to be set first.

------------------------------------------------------------------------------
### Write methods:

#### <a id="debugf">void debugf(const char* fmt, ...);</a>
Writes to FILE stdout and the trace file using printf style arguments.

---
#### <a id="debugh">void debugh(const char* fmt, ...);</a>
Writes to FILE stdout and the trace file using printf style arguments,
prefixing the message using Heading controls.

---
#### <a id="errorf">void errorf(const char* fmt, ...);</a>
Writes to FILE stderr and the trace file using printf style arguments.

---
#### <a id="errorh">void errorh(const char* fmt, ...);</a>
Writes to FILE stderr and the trace file using printf style arguments,
prefixing the message using Heading controls.

---
#### <a id="tracef">void tracef(const char* fmt, ...);</a>
Writes to the trace file using printf style arguments,

---
#### <a id="traceh">void traceh(const char* fmt, ...);</a>
Writes to the trace file using printf style arguments,
prefixing the message using Heading controls.

---
#### <a id="vdebugf">void vdebugf(const char* fmt, va_list arg);</a>
Writes to FILE stdout and the trace file using vprintf style arguments.

---
#### <a id="vdebugh">void vdebugh(const char* fmt, va_list arg);</a>
Writes to FILE stdout and the trace file using vprintf style arguments,
prefixing the message using Heading controls.

---
#### <a id="verrorf">void verrorf(const char* fmt, va_list arg);</a>
Writes to FILE stderr and the trace file using vprintf style arguments.

---
#### <a id="verrorh">void verrorh(const char* fmt, va_list arg);</a>
Writes to FILE stderr and the trace file using vprintf style arguments,
prefixing the message using Heading controls.

---
#### <a id="vtracef">void vtracef(const char* fmt, va_list arg);</a>
Writes to the trace file using vprintf style arguments,

---
#### <a id="vtraceh">void vtraceh(const char* fmt, va_list arg);</a>
Writes to the trace file using vprintf style arguments,
prefixing the message using Heading controls.

---
#### Errors

Should an error occur opening the trace file, a descriptive error message
is written to stderr and the `handle` attribute is set to stderr.

------------------------------------------------------------------------------
### Exception generation methods:

---
#### <a id="throwf">[[noreturn]] void throwf(const char* fmt, ...);</a>
Writes to FILE stderr and the trace file using printf style arguments, then
throws a runtime_error using the same resulting string with a trailing '\n'
(newline) character added to the stderr and trace file outputs.

---
#### <a id="vthrowf">[[noreturn]] void vthrowf(const char* fmt, va_list arg);</a>
Writes to FILE stderr and the trace file using vprintf style arguments,
then throws a runtime_error using the resulting string.
A trailing '\n' (newline) character is added to the stderr and trace file
outputs.

------------------------------------------------------------------------------
### Accessor and control methods:

#### <a id="backtrace">void backtrace(void);</a>
Writes debugging backtrace information.

---
#### <a id="flush">void flush(void);</a>
Flushes stdout, stderr, and the trace file.

Additionally, when the `mode` attribute equals MODE_INTENSIVE,
closes and then re-opens the trace file with mode "ab".

---
#### <a id="get_file">FILE* get_FILE(void);</a>
Returns the trace FILE handle.
This is (at least) used by and with pub::utility::dump.

---
#### <a id="get_file_mode">std::string get_file_mode(void);</a>
Returns the trace file mode, used when opening the file.

---
#### <a id="get_file_name">std::string get_file_name(void);</a>
Returns the trace file name.

---
#### <a id="set_file_mode">void set_file_mode(std::string mode);</a>
Sets the trace file mode used when opening the file.
This method may only be used before a file write method is used.

---
#### <a id="set_file_name">void set_file_name(std::string name);</a>
Sets the trace file name.
This method closes the current trace file.
The next write method uses the new trace file name, opening the file using
the then current file mode.

---
#### <a id="set_head">void set_head(Heading bits);</a>
Sets the Heading controls specified by `bits`. (This is a logical OR.)
- HEAD_THREAD: Include current thread in Heading.
- HEAD_TIME: Include time in Heading.

---
#### <a id="set_mode">void set_mode(Mode mode);</a>
Sets (replacing) the Debugging mode.
- MODE_DEFAULT: Write operations include writing to the trace file.
- MODE_IGNORE: Write operations do nothing.
- MODE_INTENSIVE: The Debug::flush method is invoked after each write
operation.

Note: The flush method closes and then re-opens the trace file with mode "ab".

------------------------------------------------------------------------------
### <a id="default-debug-object">Default Debug object access:</a>
The default Debug object is created by the Debug::get method.
It can also be set by the application using Debug::set.
(Namespace pub::debugging methods use Debug::get to access it.)

The object can be deleted by an application using Debug::set(nullptr) but
this is not strictly necessary since
Debug program termination logic will automatically delete it.

---
#### <a id="get">static Debug* get(void);</a>
Returns the current default Debug object, creating it if required.

---
#### <a id="set">static Debug* set(Debug* debug);</a>
Replaces the current default Debug object with `debug` and returns
the replaced default Debug object.

---
#### <a id="show">static Debug* show(void);</a>
Returns the current default Debug object, but does not create it.

------------------------------------------------------------------------------
### <a id="lockable">*Lockable* methods:</a>
Debug defines a static internal RecursiveLatch or recursive_mutex for locking.
This allows sequential writing functions in a multithreading environment.

---
#### <a id="lock">static void lock(void);</a>
Obtains the recursive Debug lock.

---
#### <a id="try_lock">static bool try_lock(void);</a>
Conditionally obtains the recursive Debug lock,
returning `true` if the lock was obtained.

---
#### <a id="unlock">static void unlock(void);</a>
Releases the Debug lock.

Usage:
```
   {{{{
     std::lock_guard<decltype(*pub::Debug::get())> lock(*pub::Debug::get());
     debugf("The value is: ");
     debugf("42 (of course)\n");
   }}}}
```

Without the lock_guard, another thread might write a Debug message
between the two debugf statements.
Note: Nothing prevents another thread from using printf which might also
break up the debugf statements in the stdout stream.

__NOTE__: If an application uses DLLs, it must use the DLL (shared) library
rather than the default (static) include library.
If the static library is used, each DLL includes a separate copy of the
debugging namespace, the default Debug object, and the static Debug lock.
This can result in unexpected results when using shared libraries even in a
single-threaded environment.
When multi-threading, data loss occurs unless each thead uses a different
output file name.

------------------------------------------------------------------------------
### <a id="namespace-debugging">Namespace pub::debugging:</a>

Namespace pub::debugging subroutines use the default Debug object,
creating it if necessary.

The [debugf](#debugf), [debugh](#debugh),
[errorf](#errorf), [errorh](#errorh), [tracef](#tracef), [traceh](#traceh),
[vdebugf](#vdebugf), [vdebugh](#vdebugh),
[verrorf](#verrorf), [verrorh](#verrorh),
[vtracef](#vtracef), [vtraceh](#vtraceh),
[throwf](#throwf), and [vthrowf](#vthrowf) subroutines
have the same names and interfaces as their Debug object method counterparts.

---
#### <a id="debug_backtrace">void debug_backtrace(void);</a>
Writes debugging backtrace information using the default Debug object.
[see backtrace](#backtrace)

---
#### <a id="debug_flush">void debug_flush(void);</a>
Flushes the default Debug object.
[see flush](#flush)

---
#### <a id="debug_get_file_mode">std::string debug_get_file_mode(void);</a>
Returns the file mode of the default Debug object.
[see get_file_mode](#get_file_mode)

---
#### <a id="debug_get_file_name">std::string debug_get_file_name(void);</a>
Returns the file name of the default Debug object.
[see get_file_name](#get_file_name)

---
#### <a id="debug_set_file_mode">void debug_set_file_mode(const char* mode);</a>
Sets the file mode of the default Debug object.
[see set_file_mode](#set_file_mode)

---
#### <a id="debug_set_file_name">void debug_set_file_name(const char* name);</a>
Sets the file name of the default Debug object.
[see set_file_name](#set_file_name)

---
#### <a id="debug_set_head">void debug_set_head(Heading bits);</a>
Sets Heading controls for the default Debug object.
[see set_head](#set_head)

---
#### <a id="debug_set_mode">void debug_set_mode(Mode mode);</a>
Sets the debugging mode of the default Debug object.
[see set_mode](#set_mode)

------------------------------------------------------------------------------
### Example:
This example creates and uses a default Debug object.

```
#include "pub/Debug.h"              // For namespace pub::debugging
#define PUB _LIBPUB_NAMESPACE       // (Alias)
using namespace PUB::debugging;     // Use the default Debug object

int main() {
   debugf("This appears in %s and %s\n", "TRACE", "STDOUT");
   errorf("This appears in %s and %s\n", "TRACE", "STDERR");
   tracef("This ONLY appears in %s\n",   "TRACE");
   debugh("This appears in %s and %s\n", "TRACE", "STDOUT");
   errorh("This appears in %s and %s\n", "TRACE", "STDERR");
   traceh("This ONLY appears in %s\n",   "TRACE");
}
```
