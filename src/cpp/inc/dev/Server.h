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
//       2022/10/16
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_SERVER_H_INCLUDED
#define _LIBPUB_HTTP_SERVER_H_INCLUDED

#include <cstdint>                  // For integer types
#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::mutex, super class
#include <string>                   // For std::string

#include <pub/Named.h>              // For pub::Named, super class
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread, super class

#include "pub/http/Ioda.h"          // For pub::http::Ioda
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Request.h"       // For pub::http::Request
#include "pub/http/Stream.h"        // For pub::http::Stream

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Listen;

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
typedef Ioda::Mesg                            Mesg;
typedef Ioda::Size                            Size;
typedef Socket::sockaddr_u                    sockaddr_u;
typedef std::shared_ptr<ServerStream>         stream_ptr;
typedef std::string                           string;

typedef std::function<void(void)>             f_close;
typedef std::function<void(const string&)>    f_error;

//----------------------------------------------------------------------------
// Server::Attributes
//----------------------------------------------------------------------------
protected:
// Callback handlers - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
f_close                h_close;     // The close event handler
f_error                h_error;     // The error event handler

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::weak_ptr<Server>  self;        // Self reference
std::weak_ptr<Listen>  listen;      // Our owning Listener

Ioda                   ioda_out;    // The output data area
Size                   size_inp;    // The input data area length
Size                   size_out;    // The output data area length
Socket*                socket= nullptr; // The connection Socket
stream_ptr             stream;      // The current Stream
StreamSet              stream_set;  // Our set of Streams

int                    fsm= 0;      // Finite State Machine (TODO: USED?)
bool                   operational= false; // TRUE while operational

//----------------------------------------------------------------------------
// Server::Static attributes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Server::Constructors, Creator, destructor
//----------------------------------------------------------------------------
public:
   Server(Listen*, Socket*);        // Constructor

static std::shared_ptr<Server>
   make(Listen*, Socket*);          // Creator

   ~Server( void );                 // Destructor

//----------------------------------------------------------------------------
// Server::Accessor methods
//----------------------------------------------------------------------------
void debug(const char* info= "") const; // Debugging display

int                                 // The socket handle
   get_handle( void ) const         // Get socket handle
{  return socket->get_handle(); }

std::shared_ptr<Listen>             // The Listener
   get_listen( void ) const         // Get Listener
{  return listen.lock(); }

const sockaddr_u&                   // The Server's internet address
   get_host_addr( void ) const      // Get Server's internet address
{  return socket->get_host_addr(); }

const sockaddr_u&                   // The Client's internet address
   get_peer_addr( void ) const      // Get Client's internet address
{  return socket->get_peer_addr(); }

std::shared_ptr<Server>             // Self-reference
   get_self( void ) const           // Get self-reference
{  return self.lock(); }

std::shared_ptr<Stream>             // The associated Stream
   get_stream(uint32_t id) const    // Locate the Stream given Stream::ident
{  return stream_set.get_stream(id); }

void
   on_close(const f_close& f)       // Set close event handler
{  h_close= f; }

void
   on_error(const f_error& f)       // Set error event handler
{  h_error= f; }

//----------------------------------------------------------------------------
// Server::Methods
//----------------------------------------------------------------------------
void
   close( void );                   // Close the server

void
   error(const char*);              // Handle connection error

virtual void
   join( void );                    // Wait for Server completion

virtual void
   run( void );                     // Operate the server

void write(Ioda&);                  // Write to Socket

//----------------------------------------------------------------------------
// Server::Protected methods
//----------------------------------------------------------------------------
// protected:
void read(int);                     // Handle read (line number)

void read( void )                   // Handle read
{  read(0); }

void write(int, const void*, size_t); // Write to Socket

void write(const void* addr, size_t size) // Write to Socket
{  write(0, addr, size); }
}; // class Server

//----------------------------------------------------------------------------
//
// Class-
//       ServerApp
//
// Purpose-
//       Placeholder for Server application information.
//
//----------------------------------------------------------------------------
class ServerApp {
//----------------------------------------------------------------------------
// ServerApp::Attributes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ServerApp::Constructor, destructor
//----------------------------------------------------------------------------
public:
   ServerApp( void ) = default;
   ~ServerApp( void ) = default;

//----------------------------------------------------------------------------
// ServerApp::Methods
//----------------------------------------------------------------------------
}; // class ClientApp
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_SERVER_H_INCLUDED
