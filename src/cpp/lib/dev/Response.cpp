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
//       Response.cpp
//
// Purpose-
//       Implement http/Response.h
//
// Last change date-
//       2023/06/27
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <cstring>                  // For memset
#include <stdexcept>                // For std::out_of_range, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <stdio.h>                  // For fprintf
#include <stdint.h>                 // For integer types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Statistic.h>          // For pub::Active_record
#include <pub/utility.h>            // For pub::to_string, ...

#include "pub/http/Client.h"        // For pub::http::Client
#include "pub/http/Exception.h"     // For pub::http:exceptions
#include "pub/http/HTTP.h"          // For pub::http:HTTP
#include "pub/http/Ioda.h"          // For pub::http:Ioda
#include "pub/http/Response.h"      // For pub::http::Response, implemented
#include "pub/http/Server.h"        // For pub::http::Server
#include "pub/http/Stream.h"        // For pub::http::Stream

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::utility::to_string;
using std::string;

namespace _LIBPUB_NAMESPACE::http { // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
// IODM= false                      // I/O Debug Mode?
// VERBOSITY= 1                     // Verbosity, higher is more verbose

,  RESP_LIMIT= 1'048'576            // Response size limit
,  USE_REPORT= false                // Use event Reporter?
}; // enum

enum FSM                            // Finite State Machine states
{  FSM_RESET                        // Initial reset state
,  FSM_HEAD                         // Reading Headers
,  FSM_BODY                         // Reading Body
}; // enum FSM

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
statistic::Active      Response::obj_count; // Response object count

//----------------------------------------------------------------------------
// Event reporting
//----------------------------------------------------------------------------
static Active_record   response_count("Response"); // Response counter

namespace {
static struct StaticGlobal {
   StaticGlobal(void)               // Constructor
{
   if( USE_REPORT ) {
     response_count.insert();
   }
}  // StaticGlobal

   ~StaticGlobal(void)              // Destructor
{
   if( USE_REPORT ) {
     response_count.remove();
   }
}  // StaticGlobal
}  staticGlobal;
}  // Anonymous namespace

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
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
:  h_ioda([](Ioda&) { })
,  h_end([]( void ) { })
,  h_error([](string) { })
{  if( HCDM ) debugh("Response(%p)!\n", this);

   obj_count.inc();

   if( USE_REPORT )
     response_count.inc();

   INS_DEBUG_OBJ("Response");
}

   Response::~Response( void )      // Destructor
{  if( HCDM ) debugh("Response(%p)~\n", this);

   obj_count.dec();

   if( USE_REPORT )
     response_count.dec();

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

   opts.debug(info);
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
   REM_DEBUG_OBJ("ClientResponse");
}

std::shared_ptr<ClientResponse>     // The ClientResponse
   ClientResponse::make(            // Get ClientResponse
     ClientStream*     owner,       // For this ClientStream
     const Options*    opts)        // With these Options
{
   std::shared_ptr<ClientResponse> S= std::make_shared<ClientResponse>();
   S->self= S;                      // Set self-reference
   S->stream= owner->get_self();    // Set ourClientStream

   // Copy options
   if( opts )
     S->opts= *opts;

   if( HCDM )
     debugh("%p= http::ClientResponse::make(%p)\n", S.get(), owner);
   return S;
}

//----------------------------------------------------------------------------
// ClientResponse::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<Client>
   ClientResponse::get_client( void ) const
{  return get_stream()->get_client(); }

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

//----------------------------------------------------------------------------
//
// Method-
//       ClientResponse::end
//
// Purpose-
//       Response complete
//
//----------------------------------------------------------------------------
void
   ClientResponse::end( void )      // Response complete
{  if( HCDM ) debugh("ClientResponse(%p)::end\n", this);

   h_end();                         // Drive Response::on_end
   stream= nullptr;                 // Remove Stream reference
}

//----------------------------------------------------------------------------
//
// Method-
//      ClientResponse::reject
//
// Purpose-
//       Reject the Response
//
//----------------------------------------------------------------------------
void
   ClientResponse::reject(string mess) // Reject the Response
{  if( HCDM ) debugh("Response(%p)::reject(%s)\n", this, mess.c_str());

   h_error(mess);
   get_stream()->end();
}

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
     if( protocol == "" || status == "" || message == ""
         || protocol[0] == ' ' || status[0] == ' ' || message[0] == ' ' ) {
       debugh("%4d Response: protocol(%s) status(%s) message(%s)\n", __LINE__
             , protocol.c_str(), status.c_str(), message.c_str());
       std::shared_ptr<Client> client= get_client();
       if( client )
         client->error("Invalid Start-Line");
       return true;
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
         utility::report_exception("Invalid Header-Line format");
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
       reject("Invalid content length");
       return true;
     }
     std::shared_ptr<ClientRequest> Q= get_request();
     if( Q->method != HTTP_HEAD ) {
       if( (ioda.get_used()) < size_t(content_length) )
         return false;
     }
   }

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
   if( !server ) {
     debugf("%4d %s HCDM (unexpected)\n", __LINE__, __FILE__);
     return nullptr;
   }

   std::shared_ptr<ServerResponse> S= std::make_shared<ServerResponse>();
   S->self= S;                      // Set self-reference
   S->stream= owner->get_self();    // Set our ServerStream

   // Copy options
   if( opts )
     S->opts= *opts;

   if( HCDM )
     debugh("%p= http::ServerResponse::make()\n", S.get());
   return S;
}

//----------------------------------------------------------------------------
// ServerResponse::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<Server>
   ServerResponse::get_server( void ) const
{  return get_stream()->get_server(); }

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

//----------------------------------------------------------------------------
//
// Method-
//       ServerResponse::end
//
// Purpose-
//       Response complete
//
//----------------------------------------------------------------------------
void
   ServerResponse::end( void )      // Response complete
{  if( HCDM ) debugh("ServerResponse(%p)::end\n", this);

   h_end();                         // Drive Response::on_end
   stream= nullptr;                 // Remove Stream reference
}

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
   ServerResponse::write( void )     // Write Response
{  if( HCDM ) debugh("ServerResponse(%p)::write\n", this);

   std::shared_ptr<Request> Q= get_request();
   if( !Q )
     return;

   string mess= to_string("%s %d %s\r\n", Q->proto_id.c_str(), code
                         , HTTP::status_text(code));
   for(Options::const_iterator it= opts.begin(); it != opts.end(); ++it)
     mess += to_string("%s: %s\r\n", it->first.c_str(), it->second.c_str());
   mess += "\r\n";

   std::shared_ptr<ServerStream> stream= get_stream();
   if( !stream )
     return;
   Ioda _ioda;
   _ioda += mess;
   _ioda += std::move(ioda);
   stream->write(_ioda);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   ServerResponse::write(           // Transmit
     const void*       addr,        // Data address
     size_t            size)        // Data length
{  if( HCDM )
     debugh("ServerResponse(%p)::write({%p,%zd})\n", this, addr, size);

   ioda.write(addr, size);          // Append to Data
}
}  // namespace _LIBPUB_NAMESPACE::http
