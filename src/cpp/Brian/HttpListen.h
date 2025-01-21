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
//       HttpListen.h
//
// Purpose-
//       The HTTP Listen object.
//
// Last change date-
//       2025/01/20
//
//----------------------------------------------------------------------------
#ifndef HTTPLISTEN_H_INCLUDED
#define HTTPLISTEN_H_INCLUDED

#include <memory>                   // For std::shared_ptr, std::weak_ptr, ...
#include <mutex>                    // For std::recursive_mutex
#include <string>                   // For std::string

#include "shared_ptr-debug.h"       // For shared_ptr debugging control
#include <pub/Dispatch.h>           // For pub::dispatch objects
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread, base class

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class HttpServer;                   // HttpServer

//----------------------------------------------------------------------------
//
// Class-
//       HttpListen
//
// Purpose-
//       The HttpListen Thread
//
//----------------------------------------------------------------------------
class HttpListen : public pub::Thread { // The HttpListen Thread
//----------------------------------------------------------------------------
// HttpListen::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::shared_ptr<HttpListen>           Listen_t;
typedef   std::weak_ptr<HttpListen>           Listen_wt;
typedef std::shared_ptr<HttpServer>           Server_t;
typedef std::map<std::string, Server_t>       Map_t;
typedef Map_t::const_iterator                 Map_const_iterator_t;
typedef Map_t::iterator                       Map_iterator_t;

typedef pub::Socket                 Socket_t;
typedef pub::dispatch::Task         Task_t;
typedef pub::dispatch::Work_i       Work_i;

//----------------------------------------------------------------------------
// HttpListen::Attributes
//----------------------------------------------------------------------------
protected:
Listen_wt              self;        // (Weak) self-reference

std::string            host;        // The host name:port string
Map_t                  map;         // The HttpServer map
mutable std::recursive_mutex
                       mutex;       // Mutex for synchronization control
Socket_t*              socket;      // The Listener Socket

bool                   operational= false; // TRUE while operational

//----------------------------------------------------------------------------
// HttpListen::Constructors, destructor, creator
//----------------------------------------------------------------------------
protected:
   HttpListen( void );              // (Default) constructor
   HttpListen(std::string);         // Constructor, "hostname:port"

void
   _build( void );                  // (Common construction)

public:
virtual
   ~HttpListen( void );             // Destructor

static Listen_t                     // The Listener
    make( void );                   // Make Listener, default host:port

static Listen_t                     // The Listener
    make(std::string);              // Make Listener, specific host:port

//----------------------------------------------------------------------------
// HttpListen::debug || Write debugging message
//----------------------------------------------------------------------------
void
   debug(const char* info= "") const; // Display debugging message

//----------------------------------------------------------------------------
// HttpListen::get_self || Get std::shared_ptr<Listen>
//----------------------------------------------------------------------------
Listen_t                            // The shared_ptr<Listen>
   get_self( void ) const           // Get shared_ptr<Listen>
{  return self.lock(); }

//----------------------------------------------------------------------------
// HttpListen::get_host || Get this Listener's name:port string
//----------------------------------------------------------------------------
std::string                         // The host name:port string
   get_host( void ) const           // Get host name:port string
{  return host; }

//----------------------------------------------------------------------------
// HttpListen::is_operational || Expose operational state
//----------------------------------------------------------------------------
bool                                // The operational state
   is_operational( void ) const     // Get operational state
{  return operational; }

//----------------------------------------------------------------------------
// HttpListen::close || Close and delete the Listener Socket
//----------------------------------------------------------------------------
void
   close( void );                   // Close and delete the Socket

//----------------------------------------------------------------------------
//
// Method-
//       HttpListen::insert
//       HttpListen::locate
//       HttpListen::remove
//       HttpListen::status
//
// Purpose-
//       Insert Server onto Map
//       Locate Server with Map
//       Remove Server from Map
//       Display status
//
//----------------------------------------------------------------------------
void
   insert(                          // Insert onto Map
     Server_t          server);     // This HttpServer

Server_t
   locate(                          // Locate with Map
     std::string       peer) const; // This peer address:port

void
   remove(                          // Remove from Map
     Server_t          server);     // This HttpServer

void
   status( void ) const;            // Display status

//----------------------------------------------------------------------------
// HttpListen::run || Accept new connections, creating Servers
//----------------------------------------------------------------------------
virtual void
   run( void );                     // Run the HttpListen Thread

//----------------------------------------------------------------------------
// HttpListen::start || Start the Listener; accept new connections
//----------------------------------------------------------------------------
// (Uses Thread::start)
//virtual void
//   start( void );                 // Start the HttpListen

//----------------------------------------------------------------------------
// HttpListen::stop || Stop the Listener; don't accept new connections
//----------------------------------------------------------------------------
virtual void
   stop( void );                    // Stop the HttpListen

//----------------------------------------------------------------------------
// HttpListen::wait || Wait for Listener completion
//----------------------------------------------------------------------------
virtual void
   wait( void );                    // Wait for Listen completion
}; // class HttpListen
#endif // HTTPLISTEN_H_INCLUDED
