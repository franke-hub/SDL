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
//       2022/07/16
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
#include <pub/utility.h>            // For pub::to_string, ...

#include "pub/http/Client.h"        // For pub::http::Client
#include "pub/http/Exception.h"     // For pub::http:exceptions
#include "pub/http/Listen.h"        // For pub::http::Listen
#include "pub/http/Request.h"       // For pub::http::Request, implemented
#include "pub/http/Response.h"      // For pub::http::Response // TODO: NEEDED?
#include "pub/http/Server.h"        // For pub::http::Server
#include "pub/http/Stream.h"        // For pub::http::Stream
#include "pub/http/utility.h"       // For namespace pub::http::utility

using namespace _PUB_NAMESPACE;
using namespace _PUB_NAMESPACE::debugging;
using _PUB_NAMESPACE::utility::to_string;
using std::string;

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
// Macros
//----------------------------------------------------------------------------

namespace pub::http {               // Implementation namespace
//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
pub::Statistic         Request::obj_count; // Request object count

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------

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
// Subroutine-
//       dump
//
// Purpose-
//       Storage dump
//
//----------------------------------------------------------------------------
static inline void
   dump(void* addr, size_t size)    // TODO: REMOVE
{
   std::lock_guard<decltype(*Debug::get())> lock(*Debug::get());

   ::pub::utility::dump(stdout, addr, size);
   ::pub::utility::dump(addr, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Request::~Request
//       Request::Request
//
// Purpose-
//       Destructor
//       Constructor
//
//----------------------------------------------------------------------------
   Request::~Request( void )        // Destructor
{  if( HCDM ) debugh("http::~Request(%p)\n", this);

   obj_count.dec();
}

   Request::Request( void )         // Default constructor
:  Options()
,  h_data(utility::f_data())
,  h_end(utility::f_void())
,  h_error(utility::f_error())
{  if( HCDM ) debugh("http::Request(%p)\n", this);

   obj_count.inc();
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
   Request::read(const void*, const size_t) // (ServerRequest only)
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
//       ClientRequest::~ClientRequest
//       ClientRequest::ClientRequest
//       ClientRequest::make
//
// Purpose-
//       Destructor
//       Constructor
//       Creator
//
//----------------------------------------------------------------------------
   ClientRequest::~ClientRequest( void ) // Destructor
{  if( HCDM ) debugh("http::~ClientRequest(%p)\n", this); }

   ClientRequest::ClientRequest( void ) // Default constructor
:  Request()
{  if( HCDM ) debugh("http::ClientRequest(%p)\n", this); }

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
///debugh("%4d %s shared_ptr<ClientStream>(%p)->(%p)\n", __LINE__, __FILE__, &Q->stream, Q->stream.get()); // TODO: REMOVE

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
{  return std::dynamic_pointer_cast<ClientStream>(stream); }
//{  return std::dynamic_pointer_cast<ClientStream>(stream.lock()); }

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

   data.append(addr, size);
}

void
   ClientRequest::write( void )     // Write Request
{  if( HCDM ) debugh("ClientRequest(%p)::write\n", this);

   stream->write();
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerRequest::~ServerRequest
//       ServerRequest::ServerRequest
//       ServerRequest::make
//
// Purpose-
//       Destructors
//       Constructor
//       Creator
//
//----------------------------------------------------------------------------
   ServerRequest::~ServerRequest( void ) // Destructor
{  if( HCDM ) debugh("http::~ServerRequest(%p)\n", this); }

   ServerRequest::ServerRequest( void ) // Default constructor
:  Request()
{  if( HCDM ) debugh("http::ServerRequest(%p)\n", this); }

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
{  return std::dynamic_pointer_cast<ServerStream>(stream); }
//{  return std::dynamic_pointer_cast<ServerStream>(stream.lock()); }

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
     const void*       addr,        // Data address
     size_t            size)        // Data length
{  if( HCDM ) debugh("ServerRequest(%p)::read(%p,%zd)\n", this, addr, size);

   data.append(addr, size);

   // Because we are indirectly called from the Server, this always succeeds
   std::shared_ptr<Server> server= get_stream()->get_server();
   Buffer& buffer= server->get_buffer(); // We borrow the Server's buffer
   buffer.fetch(data);
// debugh("%4d %s HCDM: ", __LINE__, __FILE__); data.debug();   // TODO: REMOVE
// debugh("%4d %s HCDM: ", __LINE__, __FILE__); buffer.debug(); // TODO: REMOVE

   if( fsm == FSM_RESET )
     fsm= FSM_HEAD;

   if( fsm == FSM_HEAD ) {
     // RFC 2616: In the interest of robustness, servers SHOULD ignore any
     // empty lines read where a Request-Line is expected.
     // Note: RFC 7230 DOES NOT specify this action for Start-Lines
     int P= buffer.peek_char();       // (P always used as peek character)
     while( P == '\r' || P == '\n' ) {
       buffer.read_char();
       P= buffer.peek_char();
     }

     // Insure header completion
// debugh("%4d %s HCDM: ", __LINE__, __FILE__); buffer.debug(); // TODO: REMOVE
     for(;;) {
       int C= buffer.read_char();
       if( C == '\n' ) {
         C= buffer.read_char();
         if( C == '\r' )
           C= buffer.read_char();
         if( C == '\n' )
           break;
       }

       if( C == 0 ) {
// debugh("%4d %s HCDM: ", __LINE__, __FILE__); buffer.debug(); // TODO: REMOVE
         if( buffer.offset < buffer.size )
           return false;

         reject(431);               // Header fields too large
         get_stream()->close();
         return true;
       }
     }

     //-----------------------------------------------------------------------
     // Header complete, parse as specified in RFC 7230
     //-----------------------------------------------------------------------
// debugh("%4d %s HCDM: ", __LINE__, __FILE__); buffer.debug(); // TODO: REMOVE
     buffer.offset= 0;
// debugh("%4d %s HCDM: ", __LINE__, __FILE__); buffer.debug(); // TODO: REMOVE
     P= buffer.peek_char();
     while( P == '\r' || P == '\n' ) {
       buffer.read_char();
       P= buffer.peek_char();
     }

     // Parse the Start-Line
// char temp[32];
// memset(temp, 0, sizeof(temp));
// if( buffer.size > 31 ) memcpy(temp, buffer.addr, 31);
// else                   memcpy(temp, buffer.addr, size);

     method= buffer.read_token(" ");
     path= buffer.read_token(" ");
     proto_id= buffer.read_token("\r\n");
// debugh("\nmethod(%s) path(%s) proto_id(%s)\n", method.c_str(), path.c_str(), proto_id.c_str()); // TODO: REMOVE

     // Start-Line validity checks
     if( method == "" || path == "" || proto_id == ""
         || method[0] == ' ' || path[0] == ' ' || proto_id[0] == ' ' ) {
// buffer.debug(); // TODO: REMOVE
// debugf("buffer(%s) method(%s) path(%s) proto_id(%s)\n", temp, method.c_str(), path.c_str(), proto_id.c_str());
       throw stream_error("Invalid Start-Line");
     }

     // Parse Header lines
     for(;;) {
       P= buffer.peek_char();
       if( P == '\r' ) {
         int C= buffer.read_char(); // (C always used as read character)
         C= buffer.read_char();
         if( C != '\n' )
           throw std::runtime_error("Invalid Header-Line: '\\r' w/o '\\n'");
         break;
       }

       std::string name= buffer.read_token(":");
       P= buffer.peek_char();
       if( P == ' ' || P == '\t' ) // Ignore first leading white space char
         buffer.read_char();
       std::string value= buffer.read_token("\r\n");

       // Check for obs-fold in a request message.
       // It's use is deprecated except within a message/http container.
       // TODO: Find out the definition of "messsage/http container."
       // TODO: Do we need to allow this if using HTTP/1.0??
       // TODO: If allowed, the value definition continues
       P= buffer.peek_char();
       if( P == ' ' || P == '\t' )
         throw std::runtime_error("Header-Line obs-fold: {'\\r','\\n',WS}");

       if( name == ""||value == "" || isspace(name[0])||isspace(value[0]) )
         throw std::runtime_error("Invalid Header-Line format");
       insert(name, value);
     }

     // Discard Header data
// buffer.debug(); // TODO: REMOVE
// data.debug(); // TODO: REMOVE
     data.discard(buffer.offset);
// data.debug(); // TODO: REMOVE
//// debugh("%4d Request HCDM %s content_length(%zd)\n", __LINE__, method.c_str(), content_length); // TODO: REMOVE
//// data.debug(); // TODO: REMOVE
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
       get_stream()->close();
       return true;
     }
     if( method != HTTP_POST && method != HTTP_PUT )
       throw std::runtime_error("Content-Length disallowed");

     // TODO: CHECK FOR OVERFLOW
     if( data.get_size() < size_t(content_length) )
       return false;
   } else if( method == HTTP_POST || method == HTTP_PUT ) {
     reject(411);
     get_stream()->close();
     return true;
   }

   // Drive Listen::on_request
   std::shared_ptr<Listen> listen= server->get_listen();
   if( !listen )
     throw "SHOULD NOT OCCUR";    // We have the Listener's Server*, after all

   listen->do_request(this);
   return true;
}
}  // namespace pub::http
