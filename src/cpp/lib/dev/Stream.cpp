//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Stream.cpp
//
// Purpose-
//       Implement http/Stream.h
//
// Last change date-
//       2023/06/27
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <atomic>                   // For std::atomic
#include <cstring>                  // For memset
#include <stdexcept>                // For std::runtime_error, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <stdio.h>                  // For fprintf
#include <stdint.h>                 // For integer types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For namespace pub::dispatch
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Statistic.h>          // For pub::Active_record
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::to_string, ...

#include "pub/http/Client.h"        // For pub::http::Client
#include "pub/http/HTTP.h"          // For pub::http::HTTP
#include "pub/http/Ioda.h"          // For pub::http::Ioda
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Request.h"       // For pub::http::Request
#include "pub/http/Response.h"      // For pub::http::Response
#include "pub/http/Server.h"        // For pub::http::Server
#include "pub/http/Stream.h"        // For pub::http::Stream, implemented

using namespace _LIBPUB_NAMESPACE;
using namespace _LIBPUB_NAMESPACE::debugging;
using _LIBPUB_NAMESPACE::utility::to_string;
using _LIBPUB_NAMESPACE::utility::visify;
using std::string;

namespace _LIBPUB_NAMESPACE::http { // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  BUFFER_SIZE= 8'096               // Input buffer size (Header collector)
,  POST_LIMIT= 1'048'576            // POST/PUT size limit
,  USE_ITRACE= true                 // Use internal trace?
,  USE_REPORT= true                 // Use event Reporter?
}; // enum

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
// Imported Options
typedef const char CC;              // (Shorthand)
static constexpr CC*   HTTP_POST= Options::HTTP_METHOD_POST;
static constexpr CC*   HTTP_PUT=  Options::HTTP_METHOD_PUT;
static constexpr CC*   HTTP_SIZE= Options::HTTP_HEADER_LENGTH;

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
statistic::Active      Stream::obj_count; // Stream object count

//----------------------------------------------------------------------------
// Event reporting
//----------------------------------------------------------------------------
static Active_record   stream_count("Stream"); // Stream counter

namespace {
static struct StaticGlobal {
   StaticGlobal(void)               // Constructor
{
   if( USE_REPORT ) {
     stream_count.insert();
   }
}

   ~StaticGlobal(void)              // Destructor
{
   if( USE_REPORT ) {
     stream_count.remove();
   }
}
}  staticGlobal;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Method-
//       Stream::Stream
//       Stream::~Stream
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Stream::Stream( void )           // Default constructor
:  h_end([]() {})
,  h_error([](const string&) {})
{  if( HCDM || VERBOSE > 1 ) debugh("Stream(%p)!\n", this);

   obj_count.inc();

   if( USE_REPORT )
     stream_count.inc();
}

