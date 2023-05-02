//----------------------------------------------------------------------------
//
//       Copyright (c) 2020-2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Console.h
//
// Purpose-
//       The Console object.
//
// Last change date-
//       2022/09/02
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_CONSOLE_H_INCLUDED
#define _LIBPUB_CONSOLE_H_INCLUDED

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Console
//
// Purpose-
//       Handle Console input/output.
//
//----------------------------------------------------------------------------
class Console {                     // The (static) Console
//----------------------------------------------------------------------------
// Console::Constructors
//----------------------------------------------------------------------------
public:
   Console( void ) = delete;        // NO Constructor

   Console(const Console&) = delete; // Disallowed copy constructor
   Console& operator=(const Console&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Console::Methods
//----------------------------------------------------------------------------
public:
//----------------------------------------------------------------------------
//
// Method-
//       getch
//
// Purpose-
//       Read input character. The character is NOT echoed.
//
//----------------------------------------------------------------------------
static int                           // The next character, NUL if stopped
   getch( void );                    // Get next character, no echo

//----------------------------------------------------------------------------
//
// Method-
//       gets
//
// Purpose-
//       Read input string from Console (using getch/putch.)
//
// Implementation notes-
//       TTY backspace echoing works as follows:
//         If the line does not contain data, no action is taken. Otherwise,
//         "\b \b" is written, causing the last character to be removed.
//         The line length is reduced by one, never going below 0.
//       TTY tab echoing works as follows:
//         The '\t' character is inserted into the line and a space is echoed.
//         (This is done to synchronize backspace handling visiblility.)
//       TTY \r characters are IGNORED.
//       TTY ctrl-u character deletes the line and all characters typed.
//
//       Like stdio fgets, the terminating '\n' is included and the input
//       string is '\0' terminated. The maximum string length is (size - 1).
//
//----------------------------------------------------------------------------
static char*                        // addr || nullptr iff stopped
   gets(                            // Get input string
     char*             addr,        // Input address
     unsigned          size);       // Input length

//----------------------------------------------------------------------------
//
// Method-
//       putch
//
// Purpose-
//       Write output character.
//
//----------------------------------------------------------------------------
static void                          // Write one character
   putch(                            // Put next character
     int               ch);          // The next character

//----------------------------------------------------------------------------
//
// Method-
//       printf
//
// Purpose-
//       Console printf().
//
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(1,2)
static void
   printf(                           // Print to Console
     const char*       fmt,          // Format
                       ...);         // Arguments

//----------------------------------------------------------------------------
//
// Method-
//       puts
//
// Purpose-
//       Write output string onto Console.
//
//----------------------------------------------------------------------------
static void
   puts(                            // Put output string
     const char*       addr);       // Output string

//----------------------------------------------------------------------------
//
// Method-
//       start
//       stop
//       wait
//
// Purpose-
//       Operational controls.
//
//----------------------------------------------------------------------------
static void
   start( void );                   // Start the Console

static void
   stop( void );                    // Stop the Console

static void
   wait( void );                    // Wait for Console termination
}; // class Console
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_CONSOLE_H_INCLUDED
