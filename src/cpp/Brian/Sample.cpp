//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Sample.cpp
//
// Purpose-
//       Include a command and a service.
//
// Last change date-
//       2024/10/04
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For namespace debugging

#include "Command.h"                // For Command (base class)
#include "Service.h"                // For Service (base class)

#define PUB _LIBPUB_NAMESPACE
using PUB::Debug;
using namespace PUB::debugging;

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
   SampleService( void )           // Constructor
:  Service("sample") {}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Methods
virtual void
   start( void )                    // Start the Service
{  if( HCDM ) debugf("SampleService::start\n"); }

virtual void
   stop( void )                     // Stop the Service
{  if( HCDM ) debugf("SampleService::stop\n"); }

virtual void
   wait( void )                     // Wait for stop completion
{  if( HCDM ) debugf("SampleService::wait\n"); }

virtual void
   work(Item*)                      // Handle work, ignoring parameter
{
   debugf("Service list:\n");

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
   work(int, char**)                // Process the Sample Command
//   int               argc,        // Argument count (UNUSED)
//   char*             argv[])      // Argument array (UNUSED)
{  if( HCDM ) debugf("SampleCommand::work\n\n");

   Service* service= Service::locate("sample");
   SampleService* sample= dynamic_cast<SampleService*>(service);
   if( sample )
     sample->work(nullptr);
   else
     debugf("Couldn't locate SampleService \"sample\"\n");

   return nullptr;
}
} sampleCommand; // class SampleCommand
