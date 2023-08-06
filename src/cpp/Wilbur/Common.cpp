//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2023 Frank Eskesen.
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
//       Implement Common object methods
//
// Last change date-
//       2023/08/04
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/Object.h>
#include <com/Thread.h>
#include <com/ThreadLogger.h>
#include <com/Unconditional.h>

#include "Background.h"
#include "DbMeta.h"
#include "NetClient.h"
#include "Global.h"
#include "HttpServerPluginMap.h"

#include "Common.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ID_AGENT   "Brian"
#define ID_VERSION "0.0-2014-06-01"

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Common*                Common::common= NULL; // THE common singleton

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     user_agent=
                          ID_AGENT "/" ID_VERSION "/Bringup"
                          " {frank@eskesystems.com, "
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
{
   logf("Common(%p)::~Common()\n", this);

   //-------------------------------------------------------------------------
   // Terminate dispatcher services
   dispatcher.wait();

   //-------------------------------------------------------------------------
   // Delete allocated services
   delete background;
   background= NULL;
   delete netClient;
   netClient= NULL;
   DbMeta::shutdown();
   dbMeta= NULL;

   //-------------------------------------------------------------------------
   // Delete plug-ins
   delete httpServerMap;
   httpServerMap= NULL;

   //-------------------------------------------------------------------------
   // Remove the Global
   free(global);
   global= NULL;

   common= NULL;                    // Delete the singleton pointer
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
:  global(NULL)
,  random()
,  fsm(FSM_RESET)
,  wilbur(ID_AGENT)
,  dispatcher()
,  httpClient()
,  httpServer()
,  properties()
,  httpServerMap(NULL)
,  background(NULL)
,  dbMeta(NULL)
,  netClient(NULL)
{
   logf("Common(%p)::Common()\n", this);

   //-------------------------------------------------------------------------
   // Initialize the Global
   global= (Global*)Unconditional::malloc(sizeof(Global));
   memset((char*)global, 0, sizeof(Global));
   strcpy((char*)global->VERSION_ID, ID_VERSION);
   global->refCounter= 1;

   //-------------------------------------------------------------------------
   // Go into READY state
   fsm= FSM_READY;
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::activate
//
// Purpose-
//       Complete Common activation.
//
//----------------------------------------------------------------------------
Common*                             // -> THE Common area (Singleton)
   Common::activate(                // Go into READY state
     const char*       logName)     // The Log file name
{
   //-------------------------------------------------------------------------
   // Environmental check: L/libcom.a MUST NOT exist
   //   (com library must be obtained from DLL.)
   //
   // Implementation notes: This is not the only error that occurs.
   //   + Loader.cpp: dlopen (frequently) hangs
   //-------------------------------------------------------------------------
   std::string s= "L/libcom.a";
   FILE* f= fopen(s.c_str(), "rb");
   if( f != NULL )                  // If file exists
   {
     fprintf(stderr, "Warning: File(%s) exists\n", s.c_str());
     fprintf(stderr, ".. Library object Debug.o must be located in a DLL"
                       " to prevent reloading\n"
                     ".. a separate copy of it each time we load a DLL.\n"
                     "!! YOU HAVE BEEN WARNED !!\n");
     fclose(f);
   }

   //-------------------------------------------------------------------------
   // Create the ThreadLogger object
   //-------------------------------------------------------------------------
   Debug::set(new ThreadLogger(logName)); // Start logging
   logf("================================================================\n");
   logf("======== Starting %s\n", user_agent);
   logf("================================================================\n");

   //-------------------------------------------------------------------------
   // Create the Common area
   //-------------------------------------------------------------------------
   common= new Common();

   //-------------------------------------------------------------------------
   // Initialize database operation
   common->dbMeta= DbMeta::get();

   //-------------------------------------------------------------------------
   // Initialize threads
   //-------------------------------------------------------------------------
   common->httpServer.start();
   common->httpClient.start();

   //-------------------------------------------------------------------------
   // Initialize plugins
   common->httpServerMap= new HttpServerPluginMap("HttpServer.xml");

   //-------------------------------------------------------------------------
   // Initialize services
   common->netClient= new NetClient(user_agent);
   common->background= new Background(); // (Automatic termination)

   //-------------------------------------------------------------------------
   // Allow time for activation to complete
   Thread::sleep(3.0);

   return common;
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::finalize
//
// Purpose-
//       Complete Common termination.
//
//----------------------------------------------------------------------------
void
   Common::finalize( void )         // Complete Common termination
{
   logf("Common(%p)::finalize() fsm(%d)...\n", this, fsm);

   //-------------------------------------------------------------------------
   // Wait for threads to complete
   httpServer.wait();
   httpClient.wait();

   //-------------------------------------------------------------------------
   // Insure garbage collection cleanup completion
   {{{{
     Ref<Object> r(new Object());
   }}}}

   logf("...Common(%p)::finalize()\n", this);

   //-------------------------------------------------------------------------
   // Delete this object
   delete this;

   //-------------------------------------------------------------------------
   // Delete the ThreadLogger object
   delete(Debug::get());
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
{
   logf("Common(%p)::shutdown() fsm(%d)\n", this, fsm);

   //-------------------------------------------------------------------------
   // Go into shutdown state
   fsm= FSM_CLOSE;

   //-------------------------------------------------------------------------
   // Terminate services
   netClient->shutdown();

   //-------------------------------------------------------------------------
   // Terminate threads
   httpClient.notify(0);
   httpServer.notify(0);
}

