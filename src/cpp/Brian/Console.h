//----------------------------------------------------------------------------
//
//       Copyright (c) 2021-2025 Frank Eskesen.
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
//       Define the ConsoleService
//
// Last change date-
//       2025/03/01
//
//----------------------------------------------------------------------------
#ifndef CONSOLE_H_INCLUDED
#define CONSOLE_H_INCLUDED

#include <pub/Thread.h>             // For pub::Thread

#include "Service.h"                // For class Service

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class ConsoleService;
extern ConsoleService  consoleService; // The ConsoleService

//----------------------------------------------------------------------------
//
// Class-
//       ConsoleThread
//
// Purpose-
//       The ConsoleThread.
//
//----------------------------------------------------------------------------
class ConsoleThread : public pub::Thread { // The ConsoleThread
//----------------------------------------------------------------------------
// ConsoleThread::Attributes
//----------------------------------------------------------------------------
public:
bool                   operational; // Operational state?
char                   inp[4096];   // The input string buffer

//----------------------------------------------------------------------------
// ConsoleThread::Constructors/Destructor
//----------------------------------------------------------------------------
   ConsoleThread( void );           // Constructor

virtual
   ~ConsoleThread( void );          // Destructor

//----------------------------------------------------------------------------
// ConsoleThread::Methods
//----------------------------------------------------------------------------
int
   getch( void );                   // Get character from stdin

void
   putch(                           // Put character onto stdout
     int               C);          // The character

char*                               // The input line
   readline( void );                // Read input line

virtual void
   run( void );                     // The operational thread

virtual void
   stop( void );                    // Terminate the thread

virtual void
   wait( void );                    // Wait for termination completion
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
,  public Service::has_wait
{
//----------------------------------------------------------------------------
// ConsoleService::Attributes
//----------------------------------------------------------------------------
public:
ConsoleThread*         console_thread= nullptr;

//----------------------------------------------------------------------------
// ConsoleService::Constructors
//----------------------------------------------------------------------------
public:
   ConsoleService( void );          // Constructor

// Disallowed copy constructor; Disallowed assignment operator
   ConsoleService(const ConsoleService&) = delete;
   ConsoleService& operator=(const ConsoleService&) = delete;

virtual
   ~ConsoleService( void );         // Destructor

//----------------------------------------------------------------------------
// ConsoleService::Methods
//----------------------------------------------------------------------------
public:
virtual void
   start(Service*);                 // Start the ConsoleService

virtual void
   stop(Service*);                  // Stop the ConsoleService

virtual void
   wait(Service*);                  // Wait for ConsoleService termination
}; // class ConsoleService
#endif // CONSOLE_H_INCLUDED
