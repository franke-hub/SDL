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
//       2022/10/19
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_RESPONSE_H_INCLUDED
#define _LIBPUB_HTTP_RESPONSE_H_INCLUDED

#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr
#include <string>                   // For std::string

#include <pub/Statistic.h>          // For pub::Statistic

#include "pub/http/Ioda.h"          // For pub::http::Ioda
#include <pub/http/Options.h>       // For pub::http::Options, super class

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Request;
class Stream;

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
// Request::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::string    string;      // (Using std::string)

// Callback handlers
typedef std::function<void(Ioda&)>            f_ioda;
typedef std::function<void(void)>             f_end;
typedef std::function<void(const string&)>    f_error;

//----------------------------------------------------------------------------
// Response::Attributes
//----------------------------------------------------------------------------
static statistic::Active
                       obj_count;   // Request object counter

protected:
std::weak_ptr<Response>self;        // Self reference
std::shared_ptr<Stream>stream;      // Associated Stream

Ioda                   ioda;        // Response data area accumulator
//size_t                 ioda_off;    // Response data area offset
int                    code= 0;     // Response code
int                    fsm= 0;      // Finite State Machine

// Callback handlers (TODO: REMOVE UNUSED)
f_ioda                 h_ioda;      // The Response data handler
f_end                  h_end;       // The Response completion handler
f_error                h_error;     // The Response (connection) error handler

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

Ioda&
   get_ioda( void )
{  return ioda; }

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
   on_ioda(const f_ioda& f)         // Set response data handler
{  h_ioda= f; }

void
   on_end(const f_end& f)           // Set completion handler
{  h_end= f; }

void
   on_error(const f_error& f)       // Set connection error handler
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
virtual bool read(Ioda&);           // (Async) read Response data

virtual void write(const void*, size_t); // (Write Server Response data)
virtual void write();               // (Server Response complete)
void write(std::string S)           // (Write Server Response data)
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
virtual bool read(Ioda&);           // (Async) read Response data
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
virtual void write(const void*, size_t); // (Write Server Response data)
virtual void write();               // (Server Response complete)
}; // class ServerRequest
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_RESPONSE_H_INCLUDED
