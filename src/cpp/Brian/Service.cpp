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
//       Service.cpp
//
// Purpose-
//       Service object methods
//
// Last change date-
//       2024/10/04
//
//----------------------------------------------------------------------------
#include <map>
#include <mutex>
#include <stdexcept>                // For std::out_of_range

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For pub::dispatch objects
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Latch.h>              // For pub::Latch
#include <pub/utility.h>            // For pub::utility::to_string

#include "Service.h"                // For Service, implemented

using pub::Exception;
using pub::utility::to_string;
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
static Service::Map_t* _map= nullptr; // The actual Service Map*
static pub::Latch      mutex;      // Map insert/remove synchronization Latch

//----------------------------------------------------------------------------
// Global destructor
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
static int             global_destructor_invoked= false;
static struct GlobalDestructor {    // Termination cleanup
inline
   ~GlobalDestructor( void )
{  if( HCDM ) debugf("Service::GlobalDestructor~\n");

   delete _map;
   _map= nullptr;

   global_destructor_invoked= true;
}
}  globalDestructor;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Subroutine-
//       insert
//       remove
//
// Purpose-
//       Locate|remove|iinsert operations
//
//----------------------------------------------------------------------------
static void
   insert(                          // Insert
     Service*          service)     // This Service
{  if( HCDM ) debugf("Service::insert(%s)\n", service->get_name().c_str());

   typedef Service::Map_t           Map_t;
   typedef Service::MapIter_t       MapIter_t;

   if( global_destructor_invoked )  // Do nothing if in unloading state
     return;

   std::string name= service->get_name();
   Map_t* map= Service::get_map();

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map->find(name);
     if( mi != map->end() )         // If it's already mapped
       throw std::out_of_range(to_string("Service::insert(%s) is a duplicate"
                                        , name.c_str()));

     (*map)[name]= service;
//   map->insert({name, service});
   }}}}
}

static void
   remove(                          // Remove
     Service*          service)     // This Service
{
   typedef Service::Map_t           Map_t;
   typedef Service::MapIter_t       MapIter_t;

   if( global_destructor_invoked )  // Do nothing if in unloading state
     return;

   std::string name= service->get_name();
   Map_t* map= Service::get_map();

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map->find(name);
     if( mi != map->end() && mi->second == service )
       map->erase(mi);
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::Service
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Service::Service(                // Constructor
     const char*       name)        // The Service name
:  pub::NamedObject(name)
{  insert(this); }

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
Service::Map_t*                     // The Map_t*
   Service::get_map( void )         // Get Map_t*
{
   if( global_destructor_invoked )  // (Should not occur)
     return nullptr;                // (SEGFAULT expected)

   if( _map == nullptr )
     _map= new Map_t();

   return _map;
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::locate
//
// Purpose-
//       Locate associated Service
//
//----------------------------------------------------------------------------
Service*                            // The associated Service, if present
   Service::locate(                 // Get the Service associated
     std::string       name)        // With this name
{
   Service* service= nullptr;       // The associated Service
   Map_t* map= get_map();

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map->find(name);
     if( mi != map->end() )          // If it's mapped
       service= mi->second;
   }}}}

   return service;
}
