//----------------------------------------------------------------------------
//
//       Copyright (c) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
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
//       2020/01/10
//
//----------------------------------------------------------------------------
#ifndef _PUB_CONSOLE_H_INCLUDED
#define _PUB_CONSOLE_H_INCLUDED

#include "config.h"                 // For _PUB_NAMESPACE, ...

namespace _PUB_NAMESPACE {
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
static void
   printf(                           // Print to Console
     const char*       fmt,          // Format
                       ...)          // Arguments
   _ATTRIBUTE_PRINTF(1, 2);          // PRINTF attribute

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
}  // namespace _PUB_NAMESPACE
#endif // _PUB_CONSOLE_H_INCLUDED
