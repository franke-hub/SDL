//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
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
//       Sample.cpp
//
// Purpose-
//       Include a command and a service.
//
// Last change date-
//       2024/12/20
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For namespace debugging
#include <pub/Dispatch.h>           // For namespace pub::dispatch

#include "Command.h"                // For Command (base class)
#include "Service.h"                // For Service (base class)

#define PUB _LIBPUB_NAMESPACE
using PUB::Debug;
using namespace PUB::debugging;
using namespace PUB::dispatch;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
// class SampleService
//----------------------------------------------------------------------------
class SampleService
   : public Service
   , public Service::has_start
   , public Service::has_stop
   , public Service::has_wait {
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Constructors
public:
   SampleService( void )            // Constructor
:  Service("sample") {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Methods
virtual void
   start(Service* S)                // Start the Service
{  if( HCDM ) debugh("SampleService::start\n");

   Service::has_start::start(S);
}

virtual void
   stop(Service* S)                 // Stop the Service
{  if( HCDM ) debugh("SampleService::stop\n");

   Service::has_stop::stop(S);
}

virtual void
   wait(Service* S)                 // Wait for stop completion

{  if( HCDM ) debugh("SampleService::wait\n");

   Service::has_wait::wait(S);
}

virtual void
   work(Item* item)                 // Handle work item
{
   debugh("Service list:\n");

   // List the Services
   Map_t* map= get_map();

   size_t column= 0;                // Output column
   for(MapIter_t mi= map->begin(); mi != map->end(); ++mi) {
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
   debugf("\n");

   if( item )
     item->post();
}
} sampleService; // class SampleService

//----------------------------------------------------------------------------
// class SampleCommand
//----------------------------------------------------------------------------
class SampleCommand : public Command {
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Constructors
public:
   SampleCommand( void )           // Constructor
:  Command("sample") {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Methods
virtual Command::resultant          // Resultant
   main(int, char**)                // Process the Sample Command
//   int               argc,        // Argument count (UNUSED)
//   char*             argv[])      // Argument array (UNUSED)
{  if( HCDM ) debugh("SampleCommand::main\n\n");

   Service* service= Service::locate("sample");
   SampleService* sample= dynamic_cast<SampleService*>(service);
   if( sample )
     sample->work(nullptr);
   else
     debugh("Couldn't locate SampleService \"sample\"\n");

   return nullptr;
}
} sampleCommand; // class SampleCommand
