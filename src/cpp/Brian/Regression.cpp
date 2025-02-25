//----------------------------------------------------------------------------
//
//       Copyright (C) 2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Regression.cpp
//
// Purpose-
//       Implement regression test
//
// Last change date-
//       2025/02/24
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <unistd.h>                 // For gethostname

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For namespace pub::dispatch
#include <pub/Thread.h>             // For pub::Thread::sleep

#include "Common.h"                 // For Common::start Signal
#include "Command.h"                // For Command
#include "Service.h"                // For Service

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For convenience
using namespace PUB::debugging;     // For debugging subroutines
using std::string;                  // For convenience

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= true                       // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// Anonymous namespace
//----------------------------------------------------------------------------
namespace {
class Task : public pub::dispatch::Task {
public:
   Task( void ) = default;

virtual
   ~Task( void ) = default;

virtual void
   work(pub::dispatch::Item* item)
{  if( HCDM ) debugh("Regression::work\n");

   pub::Thread::sleep(.30);
   debugf("\n");
   char host_name[256];             // Our HOSTNAME
   host_name[0]= '\0';
   gethostname(host_name, sizeof(host_name));
   std::string host= host_name;
   host += ":8080";

   std::string local="localhost:8081";

   Command::command("curl " + host);
   Command::command("curl " + local);

   Command::command("status");
   Command::command("quit");

   pub::Thread::sleep(.30);
   item->post(pub::dispatch::Item::CC_NORMAL);
}
}  static_task; // Class Task
}  // Anonymous namespaace

//----------------------------------------------------------------------------
//
// Class-
//       Command_regression
//
// Purpose-
//       Run regression tests
//
//----------------------------------------------------------------------------
class Command_regression : public Command {
//----------------------------------------------------------------------------
// Command_regression::Constructors/destructor
public:
   Command_regression( void )       // Constructor
:  Command("regression") {}

virtual
   ~Command_regression( void ) = default; // Destructor

//----------------------------------------------------------------------------
// Command_regression::Methods
//----------------------------------------------------------------------------
virtual resultant                   // Resultant, command dependent
   main(int, char*[])               // Process the Command
//   int               argc,        // Argument count (UNUSED)
//   char*             argv[])      // Argument array (UNUSED)
{  if( HCDM ) debugh("Regression::main\n");

   pub::dispatch::Wait wait;        // Our Wait object
   pub::dispatch::Item item(&wait); // Our work Item
   static_task.enqueue(&item);      // Schedule work
   wait.wait();                     // Wait for completion

   return nullptr;
}
}; // class Command_regression
Command_regression command_regression;
