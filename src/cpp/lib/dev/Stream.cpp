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
//       Stream.cpp
//
// Purpose-
//       Implement http/Stream.h
//
// Last change date-
//       2022/03/06
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <atomic>                   // For std::atomic
#include <cstring>                  // For memset
#include <ostream>                  // For std::ostream
#include <stdexcept>                // For std::runtime_error, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <stdio.h>                  // For fprintf
#include <stdint.h>                 // For integer types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For namespace pub::dispatch
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Statistic.h>          // For pub::Statistic
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::to_string, ...

#include "pub/http/bits/devconfig.h" // Must be first http include (TODO: REMOVE)
#include "pub/http/Client.h"        // For pub::http::Client
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
// IODM= false                      // Input/Output Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  BUFFER_SIZE= 8'096               // Input buffer size (Header collector)
,  POST_LIMIT= 1'048'576            // POST/PUT size limit
,  USE_XTRACE= false                // Use extended trace?
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
// Constants
//----------------------------------------------------------------------------
static struct{int code; const char* text;}
                       code_text[]=
{  {  0, "UNKNOWN CODE"}
,  {200, "OK"}
,  {400, "BAD REQUEST"}
,  {401, "NOT AUTHORIZED"}
,  {403, "FORBIDDEN"}
,  {404, "NOT FOUND"}
,  {405, "METHOD NOT ALLOWED"}
,  {411, "LENGTH REQUIRED"}
,  {413, "PAYLOAD TOO LARGE"}
,  {501, "INTERNAL SERVER ERROR"}
,  {599, "CLIENT DISCONNECTED"}
,  {  0, nullptr}                   // Delimiter
};

