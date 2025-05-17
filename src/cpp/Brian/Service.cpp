//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2024 Frank Eskesen.
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
//       Service.cpp
//
// Purpose-
//       Service object methods
//
// Last change date-
//       2024/12/20
//
//----------------------------------------------------------------------------
#include <forward_list>             // For std::forward_list
#include <map>                      // For std::map
#include <mutex>                    // For std::mutex
#include <stdexcept>                // For std::out_of_range

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For pub::dispatch objects
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Latch.h>              // For pub::Latch
#include <pub/utility.h>            // For pub::utility::to_string

#include "Service.h"                // For Service, implemented

#define PUB _LIBPUB_NAMESPACE
using PUB::Debug;                   // For pub::Debug::get()
using PUB::Exception;               // For pub::Exception
using PUB::utility::to_string;      // For pub::utility::to_string
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
typedef std::map<std::string, Service*>       Map_t; // The Map type
typedef Map_t::iterator                       MapIter_t; // The Map iterator

static Map_t*          _map= nullptr; // The actual Service map
static pub::Latch      mutex;      // Map synchronization Latch

//----------------------------------------------------------------------------
// Global destructor
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
static int             global_destructor_invoked= false;
static struct GlobalDestructor {    // Termination cleanup
inline
   ~GlobalDestructor( void )
{  if( HCDM ) debugh("Service::GlobalDestructor~\n");

   delete _map;
   _map= nullptr;

   global_destructor_invoked= true;
}
}  globalDestructor;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Method-
//       Service::has_start::start
//
// Purpose-
//       Default has_start::start   (Implements pure virtual method)
//
//----------------------------------------------------------------------------
void
   Service::has_start::start(       // Start
     Service*          S)           // This Service
{  if( HCDM )
     debugh("Service(%s)::has_start::start\n", S->get_name().c_str());
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::has_stop::stop
//
// Purpose-
//       Default has_start::stop    (Implements pure virtual method)
//
//----------------------------------------------------------------------------
void
   Service::has_stop::stop(         // Stop
     Service*          S)           // This Service
{  if( HCDM ) debugh("Service(%s)::has_stop::stop\n", S->get_name().c_str()); }

//----------------------------------------------------------------------------
//
// Method-
//       Service::has_wait::wait
//
// Purpose-
//       Default has_wait::wait     (Implements pure virtual method)
//
//----------------------------------------------------------------------------
void
   Service::has_wait::wait(         // Wait for
     Service*          S)           // This Service to complete
{  if( HCDM ) debugh("Service(%s)::has_wait::wait\n", S->get_name().c_str()); }

//----------------------------------------------------------------------------
//
// Method-
//       Service::Service
//
// Purpose-
//       Constructor
//
// Implementation notes-
//       NULL names (which default to "") are not even inserted into the map.
//
//----------------------------------------------------------------------------
   Service::Service(                // Constructor
     const char*       name)        // The Service name
{  if( name ) { this->name= name; insert(this); } }

//----------------------------------------------------------------------------
//
// Method-
//       Service::~Service
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Service::~Service( void )        // Destructor
{  remove(this); }

//----------------------------------------------------------------------------
//
// Method-
//       Service::get_map
//
// Purpose-
//       Return the Map_t*
//
// Implementation notes-
//       The map is required during static initialization, and can be
//       erroneously requested during static destruction.
//
//----------------------------------------------------------------------------
Map_t*                              // The Map_t*
   Service::get_map( void )         // Get Map_t*, possibly creating it
{
   if( global_destructor_invoked )  // (Should not occur)
     return nullptr;

   if( _map == nullptr )
     _map= new Map_t();

   return _map;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Service::insert
//       Service::locate
//       Service::remove
//
// Purpose-
//       Insert|locate|remove operations
//
//----------------------------------------------------------------------------
Service*                            // The inserted or current Service
   Service::insert(                 // Insert
     Service*          service)     // This Service
{  if( HCDM ) debugh("Service::insert(%s)\n", service->get_name().c_str());

   Map_t* map= get_map();           // Get/create the map
   if( map == nullptr )
     return nullptr;

   std::string name= service->get_name();
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map->find(name);
     if( mi != map->end() ) {       // If it's already mapped
       if( true )
         return mi->second;
       else
         throw std::out_of_range(to_string("Service::insert(%s) is a duplicate"
                                          , name.c_str()));
     }

     // For exposition: Both versions operate correctly
     if( false )
       (*map)[name]= service;
     else
       map->insert({name, service});
   }}}}

   return service;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Service*                            // The associated Service, if present
   Service::locate(                 // Get the Service associated
     std::string       name)        // With this name
{
   Map_t* map= get_map();           // Get/create the map
   if( map == nullptr )
     return nullptr;

   Service* service= nullptr;       // The associated Service (default= none)
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map->find(name);
     if( mi != map->end() )          // If it's mapped
       service= mi->second;
   }}}}

   return service;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Service*                            // The removed Service
   Service::remove(                 // Remove
     Service*          service)     // This Service
{  if( HCDM ) debugh("Service::remove(%s)\n", service->get_name().c_str());

   Map_t* map= get_map();           // Get/create the map
   if( map == nullptr )
     return nullptr;

   std::string name= service->get_name();
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map->find(name);
     if( mi != map->end() ) {
       if( mi->second == service )
         map->erase(mi);
       else
         service= mi->second;
     }
   }}}}

   return service;
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::start_all
//
// Purpose-
//       Start for all Services
//
//----------------------------------------------------------------------------
void
   Service::start_all( void )        // Start all Services
{  if( HCDM ) traceh("Service::start_all...\n");

   Map_t* map= get_map();           // Get/create the map
   if( map == nullptr )
     return;

   std::forward_list<Service*> list; // Service list

   // Copy the Service* from the map into the list
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);
     for(auto& it: *map) {
       list.push_front(it.second);
     }
   }}}}

   // Start all Services
   for(auto& it: list) {
     Service* service= it;
     has_start* method= dynamic_cast<has_start*>(service);
     if( method ) method->start(service);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::stop_all
//
// Purpose-
//       Stop all Services
//
//----------------------------------------------------------------------------
void
   Service::stop_all( void )        // Stop for all Services
{  if( HCDM ) debugh("Service::stop_all...\n");

   Map_t* map= get_map();           // Get/create the map
   if( map == nullptr )
     return;

   std::forward_list<Service*> list; // Service list

   // Copy the Service* from the map into the list
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);
     for(auto& it: *map) {
       list.push_front(it.second);
     }
   }}}}

   // Stop all Services
   for(auto& it: list) {
     Service* service= it;
     has_stop* method= dynamic_cast<has_stop*>(service);
     if( method ) {
       method->stop(service);
     }
   }

   if( HCDM ) debugh("...Service::stop_all (completed)\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::wait_all
//
// Purpose-
//       Wait for all Services
//
//----------------------------------------------------------------------------
void
   Service::wait_all( void )        // Wait for all Services
{  if( HCDM ) debugh("Service::wait_all...\n");

   Map_t* map= get_map();           // Get/create the map
   if( map == nullptr )
     return;

   std::forward_list<Service*> list; // Service list

   // Copy the Service* from the map into the list
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);
     for(auto& it: *map) {
       list.push_front(it.second);
     }
   }}}}

   // Wait for all Services to complete
   for(auto& it: list) {
     Service* service= it;
     has_wait* method= dynamic_cast<has_wait*>(service);
     if( method ) {
       method->wait(service);
     }
   }

   if( HCDM ) debugh("...Service::wait_all (completed)\n");
}
