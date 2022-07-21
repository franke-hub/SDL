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
//       http/Stream.h
//
// Purpose-
//       HTTP Stream object.
//
// Last change date-
//       2022/07/16
//
// Implementation notes-
//       May need Http1ClientStream, Http2ServerStream, etc.
//       May need to implement extender class to dynamically change type.
//
//----------------------------------------------------------------------------
#ifndef _PUB_HTTP_STREAM_H_INCLUDED
#define _PUB_HTTP_STREAM_H_INCLUDED

#include <cstdlib>                  // For size_t
#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string

#include <pub/Statistic.h>          // For pub::Statistic

#include "pub/http/Data.h"          // For pub::http::Data

namespace pub::http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Client;                       // pub::http::Client
class ClientRequest;                // pub::http::ClientRequest
class ClientResponse;               // pub::http::ClientResponse
class Options;                      // pub::http::Options
class Server;                       // pub::http::Server
class ServerRequest;                // pub::http::ServerRequest
class ServerResponse;               // pub::http::ServerResponse
class Request;                      // pub::http::Request
class Response;                     // pub::http::Response

//----------------------------------------------------------------------------
//
// Class-
//       Stream
//
// Purpose-
//       Define the Stream base class.
//
//----------------------------------------------------------------------------
class Stream {                      // Stream base class
//----------------------------------------------------------------------------
// Stream::Typedefs and enumerations
//----------------------------------------------------------------------------
protected:
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
std::weak_ptr<Stream>  self;        // Self reference
std::shared_ptr<Request>  request;  // Associated Request
std::shared_ptr<Response> response; // Associated Response

// Callback handlers
std::function<void(void)>
                       h_close;     // The close event handler
std::function<void(void)>
                       h_end;       // The Stream completion handler
std::function<void(const std::string&)>
                       h_error;     // The error event handler

// Controls
uint32_t               fsm= 0;      // Finite State Machine state
uint32_t               ident= 1;    // Stream identifier (uint31_t)

//----------------------------------------------------------------------------
// Stream::Static attributes
//----------------------------------------------------------------------------
public:
static pub::Statistic  obj_count;   // Stream counter

//----------------------------------------------------------------------------
// Stream::Destructor, constructors
//----------------------------------------------------------------------------
virtual
   ~Stream( void );                 // Destructor
   Stream( void );                  // Default constructor

//----------------------------------------------------------------------------
// Stream::debug
//----------------------------------------------------------------------------
virtual void debug(const char*) const; // Debugging display
void debug( void ) const;           // Debugging display

//----------------------------------------------------------------------------
// Stream::Accessor methods
//----------------------------------------------------------------------------
uint32_t                            // The state
   get_fsm( void )                  // Get state
{  return fsm; }                    // TODO: VERIFY NEED

uint32_t                            // The Stream identifier
   get_ident( void )                // Get Stream identifier
{  return ident; }

std::shared_ptr<Request>
   get_request( void )              // Get Request
{  return request; }

std::shared_ptr<Response>
   get_response( void )              // Get Response
{  return response; }

std::shared_ptr<Stream>
   get_self( void )                 // Get self-reference
{  return self.lock(); }

static const char*                  // The status text
   get_text(int code);              // Convert status code to text

void
   set_ident(                       // Set Stream identifier
     uint32_t          id)          // (The identifier value)
{  ident= id; }

// TODO: These may not be needed, substituting virtual methods instead.
void
   on_close(                        // Set close event handler
     std::function<void(void)> f)
{  h_close= f; }

void
   on_end(                          // Set completion handler
     const std::function<void(void)>& f)
{  h_end= f; }

void
   on_error(                        // Set error event handler
     std::function<void(const std::string&)> f)
{  h_error= f; }

//----------------------------------------------------------------------------
// Stream::I/O methods, MUST be overridden by subclass.
// Clients synchronously write request and asynchronously read response data
// Servers asynchronously read request and synchronously write response data
//----------------------------------------------------------------------------
virtual bool                        // Return code: TRUE if complete
   read(const void*, size_t) = 0;   // (Async) read data segment from stream

virtual void
   write(const void*, size_t) = 0;  // Transmit data segment to stream
virtual void
   write( void ) = 0;               // Transmission completed

//----------------------------------------------------------------------------
// Stream::Methods
//----------------------------------------------------------------------------
void
   close( void );                   // Close the Strea,m

void
   end( void );                     // End the Stream

virtual void
   reject(int);                     // Reject a Request

void
   reset( void );                   // Reset the Stream
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
std::weak_ptr<Client>  client;      // Our Client

//----------------------------------------------------------------------------
// ClientStream::Destructor, constructors, creators
//----------------------------------------------------------------------------
public:
virtual
   ~ClientStream( void );           // Destructor
   ClientStream(Client*);           // Constructor

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
virtual bool                        // Return code: TRUE if complete
   read(const void*, size_t);       // (Async) read response segment

virtual void
   write(const void*, size_t);      // Transmit Request segment to stream
virtual void                        // (Transmitter indicates)
   write( void );                   // Transmission completed

//----------------------------------------------------------------------------
// ClientStream::Methods
//----------------------------------------------------------------------------
virtual void
   reject(                          // Reject a Request, simulating Response
     int               code);       // The rejection status code
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
virtual bool                        // Return code: TRUE if complete
   read(const void*, size_t);       // (Async) read request segment

virtual void
   write(const void*, size_t);      // Transmit Response segment to Server
virtual void                        // (Transmitter indicates)
   write( void );                   // Indicate Response completed

//----------------------------------------------------------------------------
// ServerStream::Methods
//----------------------------------------------------------------------------
virtual void
   reject(                          // Reject a Request, writing Response
     int               code);       // The rejection status code
}; // class ServerStream

