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
//       ~/doc/cpp/pub_console.md
//
// Purpose-
//       Console.h reference manual
//
// Last change date-
//       2023/11/16
//
-------------------------------------------------------------------------- -->
## Defined in header <pub/Console.h>

## Console:: getch, gets, putch, printf, start, stop, wait

Requirements: STDIN and STDOUT must be the same tty device.

Use Console methods to control STDIN and STDOUT.
Characters read from STDIN are *NOT* automatically echoed.

---
### Methods:

#### <a id="getch">static int getch( void );</a>

Reads the next character from STDIN, but does not echo it.

Return value: The next input character, or -1 if stopped.

---
#### <a id="gets">static char* gets(char* addr, unsigned size);</a>
Reads a string from STDIN, echoing each character read to STDOUT.
The returned string is always NUL-terminated.

Errors: An exception is thrown if addr == nullptr or size == 0.

Return value: Returns `addr` if operational, otherwise nullptr.
The returned string is NUL-terminated.

Special handling:
- '\b' (backspace) deletes the last read character before backspacing.
(Backspace is echoed as 'backspace', 'space', 'backspace'.)
Backspace past the original character position is silently ignored.
- '\n' (newline) is added to the string, '\0' is also added and the
NUL-terminated string is returned.
- '\r' (carriage return) characters are IGNORED.
The '\n' (newline) character is (normally) the only character that completes
a gets operation.
- '\t' (tab) characters are inserted into the string but a ' ' (space)
character is displayed.
(This simplifies backspace processing for the tab character.)
- Ctrl-U (clear line) deletes the current input string
as if backspace was entered for each input character.
- ESC (escape) reads and ignores the next two characters and an alarm ('\a')
character is sent to STDOUT.
- If the input buffer has only one character position remaining, a NUL '\0'
is added to the string and the gets method returns `addr`.
- If the Console is no longer operational (the number of Console::stop
invocations is >= the number of Console::start invocations,) the gets method
returns nullptr.

---
#### <a id="putch">static void putch(int ch);</a>

Writes character `ch` to STDOUT.

---
#### <a id="printf">static void printf(const char* fmt, ...);</a>

Prints to STDOUT. (std::printf could be used instead.)

---
#### <a id="start">static void start( void );</a>

Start Console operation.

---
#### <a id="stop">static void stop( void );</a>

Terminate Console operation.

---
#### <a id="wait">static void wait( void );</a>

Console::wait returns when the (internal) usage counter is set to zero.

Wait is most useful in a multi-threaded environment where the main thread
invokes Console::start and then invokes Console::wait.
Other threads use the Console, invoking Console::stop when thread processing
completes.

NOTE: While Console::start is normally only invoked once, it may be invoked
multiple times.
Each Console::start invocation increments the usage counter.
Each Console::stop invocation decrements the usage counter.

---
