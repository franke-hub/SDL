//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Request.cpp
//
// Purpose-
//       Implement http/Request.h
//
// Last change date-
//       2022/10/23
//
// Implementation notes-
//       TODO: Consider moving ClientRequest::write to Client::write
//             (Since it only buffers the data)
//       TODO: Don't know status of the above, but we need to move write
//             to Stream::write for both Clients and Servers.
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <cstring>                  // For memset
#include <ostream>                  // For std::ostream
#include <stdexcept>                // For std::out_of_range, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <ctype.h>                  // For isspace
#include <stdio.h>                  // For fprintf
#include <stdint.h>                 // For integer types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Statistic.h>          // For pub::Statistic
#include <pub/utility.h>            // For pub::to_string, ...

#include "pub/http/Client.h"        // For pub::http::Client
#include "pub/http/Exception.h"     // For pub::http:exceptions
#include "pub/http/Ioda.h"          // For pub::http::Ioda
#include "pub/http/Listen.h"        // For pub::http::Listen
#include "pub/http/Request.h"       // For pub::http::Request, implemented
#include "pub/http/Response.h"      // For pub::http::Response
#include "pub/http/Server.h"        // For pub::http::Server
#include "pub/http/Stream.h"        // For pub::http::Stream

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::utility::dump;
using PUB::utility::to_string;
using PUB::utility::visify;
using std::string;

namespace _LIBPUB_NAMESPACE::http { // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?

,  POST_LIMIT= 1'048'576            // POST/PUT size limit
}; // enum

enum FSM                            // Finite State Machine states
{  FSM_RESET                        // Initial reset state
,  FSM_HEAD                         // Reading Headers
,  FSM_BODY                         // Reading Body
}; // enum FSM

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
statistic::Active      Request::obj_count; // Request object count

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
// Imported Options TODO: REMOVE UNUSED
typedef const char CC;
static constexpr CC*   HTTP_SIZE= Options::HTTP_HEADER_LENGTH;

static constexpr CC*   HTTP_POST=  Options::HTTP_METHOD_POST;
static constexpr CC*   HTTP_PUT=   Options::HTTP_METHOD_PUT;

//----------------------------------------------------------------------------
//
// Method-
//       Request::Request
//       Request::~Request
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   Request::Request( void )         // Default constructor
:  Options()
,  h_ioda([](Ioda&) { })
,  h_end([]( void ) { })
,  h_error([](string) { })
{  if( HCDM ) debugh("http::Request(%p)!\n", this);

   obj_count.inc();
}

   Request::~Request( void )        // Destructor
{  if( HCDM ) debugh("http::Request(%p)~\n", this);

   obj_count.dec();
}

//----------------------------------------------------------------------------
//
// Method-
//       Request::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Request::debug(const char* info) const // Debugging display
{  debugh("Request(%p)::debug(%s)\n", this, info);

   Options::debug(info);
}

