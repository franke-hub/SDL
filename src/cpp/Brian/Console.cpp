//----------------------------------------------------------------------------
//
//       Copyright (c) 2021-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Console.cpp
//
// Purpose-
//       Operate the input terminal
//
// Last change date-
//       2025/03/01
//
// Implementation note-
//       When running using a static library build, HCDM debugging displays in
//       start() and run() should be disabled. The debug RecursiveLatch unlock
//       may fail with a terminating error.
//       (This should not occur, but has not been debugged.)
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::mutex, std::lock_guard
#include <cctype>                   // For isspace
#include <cstring>                  // For strlen

#include <unistd.h>                 // For isatty, STDIN_FILENO, ...

#include <pub/Console.h>            // For pub::Console
#include <pub/Debug.h>              // For debugging
#include <pub/Thread.h>             // For pub::Thread

#include "Command.h"                // For class Command
#include "Common.h"                 // For class Common
#include "Console.h"                // For class CommonThread, CommonService
#include "Service.h"                // For class Service

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging subroutines

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
// External data areas
//----------------------------------------------------------------------------
ConsoleService         consoleService; // The ConsoleService

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
std::mutex             mutex;       // (Hidden) mutex

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
// Methods-
//       ConsoleThread::ConsoleThread
//       ConsoleThread::~ConsoleThread
//
// Purpose-
//       Constructor.
//       Destructor.
//
//----------------------------------------------------------------------------
   ConsoleThread::ConsoleThread( void ) // Constructor
:  pub::Thread()
{  if( HCDM ) debugh("ConsoleThread(%p).!\n", this);

   if( !isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) ) {
     errorf("ERROR: ConsoleThread only supports terminal input/output\n");
     exit(1);
   }
}

   ConsoleThread::~ConsoleThread( void ) // Destructor
{  if( HCDM ) debugh("ConsoleThread(%p).~\n", this); }

//----------------------------------------------------------------------------
// ConsoleThread::Methods
//----------------------------------------------------------------------------
int
   ConsoleThread::getch( void )     // Get character from stdin
{  return pub::Console::getch(); }

void
   ConsoleThread::putch(            // Put character onto stdout
     int               C)           // The character
{  pub::Console::putch(C); }

char*                               // The input line
   ConsoleThread::readline( void )  // Read input line
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

void
   ConsoleThread::run( void )       // The operational thread
{  if( HCDM ) debugh("ConsoleThread(%p).run\n", this);

   pub::Console::start();
   if( HCDM )
     debugh("pub::Console::started\n");

   operational= true;
   sleep(1);                        // One second startup delay

   while( operational ) {
     char* C= readline();
     if( operational )
       Command::command(C);         // Run the command, ignoring any resultant

   }

   if( HCDM && VERBOSE > 0 ) debugh("pub::Console::run complete\n");
}

void
   ConsoleThread::stop( void )      // Terminate the thread
{  if( HCDM ) debugh("ConsoleThread(%p).stop\n", this);

   if( operational )
     pub::Console::stop();

   operational= false;
}

void
   ConsoleThread::wait( void )      // Wait for termination completion
{  if( HCDM ) debugh("ConsoleThread(%p).wait\n", this);

   pub::Console::wait();            // Wait for the Console
}

//----------------------------------------------------------------------------
//
// Methods-
//       ConsoleService::ConsoleService
//       ConsoleService::~ConsoleService
//
// Purpose-
//       Constructors
//       Destructors
//
//----------------------------------------------------------------------------
   ConsoleService::ConsoleService( void )      // Constructor
:  Service("Console") {}

   ConsoleService::~ConsoleService( void )     // Destructor
{
   std::lock_guard<decltype(mutex)> lock(mutex);
   delete console_thread;
   console_thread= nullptr;
}

//----------------------------------------------------------------------------
// ConsoleService::Methods
//----------------------------------------------------------------------------
void
   ConsoleService::start(Service* S) // Start the ConsoleService
{  if( HCDM ) debugh("ConsoleService(%p).start(%p)\n", this, S);
   Service::has_start::start(this);

   std::lock_guard<decltype(mutex)> lock(mutex);
   if( console_thread ) {
     debugh("ConsoleService::start ERROR: already started\n");
     return;
   }

   console_thread= new ConsoleThread();
   console_thread->start();
}

void
   ConsoleService::stop(Service*)   // Stop the ConsoleService
{  if( HCDM ) debugh("ConsoleService(%p).stop\n", this);
   Service::has_stop::stop(this);

   std::lock_guard<decltype(mutex)> lock(mutex);
   if( console_thread )
     console_thread->stop();        // Stop the ConsoleThread
}

void
   ConsoleService::wait(Service*)   // Wait for ConsoleService termination
{  if( HCDM ) debugh("ConsoleService(%p).wait\n", this);
   Service::has_wait::wait(this);

   std::lock_guard<decltype(mutex)> lock(mutex);
   if( console_thread ) {
     console_thread->wait();        // Wait for the ConsoleThread
     console_thread->join();        // Join (complete) the ConsoleThread

     delete console_thread;
     console_thread= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Class-
//       Command_quit
//
// Purpose-
//       Terminate processing
//
//----------------------------------------------------------------------------
class Command_quit : public Command {
public:
   Command_quit() : Command("quit")
{  }

virtual Command::resultant          // Resultant
   main(int, char**)                // Handle Command
{
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);
     if( consoleService.console_thread )
       consoleService.console_thread->stop();
   }}}}

   Common::get()->shutdown();
   return nullptr;
}
}  command_quit; // static class Command_quit
