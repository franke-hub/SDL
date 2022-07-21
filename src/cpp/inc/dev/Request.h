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
//       http/Request.h
//
// Purpose-
//       HTTP Request.
//
// Last change date-
//       2022/07/16
//
//----------------------------------------------------------------------------
#ifndef _PUB_HTTP_REQUEST_H_INCLUDED
#define _PUB_HTTP_REQUEST_H_INCLUDED

#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr
#include <string>                   // For std::string

#include <pub/Statistic.h>          // For pub::Statistic

#include "pub/http/Data.h"          // For pub::http::Data, pub::http::Hunk
#include "pub/http/Options.h"       // For pub::http::Options, super class

namespace pub::http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Client;                       // pub::http::Client
class ClientResponse;               // pub::http::ClientResponse
class ClientStream;                 // pub::http::ClientStream
class Request;                      // pub::http::Request
class Response;                     // pub::http::Response
class Server;                       // pub::http::Server
class ServerResponse;               // pub::http::ServerResponse
class ServerStream;                 // pub::http::ServerStream
class Stream;                       // pub::http::Stream

//----------------------------------------------------------------------------
//
// Class-
//       Request
//
// Purpose-
//       HTTP request
//
//----------------------------------------------------------------------------
class Request : public Options {    // Http request
//----------------------------------------------------------------------------
// Request::Attributes
//----------------------------------------------------------------------------
public:
static pub::Statistic  obj_count;   // Request counter

std::string            method;      // Request method
std::string            path;        // Request path
std::string            proto_id;    // Request protocol/version

protected:
std::weak_ptr<Request> self;        // Self reference
std::shared_ptr<Stream>stream;      // Associated Stream

Data                   data;        // POST/PUT data
int                    fsm= 0;      // Finite State Machine

// Callback handlers
std::function<void(const Hunk&)>
                       h_data;      // The Request data handler
std::function<void(void)>
                       h_end;       // The Request completion handler
std::function<void(const std::string&)>
                       h_error;     // The Request connection error handler

//----------------------------------------------------------------------------
// Request::Destructor/Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Request( void );                // Destructor
   Request( void );                 // Default constructor

//----------------------------------------------------------------------------
// Request::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }

//----------------------------------------------------------------------------
// Request::Accessor methods
//----------------------------------------------------------------------------
const Data&                         // The associated (const) Data
   get_data( void ) const           // Get associated (const) Data
{  return data; }

std::shared_ptr<Response>
   get_response( void ) const;

std::shared_ptr<Request>
   get_self( void ) const
{  return self.lock(); }

std::shared_ptr<Stream>
   get_stream( void ) const
{  return stream; }

void
   on_data(                         // Set Request data handler
     const std::function<void(const Hunk&)>& f)
{  h_data= f; }

void
   on_end(                          // Set completion handler
     const std::function<void(void)>& f)
{  h_end= f; }

void
   on_error(                        // Set connection error handler
     const std::function<void(const std::string&)>& f)
{  h_error= f; }

//----------------------------------------------------------------------------
// Request::Methods
//----------------------------------------------------------------------------
virtual void end( void );           // Complete the request

void reject(int);                   // Reject the request

//----------------------------------------------------------------------------
//
// Method-
//       Request::read
//       Request::write
//
// Purpose-
//       Virtual method; only implemented in ServerRequest
//       Virtual methods; only implemented in ClientRequest
//
//----------------------------------------------------------------------------
virtual bool read(const void*, size_t); // (Async) read Request data

virtual void write(const void*, const size_t); // (ClientRequest only)
virtual void write();               // (ClientRequest only)
}; // class Request

//----------------------------------------------------------------------------
//
// Class-
//       ClientRequest
//
// Purpose-
//       Define the ClientRequest class.
//
//----------------------------------------------------------------------------
class ClientRequest : public Request { // ClientRequest class
//----------------------------------------------------------------------------
// ClientRequest::Attributes
//----------------------------------------------------------------------------
protected:

//----------------------------------------------------------------------------
// ClientRequest::Destructor/Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ClientRequest( void );          // Destructor
   ClientRequest( void );           // Default constructor

static std::shared_ptr<ClientRequest> // The ClientRequest
   make(                            // Get ClientRequest
     ClientStream*     stream,      // For this ClientStream
     const Options*    opts= nullptr); // And these Options

//----------------------------------------------------------------------------
// ClientRequest::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<ClientResponse>
   get_response( void ) const;

std::shared_ptr<ClientRequest>
   get_self( void ) const
{  return std::dynamic_pointer_cast<ClientRequest>(self.lock()); }

std::shared_ptr<ClientStream>
   get_stream( void ) const;

//----------------------------------------------------------------------------
// ClientRequest::Methods
//----------------------------------------------------------------------------
virtual void
   end( void );                     // Complete the ClientRequest

virtual void
   write(                           // Write POST/PUT data
     const void*       addr,        // Data address
     size_t            size);       // Data length

virtual void
   write( void );                   // Transmit the ClientRequest
}; // class ClientRequest

//----------------------------------------------------------------------------
//
// Class-
//       ServerRequest
//
// Purpose-
//       Define the ServerRequest class.
//
//----------------------------------------------------------------------------
class ServerRequest : public Request { // ServerRequest class
//----------------------------------------------------------------------------
// ServerRequest::Attributes
//----------------------------------------------------------------------------
protected:

//----------------------------------------------------------------------------
// ServerRequest::Destructor/Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ServerRequest( void );          // Destructor
   ServerRequest( void );           // Default constructor

static std::shared_ptr<ServerRequest> // The ServerRequest
   make(                            // Get ServerRequest
     ServerStream*     stream,      // For this ServerStream
     const Options*    opts= nullptr); // And these Options

//----------------------------------------------------------------------------
// ServerRequest::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<ServerRequest>
   get_request( void ) const
{  return std::dynamic_pointer_cast<ServerRequest>(self.lock()); }

std::shared_ptr<ServerStream>
   get_stream( void ) const;

//----------------------------------------------------------------------------
// ServerRequest::Methods
//----------------------------------------------------------------------------
virtual void
   end( void );                     // Complete the ServerRequest

virtual bool read(const void*, size_t); // (Async) read Request data
}; // class ServerRequest
}  // namespace pub::http
#endif // _PUB_HTTP_REQUEST_H_INCLUDED
