//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2025 Frank Eskesen.
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
//       2025/01/20
//
//----------------------------------------------------------------------------
#include <new>                      // For std:: (in-place operator new)

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
,  VERBOSE= 1                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ID_AGENT   "Brian"
#define ID_VERSION "0.0-2024-09-28"

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Common*                Common::common= nullptr; // THE Common singleton
StaticCommon*          static_common= nullptr; // THE StaticCommon singleton

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     user_agent=
                          ID_AGENT "/" ID_VERSION
                          "/Bringup: machine learning experiment"
                          ",Contact: {frank @ eskesystems com}";

static union {
double                 for_alignment;
char                   for_space[sizeof(StaticCommon)];
} the_static_common;

//----------------------------------------------------------------------------
// Static initialization/termination
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
static struct StaticGlobal {
   StaticGlobal(void)               // Constructor, static initialization
{  StaticCommon::make(); }          // Insure make invoked

   ~StaticGlobal(void)              // Destructor, static termination
{
   // We don't delete static common, it's constructed in-place
   static_common->~StaticCommon();  // Run StaticCommon destructor
   static_common= nullptr;          // StaticCommon is now gone
}
}  staticGlobal;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Method-
//       StaticCommon::make
//
// Purpose-
//       StaticCommon pseudo-allocator and constructor.
//
//----------------------------------------------------------------------------
StaticCommon*                       // (Can be ignored)
   StaticCommon::make( void )       // Construct StaticCommon
{  if( HCDM ) debugh("StaticCommon::make %p\n", static_common);

   // static_common is only initialized once, always in static initialization
   if( static_common == nullptr ) {
     static_common= new(&the_static_common) StaticCommon();
     if( HCDM )
       debugh("new static_common(%p)\n", static_common);
   }
   return static_common;
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
{  if( HCDM ) traceh("Common(%p)!\n", this);
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
{  if( HCDM ) traceh("Common(%p)~\n", this);

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
   Service::start_all();            // Start all services (from main task)

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

   event.post(0);                   // Indicate shutdown initiated
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::wait
//
// Purpose-
//       Wait for shutdown, Services stopped.
//
//----------------------------------------------------------------------------
void
   Common::wait( void )             // Wait for shutdown initiated
{  if( HCDM ) traceh("Common(%p)::wait() fsm(%d)...\n", this, fsm);

   //-------------------------------------------------------------------------
   // Wait for shutdown's completion post, resuming main task
   event.wait();

   pub::signals::Event event;
   static_common->shutdown_started.signal(event); // Raise shutdown started

   //-------------------------------------------------------------------------
   // Wait for all services to complete (from main task)
   Service::stop_all();             // Stop all Services
   Service::wait_all();             // Wait for Services stop completion
}
