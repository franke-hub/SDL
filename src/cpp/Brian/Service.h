//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Service.h
//
// Purpose-
//       A service is a Named and mapped DispatchTask
//
// Last change date-
//       2021/07/09
//
//----------------------------------------------------------------------------
#ifndef SERVICE_H_INCLUDED
#define SERVICE_H_INCLUDED

#include <map>
#include <string>

#include <pub/Debug.h>
#include <pub/Dispatch.h>
#include <pub/Named.h>

//----------------------------------------------------------------------------
//
// Class-
//       Service
//
// Purpose-
//       A Service is a Named dispatch::Task Object
//
//----------------------------------------------------------------------------
class Service : public pub::dispatch::Task, public pub::NamedObject { // Service
//----------------------------------------------------------------------------
// Service::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef pub::dispatch::Task Task;
typedef pub::dispatch::Item Item;
typedef pub::dispatch::Wait Wait;

//----------------------------------------------------------------------------
// Service::Attributes
//----------------------------------------------------------------------------
protected:
// None defined

//----------------------------------------------------------------------------
// Service::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Service( void ) {}              // Destructor
   Service(                         // Constructor
     const char*       name)        // The service name
:  Task(), pub::NamedObject(name) {}

   Service(const Service&) = delete; // Disallowed copy constructor
   Service& operator=(const Service&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// Service::Accessors
//----------------------------------------------------------------------------
public:
// std::string get_name() const     // Return the Named attribute

//----------------------------------------------------------------------------
// Service::Methods
//----------------------------------------------------------------------------
public:
virtual void
   start( void );                   // Start the Service

virtual void
   stop( void );                    // Stop the Service

virtual void
   wait( void );                    // Wait for stop completion

virtual void                        // (OVERRIDE this method)
   work(                            // Process
     Item*             item);       // This work Item
}; // class Service

//----------------------------------------------------------------------------
//
// Class-
//       ServiceMap
//
// Purpose-
//       The name to Service map
//
//----------------------------------------------------------------------------
extern class ServiceMap {           // The ServiceMap
//----------------------------------------------------------------------------
// ServiceMap::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef std::map<std::string, Service*>
                       Map_t;       // The Map_t
typedef Map_t::iterator
                       MapIter_t;

//----------------------------------------------------------------------------
// ServiceMap::Attributes
//----------------------------------------------------------------------------
protected:
static Map_t           map;         // The actual map

public:                             // Attribute accessors
MapIter_t begin() noexcept
{  return map.begin(); }

const MapIter_t begin() const noexcept
{  return map.begin(); }

MapIter_t end() noexcept
{  return map.end(); }

const MapIter_t end() const noexcept
{  return map.end(); }

//----------------------------------------------------------------------------
// ServiceMap::operators
//----------------------------------------------------------------------------
public:
Service*                            // The associated Service, if present
   locate(std::string name) const;  // Get associated Service

void
   remove(Service* command);        // Remove associated Service

Service&                            // The associated Service
   operator[](std::string name) const; // Locate Service, name must be registered

Service&                            // The associated Service
   operator[](Service* service);    // Insert Service, name must be unique
}  ServiceMap;

#endif // SERVICE_H_INCLUDED
