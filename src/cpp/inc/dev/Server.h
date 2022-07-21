//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       http/Server.h
//
// Purpose-
//       HTTP Server object.
//
// Last change date-
//       2022/07/16
//
//----------------------------------------------------------------------------
#ifndef _PUB_HTTP_SERVER_H_INCLUDED
#define _PUB_HTTP_SERVER_H_INCLUDED

#include <cstdlib>                  // For size_t
#include <functional>               // For std::function
#include <mutex>                    // For std::mutex, super class
#include <string>                   // For std::string
#include <netinet/in.h>             // for struct sockaddr_in6

#include <pub/Named.h>              // For pub::Named, super class
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread, super class

#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Request.h"       // For pub::http::Request
#include "pub/http/Stream.h"        // For pub::http::Stream

namespace pub::http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Listen;                       // pub::http::Listen

//----------------------------------------------------------------------------
//
// Class-
//       Server
//
// Purpose-
//       Define the Server class.
//
//----------------------------------------------------------------------------
class Server : public Named, public Thread, public std::mutex { // Server class
//----------------------------------------------------------------------------
// Server::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef Socket::sockaddr_u sockaddr_u; // Using pub::Socket::sockaddr_u

//----------------------------------------------------------------------------
// Server::Attributes
//----------------------------------------------------------------------------
protected:
// Callback handlers ---------------------------------------------------------
std::function<void(void)>
                       h_close;     // The close event handler
std::function<void(const std::string&)>
                       h_error;     // The error event handler
// ---------------------------------------------------------------------------

std::weak_ptr<Server>  self;        // Self reference
std::weak_ptr<Listen>  listen;      // Our owning Listener

mutable Buffer         buffer;      // Our input/output buffer
Socket*                socket= nullptr; // The connection Socket
StreamSet              stream_set;  // Our set of Streams

bool                   operational= false; // TRUE while operational

//----------------------------------------------------------------------------
// Server::Static attributes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Server::Destructor, constructors, creators
//----------------------------------------------------------------------------
public:
   ~Server( void );                 // Destructor
   Server(Listen*, Socket*);        // Constructor

static std::shared_ptr<Server>
   make(Listen*, Socket*);          // Creator

//----------------------------------------------------------------------------
// Server::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }

//----------------------------------------------------------------------------
// Server::Accessor methods
//----------------------------------------------------------------------------
Buffer&                             // The Buffer
   get_buffer( void ) const         // Get Buffer
{  buffer.reset(); return buffer; }

int                                 // The socket handle
   get_handle( void ) const         // Get socket handle
{  return socket->get_handle(); }

std::shared_ptr<Listen>             // The Listener
   get_listen( void ) const         // Get Listener
{  return listen.lock(); }

const sockaddr_u&                   // The connected Socket
   get_peer_addr( void ) const      // Get connected internet address
{  return socket->get_peer_addr(); }

std::shared_ptr<Server>             // Self-reference
   get_self( void ) const           // Get self-reference
{  return self.lock(); }

std::shared_ptr<Stream>             // The associated Stream
   get_stream(uint32_t id) const    // Locate the Stream given Stream::ident
{  return stream_set.get_stream(id); }

void
   on_close(                        // Set close event handler
     std::function<void(void)> f)
{  h_close= f; }

void
   on_error(                        // Set error event handler
     std::function<void(const std::string&)> f)
{  h_error= f; }

void
   test_error( void )               // Test error function
{  h_error("this is the test_error message"); }

//----------------------------------------------------------------------------
// Server::Methods
//----------------------------------------------------------------------------
void
   close( void );                   // Close the server

void
   connection_error(const char*);   // Handle connection error

virtual void
   join( void );                    // Wait for Server completion

size_t                              // Read length
   read(int, size_t);               // Line number, maximum length

size_t                              // Read length
   read(size_t);                    // Maximum length

ssize_t                             // Written length
   write(int, const void*, size_t); // Write to Socket

ssize_t                             // Written length
   write(const void*, size_t);      // Write to Socket

//----------------------------------------------------------------------------
// Server::Protected methods
//----------------------------------------------------------------------------
protected:
virtual void
   run( void );                     // Operate the server
}; // class Server
} // namespace pub::http
#endif // _PUB_HTTP_SERVER_H_INCLUDED
