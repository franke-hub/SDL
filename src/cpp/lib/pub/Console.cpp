//----------------------------------------------------------------------------
//
//       Copyright (C) 2019-2024 Frank Eskesen.
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
//       2022/10/22
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::mutex, std::lock_guard

#include <assert.h>                 // For assert
#include <ctype.h>                  // For isdigit
#include <stdarg.h>                 // For va_* macros
#include <termios.h>                // For struct termios, ...
#include <unistd.h>                 // For isatty, STDIN_FILENO, ...

#define XK_MISCELLANY               // For most keyboard keys
#define XK_XKB_KEYS                 // For XK_ISO_Left_Tab
#include <X11/keysymdef.h>          // For key code definitions

#include <pub/Clock.h>              // For pub::Clock::now
#include "pub/Console.h"            // For pub::Console, implemented
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Event.h>              // For pub::Event
#include <pub/utility.h>            // For pub::utility::visify

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging
using PUB::utility::visify;         // For convenience

using std::string;                  // For convenience

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  CTL_U= 21                        // Control-U character
,  ESC=   27                        // ESCape character
}; // (generic) enum

static constexpr const char ESC_STR[]= {ESC, 0}; // ESC character string

enum { UNI_REPLACEMENT= 0x00'FFFD }; // The Unicode error replacement character

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Event           event;       // Termination wait event
static std::mutex      mutex;       // Protects static attributes
static struct termios  oldattr;     // The attributes to restore

static string          inp_buffer;  // Enqueued input string
static int             in_getch= false; // TRUE while running getch()
static int             operational= 0; // Initialized counter
static int             registered= false; // One-time initialization flag
static int             used_tracef= false; // Does debug.out have extra info?

//----------------------------------------------------------------------------
// ESC sequences
//----------------------------------------------------------------------------
struct ESC_keydef_sequence {
const char*            text;        // The ESC string (without ESC)
int                    code;        // The X11/keysymdef.h key code
const char*            name;        // The key name (for debugging display)
}; // struct ESC_keydef_sequence

const char             ESC_ESC[]= {ESC, ESC, 0}; // ESC-ESC sequence

const static ESC_keydef_sequence key_table[]=
{  {"[A",    XK_Up,          "Up arrow"} // '[', letter
,  {"[B",    XK_Down,        "Down arrow"}
,  {"[C",    XK_Right,       "Right arrow"}
,  {"[D",    XK_Left,        "Left arrow"}
,  {"[E",    XK_KP_5,        "Keypad 5"}
,  {"[F",    XK_End,         "End"}
,  {"[H",    XK_Home,        "Home"}
,  {"[M",    XK_KP_Enter,    "Enter"}
,  {"[Z",    XK_ISO_Left_Tab, "Left tab"}

,  {"[2~",   XK_Insert,      "Insert"} // '[', digit, '~'
,  {"[3~",   XK_Delete,      "Delete"}
,  {"[5~",   XK_Page_Up,     "Page up"}
,  {"[6~",   XK_Page_Down,   "Page down"}

,  {"Oj",    XK_KP_Multiply, "Keypad *"} // 'O', letter
,  {"Ok",    XK_KP_Add,      "Keypad +"}
,  {"Om",    XK_KP_Subtract, "Keypad -"}
,  {"Oo",    XK_KP_Divide,   "Keypad /"}

,  {"OP",    XK_F1,   "F1"}         // 'O', letter
,  {"OQ",    XK_F2,   "F2"}
,  {"OR",    XK_F3,   "F3"}
,  {"OS",    XK_F4,   "F4"}
,  {"[15~",  XK_F5,   "F5"}         // '[', digit, digit, '~'
,  {"[17~",  XK_F6,   "F6"}
,  {"[18~",  XK_F7,   "F7"}
,  {"[19~",  XK_F8,   "F8"}
,  {"[20~",  XK_F9,   "F9"}
,  {"[21~",  XK_F10, "F10"}
,  {"[23~",  XK_F11, "F11"}
,  {"[24~",  XK_F12, "F12"}
,  {nullptr, -1,     "NUL"}
}; // key_table

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_sequence
//
// Purpose-
//       Check for a complete escape sequence in the input buffer
//
// Implementation note-
//       Invoked from esc_sequence
//
//----------------------------------------------------------------------------
static int                          // The decoded esc sequence, or -1
   get_sequence( void )             // Get decoded esc sequence
{  if( HCDM ) {
     tracef("Console::get_sequence inp_buffer(%s)\n"
           , visify(inp_buffer).c_str());
     used_tracef= true;
   }

   if( inp_buffer.size() < 2 )     // If inp_buffer's too small for a sequence
     return -1;

   if( inp_buffer[0] != ESC ) {    // (Should not occur)
     tracef("Console::get_sequence (correctable) logic error\n");
     used_tracef= true;
     return -1;
   }

   // Test for completed ESC sequence in inp_buffer
   if( inp_buffer.size() > 2 ) {   // If inp_buffer can contain a sequence
     string sequence= inp_buffer.substr(1);
     for(int i= 0; key_table[i].text; ++i) {
       string text= sequence;
       size_t size= strlen(key_table[i].text);
       if( size < text.size() )
         text= text.substr(0, size);
       if( text == key_table[i].text ) {
         // Note: We have to account for the ESC in the buffer
         inp_buffer= inp_buffer.substr(text.size()+1); // Remove sequence
         return key_table[i].code;
       }
     }
   } // if( inp_buffer.size() > 2 )

   // Note that in an ESC-ESC sequence, it's possible (but unlikely) that the
   // second ESC begins an ESC sequence.
   if( inp_buffer.substr(0,2) == ESC_ESC ) { // If ESC-ESC sequence remains
     inp_buffer= inp_buffer.substr(1); // Remove and return the first ESC
     return ESC;                    // Return first ESC, leaving the second
   }

   return -1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       esc_sequence_full
//
// Purpose-
//       Handle a complete but unknown ESC sequence, updating inp_buffer
//
//----------------------------------------------------------------------------
static int
   esc_sequence_full(               // Handle an unknown ESC sequence
     string            str)         // The unknown ESC sequence
{
   if( VERBOSE ) {                  // Conditionally, display error message
     tracef("Unknown ESC sequence(%s)\n", visify(str).c_str());
     used_tracef= true;
   }

   inp_buffer= inp_buffer.substr(str.size()); // (Usually empties inp_buffer)
   return -1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       esc_sequence_part
//
// Purpose-
//       Handle an ESC sequence start error, updating inp_buffer
//
//----------------------------------------------------------------------------
static int                          // ESC
   esc_sequence_part( void )        // Handle an ESC start error
{
   if( VERBOSE ) {                  // Conditionally display error message
     tracef("Invalid ESC sequence(%s)\n", visify(inp_buffer).c_str());
     used_tracef= true;
   }

   inp_buffer= inp_buffer.substr(1); // Remove the ESC
   return ESC;                      // And return it
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       esc_sequence
//
// Purpose-
//       Decode an entire escape sequence
//
// Implementation note-
//       Invoked when an ESC character is read.
//       ESC sequences are completely handled, but unprocessed data may remain
//       in inp_buffer if unknown or invalid sequences were detected.
//
//----------------------------------------------------------------------------
static int                          // The decoded esc sequence
   esc_sequence( void )             // Decode an esc sequence
{  if( HCDM ) {
     tracef("Console::esc_sequence inp_buffer(%s)\n"
           , visify(inp_buffer).c_str());
     used_tracef= true;
   }

   // Insert the ESC (probably back) into inp_buffer.
   inp_buffer.insert(0, 1, ESC);    // Put the ESC back into the buffer

   // Read the entire sequence or set of sequences
   for(;;) {
     // Get the next character
     int C= Console::getch(125);
     if( C < 0 )                    // If no more character's are present
       break;                       // The sequence is complete

     inp_buffer += C;               // Add it to the input buffer
     C= get_sequence();             // Get decoded ESC sequence
     if( C >= 0 )                   // If a valid sequence was found
       return C;                    // Return the decoded sequence
   }

   // Check for a known, complete sequence.
   int C= get_sequence();
   if( C >= 0 )
     return C;

   if( inp_buffer.size() == 1 ) {    // If stand-alone ESC
     inp_buffer= "";
     return ESC;
   }

   // We have what should be a complete ESC sequence, but maybe it isn't.
   // For incomplete or invalid ESC sequences, we invoke esc_sequence part.
   // For complete ESC sequences, we invoke esc_sequence_full.
   string sequence= inp_buffer.substr(1); //

   if( sequence.size() < 2 )        // If ESC - something
     return esc_sequence_part();    // Return ESC, removing it from the buffer

   if( sequence[0] != 'O' && sequence[0] != '[' ) // If invalid ESC sequence
     return esc_sequence_part();    // (Invalid start character);

   if( sequence.size() < 3 )        // If ESC - starter - something
     return esc_sequence_full(inp_buffer);

   if( !isdigit(sequence[1]) )      // If ESC - starter - non-digit
     return esc_sequence_part();

   for(size_t i= 2; i<sequence.size(); ++i) {
     C= sequence[i];
     if( !isdigit(C) ) {            // End of sequence
       if( C != '~' )               // If invalid sequence
         return esc_sequence_part();

       // We have a complete but unknown digital sequence.
       sequence= sequence.substr(0, i+1);
       sequence= ESC_STR + sequence;
       return esc_sequence_full(sequence);
     }
   }

   return esc_sequence_part();      // Incomplete (therefore invalid) sequence
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_buffered
//
// Purpose-
//       Get (and remove) the next buffered character, if any
//
//----------------------------------------------------------------------------
static int                          // The next buffered character, or -1
   get_buffered( void )             // Get next buffered characer
{
   if( inp_buffer.size() == 0 )     // If inp_buffer's empty
     return -1;

   if( HCDM ) {
     tracef("Console::get_buffered(%s.%zd)\n", visify(inp_buffer).c_str()
           , inp_buffer.size());
     used_tracef= true;
   }

   int C= inp_buffer[0];            // Get the first buffer character
   inp_buffer= inp_buffer.substr(1); // Remove it from the buffer
   if( C == ESC ) {
     C= esc_sequence();
     if( C < 0 ) {
       C= inp_buffer[0];            // Get the first buffer character
       inp_buffer= inp_buffer.substr(1); // Remove it from the buffer
     }
   }

   return C;                        // Return the removed character
}

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
   Console::getch(                  // Get next input character
     int               timeout)     // Timeout in milliseconds
{
   if( timeout < 0 || timeout > 25500 ) // If timout > maximum for
     timeout= 25500;

   std::lock_guard<decltype(mutex)> lock(mutex); // One user at a time

   tcgetattr(STDIN_FILENO, &oldattr); // Set restore attributes
   in_getch= true;                  // Indicate getch running

   // Update the attributes
   struct termios newattr = oldattr;
   newattr.c_lflag &= ~( ICANON | ECHO ); // NOT (cononical or echo)
   newattr.c_cc[VMIN] = 1;          // Single character
   newattr.c_cc[VTIME] = (timeout + 50)/100; // Set timeout
   if( HCDM && VERBOSE > 1 ) {
     tracef("\n%8.1f VTIME 0x%.2x\n", pub::Clock::now(), newattr.c_cc[VTIME]);
     used_tracef= true;
   }
   tcsetattr(STDIN_FILENO, TCSANOW, &newattr);

   // Read the character
   int C= ::getchar();
   if( HCDM && VERBOSE > 1 )
     tracef("%8.1f C(%.2x)\n", pub::Clock::now(), C);

   if( C == 0x007f )                // Handle nasty surprise
     C= '\b';

   // Restore the attributes
   tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
   in_getch= false;                 // Attributes restored

   return C;
}

int                                 // The next input character
   Console::getch( void )           // Get next input character
{
   while( inp_buffer.size() > 2 ) { // If possible ESC sequence(s)
     if( inp_buffer[0] == ESC ) {   // If ESC sequence present
       int C= get_buffered();       // Get (and remove from inp_buffer) the ESC
       if( C >= 0 ) {               // If found
         return C;
       }
     }

     break;
   }

   int C= get_buffered();           // Get buffered data, if any
   while( C < 0 && operational ) {
     C= getch(5000);
   }

   // If ESC, read the entire sequence
   if( C == ESC )
     C= esc_sequence();

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
   if( addr == nullptr || size < 2 ) {
     fprintf(stderr, "Console::gets(%p,%u) PARMERR\n", addr, size);
     throw std::invalid_argument("Console::gets");
   }

   unsigned used= 0;                // Number of bytes used
   while( used < (size - 2) ) {
     int C= getch();                // Get next character
     if( !operational ) {           // If not operational
       addr[0]= '\0';               // Return empty line
       return nullptr;              // And indicate EOF
     }

     switch( C ) {                  // Convert XK alias keys to ASCII
       case XK_BackSpace:
         C= '\b';
         break;

       case XK_KP_Enter:
         C= '\n';
         break;

       case XK_Tab:
         C= '\t';
         break;

       case XK_KP_Add:
         C= '+';
         break;

       case XK_KP_Subtract:
         C= '-';
         break;

       case XK_KP_Multiply:
         C= '*';
         break;

       case XK_KP_Divide:
         C= '/';
         break;

       default:
         break;
     }

     if( C < 0x00000080 ) {         // If ASCII
       switch( C ) {
         case '\b':
           if( used > 0 ) {
             puts("\b \b");
             used--;
           }
           continue;
           break;

         case '\t':
           addr[used++]= C;
           putchar(' ');
           continue;
           break;

         case CTL_U:
         case -1:
           while( used > 0 ) {
             puts("\b \b");
             used--;
           }
           if( C == -1 )
             return nullptr;
           continue;
           break;

         case '\r':                 // (Carriage return silently ignored)
           continue;

         default:
           break;
       }
     } else {                       // If extended key
       switch( C ) {
         case XK_F1:
         case XK_F2:
         case XK_F3:
         case XK_F4:
         case XK_F5:
         case XK_F6:
         case XK_F7:
         case XK_F8:
         case XK_F9:
         case XK_F10:
         case XK_F11:
         case XK_F12:
           tracef("F%d key has no function\n", C - XK_F1 + 1);
           used_tracef= true;
           continue;

         // NEED TO HANDLE CURSOR MOVEMENT, UP, DOWN, LEFT, RIGHT, HOME, END
         // NEED TO HANDLE INSERT/DELETE
         case XK_Up:
         case XK_Down:
         case XK_Left:
         case XK_Right:
         case XK_Home:
         case XK_End:
         case XK_Insert:
         case XK_Delete:
         default:
           tracef("Key 0x%.4x NOT CODED YET, ignored\n", C);
           used_tracef= true;
           continue;
       }
     }

     addr[used++]= C;
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

   if( !registered ) {              // If not registered
     atexit(handle_atexit);         // Register exit handler
     registered= true;              // Indicate registered
   }

   if( operational == 0 )
     event.reset();

   operational++;
   used_tracef= false;
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
   Console::stop( void )            // Stop the Console
{
   std::lock_guard<decltype(mutex)> lock(mutex); // One user at a time

   if( operational > 0 )
     operational--;

   if( operational == 0 )
     event.post(0);

   if( used_tracef ) {
     debugf("\ndebug.out contains tracef information\n");
     used_tracef= false;
   }
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
