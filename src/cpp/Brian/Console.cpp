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
//       2024/11/15
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
{  HCDM= true                       // Hard Core Debug Mode?
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
   pub::Console::puts(">>> ");           // Input prompt
   pub::Console::gets(inp, sizeof(inp)-1);
   char* C= strip(inp);
   if( USE_COMMAND_ECHOING ) {
     std::lock_guard<Debug> lock(*Debug::get());

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

debugh("ConsoleThread(%p)::run EXIT\n", this);
}

virtual void
   stop( void )                     // Terminate the thread
{  if( HCDM ) debugh("ConsoleThread(%p).stop\n", this);

   operational= false;
   pub::Console::stop();
debugh("ConsoleThread.stopped (invoked pub::Console::stop)\n");
}

virtual void
   wait( void )                     // Wait for termination completion
{  if( HCDM ) debugh("ConsoleThread(%p).wait\n", this);

debugh("ConsoleThread(%p)::wait()\n", this);
debugh("...pub::Console::wait()...\n");
   pub::Console::wait();            // Wait for the Console
debugh("...pub::Console::...wait() complete\n");
debugh("...Thread::current(%p)\n", Thread::current());
debugh("...ConsoleThread(%p)::joinable(%d)\n", this, joinable());

   // We have unexplained Cygwin-only issues with join never completing.
   // Extra super debug mode diagnostics...
   if( true  ) {
debugh("%4d ...BEFORE ConsoleThread(%p)::join()\n", __LINE__, this);
debugh(" "); Thread::static_debug("ConsoleThread join");
debugh(" "); debug("ConsoleThread join invoked");
   }

debugf("\n\n\n");
debugh("%4d ...ConsoleThread(%p)::join() NOW <<<PROBLEM>>\n", __LINE__, this);
   join();
debugh("%4d ...JOINED, HOORAY! ConsoleThread(%p)::join() DONE\n", __LINE__, this);

debugh(" "); debug("ConsoleThread join complete");
debugh("...ConsoleThread(%p)::join() complete\n", this);
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
debugh("ConsoleService: ConsoleThread(%p) started\n", console_thread);
}

virtual void
   stop(Service*)                   // Stop the ConsoleService
{  if( HCDM ) debugh("ConsoleService(%p).stop\n", this);
   Service::has_stop::stop(this);

   console_thread->stop();          // Stop the ConsoleThread
debugh("ConsoleService: ConsoleThread(%p) stopped\n", console_thread);
}

virtual void
   wait(Service* )                  // Wait for ConsoleService termination
{  if( HCDM ) debugh("ConsoleService(%p).wait\n", this);
   Service::has_wait::wait(this);

   console_thread->wait();          // Wait for the ConsoleThread
debugh("ConsoleService: ConsoleThread(%p) wait complete\n", console_thread);

   debugh("DELETING console_thread\n");
   delete console_thread;
   console_thread= nullptr;
}
}  consoleService; // class ConsoleService