//----------------------------------------------------------------------------
// Request::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<Response>
   Request::get_response( void ) const
{
   std::shared_ptr<Stream> stream= get_stream();
   if( stream )
     return stream->get_response();

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Request::end
//
// Purpose-
//       Terminate the Request
//
//----------------------------------------------------------------------------
void
   Request::end( void )             // Terminate the Request
{  if( HCDM ) debugh("Request(%p)::end\n", this);

   h_end();                         // Drive Request::on_end
   stream= nullptr;                 // Remove Stream reference
}

//----------------------------------------------------------------------------
//
// Method-
//       Request::reject
//
// Purpose-
//       Reject the Request
//
//----------------------------------------------------------------------------
void
   Request::reject(int code)        // Reject the Request
{  if( HCDM ) debugh("Request(%p)::reject(%d)\n", this, code);

   stream->reject(code);
   stream->close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Request::read
//       Request::write
//
// Purpose-
//       Virtual methods; only implemented in ServerRequest
//       Virtual methods; only implemented in ClientRequest
//
//----------------------------------------------------------------------------
bool                                // Return code, TRUE when complete
   Request::read(Ioda&)             // (ServerRequest only)
{  utility::should_not_occur(__LINE__, __FILE__); return true; }

void
   Request::write(const void*, const size_t) // (ClientRequest only)
{  utility::should_not_occur(__LINE__, __FILE__); }

void
   Request::write()                // (ClientRequest only)
{  utility::should_not_occur(__LINE__, __FILE__); }

//----------------------------------------------------------------------------
//
// Method-
//       ClientRequest::ClientRequest
//       ClientRequest::~ClientRequest
//       ClientRequest::make
//
// Purpose-
//       Constructor
//       Destructor
//       Creator
//
//----------------------------------------------------------------------------
   ClientRequest::ClientRequest( void ) // Default constructor
:  Request()
{  if( HCDM ) debugh("http::ClientRequest(%p)!\n", this);
   INS_DEBUG_OBJ("ClientRequest");
}

   ClientRequest::~ClientRequest( void ) // Destructor
{  if( HCDM ) debugh("http::ClientRequest(%p)~\n", this);
   REM_DEBUG_OBJ("ClientRequest");
}

std::shared_ptr<ClientRequest>      // The ClientRequest
   ClientRequest::make(             // Get ClientRequest
     ClientStream*     owner,       // For this ClientStream
     const Options*    opts)        // With these Options
{
   std::shared_ptr<Client> client= owner->get_client();
   if( !client )
     utility::should_not_occur(__LINE__, __FILE__); // TODO: Verify should not occur

   std::shared_ptr<ClientRequest> Q= std::make_shared<ClientRequest>();
   Q->self= Q;                      // Set self-reference
   Q->stream= owner->get_self();    // Set ourClientStream

   // Extract options
   typedef Options::const_iterator iterator;
   Q->method= ".";                  // (Default, invalid method)
   Q->path= ".";                    // (Default, invalid path)
   Q->proto_id= client->get_proto_id();
   if( opts ) {
     for(iterator i= opts->begin(); i != opts->end(); ++i)
       Q->insert(i->first, i->second);
   }

   if( HCDM )
     debugh("%p= http::ClientRequest::make(%p)\n", Q.get(), owner);
   return Q;
}

//----------------------------------------------------------------------------
// ClientRequest::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<ClientResponse>
   ClientRequest::get_response( void ) const
{
   std::shared_ptr<ClientStream> stream= get_stream();
   if( stream )
     return stream->get_response();

   return nullptr;
}

std::shared_ptr<ClientStream>
   ClientRequest::get_stream( void ) const
{
   return std::dynamic_pointer_cast<ClientStream>(stream);
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientRequest::end
//
// Purpose-
//       Terminate the Request
//
//----------------------------------------------------------------------------
void
   ClientRequest::end( void )       // Terminate the Request
{  if( HCDM ) debugh("ClientRequest(%p)::end\n", this);

   h_end();                         // Drive Request::on_end
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientRequest::write
//
// Purpose-
//       Write the Request
//
//----------------------------------------------------------------------------
void
   ClientRequest::write(            // Write POST/PUT data
     const void*       addr,        // Data address
     size_t            size)        // Data length
{  if( HCDM ) debugh("ClientRequest(%p)::write(%p,%zd)\n", this, addr, size);

   ioda.write(addr, size);
}

void
   ClientRequest::write( void )     // Write Request
{  if( HCDM ) debugh("ClientRequest(%p)::write\n", this);

   stream->write();
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerRequest::ServerRequest
//       ServerRequest::~ServerRequest
//       ServerRequest::make
//
// Purpose-
//       Constructors
//       Destructor
//       Creator
//
//----------------------------------------------------------------------------
   ServerRequest::ServerRequest( void ) // Default constructor
:  Request()
{  if( HCDM ) debugh("http::ServerRequest(%p)!\n", this);
   INS_DEBUG_OBJ("ServerRequest");
}

   ServerRequest::~ServerRequest( void ) // Destructor
{  if( HCDM ) debugh("http::~ServerRequest(%p)~\n", this);

   REM_DEBUG_OBJ("ServerRequest");
}

std::shared_ptr<ServerRequest>      // The ServerRequest
   ServerRequest::make(             // Get ServerRequest
     ServerStream*     owner,       // For this ServerStream
     const Options*    opts)        // With these Options
{
   std::shared_ptr<Server> server= owner->get_server();
   if( !server )
     utility::should_not_occur(__LINE__, __FILE__); // TODO: Verify should not occur

   std::shared_ptr<ServerRequest> Q= std::make_shared<ServerRequest>();
   Q->self= Q;                      // Set self-reference
   Q->stream= owner->get_self();    // Set our ServerStream

   // Extract options
   (void)opts; // TODO: NOT CODED YET

   if( HCDM )
     debugh("%p= http::Request::make()\n", Q.get());
   return Q;
}

//----------------------------------------------------------------------------
// ServerRequest::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<ServerStream>
   ServerRequest::get_stream( void ) const
{
   return std::dynamic_pointer_cast<ServerStream>(stream);
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerRequest::end
//
// Purpose-
//       Terminate the Request
//
//----------------------------------------------------------------------------
void
   ServerRequest::end( void )       // Terminate the Request
{  if( HCDM ) debugh("ServerRequest(%p)::end\n", this);

   ioda.reset();                    // (Release I/O Data Area pages)
   h_end();                         // Drive Request::on_end
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerRequest::read
//
// Purpose-
//       Read the Request
//
// Implementation notes-
//       Called via Server->ServerStream->ServerRequest.
//       TODO: Handle the Request from HTTP2 Stream
//       TODO: Error recovery if headers not complete within Buffer.size
//       TODO: Error recovery if Stream error
//
//----------------------------------------------------------------------------
bool                                // Return code, TRUE when complete
   ServerRequest::read(             // Read data
     Ioda&             data)        // The I/O Data Area
{  if( HCDM )
     debugh("ServerRequest(%p)::read(*,%zd)\n", this, data.get_used());

   ioda += std::move(data);

   std::shared_ptr<ServerStream> stream= get_stream();
   std::shared_ptr<Server> server= stream->get_server();
   if( server.get() == nullptr )
     return true;

   if( fsm == FSM_RESET )
     fsm= FSM_HEAD;

   IodaReader reader(ioda);         // The Ioda Reader
   if( fsm == FSM_HEAD ) {
     // RFC 2616: In the interest of robustness, servers SHOULD ignore any
     // empty lines read where a Request-Line is expected.
     // Note: RFC 7230 DOES NOT specify this action for Start-Lines
     int P= reader.peek();          // (P always used as peek character)
     while( P == '\r' || P == '\n' ) {
       reader.get();
       P= reader.peek();
     }

     // Insure header completion
// debugh("%4d %s HCDM: ", __LINE__, __FILE__); reader.debug(); // TODO: REMOVE
     for(;;) {
       int C= reader.get();
       if( C == '\n' ) {
         C= reader.get();
         if( C == '\r' )
           C= reader.get();
         if( C == '\n' )
           break;
       }

       if( C == EOF )
         return false;
     }

     //-----------------------------------------------------------------------
     // Header complete, parse as specified in RFC 7230
     //-----------------------------------------------------------------------
     reader.set_offset(0);
     P= reader.peek();
     while( P == '\r' || P == '\n' ) {
       reader.get();
       P= reader.peek();
     }

     // Parse the Start-Line
     method= reader.get_token(" ");
     path= reader.get_token(" ");
     proto_id= reader.get_token("\r\n");

     // Start-Line validity checks
     if( method == "" || path == "" || proto_id == ""
         || method[0] == ' ' || path[0] == ' ' || proto_id[0] == ' ' ) {
debugh("\nmethod(%s) path(%s) proto_id(%s)\n", method.c_str(), path.c_str(), proto_id.c_str()); // TODO: REMOVE
       throw stream_error("Invalid Start-Line");
     }

     // Parse Header lines
     for(;;) {
       P= reader.peek();
       if( P == '\r' ) {
         int C= reader.get();       // (C always used as read character)
         C= reader.get();
         if( C != '\n' )
           throw std::runtime_error("Invalid Header-Line: '\\r' w/o '\\n'");
         break;
       }

       std::string name= reader.get_token(":");
       P= reader.peek();
       if( P == ' ' || P == '\t' ) // Ignore first leading white space char
         reader.get();
       std::string value= reader.get_token("\r\n");

       // Check for obs-fold in a request message.
       // It's use is deprecated except within a message/http container.
       // TODO: Find out the definition of "messsage/http container."
       // TODO: Do we need to allow this if using HTTP/1.0??
       // TODO: If allowed, the value definition continues
       P= reader.peek();
       if( P == ' ' || P == '\t' )
         throw std::runtime_error("Header-Line obs-fold: {'\\r','\\n',WS}");

       if( name == ""||value == "" || isspace(name[0])||isspace(value[0]) ) {
debugf("Request name(%s) value(%s)\n", name.c_str(), value.c_str());
         throw std::runtime_error("Invalid Header-Line format");
       }
       insert(name, value);
     }

     // Discard Header data
     size_t offset= reader.get_offset();
     ioda.discard(offset);
     reader.set_offset(0);
     fsm= FSM_BODY;
   }

   //-------------------------------------------------------------------------
   // Load POST/PUT data
   //-------------------------------------------------------------------------
   const char* value= locate(HTTP_SIZE);
   if( value ) {
     ssize_t content_length= atol(value);
     if( content_length < 0 || content_length > POST_LIMIT ) {
       reject(413);
       return true;
     }
     if( method != HTTP_POST && method != HTTP_PUT ) {
       reject(400);
       return true;
     }

     if( (ioda.get_used()) < size_t(content_length) )
       return false;
   } else if( method == HTTP_POST || method == HTTP_PUT ) {
     reject(411);
     return true;
   }

   // Drive Listen::on_request
   stream->set_record(TimingRecord::IX_SRV_REQ_DO);
   server->get_listen()->do_request(this);
   stream->set_record(TimingRecord::IX_SRV_REQ_DONE);
   return true;
}
}  // namespace _LIBPUB_NAMESPACE::http
