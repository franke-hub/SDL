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
//       Response.cpp
//
// Purpose-
//       Implement http/Response.h
//
// Last change date-
//       2022/07/16
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <cstring>                  // For memset
#include <mutex>                    // For std::lock_guard TODO: REMOVE
#include <ostream>                  // For std::ostream
#include <stdexcept>                // For std::out_of_range, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <stdio.h>                  // For fprintf
#include <stdint.h>                 // For integer types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/utility.h>            // For pub::to_string, ...

#include "pub/http/Client.h"        // For pub::http::Client
#include "pub/http/Exception.h"     // For pub::http:exceptions
#include "pub/http/Request.h"       // For pub::http::Request // TODO: NEEDED?
#include "pub/http/Response.h"      // For pub::http::Response, implemented
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

,  RESP_LIMIT= 1'048'576            // Response size limit
}; // enum

enum FSM                            // Finite State Machine states
{  FSM_RESET                        // Initial reset state
,  FSM_HEAD                         // Reading Headers
,  FSM_BODY                         // Reading Body
}; // enum FSM

//----------------------------------------------------------------------------
// Namespace pub::http
//----------------------------------------------------------------------------
namespace pub::http {               // Implementation namespace
//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
pub::Statistic         Response::obj_count; // Response object count

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
// Imported Options TODO: REMOVE UNUSED
typedef const char CC;
static constexpr CC*   HTTP_SIZE= Options::HTTP_HEADER_LENGTH;

static constexpr CC*   HTTP_HEAD=  Options::HTTP_METHOD_HEAD;

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
//       Response::~Response
//       Response::Response
//       Response::make
//
// Purpose-
//       Destructor
//       Constructors
//       Creator
//
//----------------------------------------------------------------------------
   Response::~Response( void )      // Destructor
{  if( HCDM ) debugh("Response(%p)::~Response\n", this);

   obj_count.dec();
}

