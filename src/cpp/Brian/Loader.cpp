//----------------------------------------------------------------------------
//
//       Copyright (c) 2024-2025 Frank Eskesen.
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
//       Loader.cpp
//
// Purpose-
//       Load built-in classes
//
// Last change date-
//       2025/01/16
//
// Implementation notes-
//       TODO: REMOVE TESTING CODE
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::mutex, std::lock_guard

#include <pub/Debug.h>              // For namespace debugging
#include <pub/diag-counter.h>       // For pub::diag::Counter::debug
#include <pub/Thread.h>             // For pub::Thread::static_debug

#include "Common.h"                 // For Common::shutdown
#include "Command.h"                // For Command (base class)
#include "Loader.h"                 // For Loader, implemented
#include "Service.h"                // For Service (base class)

#include "HttpMapper.h"             // For HttpMapper::status

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging subroutines
using PUB::Debug;                   // For class Debug

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
// Global constructors/destructors
// Order:
//   GlobalDestructor::constructor
//   SecondDestructor::constructor
//     :
//   SecondDestructor::destructor
//   GlobalDestructor::destructor
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
static int             global_destructor_invoked= false;
static struct GlobalDestructor {
   GlobalDestructor( void )
{  if( HCDM ) debugh("Loader::GlobalDestructor!\n"); }

   ~GlobalDestructor( void )
{  if( HCDM ) debugh("Loader::GlobalDestructor~\n");

   global_destructor_invoked= true;
}
}  globalDestructor;
}  // Anonymous namespace

namespace {                         // Anonymous namespace
static int             second_destructor_invoked= false;
static struct SecondDestructor {
   SecondDestructor( void )
{  if( HCDM ) debugh("Loader::SecondDestructor!\n"); }

   ~SecondDestructor( void )
{  if( HCDM ) debugh("Loader::SecondDestructor~\n");

   second_destructor_invoked= true;
}
}  secondDestructor;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Method-
//       Loader::Loader
//
// Purpose-
//       Load extra module dependencies
//
// Implementation notes-
//       Brian.cpp defines: Loader loader, which invokes Loader::Loader.
//       In here, we just reference entry points to get them loaded.
//       Note that we don't need to include any definition files; we just need
//       to know the types and the entry point names in the source files.
//
//----------------------------------------------------------------------------
class  ConsoleService;              // Forward references
class  Command_curl;
class  Command_init;
struct startup_event_handler_t;

extern ConsoleService  consoleService; // (In Console.cpp)
extern Command_curl    command_curl; // (In Curl.cpp)
extern Command_init    command_init; // (In HttpServer.cpp)
extern struct startup_event_handler_t
                       startup_event_handler; // (In HttpServer.cpp)

   Loader::Loader( void )           // Create module dependencies
{  if( HCDM ) debugh("Loader::Loader!\n");

   // TODO: TEST: DO WE NEED TO REFERENCE EVERY MODULE ENTRY POINT?
   //             (HttpServer contains command_init, startup_event_handler)
#if 1 // DO WE NEED TO ACTUALLY REFERENCE THE EXTERNAL? YES!
   ConsoleService* service= &consoleService;
   Command_curl*   curl= &command_curl;
   Command_init*   init= &command_init;
   startup_event_handler_t*
                   se_handler= nullptr;
#if 0 // DO WE NEED TO REFERENCE EVERY MODULE ENTRY POINT? NO!
                   se_handler= &startup_event_handler;
#endif

   if( HCDM )
     debugh("service(%p) curl(%p) init(%p) handler(%p)\n"
           , service, curl, init, se_handler);
#endif
};

//----------------------------------------------------------------------------
//
// Class-
//       Command_exit
//
// Purpose-
//       Immediate exit
//
//----------------------------------------------------------------------------
class Command_exit : public Command {
public:
   Command_exit() : Command("exit")
{  }

virtual Command::resultant          // Resultant
   main(int, char**)                // Handle Command
{  exit(0); }
}  command_exit; // static class Command_exit

//----------------------------------------------------------------------------
//
// Class-
//       Command_list
//
// Purpose-
//       List commands or services
//
//----------------------------------------------------------------------------
class Command_list : public Command {
public:
   Command_list() : Command("list")
{  }

virtual Command::resultant          // Resultant
   main(                            // Handle Command
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   std::lock_guard<Debug> lock(*Debug::get());

   std::string arg1("command");
   if( argc > 1 )
     arg1= argv[1];

   if( arg1 != "service" && arg1 != "services" ) {
     debugh("Commands:\n");
     Command::Map_t* map= Command::get_map();

     size_t column= 0;                // Output column
     for(Command::MapIter_t mi= map->begin(); mi != map->end(); ++mi) {
       std::string s= mi->first;
       if( column + s.size() > 78 ) {
         debugf("\n");
         column= 0;
       }

       if( column != 0 ) {
         debugf(", ");
         column += 2;
       }
       debugf("%s", s.c_str());

       column += s.size();
     }
   } else {
     debugh("Services:\n");
     Service::Map_t* map= Service::get_map();

     size_t column= 0;                // Output column
     for(Service::MapIter_t mi= map->begin(); mi != map->end(); ++mi) {
       std::string s= mi->first;
       if( column + s.size() > 78 ) {
         debugf("\n");
         column= 0;
       }

       if( column != 0 ) {
         debugf(", ");
         column += 2;
       }
       debugf("%s", s.c_str());

       column += s.size();
     }
   }
   debugf("\n");

   return nullptr;
}
}  command_list; // static class Command_list

//----------------------------------------------------------------------------
//
// Class-
//       Command_status
//
// Purpose-
//       Display state information
//
//----------------------------------------------------------------------------
class Command_status : public Command {
public:
   Command_status() : Command("status")
{  }

virtual Command::resultant          // Resultant
   main(int, char**)                // Handle Command
{
   // Reset WorkerPool
   pub::WorkerPool::reset();
   pub::Thread::sleep(0.25);

   // Display Thread status
   pub::Thread::static_debug("status");

   // Display Listener status
   debugf("\n");
   HttpMapper::get()->status();

   return nullptr;
}
}  command_status; // static class Command_status