   Stream::~Stream( void )          // Destructor
{  if( HCDM || VERBOSE > 1 ) debugh("Stream(%p)~\n", this);

   obj_count.dec();

   if( USE_REPORT )
     stream_count.dec();
}

//----------------------------------------------------------------------------
//
// Method-
//       Stream::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Stream::debug(const char* info) const  // Debugging display
{  debugf("Stream(%p)::debug(%s)\n", this, info);

   debugf("..self(%p) request(%p) response(%p)\n"
         , get_self().get(), request.get(), response.get());
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientStream::ClientStream
//       ClientStream::~ClientStream
//       ClientStream::make
//
// Purpose-
//       Constructors
//       Destructor
//       Creators
//
//----------------------------------------------------------------------------
   ClientStream::ClientStream(      // Constructor
     Client*           owner)       // Associated Client
:  Stream()
,  client(owner->get_self())
{  if( HCDM || VERBOSE > 0 ) debugh("ClientStream(%p)!(%p)\n", this, owner);
   if( USE_ITRACE )
     Trace::trace(".NEW", "CSTR", this);

   INS_DEBUG_OBJ("ClientStream");
}

   ClientStream::~ClientStream( void ) // Destructor
{  if( HCDM || VERBOSE > 0 ) debugh("ClientStream(%p)~\n", this);
   if( USE_ITRACE )
     Trace::trace(".DEL", "CSTR", this);

   REM_DEBUG_OBJ("ClientStream");
}

std::shared_ptr<ClientStream>       // The ClientStream
   ClientStream::make(              // Create ClientStream
     Client*           owner,       // Associated Client
     const Options*    opts)        // Associated Request Options
{
   std::shared_ptr<ClientStream> S= std::make_shared<ClientStream>(owner);
   S->self= S;
   S->request=  ClientRequest::make(S.get(), opts);
   S->response= ClientResponse::make(S.get());

   if( HCDM )
     debugh("%p= ClientStream::make(%p,%p)\n", S.get(), owner, opts);

   return S;
}

//----------------------------------------------------------------------------
// ClientStream::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<ClientRequest>
   ClientStream::get_request( void ) // Get ClientRequest
{  return std::dynamic_pointer_cast<ClientRequest>(request); }

std::shared_ptr<ClientResponse>
   ClientStream::get_response( void ) // Get ClientResponse
{  return std::dynamic_pointer_cast<ClientResponse>(response); }

//----------------------------------------------------------------------------
//
// Method-
//       ClientStream::end
//
// Purpose-
//       Terminate the ClientStream
//
//----------------------------------------------------------------------------
void
   ClientStream::end( void )        // Terminate the ClientStream
{  if( HCDM ) debugh("ClientStream(%p)::end\n", this);

   std::shared_ptr<Stream> stream= get_self(); // Stream keep-alive
   if( response )                   // (Can be nullptr if end already called)
     get_response()->end();         // Complete the Response
   if( request )
     get_request()->end();          // Complete the Request

   h_end();                         // Drive Stream::on_end
   response= nullptr;               // Remove Response reference
   request= nullptr;                // Remove Request reference
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientStream::read (Asynchronous) I/O Method
//
// Purpose-
//       (Asynchronously) read data segment from Client.
//
// Implementation note-
//       Field response is protected, so Client can't call response->read.
//
//----------------------------------------------------------------------------
bool                                // Return code: TRUE if complete
   ClientStream::read(              // (Async) read data segment
     Ioda&             ioda)        // I/O Data Area
{
   bool cc= get_response()->read(ioda);
   if( HCDM )
     debugh("%d= ClientStream(%p)::read(*,%zd)\n", cc, this, ioda.get_used());

   return cc;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientStream::write (I/O Method)
//
// Purpose-
//       Write data segment response
//
//----------------------------------------------------------------------------
void
   ClientStream::write( void )      // Write data (completed)
{  if( HCDM ) debugh("ClientStream(%p)::write\n", this);

   std::shared_ptr<Client> client= get_client();
   int rc= -1;                      // Default, rejected
   if( client )
     rc= client->write(this);

   if( rc ) {
     debugh("%4d ClientStream(%p)::write failure\n", __LINE__, this);
     h_error("Client write failure");
     end();
   }
}

void
   ClientStream::write(Ioda&)       // Write data segment to stream
{  utility::checkstop(__LINE__, __FILE__, "Should not occur"); }

//----------------------------------------------------------------------------
//
// Method-
//       ServerStream::ServerStream
//       ServerStream::~ServerStream
//       ServerStream::make
//
// Purpose-
//       Constructors
//       Destructor
//       Creators
//
//----------------------------------------------------------------------------
   ServerStream::ServerStream(      // Constructor
     Server*           owner)       // Associated Server
:  Stream()
,  server(owner->get_self())
{  if( HCDM || VERBOSE > 0 ) debugh("ServerStream(%p)!(%p)\n", this, owner);
   if( USE_ITRACE )
     Trace::trace(".NEW", "SSTR", this);

   INS_DEBUG_OBJ("ServerStream");
}

   ServerStream::~ServerStream( void ) // Destructor
{  if( HCDM || VERBOSE > 0 ) debugh("ServerStream(%p)~\n", this);
   if( USE_ITRACE )
     Trace::trace(".DEL", "SSTR", this);

   REM_DEBUG_OBJ("ServerStream");
}

std::shared_ptr<ServerStream>       // The ServerStream
   ServerStream::make(              // Create ServerStream
     Server*           owner)       // Associated Server
{
   std::shared_ptr<ServerStream> S= std::make_shared<ServerStream>(owner);
   S->self= S;
   S->request=  ServerRequest::make(S.get());
   S->response= ServerResponse::make(S.get());

   if( HCDM )
     debugh("%p= ServerStream::make(%p)\n", S.get(), owner);

   if( S->request.get() == nullptr || S->response.get() == nullptr )
     S= nullptr;

   return S;
}

//----------------------------------------------------------------------------
// ServerStream::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<ServerRequest>
   ServerStream::get_request( void ) // Get ServerRequest
{  return std::dynamic_pointer_cast<ServerRequest>(request); }

std::shared_ptr<ServerResponse>
   ServerStream::get_response( void ) // Get ServerResponse
{  return std::dynamic_pointer_cast<ServerResponse>(response); }

//----------------------------------------------------------------------------
//
// Method-
//       ServerStream::end
//
// Purpose-
//       Terminate the ServerStream
//
//----------------------------------------------------------------------------
void
   ServerStream::end( void )        // Terminate the ServerStream
{  if( HCDM ) debugh("ServerStream(%p)::end\n", this);

INS_DEBUG_OBJ("SS.end");
   std::shared_ptr<Stream> stream= get_self(); // Stream keep-alive
   if( response )                   // (Can be nullptr if end already called)
     get_response()->end();         // Complete the Response
   if( request )
     get_request()->end();          // Complete the Request

   h_end();                         // Drive Stream::on_end
   response= nullptr;               // Remove Response reference
   request= nullptr;                // Remove Request reference
REM_DEBUG_OBJ("SS.end");
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerStream::read (Asynchronous) I/O Method
//
// Purpose-
//       (Asynchronously) read data segment from server.
//
//----------------------------------------------------------------------------
bool                                // Return code: TRUE if complete
   ServerStream::read(              // (Async) read data segment
     Ioda&             ioda)        // I/O Data Area
{  if( HCDM )
     debugh("ServerStream(%p)::read(*,%zd)\n", this, ioda.get_used());

   return get_request()->read(ioda);
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerStream::write (I/O Method)
//
// Purpose-
//       Write data segment response
//
//----------------------------------------------------------------------------
void
   ServerStream::write(             // Write to Server
     int               line,        // Caller's line number
     const void*       addr,        // Data address
     size_t            size)        // Data length
{  if( HCDM )
     debugh("%4d ServerStream(%p)::write(%p,%zd)\n", line, this, addr, size);

   Ioda ioda;
   ioda.write(addr, size);
   write(ioda);
}

void
   ServerStream::write( void )      // Write data (completed)
{  utility::checkstop(__LINE__, __FILE__, "Should not occur"); }

void
   ServerStream::write(Ioda& ioda) // Write data segment to stream
{
   std::shared_ptr<Server> server= get_server();
   if( server )
     server->write(ioda);
   else
     ioda.reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerStream::reject
//
// Purpose-
//       Reject a Request
//
//----------------------------------------------------------------------------
void
   ServerStream::reject(            // Reject a Request, writing Response
     int               code)        // The rejection status code
{  if( HCDM )
     debugh("\nServerStream(%p)::reject(%d) %s\n\n", this, code
           , HTTP::status_text(code));

   char buff[128];
   size_t L= sprintf(buff, "HTTP/1.1 %.3d %s\r\n\r\n", code
                         , HTTP::status_text(code));

   response->set_code(code);
   response->get_ioda().reset();
   write(buff, L);
   end();
}
}  // namespace _LIBPUB_NAMESPACE::http
