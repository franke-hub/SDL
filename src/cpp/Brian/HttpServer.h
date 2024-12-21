//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpServer.h
//
// Purpose-
//       HTTP Server object.
//
// Last change date-
//       2024/12/20
//
//----------------------------------------------------------------------------
#ifndef _HTTPSERVER_H_INCLUDED
#define _HTTPSERVER_H_INCLUDED

#include <mutex>                    // For std::recursive_mutex
#include <string>                   // For std::string

#include "pub/Dispatch.h"           // For pub::dispatch::Item, ...
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread

//----------------------------------------------------------------------------
//
// Class-
//       Listen
//
// Purpose-
//       The Listen Thread
//
//----------------------------------------------------------------------------
class Listen : public pub::Thread { // The Listen Thread
//----------------------------------------------------------------------------
// Listen::attributes
//----------------------------------------------------------------------------
protected:
std::recursive_mutex   mutex;       // Mutex, protects termination sequence
std::string            url;         // Our hostname:port URL
pub::Socket*           socket= nullptr; // The Listener Socket
int                    port= 0;     // The connection port number
bool                   operational= false; // TRUE while operational

//----------------------------------------------------------------------------
// Listen::Constructors, destructor
//----------------------------------------------------------------------------
void
   build( void );                   // (Common construction)

public:
   Listen( void );                  // (Default) constructor
   Listen(std::string);             // Constructor, "hostname:port"

virtual
   ~Listen( void );                 // Destructor

//----------------------------------------------------------------------------
// Listen::get_url || Get this Listener's URL
//----------------------------------------------------------------------------
std::string                         // The URL
   get_url( void ) const            // Get this Listener's URL
{  return url; }

//----------------------------------------------------------------------------
// Listen::is_operational || Expose operational state
//----------------------------------------------------------------------------
bool                                // The operational state
   is_operational( void ) const     // Get operational state
{  return operational; }

//----------------------------------------------------------------------------
// Listen::close || Close and delete the Listener Socket
//----------------------------------------------------------------------------
void
   close( void );                   // Close and delete the Socket

//----------------------------------------------------------------------------
// Listen::run || Accept new connections, creating Servers
//----------------------------------------------------------------------------
virtual void
   run( void );                     // Run the Listen Thread

//----------------------------------------------------------------------------
// Listen::start || Start the Listener; accept new connections
//----------------------------------------------------------------------------
// (Uses Thread::start)
//virtual void
//   start( void );                 // Start the Listener

//----------------------------------------------------------------------------
// Listen::stop || Stop the Listener; don't accept new connections
//----------------------------------------------------------------------------
virtual void
   stop( void );                    // Stop the Listener

//----------------------------------------------------------------------------
// Listen::wait || Wait for Listener completion
//----------------------------------------------------------------------------
virtual void
   wait( void );                    // Wait for Server completion
}; // class Listen

//----------------------------------------------------------------------------
//
// Class-
//       Server
//
// Purpose-
//       Define the Server class.
//
// Implementation note-
//       Server objects delete themself when complete.
//
//----------------------------------------------------------------------------
class Server : pub::dispatch::Item, public pub::Thread { // Server class
//----------------------------------------------------------------------------
// Server::Attributes
//----------------------------------------------------------------------------
protected:
std::recursive_mutex   mutex;       // Mutex, protects termination sequence
pub::Socket*           socket= nullptr; // The connection Socket
bool                   operational= false; // TRUE while operational

//----------------------------------------------------------------------------
// Server::Constructor/destructor
//----------------------------------------------------------------------------
public:
   Server(pub::Socket*);            // Constructor

virtual
   ~Server( void );                 // Destructor

//----------------------------------------------------------------------------
// Server::is_operational || Expose operational state
//----------------------------------------------------------------------------
bool                                // The operational state
   is_operational( void ) const     // Get operational state
{  return operational; }

//----------------------------------------------------------------------------
// Server::Methods
//----------------------------------------------------------------------------
void
   close( void );                   // Close and delete the Socket

void
   request(                         // Handle request
     std::string       text);       // The request text

void
   response(                        // Send a (final) response
     int               status,      // The status code
     std::string       html);       // The response HTML

virtual void
   run( void );                     // Operate the Server

virtual void
   stop( void );                    // Stop the Server

protected:
void
   write(                           // Write response text
     std::string       text);       // The response text
}; // class Server
#endif // _HTTPSERVER_H_INCLUDED
