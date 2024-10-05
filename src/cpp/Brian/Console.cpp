//----------------------------------------------------------------------------
//
//       Copyright (c) 2021-2024 Frank Eskesen.
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
//       ConsoleCommand and ConsoleService object methods
//
// Last change date-
//       2024/10/04
//
//----------------------------------------------------------------------------
#include <ctype.h>                  // For isspace
#include <string.h>                 // For strlen
#include <unistd.h>                 // For isatty, STDIN_FILENO, ...

#include <pub/Console.h>            // For pub::Console
#include <pub/Debug.h>              // For debugging
#include <pub/Thread.h>             // For ConsoleThread
#include <pub/utility.h>            // For pub::utility::visify

#include "ConsoleCommand.h"         // For class ConsoleCommand
#include "ConsoleService.h"         // For class ConsoleService

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging subroutines
using PUB::utility::visify;         // Using pub::utility::visify

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
//
// Subroutine-
//       strip
//
// Purpose-
//       Strip leading and trailing whitespace.
//
//----------------------------------------------------------------------------
static char*                        // The stripped string
   strip(                           // Strip leading and trailing whitespace
     char*             C)           // From this string (MODIFIED)
{
   while( isspace(*C) )             // Strip leading whitespace
     ++C;

   int L= strlen((char*)C);
   while( L > 0 && isspace(C[L-1]) ) // Strip trailing whitespace
     C[--L]= '\0';

   return C;
}

//----------------------------------------------------------------------------
//
// Class-
//       ConsoleThread
//
// Purpose-
//       The ConsoleThread.
//
//----------------------------------------------------------------------------
static class ConsoleThread : public pub::Thread { // The ConsoleThread
//----------------------------------------------------------------------------
// ConsoleThread::Attributes
//----------------------------------------------------------------------------
bool                   operational; // Operational state?
unsigned               used;        // The input string length (used)
char                   inp[4096];   // The input string buffer

//----------------------------------------------------------------------------
// ConsoleThread::Constructors
//----------------------------------------------------------------------------
public:
   ConsoleThread( void )            // Constructor
:  Thread()
{  if( HCDM ) debugf("ConsoleThread(%p).!\n", this);

   if( !isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) ) {
     errorf("ERROR: ConsoleThread only supports terminal input/output\n");
     exit(1);
   }

   pub::Console::start();
   start();
}

virtual
   ~ConsoleThread( void )           // Destructor
{  if( HCDM ) debugf("ConsoleThread(%p).~\n", this); }

//----------------------------------------------------------------------------
// ConsoleThread::Methods
//----------------------------------------------------------------------------
public:
int
   getch( void )                    // Get character from stdin
{ return pub::Console::getch(); }

void
   putch(                           // Put character onto stdout
     int               C)           // The character
{  pub::Console::putch(C); }

char*                               // The input line
   readline( void )                 // Read input line
{
   pub::Console::puts(">>> ");      // Input prompt
   pub::Console::gets(inp, sizeof(inp)-1);
   char* C= strip(inp);
   if( HCDM )
     debugf("==> %s\n", C);
   return C;
}

virtual void
   run( void )                      // The operational thread
{  if( HCDM ) debugf("ConsoleThread(%p).run\n", this);

   operational= true;
   sleep(1);                        // One second startup delay

   while( operational ) {
     char* C= readline();
     if( operational ) {
       if( *C == '\0' ) continue;   // Ignore empty command line

       enum{ MAXV= 128 };           // Argument array size
       int   argc;
       char* argv[MAXV];

       for(argc= 0; argc<MAXV-1; argc++) {
         if( *C == '\"' || *C == '\'' ) { // If quoted parameter
           int Q= *C;               // Quote delimiter
           C++;                     // Skip the quote
           argv[argc]= (char*)C;
           while( *C != Q && *C != '\0' )
             C++;
         } else {
           argv[argc]= (char*)C;
           while( !isspace(*C) && *C != '\0' )
             C++;
         }

         if( *C == '\0' )
           break;
         *C= '\0';
         C++;

         while( isspace(*C) )
           C++;
         if( *C == '\0' )
           break;
       }
       argv[++argc]= nullptr;

       Command* command= Command::locate(argv[0]);
       if( command ) {
         command->work(argc, argv);
       } else {
         debugf("Command '%s' not found\n", visify(argv[0]).c_str());
       }
     }
   }
}

virtual void
   stop( void )                     // Terminate the thread
{  if( HCDM ) debugf("ConsoleThread(%p).stop\n", this);

   pub::Console::stop();
   operational= false;
}

virtual void
   wait( void )                     // Wait for termination completion
{  if( HCDM ) debugf("ConsoleThread(%p).wait\n", this);

   pub::Console::wait();
   join();
}
} consoleThread; // Our ConsoleThread

//----------------------------------------------------------------------------
//
// Method-
//       ConsoleCommand::work
//
// Purpose-
//       Process ConsoleCommand work item.
//
//----------------------------------------------------------------------------
Command::resultant                  // Resultant
   ConsoleCommand::work(            // Handle
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{  Command::work(argc, argv);

   for(int i= 0; i<argc; i++)
     debugf("[%2d] \"%s\"\n", i, argv[i]);

   return nullptr;
}

//============================================================================
//
// Method-
//       ConsoleService::stop
//
// Purpose-
//       Stop the ConsoleService, stopping the ConsoleThread.
//
//----------------------------------------------------------------------------
void
   ConsoleService::stop( void )     // Stop the ConsoleService
{  if( HCDM ) debugf("ConsoleService(%p).stop\n", this);

   consoleThread.stop();            // Stop the ConsoleThread
}

//----------------------------------------------------------------------------
//
// Method-
//       ConsoleService::wait
//
// Purpose-
//       Wait for ConsoleService termination.
//
//----------------------------------------------------------------------------
void
   ConsoleService::wait( void )     // Wait for ConsoleService termination
{  if( HCDM ) debugf("ConsoleService(%p).wait\n", this);

   consoleThread.wait();            // Wait for ConsoleThread
}
