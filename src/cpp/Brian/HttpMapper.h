//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpMapper.h
//
// Purpose-
//       The HttpListen agent
//
// Last change date-
//       2025/01/20
//
//----------------------------------------------------------------------------
#ifndef HTTPMAPPER_H_INCLUDED
#define HTTPMAPPER_H_INCLUDED

#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr, std::weak_ptr, ...
#include <mutex>                    // For std::recursive_mutex
#include <string>                   // For std::string

#include "shared_ptr-debug.h"       // For shared_ptr debugging control
#include "HttpListen.h"             // For HttpListen
#include "HttpServer.h"             // For HttpServer

//----------------------------------------------------------------------------
//
// Class-
//       HttpMapper
//
// Purpose-
//       The HttpListen Agent
//
//----------------------------------------------------------------------------
class HttpMapper {                  // The HttpListen agent
//----------------------------------------------------------------------------
// HttpMapper::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::shared_ptr<HttpListen>           Listen_t;
typedef std::map<std::string, Listen_t>       Map_t;
typedef Map_t::iterator                       MapIter_t;
typedef std::recursive_mutex                  Mutex_t;

//----------------------------------------------------------------------------
// HttpMapper::Attributes
//----------------------------------------------------------------------------
protected:
mutable Mutex_t        mutex;       // Mutex, protects Listener Map
Map_t                  map;         // The Listener Map

static HttpMapper*     singleton;   // The HttpMapper singleton

//----------------------------------------------------------------------------
// HttpMapper::Creator, constructor, destructor
//----------------------------------------------------------------------------
protected:
static HttpMapper*                  // The (singleton) HttpMapper
   make( void );                    // Creator

   HttpMapper( void );              // (Default) constructor

public:
   ~HttpMapper( void );             // Destructor

//----------------------------------------------------------------------------
// HttpMapper::debug || Write debugging message
//----------------------------------------------------------------------------
void
   debug(const char* info= "") const; // Display debugging message

//----------------------------------------------------------------------------
// HttpMapper::get || Get the HttpMapper singleton
// HttpMapper::shutdown || Delete the HttpMapper singleton
//----------------------------------------------------------------------------
static HttpMapper*                  // The HttpMapper singleton
   get( void )                      // Get HttpMapper singleton
{  return singleton ? singleton : make(); }

static void
   shutdown( void );                // Delete the HttpMapper singleton

//----------------------------------------------------------------------------
//
// Method-
//       HttpMapper::insert
//       HttpMapper::locate
//       HttpMapper::remove
//       HttpMapper::status
//
// Purpose-
//       Insert Listener onto Map
//       Locate Listener with Map
//       Remove Listener from Map
//       Display Listener Map
//
//----------------------------------------------------------------------------
void
   insert(                          // Insert onto Map
     Listen_t          listen);     // This Listener

std::shared_ptr<HttpListen>
   locate(                          // Locate with Map
     std::string       host) const; // This host address:port

void
   remove(                          // Remove from Map
     Listen_t          listen);     // This Listener

void
   status( void ) const;            // Display Listener Map

//----------------------------------------------------------------------------
//
// Method-
//       HttpMapper::stop(void)
//       HttpMapper::stop(const char*)
//
// Purpose-
//       Stop all Listeners
//       Stop one Listener
//
//----------------------------------------------------------------------------
void
   stop( void );                    // Stop *ALL* Listeners

void
   stop(                            // Stop the Listener
     const char*       _url);       // At this URL

//----------------------------------------------------------------------------
//
// Method-
//       HttpMapper::wait(void)
//       HttpMapper::wait(const char*)
//
// Purpose-
//       Wait for all Listeners
//       Wait for one Listener
//
//----------------------------------------------------------------------------
void
   wait( void );                    // Wait for *ALL* Listeners

void
   wait(                            // Wait for the Listener
     const char*       _url);       // At this URL
}; // class HttpMapper
#endif // HTTPMAPPER_H_INCLUDED
