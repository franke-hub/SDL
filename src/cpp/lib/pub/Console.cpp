//----------------------------------------------------------------------------
//
//       Copyright (C) 2019-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Console.cpp
//
// Purpose-
//       Console subroutine methods.
//
// Last change date-
//       2022/09/02
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::mutex, std::lock_guard

#include <stdarg.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "pub/Console.h"            // For pub::Console, implemented
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Event.h>              // For pub::Event
#include <pub/Exception.h>          // For pub::Exception

using namespace _LIBPUB_NAMESPACE::debugging; // For debugging

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Event           event;       // Termination wait event
static std::mutex      mutex;       // Protects static attributes
static struct termios  oldattr;     // The attributes to restore

static int             in_getch= false; // TRUE while running getch()
static int             operational= 0; // Initialized counter
static int             registered= false; // One-time initialization flag

//----------------------------------------------------------------------------
//
// Subroutine-
//       handle_atexit
//
// Purpose-
//       Restore original termios settings.
//
// Implementation notes-
//       We need to restore the original termios settings if the main thread
//       exits while getch is running
//
//----------------------------------------------------------------------------
static void handle_atexit( void ) { // atexit target subroutine
   if( in_getch ) {
     operational= 0;
     tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Console::getch
//
// Purpose-
//       Read input character. The character is NOT echoed.
//
//----------------------------------------------------------------------------
int                                 // The next input character
   Console::getch( void )           // Get next input character
{
   std::lock_guard<decltype(mutex)> lock(mutex); // One user at a time

   tcgetattr(STDIN_FILENO, &oldattr); // Set restore attributes
   in_getch= true;                  // Indicate getch running

   // Update the attributes
   struct termios newattr = oldattr;
   newattr.c_lflag &= ~( ICANON | ECHO ); // NOT (cononical or echo)
   newattr.c_cc[VMIN] = 1;          // Single character
   newattr.c_cc[VTIME] = 50;        // 5 Second timeout
   tcsetattr(STDIN_FILENO, TCSANOW, &newattr);

   // Read the character
   int C= -1;
   while( C < 0 && operational )
     C= ::getchar();

   if( C == 0x007f )                // Handle nasty surprise
     C= '\b';

   // Restore the attributes
   tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
   in_getch= false;                 // Attributes restored

   return C;
}

//----------------------------------------------------------------------------
//
// Method-
//       Console::gets
//
// Purpose-
//       Read input string from stdin.
//
//----------------------------------------------------------------------------
char*                               // addr || nullptr iff non-operational
   Console::gets(                   // Get input string
     char*             addr,        // Input address
     unsigned          size)        // Input length
{
   if( addr == nullptr || size == 0 )
   {
     fprintf(stderr, "Console::gets(%p,%u) PARMERR\n", addr, size);
     throw Exception("Invalid parameter");
   }

   unsigned used= 0;                // Number of bytes used
   while( used < (size - 1) )
   {
     int C= getch();                // Get next character
     if( !operational ) {           // If not operational
       addr[0]= '\0';               // Return empty line
       return nullptr;              // And indicate EOF
     }

     if( C == '\b' ) {              // If backspace
       if( used > 0 )
       {
         puts("\b \b");
         used--;
       }

       continue;
     }
     else if( C == 21 || C == -1 ) { // If Ctrl-U (or not operational)
       while( used > 0 )
       {
         puts("\b \b");
         used--;
       }

       if( C == -1 )
         return nullptr;
       continue;
     }
     else if( C == 27 ) {           // If ESCape, ignore 3 character sequence
       getch();
       getch();
       putch('\a');
     }
     else if( C == '\r' )           // Silently ignore carriage return
       continue;

     addr[used++]= C;
     if( C == '\t' )
       C= ' ';
     putchar(C);

     if( C == '\n' )
       break;
   }
   addr[used]= '\0';

   return addr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Console::printf
//
// Purpose-
//       Console printf.
//
//----------------------------------------------------------------------------
void
   Console::printf(                 // Console printf
     const char*       fmt,         // The format string
                       ...)         // Remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vprintf(fmt, argptr);            // Print the data
   va_end(argptr);                  // Close va_ functions

   fflush(stdout);
}

//----------------------------------------------------------------------------
//
// Method-
//       Console::putch
//
// Purpose-
//       Write output character.
//
//----------------------------------------------------------------------------
void
   Console::putch(                  // Put next output character
     int               out)         // The next output character
{
   ::putchar(out);
   fflush(stdout);
}

//----------------------------------------------------------------------------
//
// Method-
//       Console::puts
//
// Purpose-
//       Write output string.
//
//----------------------------------------------------------------------------
void
   Console::puts(                   // Write
     const char*       str)         // This output string
{
   ::printf("%s", str);
   fflush(stdout);
}

//----------------------------------------------------------------------------
//
// Method-
//       Console::start
//
// Purpose-
//       Go operational.
//
//----------------------------------------------------------------------------
void
   Console::start( void )           // Start the Console
{
   std::lock_guard<decltype(mutex)> lock(mutex); // One user at a time

   if( !isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) )
     throwf("Console only supports terminal input/output");

   if( registered == false )        // If not registered
   {
     atexit(handle_atexit);         // Register exit handler
     registered= true;              // Indicate registered
   }

   if( operational == 0 )
     event.reset();

   operational++;
}

//----------------------------------------------------------------------------
//
// Method-
//       Console::stop
//
// Purpose-
//       Terminate processing.
//
//----------------------------------------------------------------------------
void
   Console::stop( void )            // Start the Console
{
   std::lock_guard<decltype(mutex)> lock(mutex); // One user at a time

   if( operational > 0 )
     operational--;

   if( operational == 0 )
     event.post(0);
}

//----------------------------------------------------------------------------
//
// Method-
//       Console::wait
//
// Purpose-
//       Wait for termination.
//
//----------------------------------------------------------------------------
void
   Console::wait( void )            // Wait for termination
{  event.wait(); }
}  // namespace _LIBPUB_NAMESPACE
