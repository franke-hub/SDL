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
//       A service is a Named and mapped DispatchTask
//
// Last change date-
//       2024/10/04
//
//----------------------------------------------------------------------------
#ifndef SERVICE_H_INCLUDED
#define SERVICE_H_INCLUDED

#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr, std::weak_ptr
#include <string>                   // For std::string

#include <pub/Dispatch.h>           // For pub::dispatch::Task, base class
#include <pub/Named.h>              // For pub::Named, base class

//----------------------------------------------------------------------------
//
// Class-
//       Service
//
// Purpose-
//       A Service is a NamedObject
//
//----------------------------------------------------------------------------
class Service : public pub::NamedObject { // Service
//----------------------------------------------------------------------------
// Service::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef std::map<std::string, Service*>       Map_t; // The Map type
typedef Map_t::iterator                       MapIter_t; // The Map iterator

// Import pub::dispatch::classes
typedef pub::dispatch::Task                   Task; // (Base class)
typedef pub::dispatch::Item                   Item; // (Method work parameter)

//----------------------------------------------------------------------------
// Service::Attributes
//----------------------------------------------------------------------------
protected:
// No attributes defined

//----------------------------------------------------------------------------
// Service::Constructors/destructor
//----------------------------------------------------------------------------
public:
   Service(                         // Constructor
     const char*       name);       // The service name

   Service(const Service&) = delete; // Disallowed copy constructor
   Service& operator=(const Service&) = delete; // Disallowed assignment operator

virtual
   ~Service( void );                // Destructor

//----------------------------------------------------------------------------
// Service::Accessors
//----------------------------------------------------------------------------
static Map_t*                       // The Service Map*
   get_map( void );                 // Get the Service Map

static Service*                     // The associated Service, if present
   locate(std::string);             // Get associated Service

//----------------------------------------------------------------------------
// Service::Optional methods
//----------------------------------------------------------------------------
struct has_start {
virtual void
   start( void )
{  }
}; // struct has_start

struct has_stop {
virtual void
   stop( void )
{  }
}; // struct has_stop

struct has_wait {
virtual void
   wait( void )
{  }
}; // struct has_wait
}; // class Service
#endif // SERVICE_H_INCLUDED
