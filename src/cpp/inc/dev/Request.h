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
//       2022/10/03
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_REQUEST_H_INCLUDED
#define _LIBPUB_HTTP_REQUEST_H_INCLUDED

#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr
#include <string>                   // For std::string

#include <pub/Statistic.h>          // For pub::Statistic

#include "pub/http/Ioda.h"          // For pub::http::Ioda
#include "pub/http/Options.h"       // For pub::http::Options, super class

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Client;
class ClientResponse;
class ClientStream;
class Request;
class Response;
class Server;
class ServerResponse;
class ServerStream;
class Stream;

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
// Request::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::string    string;      // (Using std::string)

// Callback handlers
typedef std::function<void(Ioda&)>            f_ioda;
typedef std::function<void(void)>             f_end;
typedef std::function<void(const string&)>    f_error;

//----------------------------------------------------------------------------
// Request::Attributes
//----------------------------------------------------------------------------
static Statistic       obj_count;   // Request counter

string                 method;      // Request method
string                 path;        // Request path
string                 proto_id;    // Request protocol/version

protected:
std::weak_ptr<Request> self;        // Self reference
std::shared_ptr<Stream>stream;      // Associated Stream

Ioda                   ioda;        // I/O Data Area
int                    fsm= 0;      // Finite State Machine

// Callback handlers
f_ioda                 h_ioda;      // The Request data handler
f_end                  h_end;       // The Request completion handler
f_error                h_error;     // The Request connection error handler

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
Ioda&                               // The associated Ioda
   get_ioda( void )                 // Get associated Ioda
{  return ioda; }

std::shared_ptr<Response>
   get_response( void ) const;

std::shared_ptr<Request>
   get_self( void ) const
{  return self.lock(); }

std::shared_ptr<Stream>
   get_stream( void ) const
{  return stream; }

void
   on_ioda(const f_ioda& f)         // Set Request data handler
{  h_ioda= f; }

void
   on_end(const f_end& f)           // Set completion handler
{  h_end= f; }

void
   on_error(const f_error& f)       // Set connection error handler
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
virtual bool read(Ioda&);           // (Async) read Request data

virtual void write(const void*, size_t); // (Write Client Request data)
virtual void write();               // (Client Request complete)
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
virtual void end( void );           // Complete the ClientRequest

virtual void write(const void*, size_t); // (Write Client Request data)
virtual void write();               // (Client Request complete)
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
//size_t                 ioda_off;    // I/O Data Area read offset

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

virtual bool read(Ioda&);           // (Async) read Request data
}; // class ServerRequest
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_REQUEST_H_INCLUDED
