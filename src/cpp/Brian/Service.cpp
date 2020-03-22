//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2020 Frank Eskesen.
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
//       2020/01/25
//
//----------------------------------------------------------------------------
#include <map>
#include <mutex>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pub/Debug.h>              // For debugging
#include <pub/utility.h>            // For to_string

#include "Service.h"

using pub::Exception;
using pub::utility::to_string;
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <pub/ifmacro.h>

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
ServiceMap::Map_t       ServiceMap::map; // The actual ServiceMap
class ServiceMap        ServiceMap; // The map accessor object

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::mutex       mutex;      // Synchronization control

//----------------------------------------------------------------------------
//
// Method-
//       ServiceMap::locate()
//       ServiceMap::remove()
//       ServiceMap::operator[]
//
// Purpose-
//       Locate|remove|insert operations
//
//----------------------------------------------------------------------------
Service*                            // The associated Service, if present
   ServiceMap::locate(              // Get associated Service
     std::string       name) const  // With this name
{
   Service* service= nullptr;       // The associated Service

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map.find(name);
     if( mi != map.end() )          // If it's mapped
       service= mi->second;
   }}}}

   return service;
}

void
   ServiceMap::remove(              // Remove
     Service*          service)     // This Service
{
   std::string name= service->get_name();

   std::lock_guard<decltype(mutex)> lock(mutex);

   const MapIter_t mi= map.find(name);
   if( mi != map.end() && mi->second == service ) {
     service->stop();
     service->wait();
     map.erase(mi);
   }
}

Service&                            // The associated Service
   ServiceMap::operator[](          // Locate a Service
     std::string       name) const  // With this name
{
   Service* service= locate(name);  // Get associated Service

   if( service == nullptr )         // If the service isn't mapped
       throw Exception(to_string("ServiceMap::[%s] not found",
                       name.c_str()));

   return *service;
}

Service&                            // The associated Service
   ServiceMap::operator[](          // Insert
     Service*          service)     // This Service
{
   std::string name= service->get_name();

   std::lock_guard<decltype(mutex)> lock(mutex);

   const MapIter_t mi= map.find(name);
   if( mi != map.end() )            // If it's already mapped
     throw Exception(to_string("ServiceMap::insert(%s) duplicated",
                     name.c_str()));

   map[name]= service;
   service->start();
   return *service;
}

//----------------------------------------------------------------------------
//
// Method-
//       Service::start
//
// Purpose-
//       Start the Service
//
//----------------------------------------------------------------------------
void
   Service::start( void )           // Start the Service
{  traceh("%s(%s)::start()\n", get_class_name().c_str(), get_name().c_str()); }

//----------------------------------------------------------------------------
//
// Method-
//       Service::stop
//
// Purpose-
//       Stop the Service
//
//----------------------------------------------------------------------------
void
   Service::stop( void )            // Stop the Service
{  traceh("%s(%s)::stop()\n", get_class_name().c_str(), get_name().c_str()); }

//----------------------------------------------------------------------------
//
// Method-
//       Service::wait
//
// Purpose-
//       Wait for stop completion
//
//----------------------------------------------------------------------------
void
   Service::wait( void )            // Wait for stop completion
{  reset(); }                       // Reset the Service

//----------------------------------------------------------------------------
//
// Method-
//       Service::work
//
// Purpose-
//       Process one work Item
//
//----------------------------------------------------------------------------
void
   Service::work(                   // Process
     pub::Dispatch::Item*
                       item)        // This work Item
{  traceh("Service(%s)::work(%p) default\n", get_name().c_str(), item);
   item->post(127);
}
