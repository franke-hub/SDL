//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
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
//       2023/06/02
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_SERVER_H_INCLUDED
#define _LIBPUB_HTTP_SERVER_H_INCLUDED

#include <cstdint>                  // For integer types
#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::mutex, super class
#include <string>                   // For std::string

#include <pub/Dispatch.h>           // For namespace pub::dispatch objects
#include <pub/Event.h>              // For pub::Event
#include <pub/Socket.h>             // For pub::Socket

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
class ServerItem;                   // (Internal)

//----------------------------------------------------------------------------
//
// Class-
//       Server
//
// Purpose-
//       Define the Server class.
//
//----------------------------------------------------------------------------
class Server : public std::mutex {  // Server class (lockable)
//----------------------------------------------------------------------------
// Server::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::function<void(dispatch::Item*)>  f_iotask; // (Internal)
typedef std::function<void(void)>             f_reader; // (Internal)
typedef std::function<void(void)>             f_writer; // (Internal)

typedef Ioda::Mesg                            Mesg;
typedef Socket::sockaddr_u                    sockaddr_u;
typedef dispatch::LambdaTask                  LambdaTask;

typedef std::shared_ptr<Server>               server_ptr;
typedef std::shared_ptr<ServerStream>         stream_ptr;
typedef std::string                           string;

enum FSM                            // Finite State Machine states
{  FSM_RESET= 0                     // Reset - closed
,  FSM_READY= 1                     // Operational
,  FSM_CLOSE= 2                     // Close in progress
}; // enum FSM

//----------------------------------------------------------------------------
// Server::Attributes
//----------------------------------------------------------------------------
protected:
public:                             // TODO: REMOVE
// Callback handlers - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
f_reader               h_reader;    // The (reader) protocol handler
f_writer               h_writer;    // The (writer) protocol handler
f_iotask               inp_task;    // The input (reader) task
f_iotask               out_task;    // The output (writer) task

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::weak_ptr<Server>  self;        // Self reference
Listen*                listen;      // Our owning Listener

Ioda                   ioda_out;    // The output data area
const char*            proto_id;    // The Server's protocol/version
StreamSet::Node        root;        // Stream[0]
size_t                 size_inp;    // The input data area length
size_t                 size_out;    // The output data area length
Socket*                socket= nullptr; // The connection Socket
stream_ptr             stream;      // The current Stream
// StreamSet              stream_set;  // Our set of Streams
LambdaTask             task_inp;    // Reader task
LambdaTask             task_out;    // Writer task

int                    events= 0;   // Current polling events
int                    fsm= FSM_RESET; // Finite State Machine state
int                    serialno= 0; // Server serial number
int                    sequence= 0; // ServerItem sequence number

//----------------------------------------------------------------------------
// Server::Constructor, creator, destructor
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

Listen*                             // The Listener
   get_listen( void ) const         // Get Listener
{  return listen; }

const sockaddr_u&                   // The Server's internet address
   get_host_addr( void ) const      // Get Server's internet address
{  return socket->get_host_addr(); }

const sockaddr_u&                   // The Client's internet address
   get_peer_addr( void ) const      // Get Client's internet address
{  return socket->get_peer_addr(); }

server_ptr                          // Self-reference
   get_self( void ) const           // Get self-reference
{  return self.lock(); }

stream_ptr                          // The associated Stream
   get_stream(uint32_t id) const    // Locate the Stream given Stream::ident
{  (void)id; return stream; }       // (TODO: Code stream_set)

void
   set_stream(                      // Add Stream to stream_set for
     uint32_t          id,          // This stream id and
     stream_ptr        stream)      // This Stream
{  (void)id; this->stream= stream; } // (TODO: Code stream_set)

//----------------------------------------------------------------------------
// Server::Methods
//----------------------------------------------------------------------------
void
   async(int);                      // Handle asynchronous event

void
   close( void );                   // Close the Server

void
   close_enq( void );               // Schedule Server close

void
   error(const char*);              // Handle connection error

void
   wait( void );                    // Wait until idle

void
   write(Ioda&);                    // Write to Socket

//----------------------------------------------------------------------------
// Server::Protected methods
//----------------------------------------------------------------------------
protected:
void _http1( void );                // Use HTTP/0, HTTP/1 protocol handlers
void _http2( void );                // Use HTTP/2 protocol handlers

void _read(int line= 0);            // Handle read (line number)
void _write(int line= 0);           // Handle write (line number)
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
// NOT CODED YET

//----------------------------------------------------------------------------
// ServerApp::Constructor, destructor
//----------------------------------------------------------------------------
public:
   ServerApp( void ) = default;
   ~ServerApp( void ) = default;

//----------------------------------------------------------------------------
// ServerApp::Methods
//----------------------------------------------------------------------------
}; // class ServerApp
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_SERVER_H_INCLUDED
