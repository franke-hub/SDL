//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Common.cpp
//
// Purpose-
//       Brian Common object methods
//
// Last change date-
//       2024/10/22
//
//----------------------------------------------------------------------------
#include <sys/stat.h>               // For struct stat

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Thread.h>             // For pub::Thread::sleep

#include "Command.h"                // For Command
#include "Service.h"                // For Service
#include "Common.h"                 // For Common, implemented

using pub::Debug;
using namespace pub::debugging;     // For debugging
using pub::Exception;
using pub::Thread;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ID_AGENT   "Brian"
#define ID_VERSION "0.0-2024-09-28"

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Common*                Common::common= nullptr; // THE common singleton

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     user_agent=
                          ID_AGENT "/" ID_VERSION
                          "/Bringup: machine learning experiment"
                          ",Contact: {frank @ eskesystems com}";

//----------------------------------------------------------------------------
//
// Subroutine-
//       force_load
//
// Purpose-
//       Create module dependencies
//
// Implementation notes-
//       We need to reference these entry points to get them loaded.
//       Note that we don't need to include any files.
//       (The entry points are externally accessible.)
//
//----------------------------------------------------------------------------
class  ConsoleService;              // Forward references
class  Command_list;
class  Command_quit;

extern ConsoleService  consoleService; // (In Console.cpp)
extern Command_list    command_list; // (In Loader.cpp)
extern Command_quit    command_quit; // (In Loader.cpp)
static void
   force_load( void )               // Create module dependencies
{
   ConsoleService* service= &consoleService; // Needed? Maybe. (Untested)
   Command_list*   list= &command_list;
   Command_quit*   quit= &command_quit;

   debugf("You might want or need to update Common::force_load()\n");

   if( HCDM )
     debugf("service(%p) list(%p) quit(%p)\n", service, list, quit);
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::Common
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Common::Common( void )           // Default constructor
:  event()
,  fsm(FSM_RESET)
,  brian(user_agent)
{  if( HCDM ) traceh("Common(%p)::Common()\n", this);
   common= this;

   //-------------------------------------------------------------------------
   // Go into READY state
   fsm= FSM_READY;
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::~Common
//
// Purpose-
//       Destructor.
//
// Notes-
//       All Threads have completed or we wouldn't be here.
//
//----------------------------------------------------------------------------
   Common::~Common( void )          // Destructor
{  if( HCDM ) traceh("Common(%p)::~Common()\n", this);

   //-------------------------------------------------------------------------
   // Terminate dispatcher services
   pub::dispatch::Disp::shutdown();

   common= nullptr;                // Delete the singleton pointer
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::make
//
// Purpose-
//       Create the Common singleton.
//
//----------------------------------------------------------------------------
Common*                             // -> THE Common area (Singleton)
   Common::make( void )             // Go into READY state
{
#if 1
   //-------------------------------------------------------------------------
   // Environmental check: L/libBrian.a MUST NOT exist
   //   (We must be running using DLLs.)
   //
   // Implementation notes: This is not the only error that can occur.
   //   + The Debug RecursiveLatch can be obtained in one thread and released
   //     in another, causing a terminating abort. See Console.cpp heading.
   //   + Loader.cpp: dlopen (frequently) hangs
   //-------------------------------------------------------------------------
   const char* file_name= "./libBrian.a";
   struct stat info;
   int rc= stat(file_name, &info);
   if( rc == 0 ) {
     debugf("Warning: File(%s) exists\n", file_name);
     debugf(".. This implies that you probably aren't using DLLs\n");
     debugf(".. Library object Debug.o must be located in a DLL"
               " to prevent reloading\n"
             ".. a separate copy of it each time we load a DLL.\n"
             "!! YOU HAVE BEEN WARNED !!\n");

     force_load();                  // Force module dependencies
   }
#endif

   //-------------------------------------------------------------------------
   // Create the Common area
   //-------------------------------------------------------------------------
   if( Common::common )
     throw Exception("Common::Common duplicated");

   Common* common= new Common();

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------

   //-------------------------------------------------------------------------
   // Allow time for activation to complete
   Thread::sleep(0.5);

   return common;
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::shutdown
//
// Purpose-
//       Go into CLOSE state.
//
//----------------------------------------------------------------------------
void
   Common::shutdown( void )         // Go into CLOSE state
{  if( HCDM ) traceh("Common(%p)::shutdown() fsm(%d)\n", this, fsm);

   //-------------------------------------------------------------------------
   // Go into shutdown state
   fsm= FSM_CLOSE;

   //-------------------------------------------------------------------------
   // Stop all *Stoppable" services
   typedef Service::Map_t           Map_t;
   typedef Service::MapIter_t       MapIter_t;
   Map_t* map= Service::get_map();

#if 0
// Haven't figured out a gnu++17 foreach syntax that works
   if( false ) {
     for(MapIter_t mi: *map)        // ** DOES NOT COMPILE **
       mi.second->stop();
   }
#endif

   for(MapIter_t mi= map->begin(); mi != map->end(); ++mi) {
     Service* service= mi->second;
     Service::has_stop* method= dynamic_cast<Service::has_stop*>(service);
     if( method )
       method->stop();
   }

   event.post(0);                   // Termination initiated
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::wait
//
// Purpose-
//       Complete Common termination.
//
//----------------------------------------------------------------------------
void
   Common::wait( void )             // Wait for termination
{  if( HCDM ) traceh("Common(%p)::wait() fsm(%d)...\n", this, fsm);

   //-------------------------------------------------------------------------
   // Wait for termination signal
   event.wait();

   //-------------------------------------------------------------------------
   // Wait for service terminations
   typedef Service::Map_t           Map_t;
   typedef Service::MapIter_t       MapIter_t;
   Map_t* map= Service::get_map();
   for(MapIter_t mi= map->begin(); mi != map->end(); ++mi) {
     Service* service= mi->second;
     Service::has_wait* method= dynamic_cast<Service::has_wait*>(service);
     if( method )
       method->wait();
   }

   //-------------------------------------------------------------------------
   // Complete shutdown
   fsm= FSM_RESET;
}
