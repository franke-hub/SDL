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
//       http/Stream.h
//
// Purpose-
//       HTTP Stream object.
//
// Last change date-
//       2023/06/04
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_STREAM_H_INCLUDED
#define _LIBPUB_HTTP_STREAM_H_INCLUDED

#include <cstdlib>                  // For size_t
#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string

#include <pub/Dispatch.h>           // For pub::dispatch objects
#include <pub/Statistic.h>          // For pub::Statistic

#include "pub/http/Ioda.h"          // For pub::http::Ioda, ...
#include "pub/http/StreamSet.h"     // For pub::StreamSet::Node, base class

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Client;
class ClientRequest;
class ClientResponse;
class Options;
class Request;
class Response;
class Server;
class ServerRequest;
class ServerResponse;

//----------------------------------------------------------------------------
//
// Class-
//       Stream
//
// Purpose-
//       Define the Stream base class.
//
//----------------------------------------------------------------------------
class Stream : public StreamSet::Node { // Stream base class
//----------------------------------------------------------------------------
// Stream::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::shared_ptr<Request>    request_ptr;
typedef std::shared_ptr<Response>   response_ptr;
typedef std::string                 string;
typedef uint32_t                    uint31_t; // (High order bit used as flag)

// Callback handler types
typedef std::function<void(void)>             f_end;
typedef std::function<void(const string&)>    f_error;

enum FSM                            // Finite State Machine states
{  F_IDLE= 0                        // IDLE (initial state)
,  F_OPEN                           // OPEN (active)
,  F_HALF_CLOSED_LOCAL              // HALF-CLOSED (local)
,  F_HALF_CLOSED_REMOTE             // HALF-CLOSED (remote)
,  F_RESERVED_LOCAL                 // RESERVED (local)
,  F_RESERVED_REMOTE                // RESERVED (remote)
,  F_CLOSED                         // CLOSED
}; // enum FSM

//----------------------------------------------------------------------------
// Stream::Attributes
//----------------------------------------------------------------------------
protected:
std::weak_ptr<Stream>  self;        // Self reference
request_ptr            request;     // The associated Request
response_ptr           response;    // The associated Response

// Callback handlers
f_end                  h_end;       // The Stream completion handler
f_error                h_error;     // The error event handler

// Controls
uint32_t               fsm= 0;      // Finite State Machine state (Placeholder)
uint31_t               ident= 1;    // Stream identifier

//----------------------------------------------------------------------------
// Stream::Static attributes
//----------------------------------------------------------------------------
public:
static statistic::Active
                       obj_count;   // Stream object counter

//----------------------------------------------------------------------------
// Stream::Constructors, destructor
//----------------------------------------------------------------------------
   Stream( void );                  // Default constructor

virtual
   ~Stream( void );                 // Destructor

//----------------------------------------------------------------------------
// Stream::debug
//----------------------------------------------------------------------------
virtual void
   debug(const char* info= "") const; // Debugging display

//----------------------------------------------------------------------------
// Stream::Accessor methods
//----------------------------------------------------------------------------
uint31_t                            // The Stream identifier
   get_ident( void ) const          // Get Stream identifier
{  return ident; }

std::shared_ptr<Request>
   get_request( void ) const        // Get Request
{  return request; }

std::shared_ptr<Response>
   get_response( void ) const        // Get Response
{  return response; }

std::shared_ptr<Stream>
   get_self( void ) const           // Get self-reference
{  return self.lock(); }

static const char*                  // The status text
   get_text(int code);              // Convert status code to text

void
   set_ident(                       // Set Stream identifier
     uint32_t          id)          // (The identifier value)
{  ident= id; }

void
   on_end(const f_end& f)           // Set completion handler
{  h_end= f; }

void
   on_error(const f_error& f)       // Set error event handler
{  h_error= f; }
}; // class Stream