//============================================================================
//
// Method-
//       StreamSet::Node::~Node
//
// Purpose-
//       Destructor
//
// Implementation notes-
//       It is an error to delete a Node that has a parent or child.
//
//----------------------------------------------------------------------------
   StreamSet::Node::~Node( void )
{
   assert( parent == nullptr && child == nullptr );
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::Node::insert
//
// Purpose-
//       Insert a child Node
//
// Implementation notes-
//       REQUIRES: The StreamSet must be locked
//       The inserted Node's child list is NOT inspected or modified.
//
//----------------------------------------------------------------------------
void
   StreamSet::Node::insert(         // Insert (at beginning of child list)
     Node*             node)        // This Node
{
   assert( node->parent == nullptr ); // Must not already be on a list
   node->parent= this;
   node->peer= child;
   child= node;
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::Node::remove
//
// Purpose-
//       Remove a child Node
//
// Implementation notes-
//       REQUIRES: The StreamSet must be locked
//       The removed Node's child list is NOT inspected or modified.
//
//----------------------------------------------------------------------------
void
   StreamSet::Node::remove(         // Remove (from the child list)
     Node*             node)        // This Node
{
   assert( node->parent == this );  // Must be our child Node

   node->parent= nullptr;           // Consider it already removed
   if( child == node ) {            // If removing the first child
     child= node->peer;             // Remove it from the list
     node->peer= nullptr;           // (Not strictly necessary)
     return;
   }

   Node* prev= child;
   while( prev ) {                  // Search the child list
     if( prev->peer == node ) {     // If prev->(Node to be removed)
       prev->peer= node->peer;      // Remove the Node from the list
       node->peer= nullptr;         // (Not strictly necessary)
       return;
     }

     prev= prev->peer;              // Follow the list
   }

   // SHOULD NOT OCCUR: The Node to be removed wasn't on the list
   debugf("StreamSet::Node(%p)::remove(%p), but it's not on the child list\n"
         , this, node);
   node->peer= nullptr;             // (Even now, not strictly necessary)
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   StreamSet::Node::remove( void )  // Remove THIS node from its parent
{
   assert( parent != nullptr );     // (It must actually *have* a parent)
   parent->remove(this);
}

//============================================================================
//
// Method-
//       StreamSet::~StreamSet
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   StreamSet::~StreamSet( void )    // Destructor
{  assert( root->child == nullptr ); }  // The StreamSet must be empty

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::get_stream
//
// Purpose-
//       Locate Stream by identifier
//
//----------------------------------------------------------------------------
StreamSet::stream_ptr               // The associated Stream
   StreamSet::get_stream(           // Locate the Stream given Stream::ident
     stream_id         id) const    // For this Stream identifier
{  std::lock_guard<decltype(mutex)> lock(mutex);

   const_iterator it= map.find(id);
   if( it != map.end() )
     return it->second;

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   StreamSet::debug(const char* info) const  // Debugging display
{  debugh("StreamSet(%p)::debug(%s)\n", this, info);

   debugf("root->parent(%p) ", root->parent);
   debugf("root->child(%p) ",  root->child);
   debugf("root->peer(%p) ",   root->peer);
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::assign_stream_id
//
// Purpose-
//       Assign a Stream identifier
//
//----------------------------------------------------------------------------
StreamSet::stream_id                // The next available Stream identifier
   StreamSet::assign_stream_id(     // Assign a Stream identifier
     int               addend)      // After incrementing it by this value
{
   std::atomic<stream_id>* ident_ptr= (std::atomic<stream_id>*)&ident;
   stream_id old_value= ident_ptr->load();
   for(;;) {
     stream_id new_value= old_value + addend;
     if( new_value < 0 )            // If 31-bit arithmetic overflow
       return -1;                   // We're out of identifiers
     if( ident_ptr->compare_exchange_strong(old_value, new_value) )
       return new_value;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::change
//
// Purpose-
//       Locate Stream by identifier
//
//----------------------------------------------------------------------------
void
   StreamSet::change(               // Change a Stream's parent
     Stream*           parent,      // The new parent Stream
     Stream*           stream)      // The Stream to move
{  std::lock_guard<StreamSet> lock(*this);

   (void)parent; (void)stream;
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::insert
//
// Purpose-
//       Locate Stream by identifier
//
//----------------------------------------------------------------------------
void
   StreamSet::insert(               // Insert Stream
     Stream*           parent,      // The parent Stream
     Stream*           stream)      // The Stream to insert
{  std::lock_guard<StreamSet> lock(*this);

   parent->insert(stream);
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamSet::remove
//
// Purpose-
//       Locate Stream by identifier
//
//----------------------------------------------------------------------------
void
   StreamSet::remove(               // Remove Stream
     Stream*           stream)      // The Stream to remove
{  std::lock_guard<StreamSet> lock(*this);

   stream->remove();
}

//============================================================================
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
{  if( HCDM ) debugh("Stream(%p)!\n", this);

   if( USE_XTRACE )
     Trace::trace(".NEW", ".STR", this); // (Trace ClientStream, ServerStream)
   obj_count.inc();
   INS_DEBUG_OBJ("Stream");
}

   Stream::~Stream( void )          // Destructor
{  if( HCDM ) debugh("Stream(%p)~\n", this);

   if( USE_XTRACE )
     Trace::trace(".DEL", ".STR", this); // (Trace ~ClientStream, ~ServerStream)
   obj_count.dec();
   REM_DEBUG_OBJ("Stream");
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
//       Stream::get_text
//
// Purpose-
//       Get text for status code
//
//----------------------------------------------------------------------------
const char*                         // The status text
   Stream::get_text(int code)       // Convert status code to text
{
   for(int i= 1; code_text[i].text; ++i) { // (code_text[0] is internal error)
     if( code == code_text[i].code )
       return code_text[i].text;
   }

   debugh("%4d %s code(%d) undefined\n", __LINE__, __FILE__, code);
   return code_text[0].text;
}

//----------------------------------------------------------------------------
//
// Method-
//       Stream::use_count
//
// Purpose-
//       Display the use count
//
// Implemenation notes-
//       Call *after* incrementing or *before* decrementing.
//       std::shared_ptr<Stream> S->use_count(__LINE__, __FILE__, "info");
//
//----------------------------------------------------------------------------
void
   Stream::use_count(               // Display the use count
     int               line,        // Caller's line number
     const char*       file,        // Caller's file name
     const char*       info) const  // Caller's information
{
   static std::mutex mutex;
   std::lock_guard<std::mutex> lock(mutex);

   debugf("%2zd use_count Stream(%#12zx) %4d %s %s\n", self.use_count()
         , intptr_t(this), line, file, info);
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
{  if( HCDM )
     debugh("ClientStream(%p)!(%p)\n", this, owner);

   if( USE_XTRACE )
     Trace::trace(".NEW", "CSTR", this);
   INS_DEBUG_OBJ("ClientStream");
// http1();                         // TODO: HANDLE HTTP2, etc
}

   ClientStream::~ClientStream( void ) // Destructor
{  if( HCDM )
     debugh("ClientStream(%p)~\n", this);

   if( USE_XTRACE )
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
///debugh("%4d %s shared_ptr<ClientRequest>(%p)->(%p)\n", __LINE__, __FILE__, &S->request, S->request.get()); // TODO: REMOVE
///debugh("%4d %s shared_ptr<ClientResponse>(%p)->(%p)\n", __LINE__, __FILE__, &S->response, S->response.get()); // TODO: REMOVE

   if( HCDM )
     debugh("%p= ClientStream::make(%p,%p)\n", S.get(), owner, opts);
///debugh("%4d %s shared_ptr<ClientStream.self>(%p)->(%p)\n", __LINE__, __FILE__, &S->self, S->self.lock().get()); // TODO: REMOVE

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

   if( utility::is_null(this) ) {   // TODO: REMOVE
     utility::on_exception("Stream::end");
     return;
   }

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
   ClientStream::write(             // Write data segment to stream
     Ioda&             ioda)        // I/O Data Area
{  if( HCDM )
     debugh("ClientStream(%p)::write(*,%zd)\n", this, ioda.get_used());

   utility::should_not_occur(__LINE__, __FILE__);
}

void
   ClientStream::write( void )      // Write data (completed)
{  if( HCDM ) debugh("ClientStream(%p)::write\n", this);

   std::shared_ptr<Client> client= get_client();
   int rc= -1;                      // Default, rejected
   if( client )
     rc= client->write(this);

   if( rc )
     utility::not_coded_yet(__LINE__, __FILE__);
}

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
{  if( HCDM )
     debugh("ServerStream(%p)!(%p)\n", this, owner);
   if( USE_XTRACE )
     Trace::trace(".NEW", "SSTR", this);

   INS_DEBUG_OBJ("ServerStream");
}

   ServerStream::~ServerStream( void ) // Destructor
{  if( HCDM )
     debugh("ServerStream(%p)~\n", this);
   if( USE_XTRACE )
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
///debugh("%4d %s shared_ptr<ServerRequest>(%p)->(%p)\n", __LINE__, __FILE__, &S->request, S->request.get()); // TODO: REMOVE
///debugh("%4d %s shared_ptr<ServerResponse>(%p)->(%p)\n", __LINE__, __FILE__, &S->response, S->response.get()); // TODO: REMOVE

   if( HCDM )
     debugh("%p= ServerStream::make(%p)\n", S.get(), owner);
///debugh("%4d %s shared_ptr<ServerStream.self>(%p)->(%p)\n", __LINE__, __FILE__, &S->self, S->self.lock().get()); // TODO: REMOVE

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

   if( utility::is_null(this) ) {   // TODO: REMOVE
     utility::on_exception("Stream::end");
     return;
   }

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
//       ServerStream::write (I/O Method)
//
// Purpose-
//       Response complete, write data.
//
//----------------------------------------------------------------------------
void
   ServerStream::write( void )      // Write data (completed)
{  utility::should_not_occur(__LINE__, __FILE__); }

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
     debugh("\nServerStream(%p)::reject(%d) %s\n\n", this, code, get_text(code));

   char buff[128];                  // TODO: FIX PROTOCOL
   size_t L= sprintf(buff, "HTTP/1.1 %.3d %s\r\n\r\n", code, get_text(code));

   response->set_code(code);
   response->get_ioda().reset();
   write(buff, L);
   end();
}
}  // namespace _LIBPUB_NAMESPACE::http
