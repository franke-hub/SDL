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
//       Console.cpp
//
// Purpose-
//       ConsoleCommand and ConsoleService object methods
//
// Last change date-
//       2020/01/10
//
//----------------------------------------------------------------------------
#include <mutex>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <pub/Console.h>            // For pub::Console
#include <pub/Debug.h>              // For debugging
#include <pub/Thread.h>             // For ConsoleThread

#include "ConsoleCommand.h"
#include "ConsoleService.h"
#include "Install.h"

using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <pub/ifmacro.h>

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::mutex      consoleMutex; // Unlocked when ConsoleThread completes

//----------------------------------------------------------------------------
//
// Subroutine-
//       strip
//
// Purpose-
//       Strip leading and trailing whitespace.
//
//----------------------------------------------------------------------------
static char*
   strip(                           // Strip leading and trailing whitespace
     char*             C)           // From this string
{
   int L= strlen((char*)C);
   while( L > 0 && isspace(C[L-1]) ) // Strip trailing whitespace
     C[--L]= '\0';

   while( isspace(*C) )             // Strip leading whitespace
     ++C;

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
virtual
   ~ConsoleThread( void )           // Destructor
{  }

   ConsoleThread( void )            // Constructor
:  Thread()
{
   if( !isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO) ) {
     errorf("ERROR: ConsoleThread only supports terminal input/output\n");
     exit(1);
   }

   pub::Console::start();
   start();
}

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
   debugf("==> %s\n", C);
   return C;
}

virtual void
   run( void )                      // The operational thread
{
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

       std::string cmd= argv[0];
       Command* command= CommandMap.locate(cmd);
       if( command ) {
         command->work(argc, argv);
       } else {
         char* C= argv[0];
         bool buggy= false;
         for(int i= 0; C[i] != '\0'; i++) {
           if( !isprint(C[i]) ) {
             buggy= true;
             break;
           }
         }

         if( buggy ) {
           for(int i= 0; C[i] != '\0'; i++) {
             debugf("%.2x ", C[i]);
           }
         } else {
           debugf("%s", argv[0]);
         }
           debugf(": Command not found\n");
       }
     }
   }
}

virtual void
   stop( void )                     // Terminate the thread
{
   traceh("ConsoleThread::stop()\n");
   pub::Console::stop();
   operational= false;
}

virtual void
   wait( void )                     // Wait for termination completion
{  traceh("ConsoleThread::wait()\n");
   pub::Console::wait();
   join();
}
} consoleThread; // Our ConsoleWorker

//----------------------------------------------------------------------------
//
// Method-
//       ConsoleCommand::work
//
// Purpose-
//       Process ConsoleCommand work item.
//
//----------------------------------------------------------------------------
void
   ConsoleCommand::work(            // Handle
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{  Command::work(argc, argv);

   for(int i= 0; i<argc; i++)
     debugf("[%2d] \"%s\"\n", i, argv[i]);
}

//----------------------------------------------------------------------------
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
{  consoleThread.stop(); }          // Stop the ConsoleThread

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
{  consoleThread.wait(); }          // Wait for ConsoleThread

//----------------------------------------------------------------------------
//
// Method-
//       ConsoleService::work
//
// Purpose-
//       Process ConsoleService work item.
//
//----------------------------------------------------------------------------
void
   ConsoleService::work(            // Handle
     pub::Dispatch::Item*
                       item)        // This work Item
{  Service::work(item); }           // NOT CODED YET
