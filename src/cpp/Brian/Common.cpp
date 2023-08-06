//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2023 Frank Eskesen.
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
//       2023/08/04
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Thread.h>             // For pub::Thread::sleep

#include "Common.h"
#include "Service.h"                // For Service

using pub::Debug;
using namespace pub::debugging;     // For debugging
using pub::Exception;
using pub::Thread;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <pub/ifmacro.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ID_AGENT   "Brian"
#define ID_VERSION "0.0-2023-08-04"

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Common*                Common::common= nullptr; // THE common singleton

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     user_agent=
                          ID_AGENT "/" ID_VERSION "/Bringup"
                          " {f.n.eskesen@gmail.com, "
                          "machine learning experiment}";

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
{  traceh("Common(%p)::~Common()\n", this);

   //-------------------------------------------------------------------------
   // Terminate dispatcher services
   pub::dispatch::Disp::shutdown();

   common= nullptr;                // Delete the singleton pointer
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
{  traceh("Common(%p)::Common()\n", this);
   common= this;

   //-------------------------------------------------------------------------
   // Go into READY state
   fsm= FSM_READY;
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
#if 0
   //-------------------------------------------------------------------------
   // Environmental check: L/libpub.a MUST NOT exist
   //   (com library must be obtained from DLL.)
   //
   // Implementation notes: This is not the only error that occurs.
   //   + Loader.cpp: dlopen (frequently) hangs
   //-------------------------------------------------------------------------
   std::string s= "L/libpub.a";
   FILE* f= fopen(s.c_str(), "rb");
   if( f != nullptr )              // If file exists
   {
     fprintf(stderr, "Warning: File(%s) exists\n", s.c_str());
     fprintf(stderr, ".. Library object Debug.o must be located in a DLL"
                       " to prevent reloading\n"
                     ".. a separate copy of it each time we load a DLL.\n"
                     "!! YOU HAVE BEEN WARNED !!\n");
     fclose(f);
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
{  traceh("Common(%p)::shutdown() fsm(%d)\n", this, fsm);

   //-------------------------------------------------------------------------
   // Go into shutdown state
   fsm= FSM_CLOSE;

   //-------------------------------------------------------------------------
   // Terminate services
   typedef ServiceMap::MapIter_t MapIter_t;
// Haven't figured out a gnu++17 foreach syntax that works
// for(MapIter_t mi : ServiceMap) mi.second->stop(); // DOES NOT COMPILE

   for(MapIter_t mi= ServiceMap.begin(); mi != ServiceMap.end(); ++mi) {
     Service* service= mi->second;
     service->stop();
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
{  traceh("Common(%p)::wait() fsm(%d)...\n", this, fsm);

   //-------------------------------------------------------------------------
   // Wait for termination signal
   event.wait();

   //-------------------------------------------------------------------------
   // Wait for service terminations
   typedef ServiceMap::MapIter_t MapIter_t;
   for(MapIter_t mi= ServiceMap.begin(); mi != ServiceMap.end(); ++mi) {
     Service* service= mi->second;
     service->wait();
   }

   //-------------------------------------------------------------------------
   // Complete shutdown
   fsm= FSM_RESET;
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::work
//
// Purpose-
//       Drive a service
//
//----------------------------------------------------------------------------
void
   Common::work(                    // Drive a Service
     std::string       name,        // The Service's name
     pub::dispatch::Item*
                       item)        // The associated work Item
{  // debugf("Common::work(%s,%p)\n", name.c_str(), item);
   Service& service= ServiceMap[name]; // Locate the Service
   work(&service, item);            // Drive the Service
}
