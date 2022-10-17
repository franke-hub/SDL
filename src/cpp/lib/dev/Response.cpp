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
//       2022/10/16
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
#include "pub/http/Ioda.h"          // For pub::http:Ioda
#include "pub/http/Request.h"       // For pub::http::Request // TODO: NEEDED?
#include "pub/http/Response.h"      // For pub::http::Response, implemented
#include "pub/http/Server.h"        // For pub::http::Server
#include "pub/http/Stream.h"        // For pub::http::Stream

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::utility::dump;          // TODO: REMOVE
using PUB::utility::to_string;
using std::string;

namespace _LIBPUB_NAMESPACE::http { // Implementation namespace
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
// External data areas
//----------------------------------------------------------------------------
Statistic              Response::obj_count; // Response object count

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
// Imported Options TODO: REMOVE UNUSED
typedef const char CC;
static constexpr CC*   HTTP_SIZE= Options::HTTP_HEADER_LENGTH;

static constexpr CC*   HTTP_HEAD=  Options::HTTP_METHOD_HEAD;

//----------------------------------------------------------------------------
//
// Method-
//       Response::Response
//       Response::~Response
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Response::Response( void )       // Default constructor
:  Options()
,  h_ioda([](Ioda&) { })
,  h_end([]( void ) { })
,  h_error([](string) { })
{  if( HCDM ) debugh("Response(%p)!\n", this);

   obj_count.inc();
   INS_DEBUG_OBJ("Response");
}

   Response::~Response( void )      // Destructor
{  if( HCDM ) debugh("Response(%p)~\n", this);

   obj_count.dec();

   if( stream )
     stream->use_count(__LINE__, __FILE__, "--Response~");
   REM_DEBUG_OBJ("Response");
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
   Response::reject(int code)       // Reject the Response
{  if( HCDM ) debugh("Response(%p)::reject(%d)\n", this, code);

   stream->reject(code);
   stream->close();
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
bool Response::read(Ioda&)          // (ClientResponse only)
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
//       ClientResponse::ClientResponse
//       ClientResponse::~ClientResponse
//       ClientResponse::make
//
// Purpose-
//       Constructor
//       Destructor
//       Creator
//
//----------------------------------------------------------------------------
   ClientResponse::ClientResponse( void ) // Default constructor
:  Response()
{  if( HCDM ) debugh("http::ClientResponse(%p)!\n", this);
   INS_DEBUG_OBJ("ClientResponse");
}

   ClientResponse::~ClientResponse( void ) // Destructor
{  if( HCDM ) debugh("http::ClientResponse(%p)~\n", this);
}

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
   ClientResponse::read(            // Read from
     Ioda&             data)        // This I/O Data Area
{  if( HCDM )
     debugh("ClientResponse(%p)::read({*,%zd})\n", this, data.get_used());

   ioda += std::move(data);         // Append the I/O Data Area

   std::shared_ptr<Client> client= get_stream()->get_client();
   if( client.get() == nullptr )
     return true;

   if( fsm == FSM_RESET )
     fsm= FSM_HEAD;

   IodaReader reader(ioda);         // The Ioda reader
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
// debugh("%4d %s HCDM\n", __LINE__, __FILE__); // reader.debug(); // TODO: REMOVE
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
     string protocol= reader.get_token(" ");
     string status= reader.get_token(" ");
     string message= reader.get_token("\r\n");

     // Start-Line validity checks
     if( protocol == "" || status == "" || message == "" ) {
debugh("%4d Response: protocol(%s) status(%s) message(%s)\n", __LINE__, protocol.c_str(), status.c_str(), message.c_str()); // TODO: REMOVE
       throw stream_error("Invalid Start-Line");
     }
     code= atoi(status.c_str());    // TODO: IMPROVE ROBUSTNESS

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

       // Check for obs-fold in a response message.
       // It's use is deprecated except within a message/http container.
       // TODO: Find out the definition of "messsage/http container."
       // TODO: Do we need to allow this if using HTTP/1.0??
       // TODO: If allowed, the value definition continues
       P= reader.peek();
       if( P == ' ' || P == '\t' )
         throw std::runtime_error("Header-Line obs-fold: {'\\r','\\n',WS}");

       if( name == ""||value == "" || isspace(name[0])||isspace(value[0]) ) {
debugf("Response name(%s) value(%s)\n", name.c_str(), value.c_str());
utility::on_exception("Invalid Header-Line format");
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
   // Load response data
   //-------------------------------------------------------------------------
   const char* value= locate(HTTP_SIZE);
   if( value ) {
     ssize_t content_length= atol(value);
     if( content_length < 0 || content_length > RESP_LIMIT ) {
       reject(413);
       return true;
     }
     std::shared_ptr<ClientRequest> Q= get_request();
     if( Q->method != HTTP_HEAD ) {
       if( (ioda.get_used()) < size_t(content_length) )
         return false;
     }
   }

// traceh("%4d %s HCDM (ClientResponse::read complete)\n", __LINE__, __FILE__);
   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerResponse::ServerResponse
//       ServerResponse::~ServerResponse
//       ServerResponse::make
//
// Purpose-
//       Constructors
//       Destructor
//       Creator
//
//----------------------------------------------------------------------------
   ServerResponse::ServerResponse( void ) // Default constructor
:  Response()
{  if( HCDM ) debugh("http::ServerResponse(%p)!\n", this);
   INS_DEBUG_OBJ("ServerResponse");
}

   ServerResponse::~ServerResponse( void ) // Destructor
{  if( HCDM ) debugh("http::ServerResponse(%p)~\n", this);
   REM_DEBUG_OBJ("ServerResponse");
}

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
   Q->stream= owner->get_self();    // Set our ServerStream

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
     debugh("ServerResponse(%p)::write({%p,%zd})\n", this, addr, size);

   ioda.write(addr, size);          // Append to Data
// debugf("%4d ServerResponse::write(%p,%zd)\n", __LINE__, addr, size);
// dump(addr, size);
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
   Ioda temp;
   temp.put(mess);
   stream->write(temp);

// ioda.debug("ServerResponse.write");
   if( ioda.get_used() > 0 )
     stream->write(ioda);
}
}  // namespace _LIBPUB_NAMESPACE::http