//----------------------------------------------------------------------------
//
// Class-
//       ClientStream
//
// Purpose-
//       Define the ClientStream class. (Http1ClientStream)
//
//----------------------------------------------------------------------------
class ClientStream : public Stream { // ClientStream descriptor
//----------------------------------------------------------------------------
// ClientStream::Attributes
//----------------------------------------------------------------------------
public:
protected:
std::weak_ptr<Client>  client;      // Our Client

//----------------------------------------------------------------------------
// ClientStream::Protocol handlers
//----------------------------------------------------------------------------
void
   http1( void );                    // HTTP/0, HTTP/1 protocol handler

void
   http2( void );                    // HTTP/2 protocol handler

//----------------------------------------------------------------------------
// ClientStream::Constructors, destructor , creators
//----------------------------------------------------------------------------
public:
   ClientStream(Client*);           // Constructor

virtual
   ~ClientStream( void );           // Destructor

static std::shared_ptr<ClientStream> // The ClientStream
   make(Client*, const Options* opts=nullptr); // Create ClientStream

//----------------------------------------------------------------------------
// ClientStream::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<Client>
   get_client( void ) const         // Get associated client
{  return client.lock(); }

std::shared_ptr<ClientRequest>
   get_request( void );             // Get ClientRequest

std::shared_ptr<ClientResponse>
   get_response( void );            // Get ClientResponse

std::shared_ptr<ClientStream>
   get_self( void )                 // Get self-reference
{  return std::dynamic_pointer_cast<ClientStream>(self.lock()); }

//----------------------------------------------------------------------------
// ClientStream::I/O methods
//----------------------------------------------------------------------------
bool                                // Return code: TRUE if complete
   read(Ioda&);                     // (Async) read Response segment

void
   write( void );                   // Write transmission complete

void
   write(Ioda&);                    // Write Request segment to stream

//----------------------------------------------------------------------------
// ClientStream::Methods
//----------------------------------------------------------------------------
void
   end( void );                     // End the ClientStream
}; // class ClientStream

//----------------------------------------------------------------------------
//
// Class-
//       ServerStream
//
// Purpose-
//       Define the ServerStream class.
//
//----------------------------------------------------------------------------
class ServerStream : public Stream { // ServerStream descriptor
//----------------------------------------------------------------------------
// ServerStream::Attributes
//----------------------------------------------------------------------------
std::weak_ptr<Server>  server;      // Our Server

//----------------------------------------------------------------------------
// ServerStream::Destructor, constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ServerStream( void );           // Destructor
   ServerStream(Server*);           // Constructor

static std::shared_ptr<ServerStream> // The ServerStream
   make(Server*);                   // Create ServerStream

//----------------------------------------------------------------------------
// ServerStream::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<ServerStream>
   get_self( void ) const           // Get self reference
{  return std::dynamic_pointer_cast<ServerStream>(self.lock()); }

std::shared_ptr<ServerRequest>
   get_request( void );             // Get ServerRequest

std::shared_ptr<ServerResponse>
   get_response( void );            // Get ServerResponse

std::shared_ptr<Server>
   get_server( void ) const         // Get associated server
{  return server.lock(); }

//----------------------------------------------------------------------------
// ServerStream::I/O methods
//----------------------------------------------------------------------------
bool                                // Return code: TRUE if complete
   read(Ioda&);                     // (Async) read request segment

void
   write( void );                   // Write Response complete

void
   write(int, const void*, size_t); // Write to Socket (line, address, length)

void
   write(const void* addr, size_t size) // Write to Socket
{  write(0, addr, size); }

void
   write(Ioda&);                    // Write Response segment to Server

//----------------------------------------------------------------------------
// ServerStream::Methods
//----------------------------------------------------------------------------
void
   end( void );                     // End the ClientStream

void
   reject(                          // Reject a Request, writing Response
     int               code);       // The rejection status code
}; // class ServerStream
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_STREAM_H_INCLUDED
