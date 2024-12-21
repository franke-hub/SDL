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
//       Operate the input terminal
//
// Last change date-
//       2024/12/20
//
// Implementation note-
//       When running using a static library build, HCDM debugging displays in
//       start() and run() should be disabled. The debug RecursiveLatch unlock
//       may fail with a terminating error.
//       (This should not occur, but has not been debugged.)
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::mutex, std::lock_guard
#include <ctype.h>                  // For isspace
#include <string.h>                 // For strlen
#include <unistd.h>                 // For isatty, STDIN_FILENO, ...

#include <pub/Console.h>            // For pub::Console
#include <pub/Debug.h>              // For debugging
#include <pub/Thread.h>             // For pub::Thread
#include <pub/utility.h>            // For pub::utility::visify

#include "Command.h"                // For class Command
#include "Service.h"                // For class Service

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging subroutines
using PUB::utility::visify;         // For method pub::utility::visify

using PUB::Debug;                   // For class pub::Debug
using PUB::Thread;                  // For class pub::Thread

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  USE_COMMAND_ECHOING= true        // Echo commands to trace file?
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
class ConsoleThread : public Thread { // The ConsoleThread
//----------------------------------------------------------------------------
// ConsoleThread::Attributes
//----------------------------------------------------------------------------
bool                   operational; // Operational state?
char                   inp[4096];   // The input string buffer

//----------------------------------------------------------------------------
// ConsoleThread::Constructors
//----------------------------------------------------------------------------
public:
   ConsoleThread( void )            // Constructor
:  Thread()
{  if( HCDM ) debugh("ConsoleThread(%p).!\n", this);

   if( !isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) ) {
     errorf("ERROR: ConsoleThread only supports terminal input/output\n");
     exit(1);
   }

   pub::Console::start();
   if( HCDM )
     debugh("pub::Console::start completed\n");
}

virtual
   ~ConsoleThread( void )           // Destructor
{  if( HCDM ) debugh("ConsoleThread(%p).~\n", this); }

//----------------------------------------------------------------------------
// ConsoleThread::Methods
//----------------------------------------------------------------------------
public:
int
   getch( void )                    // Get character from stdin
{  return pub::Console::getch(); }

void
   putch(                           // Put character onto stdout
     int               C)           // The character
{  pub::Console::putch(C); }

char*                               // The input line
   readline( void )                 // Read input line
{
   if( USE_COMMAND_ECHOING ) {
     tracef("\n");
   }
   pub::Console::puts(">>> ");           // Input prompt
   pub::Console::gets(inp, sizeof(inp)-1);
   char* C= strip(inp);
   if( USE_COMMAND_ECHOING ) {
     traceh("==> '%s'\n", C);
   }
   return C;
}

virtual void
   run( void )                      // The operational thread
{  if( HCDM ) debugh("ConsoleThread(%p).run\n", this);

   operational= true;
   sleep(1);                        // One second startup delay

   while( operational ) {
     char* C= readline();
     if( operational )
       Command::command(C);         // Run the command, ignoring any resultant
   }
}

virtual void
   stop( void )                     // Terminate the thread
{  if( HCDM ) debugh("ConsoleThread(%p).stop\n", this);

   operational= false;
   pub::Console::stop();
}

virtual void
   wait( void )                     // Wait for termination completion
{  if( HCDM ) debugh("ConsoleThread(%p).wait\n", this);

   pub::Console::wait();            // Wait for the Console
   join();
}
}; // ConsoleThread

//----------------------------------------------------------------------------
//
// Class-
//       ConsoleService
//
// Purpose-
//       Control ConsoleThread termination
//
//----------------------------------------------------------------------------
class ConsoleService                // The ConsoleService
:  public Service
,  public Service::has_start
,  public Service::has_stop
,  public Service::has_wait {
//----------------------------------------------------------------------------
// ConsoleService::Attributes
//----------------------------------------------------------------------------
public:
ConsoleThread*         console_thread= nullptr;

//----------------------------------------------------------------------------
// ConsoleService::Constructors
//----------------------------------------------------------------------------
public:
   ConsoleService( void )           // Constructor
:  Service("Console") {}

   ConsoleService(const ConsoleService&) = delete; // Disallowed copy constructor
   ConsoleService& operator=(const ConsoleService&) = delete; // Disallowed assignment operator

virtual
   ~ConsoleService( void )          // Destructor
{
   delete console_thread;
   console_thread= nullptr;
}

//----------------------------------------------------------------------------
// ConsoleService::Methods
//----------------------------------------------------------------------------
public:
virtual void
   start(Service* S)                // Start the ConsoleService
{  if( HCDM ) debugh("ConsoleService(%p).start(%p)\n", this, S);
   Service::has_start::start(this);

   if( console_thread ) {
     debugh("ConsoleService::start ERROR: already started\n");
     return;
   }

   console_thread= new ConsoleThread();
   console_thread->start();
}

virtual void
   stop(Service*)                   // Stop the ConsoleService
{  if( HCDM ) debugh("ConsoleService(%p).stop\n", this);
   Service::has_stop::stop(this);

   console_thread->stop();          // Stop the ConsoleThread
}

virtual void
   wait(Service* )                  // Wait for ConsoleService termination
{  if( HCDM ) debugh("ConsoleService(%p).wait\n", this);
   Service::has_wait::wait(this);

   console_thread->wait();          // Wait for the ConsoleThread
   delete console_thread;
   console_thread= nullptr;
}
}  consoleService; // class ConsoleService
