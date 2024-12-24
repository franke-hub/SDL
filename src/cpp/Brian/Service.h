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
//       Service.h
//
// Purpose-
//       The Service interface
//
// Last change date-
//       2024/11/02
//
// Implementation notes-
//       While Service.h documents its interfaces, that's not enough for a
//       novice user to understand what a Service is or how it might be used.
//       That information (and more) can be found in "./Service.md",
//
//----------------------------------------------------------------------------
#ifndef SERVICE_H_INCLUDED
#define SERVICE_H_INCLUDED

#include <string>                   // For std::string

#include <pub/Named.h>              // For pub::Named, base class

//----------------------------------------------------------------------------
//
// Class-
//       Service
//
// Purpose-
//       All Services are Named. Service names must be unique.
//
//----------------------------------------------------------------------------
class Service : public pub::Named { // Service
friend class Common;
//----------------------------------------------------------------------------
// Service::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef std::map<std::string, Service*>       Map_t; // The Map type
typedef Map_t::iterator                       MapIter_t; // The Map iterator

//----------------------------------------------------------------------------
// Service::(Base class) attributes (See Service.md)
//----------------------------------------------------------------------------
struct has_start {
virtual void
   start(Service*) = 0;
}; // struct has_start

struct has_stop {
virtual void
   stop(Service*) = 0;
}; // struct has_stop

struct has_wait {
virtual void
   wait(Service*) = 0;
}; // struct has_wait

//----------------------------------------------------------------------------
// Service::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Service(                         // Constructor
     const char*       name= nullptr); // The service name

   Service(const Service&) = delete; // Disallowed copy constructor
   Service& operator=(const Service&) = delete; // Disallowed assignment operator

virtual
   ~Service( void );                // Destructor

//----------------------------------------------------------------------------
// Service::Accessors
//----------------------------------------------------------------------------
static Map_t*                       // The Service Map*
   get_map( void );                 // Get the Service Map

//----------------------------------------------------------------------------
// Service::Static methods
//----------------------------------------------------------------------------
static Service*                     // The Service now mapped to the same name
   insert(Service*);                // Insert Service into the map

static Service*                     // The Service mapped to the name (if any)
   locate(std::string);             // Locate the associated Service

Service*                            // The removed or current Service
   remove(Service*);                // Get associated Service

//----------------------------------------------------------------------------
// Service::Methods
//----------------------------------------------------------------------------
// Note: these methods just invoke the associated static method.
//       TODO: DETERMINE USABILITY
// Service*                            // The Service now mapped to the same name
//    insert( void )                   // Insert this Service into the map
// {  return insert(this); }
//
// Service*                            // The Service mapped to the name (if any)
//    locate( void )                   // Locate this Service (by name)
// {  return locate get_name(); )
//
// Service*                            // (This Service, if removed)
//    remove( void )                   // Remove this Service from the map
// {  return remove(this); }

//----------------------------------------------------------------------------
// Service::Service Manager methods.
//----------------------------------------------------------------------------
protected:
static void
   start_all( void );               // Start all Services

static void
   stop_all( void );                // Stop all Services

static void
   wait_all( void );                // Wait for all Services

// When used, the start, stop, and/or wait methods are usually protected and
// unused except by "Service Manager" methods. (See Service.md)
}; // class Service
#endif // SERVICE_H_INCLUDED
