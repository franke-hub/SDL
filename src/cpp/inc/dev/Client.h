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
//       http/Client.h
//
// Purpose-
//       HTTP Client object.
//
// Last change date-
//       2023/04/16
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_CLIENT_H_INCLUDED
#define _LIBPUB_HTTP_CLIENT_H_INCLUDED

#include <new>                      // For in-place constructor
#include <cstdint>                  // For integer types
#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::mutex, super class
#include <string>                   // For std::string

#include <pub/Dispatch.h>           // For pub::Dispatch objects
#include <pub/Event.h>              // For pub::Event
#include <pub/Semaphore.h>          // For pub::Semaphore
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread

#include "pub/http/Ioda.h"          // For pub::http::Ioda
#include "pub/http/Stream.h"        // For pub::http::Stream, ...

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class ClientAgent;
class ClientItem;                   // (Internal)
class ClientRequest;
class ClientResponse;
class ClientStream;
class Options;
class Request;
class Response;

//----------------------------------------------------------------------------
//
// Class-
//       Client
//
// Purpose-
//       Define the (lockable) Client class.
//
//----------------------------------------------------------------------------
class Client : public std::mutex {  // Client class (lockable)
//----------------------------------------------------------------------------
// Client::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::function<void(dispatch::Item*)>  f_iotask; // (Internal)
typedef std::function<void(void)>             f_reader; // (Internal)
typedef std::function<void(void)>             f_writer; // (Internal)

typedef ClientAgent*                          agent_ptr;
typedef std::shared_ptr<ClientStream>         stream_ptr;
typedef Socket::sockaddr_u                    sockaddr_u;
typedef dispatch::LambdaDone                  LambdaDone;
typedef dispatch::LambdaTask                  LambdaTask;

enum FSM                            // Finite State Machine states
{  FSM_RESET= 0                     // Reset - closed
,  FSM_READY= 1                     // Operational
,  FSM_CLOSE= 2                     // Close in progress
}; // enum FSM

//----------------------------------------------------------------------------
// Client::Attributes
//----------------------------------------------------------------------------
protected:
public:                             // TODO: REMOVE
// Callback handlers - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
f_reader               h_reader;    // The (reader) protocol handler
f_writer               h_writer;    // The (writer) protocol handler
f_iotask               inp_task;    // The input (reader) task
f_iotask               out_task;    // The output (writer) task

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::weak_ptr<Client>  self;        // Self-reference
agent_ptr              agent;       // Our owning Agent

SSL_CTX*               context= nullptr; // SSL context
Ioda                   ioda_out;    // The output buffer
size_t                 ioda_off;    // The output buffer offset
const char*            proto_id;    // The Client's protocol/version
Event                  rd_complete; // HTTP/1 operation completed event
StreamSet::Node        root;        // Stream[0]
size_t                 size_inp;    // The input buffer length
size_t                 size_out;    // The output buffer length
Socket*                socket= nullptr; // Connection Socket
stream_ptr             stream;      // The active stream
ClientItem*            stream_item; // The active ClientItem
StreamSet              stream_set;  // Our set of Streams
LambdaTask             task_inp;    // Reader task
LambdaTask             task_out;    // Writer task

int                    events= 0;   // Current polling events
int                    fsm= FSM_RESET; // Finite State Machine state

//----------------------------------------------------------------------------
// Client::Constructor, creator, destructor
//----------------------------------------------------------------------------
public:
   Client(                          // Constructor
     ClientAgent*      owner);      // Our Agent

static std::shared_ptr<Client>      // The Client
   make(                            // Create Client
     ClientAgent*      owner);      // Our Agent

   ~Client( void );                 // Destructor

//----------------------------------------------------------------------------
// Client::debug
//----------------------------------------------------------------------------
void debug(const char* info= "") const; // Debugging display

//----------------------------------------------------------------------------
// Client::Accessor methods
//----------------------------------------------------------------------------
bool
   is_operational( void ) const     // Is the Client operational?
{  return fsm == FSM_READY; }

int                                 // The socket handle (<0 if not connected)
   get_handle( void ) const         // Get socket handle
{  return socket->get_handle(); }

const sockaddr_u&                   // The Client's internet address
   get_host_addr( void ) const      // Get Client's internet address
{  return socket->get_host_addr(); }

const sockaddr_u&                   // The Server's internet address
   get_peer_addr( void ) const      // Get Server's internet address
{  return socket->get_peer_addr(); }

const char*
   get_proto_id( void ) const       // Get protocol/version
{  return proto_id; }

std::shared_ptr<Client>
   get_self( void ) const           // Get self-reference
{  return self.lock(); }

std::shared_ptr<Stream>             // The associated Stream
   get_stream(uint32_t id) const    // Locate the Stream given Stream::ident
{  return stream_set.get_stream(id); }

//----------------------------------------------------------------------------
// Client::Methods
//----------------------------------------------------------------------------
void
   async(                           // Handle asynchronous polling event
     int               revents);    // Polling revents

void
   close( void );                   // Close the Client

void
   close_enq( void );               // Schedule Client close

Socket*                             // The connected Socket
   connect(                         // Connect using
     const sockaddr*   addr,        // This target internet address
     socklen_t         size,        // This target internet address length
     const Options*    opts= nullptr); // Connection Options

void
   error(const char*);              // Handle connection error

std::shared_ptr<ClientStream>       // The ClientStream
   make_stream(                     // Create a ClientStream
     const Options*    opts= nullptr); // The associated Options

void
   wait( void );                    // Wait until idle

int                                 // Return code, 0 expected
   write(ClientStream*);            // Write ClientStream Request

//----------------------------------------------------------------------------
// Client::Protected methods
//----------------------------------------------------------------------------
protected:
void    http1( void );              // Use HTTP/0, HTTP/1 protocol handlers
void    http2( void );              // Use HTTP/2 protocol handlers

void    read(int line= 0);          // Read from Socket (caller __LINE__)
ssize_t write(int line= 0);         // Write into Socket (caller __LINE__)
}; // class Client

//----------------------------------------------------------------------------
//
// Class-
//       ClientApp
//
// Purpose-
//       Placeholder for Client application, NOT IMPLEMENTED.
//
//----------------------------------------------------------------------------
class ClientApp {
//----------------------------------------------------------------------------
// ClientApp::Attributes
//----------------------------------------------------------------------------
typedef std::function<void(Socket*)>          f_socket; // Socket ready
f_socket               h_socket;    // The Socket ready handler

//----------------------------------------------------------------------------
// ClientApp::Constructor, destructor
//----------------------------------------------------------------------------
public:
   ClientApp( void ) = default;
   ~ClientApp( void ) = default;

//----------------------------------------------------------------------------
// ClientApp::Methods
//----------------------------------------------------------------------------
void
   on_socket(const f_socket& f)     // Set socket selection handler
{  h_socket= f; }
}; // class ClientApp
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_CLIENT_H_INCLUDED