   Response::Response( void )       // Default constructor
:  Options()
,  h_data(utility::f_data())
,  h_end(utility::f_void())
,  h_error(utility::f_error())
{  if( HCDM ) debugh("Response(%p)::Response\n", this);

   obj_count.inc();
}

//----------------------------------------------------------------------------
//
// Method-
//       Response::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Response::debug(const char* info) const // Debugging display
{  debugh("Response(%p)::debug(%s)\n", this, info);
}

//----------------------------------------------------------------------------
// Response::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<Request>
   Response::get_request( void ) const
{
   std::shared_ptr<Stream> stream= get_stream();
   if( stream )
     return stream->get_request();

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Response::end
//
// Purpose-
//       Terminate the Response
//
//----------------------------------------------------------------------------
void
   Response::end( void )            // Terminate the Response
{  if( HCDM ) debugh("Response(%p)::end\n", this);

   h_end();                         // Drive Response::on_end
   stream= nullptr;                 // Remove Stream reference
}

//----------------------------------------------------------------------------
//
// Method-
//       Response::reject
//
// Purpose-
//       Reject the Response
//
//----------------------------------------------------------------------------
void
   Response::reject(                // Reject the Response
     int               code)        // With this error code
{  if( HCDM ) debugh("Response(%p)::reject(%d)\n", this, code);

   utility::not_coded_yet(__LINE__, __FILE__);
}

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
bool Response::read(const void*, const size_t) // (ClientResponse only)
{  utility::should_not_occur(__LINE__, __FILE__); return false; }

void
   Response::write(const void*, const size_t) // (ServerResponse only)
{  utility::should_not_occur(__LINE__, __FILE__); }

void
   Response::write()                // (ServerResponse only)
{  utility::should_not_occur(__LINE__, __FILE__); }

//----------------------------------------------------------------------------
//
// Method-
//       ClientResponse::~ClientResponse
//       ClientResponse::ClientResponse
//       ClientResponse::make
//
// Purpose-
//       Destructor
//       Constructor
//       Creator
//
//----------------------------------------------------------------------------
   ClientResponse::~ClientResponse( void ) // Destructor
{  if( HCDM ) debugh("http::~ClientResponse(%p)\n", this); }

   ClientResponse::ClientResponse( void ) // Default constructor
:  Response()
{  if( HCDM ) debugh("http::ClientResponse(%p)\n", this); }

std::shared_ptr<ClientResponse>     // The ClientResponse
   ClientResponse::make(            // Get ClientResponse
     ClientStream*     owner,       // For this ClientStream
     const Options*    opts)        // With these Options
{
   std::shared_ptr<Client> client= owner->get_client();
   if( !client )
     utility::should_not_occur(__LINE__, __FILE__); // TODO: Verify correctness

   std::shared_ptr<ClientResponse> Q= std::make_shared<ClientResponse>();
   Q->self= Q;                      // Set self-reference
   Q->stream= owner->get_self();    // Set ourClientStream
///debugh("%4d %s shared_ptr<ClientStream>(%p)->(%p)\n", __LINE__, __FILE__, &Q->stream, Q->stream.get()); // TODO: REMOVE

   // Extract options
   if( opts )
     Q->append(*opts);

   if( HCDM )
     debugh("%p= http::ClientResponse::make(%p)\n", Q.get(), owner);
   return Q;
}

//----------------------------------------------------------------------------
// ClientResponse::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<ClientRequest>
   ClientResponse::get_request( void ) const
{
   std::shared_ptr<ClientStream> stream= get_stream();
   if( stream )
     return stream->get_request();

   return nullptr;
}

std::shared_ptr<ClientStream>
   ClientResponse::get_stream( void ) const
{  return std::dynamic_pointer_cast<ClientStream>(stream); }
//{  return std::dynamic_pointer_cast<ClientStream>(stream.lock()); }

//----------------------------------------------------------------------------
//
// Method-
//       ClientResponse::read
//
// Purpose-
//       Read the Response
//
//----------------------------------------------------------------------------
bool                                // TRUE if read complete
   ClientResponse::read(            // Read
     const void*       addr,        // Data address
     size_t            size)        // Data length
{  if( HCDM )
     debugh("ClientResponse(%p)::read({%p,%zd})\n", this, addr, size);

   data.append(addr, size);         // Append the read segment

   // Because we are indirectly called from the Client, this always succeeds
   std::shared_ptr<Client> client= get_stream()->get_client();
   Buffer& buffer= client->get_buffer(); // Borrow the Client's input buffer
   buffer.fetch(data);
// traceh("%4d %s HCDM\n", __LINE__, __FILE__); // data.debug(); // TODO: REMOVE
// dump(buffer.addr, buffer.length);

   if( fsm == FSM_RESET )
     fsm= FSM_HEAD;

   if( fsm == FSM_HEAD ) {
// traceh("%4d %s HCDM\n", __LINE__, __FILE__);
     // RFC 2616: In the interest of robustness, servers SHOULD ignore any
     // empty lines read where a Request-Line is expected.
     // Note: RFC 7230 DOES NOT specify this action for Start-Lines
     int P= buffer.peek_char();       // (P always used as peek character)
     while( P == '\r' || P == '\n' ) {
       buffer.read_char();
       P= buffer.peek_char();
     }

     // Insure header completion
// traceh("%4d %s HCDM\n", __LINE__, __FILE__); // buffer.debug(); // TODO: REMOVE
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
// traceh("%4d %s HCDM\n", __LINE__, __FILE__); // buffer.debug(); // TODO: REMOVE
// dump(buffer.addr, buffer.length);
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
// traceh("%4d %s HCDM\n", __LINE__, __FILE__);
// traceh("%4d %s HCDM: ", __LINE__, __FILE__); buffer.debug(); // TODO: REMOVE
     buffer.offset= 0;
// traceh("%4d %s HCDM: ", __LINE__, __FILE__); buffer.debug(); // TODO: REMOVE
     P= buffer.peek_char();
     while( P == '\r' || P == '\n' ) {
       buffer.read_char();
       P= buffer.peek_char();
     }

     // Parse the Start-Line
// char temp[32];
// memset(temp, 0, sizeof(temp));
// if( buffer.size > 31 ) memcpy(temp, buffer.addr, 31);
// else                   memcpy(temp, buffer.addr, buffer.size);

     string protocol= buffer.read_token(" ");
     string status= buffer.read_token(" ");
     string message= buffer.read_token("\r\n");
// traceh("\nprotocol(%s) status(%s) message(%s)\n", protocol.c_str(), status.c_str(), message.c_str()); // TODO: REMOVE

     // Start-Line validity checks
     if( protocol == "" || status == "" || message == "" ) {
// buffer.debug(); // TODO: REMOVE
// debugf("temp(%s) method(%s) path(%s) proto_id(%s)\n", temp, protocol.c_str(), status.c_str(), message.c_str());
       throw stream_error("Invalid Start-Line");
     }
     code= atoi(status.c_str());    // TODO: IMPROVE ROBUSTNESS

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

       // Check for obs-fold in a response message.
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
     fsm= FSM_BODY;
   }

   //-------------------------------------------------------------------------
   // Load response data
   //-------------------------------------------------------------------------
// traceh("%4d %s HCDM (Load Response Data)\n", __LINE__, __FILE__);
   std::shared_ptr<ClientRequest> Q= get_request();
   const char* value= locate(HTTP_SIZE);
   if( value ) {
// traceh("%4d %s HCDM\n", __LINE__, __FILE__);
     ssize_t content_length= atol(value);
// traceh("%4d %s HCDM %s want(%zd) have(%zd)\n", __LINE__, __FILE__, Q->method.c_str(), content_length, data.get_size()); // TODO: REMOVE
// data.debug(); // TODO: REMOVE
     if( content_length < 0 || content_length > RESP_LIMIT ) {
       reject(413);
       get_stream()->close();
       return true;
     }

     // TODO: CHECK FOR OVERFLOW
     if( data.get_size() < size_t(content_length) && Q->method != HTTP_HEAD ) {
// traceh("%4d %s HCDM %s have(%zd) < want(%zd) (get more)\n", __LINE__, __FILE__, Q->method.c_str(), data.get_size(), content_length);
       return false;
     }
   }

// traceh("%4d %s HCDM (ClientResponse::read complete)\n", __LINE__, __FILE__);
   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerResponse::~ServerResponse
//       ServerResponse::ServerResponse
//       ServerResponse::make
//
// Purpose-
//       Destructors
//       Constructor
//       Creator
//
//----------------------------------------------------------------------------
   ServerResponse::~ServerResponse( void ) // Destructor
{  if( HCDM ) debugh("http::~ServerResponse(%p)\n", this); }

   ServerResponse::ServerResponse( void ) // Default constructor
:  Response()
{  if( HCDM ) debugh("http::ServerResponse(%p)\n", this); }

std::shared_ptr<ServerResponse>     // The ServerResponse
   ServerResponse::make(            // Get ServerResponse
     ServerStream*     owner,       // For this ServerStream
     const Options*    opts)        // And these Options
{
   std::shared_ptr<Server> server= owner->get_server();
   if( !server )
     utility::should_not_occur(__LINE__, __FILE__); // TODO: Verify correctness

   std::shared_ptr<ServerResponse> Q= std::make_shared<ServerResponse>();
   Q->self= Q;                      // Set self-reference
   Q->stream= owner->get_self();    // Set ourClientStream

   // Extract options
   if( opts )
     Q->append(*opts);

   if( HCDM )
     debugh("%p= http::ServerResponse::make()\n", Q.get());
   return Q;
}

//----------------------------------------------------------------------------
// ServerResponse::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<ServerRequest>
   ServerResponse::get_request( void ) const
{
   std::shared_ptr<ServerStream> stream= get_stream();
   if( stream )
     return stream->get_request();

   return nullptr;
}

std::shared_ptr<ServerStream>
   ServerResponse::get_stream( void ) const
{  return std::dynamic_pointer_cast<ServerStream>(stream); }
//{  return std::dynamic_pointer_cast<ServerStream>(stream.lock()); }

//----------------------------------------------------------------------------
//
// Method-
//       ServerResponse::write
//
// Purpose-
//       Write the Response
//
//----------------------------------------------------------------------------
void
   ServerResponse::write(           // Transmit
     const void*       addr,        // Data address
     size_t            size)        // Data length
{  if( HCDM )
     debugh("ClientResponse(%p)::write({%p,%zd})\n", this, addr, size);

   data.append(addr, size);         // Append to Data
}

void
   ServerResponse::write( void )     // Write Response
{  if( HCDM ) debugh("ServerResponse(%p)::write\n", this);

   std::shared_ptr<Request> Q= get_request();
   if( !Q )
     utility::not_coded_yet(__LINE__, __FILE__);

   string mess= to_string("%s %d %s\r\n", Q->proto_id.c_str(), code
                         , Stream::get_text(code));
   Options& opts= (Options&)*this;
   for(Options::const_iterator it= opts.begin(); it != opts.end(); ++it)
     mess += to_string("%s: %s\r\n", it->first.c_str(), it->second.c_str());
   mess += "\r\n";

   std::shared_ptr<ServerStream> stream= get_stream();
   if( !stream )
     utility::not_coded_yet(__LINE__, __FILE__);
   stream->write(mess.c_str(), mess.size());

   mess= data.get_string();
   if( mess.size() > 0 )
     stream->write(mess.c_str(), mess.size());
}
}  // namespace pub::http