//----------------------------------------------------------------------------
//
// Class-
//       StreamSet
//
// Purpose-
//       Control a set of Stream objects.
//
//----------------------------------------------------------------------------
class StreamSet {                   // A set of Stream objects
protected:
struct Node {                       // A Stream Node
Node*                  parent= nullptr; // The parent Node
Node*                  peer= nullptr;   // The next peer Node
Node*                  child= nullptr;  // The first child Node
std::shared_ptr<Stream>stream;      // The associated Stream
}; // Node

//----------------------------------------------------------------------------
// StreamSet::Attributes
//----------------------------------------------------------------------------
mutable std::mutex     mutex;       // The SteamSet mutex
void*                  cache= nullptr; // A opaque Node cache
Node                   root;        // The root Node

uint32_t               ident= 0;    // The next available Stream identifier

//----------------------------------------------------------------------------
// StreamSet::Destructor, constructors, operators
//----------------------------------------------------------------------------
public:
   ~StreamSet( void ) = default;    // Destructor
   StreamSet( void ) = default;     // Constructor

   StreamSet(const StreamSet&) = delete; // Disallowed copy constructor
StreamSet&
   operator=(const StreamSet&) = delete; // Disallowed assignment operator

//----------------------------------------------------------------------------
// StreamSet::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }

//----------------------------------------------------------------------------
// StreamSet::Accessor methods
//----------------------------------------------------------------------------
uint32_t                            // The next available Stream identifier
   assign( void );                  // Assign a Stream identifier

Node*                               // The root Node*
   get_root( void ) const           // Get root Node*
{  return const_cast<Node*>(&root); }

std::shared_ptr<Stream>             // The associated Stream
   get_stream(uint32_t) const;      // Locate the Stream given Stream::ident

//----------------------------------------------------------------------------
// StreamSet::methods
//----------------------------------------------------------------------------
void
   change(                          // Change a Stream's parent
     Node*             parent,      // The new parent Stream
     std::shared_ptr<Stream>
                       stream);     // The Stream to move

void
   insert(                          // Insert Stream
     Node*             parent,      // The parent Stream
     std::shared_ptr<Stream>
                       stream);     // The Stream to insert

void
   remove(                          // Remove Stream
     std::shared_ptr<Stream>
                       stream);     // The Stream to remove
}; // class StreamSet
} // namespace pub::http
#endif // _PUB_HTTP_STREAM_H_INCLUDED
