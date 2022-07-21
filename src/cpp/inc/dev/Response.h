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
//       http/Response.h
//
// Purpose-
//       HTTP Response information.
//
// Last change date-
//       2022/07/16
//
//----------------------------------------------------------------------------
#ifndef _PUB_HTTP_RESPONSE_H_INCLUDED
#define _PUB_HTTP_RESPONSE_H_INCLUDED

#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr
#include <string>                   // For std::string

#include <pub/Statistic.h>          // For pub::Statistic

#include <pub/http/Data.h>          // For pub::http::Hunk, pub::http::Data
#include <pub/http/Options.h>       // For pub::http::Options, super class

namespace pub::http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Request;                      // pub::http::Request
class Stream;                       // pub::http::Stream

//----------------------------------------------------------------------------
//
// Class-
//       Response
//
// Purpose-
//       HTTP Response
//
//----------------------------------------------------------------------------
class Response : public Options {   // Http Response
//----------------------------------------------------------------------------
// Response::Attributes
//----------------------------------------------------------------------------
public:
static pub::Statistic  obj_count;   // Request counter

protected:
std::weak_ptr<Response>self;        // Self reference
std::shared_ptr<Stream>stream;      // Associated Stream

Data                   data;        // Response data
int                    code= 0;     // Response code
int                    fsm= 0;      // Finite State Machine

// Callback handlers (TODO: REMOVE UNUSED)
std::function<void(const Hunk&)>
                       h_data;      // The Response data handler
std::function<void(void)>
                       h_end;       // The Response completion handler
std::function<void(const std::string&)>
                       h_error;     // The Response (connection) error handler

//----------------------------------------------------------------------------
// Response::Destructor/Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Response( void );               // Destructor
   Response( void );                // Default constructor

//----------------------------------------------------------------------------
// Response::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }

//----------------------------------------------------------------------------
// Response::Accessor methods
//----------------------------------------------------------------------------
int
   get_code( void ) const           // Get response code
{  return code; }

Data&
   get_data( void ) const
{  return const_cast<Data&>(data); }

std::shared_ptr<Request>
   get_request( void ) const;

std::shared_ptr<Response>
   get_self( void ) const
{  return self.lock(); }

std::shared_ptr<Stream>
   get_stream( void ) const
{  return stream; }
//{  return stream.lock(); }

void
   set_code(int code_)              // Set response code
{  this->code= code_; }

void
   on_data(                         // Set response data handler
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
// Response::Methods
//----------------------------------------------------------------------------
virtual void
   end( void );                     // Complete the response

void
   reject(int);                     // Internal reject

//----------------------------------------------------------------------------
//
// Method-
//       Response::read
//       Response::write
//
// Purpose-
//       Virtual methods; only implemented in ClientResponse
//       Virtual methods; only implemented in ServerResponse
//
//----------------------------------------------------------------------------
virtual bool read(const void*, size_t); // (Async) read Response data

virtual void write(const void*, size_t); // (ServerResponse only)
virtual void write( void );         // (ServerResponse only)
void write(std::string S)           // (ServerResponse only)
{  write(S.c_str(), S.size()); }
}; // class Response

//----------------------------------------------------------------------------
//
// Class-
//       ClientResponse
//
// Purpose-
//       Define the ClientResponse class.
//
//----------------------------------------------------------------------------
class ClientResponse : public Response { // ClientResponse class
//----------------------------------------------------------------------------
// ClientResponse::Attributes
//----------------------------------------------------------------------------
protected:

//----------------------------------------------------------------------------
// ClientResponse::Destructor/Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ClientResponse( void );         // Destructor
   ClientResponse( void );          // Default constructor

static std::shared_ptr<ClientResponse> // The ClientResponse
   make(                            // Get ClientResponse
     ClientStream*     stream,      // For this ClientStream
     const Options*    opts= nullptr); // And these Options

//----------------------------------------------------------------------------
// ClientResponse::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<ClientRequest>
   get_request( void ) const;

std::shared_ptr<ClientResponse>
   get_self( void ) const
{  return std::dynamic_pointer_cast<ClientResponse>(self.lock()); }

std::shared_ptr<ClientStream>
   get_stream( void ) const;

//----------------------------------------------------------------------------
// ClientResponse::Methods
//----------------------------------------------------------------------------
virtual bool read(const void*, size_t); // (Async) read Response data
}; // class ClientResponse

//----------------------------------------------------------------------------
//
// Class-
//       ServerResponse
//
// Purpose-
//       Define the ServerResponse class.
//
//----------------------------------------------------------------------------
class ServerResponse : public Response { // ServerResponse class
//----------------------------------------------------------------------------
// ServerResponse::Attributes
//----------------------------------------------------------------------------
protected:

//----------------------------------------------------------------------------
// ServerResponse::Destructor/Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ServerResponse( void );         // Destructor
   ServerResponse( void );          // Default constructor

static std::shared_ptr<ServerResponse> // The ServerResponse
   make(                            // Get ServerResponse
     ServerStream*     stream,      // For this ServerStream
     const Options*    opts= nullptr); // And these Options

//----------------------------------------------------------------------------
// ServerResponse::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<ServerRequest>
   get_request( void ) const;

std::shared_ptr<ServerResponse>
   get_self( void ) const
{  return std::dynamic_pointer_cast<ServerResponse>(self.lock()); }

std::shared_ptr<ServerStream>
   get_stream( void ) const;

//----------------------------------------------------------------------------
// ServerResponse::Methods
//----------------------------------------------------------------------------
virtual void
   write(const void*, size_t);      // Write response data

virtual void
   write( void );                   // Transmit the response
}; // class ServerRequest
}  // namespace pub::http
#endif // _PUB_HTTP_RESPONSE_H_INCLUDED
