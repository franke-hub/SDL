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
//       ~/src/doc/cpp/pub_debug.md
//
// Purpose-
//       Debug.h reference manual
//
// Last change date-
//       2023/07/16
//
-------------------------------------------------------------------------- -->
## debugf, errorf, tracef, vdebugf, verrorf, vtracef, debugh, errorh, traceh, vdebugh, verrorh, vtraceh

###### Defined in header &lt;pub/Debug.h&gt

(Partially documented)

__TODO__
- Describe printf-style arguments
- Error handling.
- Constructor
- Other methods
- debugging namespace

####
---
#### void debugf(const char* fmt, ...);
---
#### void errorf(const char* fmt, ...);
---
#### void tracef(const char* fmt, ...);
---
#### void vdebugf(const char* fmt, va_list arg);
---
#### void verrorf(const char* fmt, va_list arg);
---
#### void vtracef(const char* fmt, va_list arg);
---
#### void debugh(const char* fmt, ...);
---
#### void errorh(const char* fmt, ...);
---
#### void traceh(const char* fmt, ...);
---
#### void vdebugh(const char* fmt, va_list arg);
---
#### void verrorh(const char* fmt, va_list arg);
---
#### void vtraceh(const char* fmt, va_list arg);
---

Functions debugf, vdebugf, debugh and vdebugh write the C string *fmt*
with its (v)printf style arguments to both stdout and the trace file.

Functions errorf, verrorf, errorh and verrorh write the C string *fmt*
with its (v)printf style arguments to both stderr and the trace file.

Functions tracef, vtracef, traceh and vtraceh write the C string *fmt*
with its (v)printf style arguments to the trace file.

#### Return value

None

#### Errors

errno:

Debug uses vfprintf to write to stdout, stderr, and the trace file,
and vfprintf may set errno.
This value is returned, but errno is preserved for other situations.

For example, errno remains unchanged for errors detected when opening or
closing the trace file.
An error message is written to stderr for these conditions.

#### Notes

- __TODO__ Describe intensive mode.
- __TODO__ Should we throw an exception when errors are detected?
