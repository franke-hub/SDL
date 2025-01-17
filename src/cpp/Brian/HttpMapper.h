//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2025 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
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
//       2025/01/16
//
//----------------------------------------------------------------------------
#ifndef HTTPMAPPER_H_INCLUDED
#define HTTPMAPPER_H_INCLUDED

#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr, std::weak_ptr, ...
#include <mutex>                    // For std::recursive_mutex
#include <string>                   // For std::string

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
//----------------------------------------------------------------------------
static HttpMapper*                  // The HttpMapper singleton
   get( void )                      // Get HttpMapper singleton
{  return singleton ? singleton : make(); }

static void*                        // Get HttpMapper terminator
   get_terminator( void );          // Get HttpMapper terminator

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
     HttpListen*       listen);     // This Listener

std::shared_ptr<HttpListen>
   locate(                          // Locate with Map
     std::string       host) const; // This host address:port

void
   remove(                          // Remove from Map
     HttpListen*       listen);     // This Listener

void
   status( void ) const;            // Display Listener Map
}; // class HttpMapper
#endif // HTTPMAPPER_H_INCLUDED
