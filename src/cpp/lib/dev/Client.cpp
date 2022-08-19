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
//       Client.cpp
//
// Purpose-
//       Implement http/Client.h
//
// Last change date-
//       2022/08/16
//
// Implementation notes-
//       Throughput: W: 5584.4/sec  L: 518.6/sec protocol1a
//       Throughput: W: 4088.7/sec  L: 307.5/sec protocol1b
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <cstring>                  // For memcmp, memset
#include <ostream>                  // For std::ostream
#include <stdexcept>                // For std::runtime_error, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <errno.h>                  // For errno
#include <stdio.h>                  // For fprintf
#include <stdint.h>                 // For integer types
#include <arpa/inet.h>              // For inet_ntop
#include <openssl/err.h>            // For ERR_error_string

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For pub::Dispatch objects
#include <pub/Event.h>              // For pub::Event
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Named.h>              // For pub::Named
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For namespace pub::utility

#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Agent.h"         // For pub::http::ClientAgent (owner)
#include "pub/http/Client.h"        // For pub::http::Client, implementated
#include "pub/http/Exception.h"     // For pub::http::exceptions
#include "pub/http/Request.h"       // For pub::http::Request
#include "pub/http/Response.h"      // For pub::http::Response
#include "pub/http/Stream.h"        // For pub::http::Stream
#include "pub/http/utility.h"       // For namespace pub::http::utility

using namespace _PUB_NAMESPACE;
using namespace _PUB_NAMESPACE::debugging;
using _PUB_NAMESPACE::utility::to_string;
using _PUB_NAMESPACE::utility::visify;
using std::string;

namespace pub::http {               // Implementation namespace
//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#if EAGAIN == EWOULDBLOCK
#define IS_BLOCKED (errno == EAGAIN || errno == EINTR)
#else
#define IS_BLOCKED (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  IODM= false                      // Input/Output Debug Mode?
,  VERBOSE= 1                       // Verbosity, range 0..5

,  BUFFER_SIZE= 1'048'576           // Input buffer size
,  USE_PROTOCOL1A= true             // (Not PROTOCOL1B)
}; // enum

// Imported Options
typedef const char CC;
static constexpr CC*   HTTP_SIZE= Options::HTTP_HEADER_LENGTH;

static constexpr CC*   HTTP_POST= Options::HTTP_METHOD_POST;
static constexpr CC*   HTTP_PUT=  Options::HTTP_METHOD_PUT;

static constexpr CC*   OPT_PROTO= Options::HTTP_OPT_PROTOCOL; // Protocol type
static constexpr CC*   HTTP_H0=   Options::HTTP_PROTOCOL_H0; // HTTP/1.0
static constexpr CC*   HTTP_H1=   Options::HTTP_PROTOCOL_H1; // HTTP/1.1
static constexpr CC*   HTTP_H2=   Options::HTTP_PROTOCOL_H2; // HTTP/2
static constexpr CC*   HTTP_S0=   Options::HTTP_PROTOCOL_S0; // HTTPS/1.0
static constexpr CC*   HTTP_S1=   Options::HTTP_PROTOCOL_S1; // HTTPS/1.1
static constexpr CC*   HTTP_S2=   Options::HTTP_PROTOCOL_S2; // HTTPS/2

#include "Client.hpp"               // (Client.cpp internal definitions)

//----------------------------------------------------------------------------
//
// Method-
//       Client::~Client   (Destructor)
//       Client::Client    (Constructor)
//       Client::make      (Creator Constructor)
//
//----------------------------------------------------------------------------
   Client::~Client( void )          // Destructor
{  if( HCDM )
     debugh("Client(%p)::~Client\n", this);

   // Wait for ClientThread completion
   if( thread ) {
     thread->join();
     delete thread;
   }

   // Release allocated storage
   if( context )                    // If context exists
     SSL_CTX_free(context);
   delete socket;
}

   Client::Client(                  // Constructor
     ClientAgent*      owner,       // Our ClientAgent
     const sockaddr_u& addr,        // Target internet address
     socklen_t         size,        // Target internet address length
     const Options*    opts)        // Client Options
:  std::mutex()
,  h_close(utility::f_void())
,  agent(owner->get_self())
,  ibuffer(BUFFER_SIZE)
,  obuffer(BUFFER_SIZE)
,  proto_id(HTTP_H1)
,  semaphore(1)
,  task([this](dispatch::Item* item) { h_writer((ClientItem*)item); })
{  if( HCDM )
     debugh("\nClient(%p)::Client(%p) %s\n", this, owner
           , addr.to_string().c_str());

   // Internal consistency check
// assert( size_t(size) <= sizeof(sockaddr_u)); // (Checked in Socket::connect)

   // Handle Options
   bool encrypt= false;             // Default, not encrypted
   if( USE_PROTOCOL1A ) {
     h_writer= protocol1a();        // Default, protocol1a
     if( VERBOSE ) debugf("PROTOCOL1A\n");
   } else {
     h_writer= protocol1b();        // Default, protocol1b
     if( VERBOSE ) debugf("PROTOCOL1B\n");
   }
   if( opts ) {
     const char* type= opts->locate(OPT_PROTO); // Get specified protocol
     if( type ) {                   // If protocol specified
       if( strcmp(type, HTTP_H2) == 0 || strcmp(type, HTTP_S2) == 0 ) {
         proto_id= HTTP_H2;
         h_writer= protocol2();
         if( strcmp(type, HTTP_S2) == 0 )
           encrypt= true;
       } else if( strcmp(type, HTTP_S1) == 0 ) {
         encrypt= true;
       } else if( strcmp(type, HTTP_H1) != 0 ) {
         string S= to_string("Client::Client %s='%s'", OPT_PROTO, type);
         throw std::runtime_error(S); // Protocol specified, but invalid
       }
     }
   }

   // Create connection
   if( !encrypt ) {
     socket= new pub::Socket();
     int
     rc= socket->open(addr.su_af, SOCK_STREAM, PF_UNSPEC);
     if( IODM )
       traceh("%4d Client %d= open(%d,%d,%d)\n", __LINE__, rc
             , addr.su_af, SOCK_STREAM, PF_UNSPEC);
     if( rc ) {                       // If unable to open
       utility::report_error(__LINE__, __FILE__, "open");
       return;
     }

     rc= socket->connect((const sockaddr*)&addr, size); // Connect
     int ERRNO= errno;
     if( IODM )
       traceh("%4d Client %d= connect(%s)\n", __LINE__, rc
             , addr.to_string().c_str());
     if( rc ) {                       // If unable to connect
       errno= ERRNO;
       utility::report_error(__LINE__, __FILE__, "connect");
       socket->close();
     } else if( HCDM ) {
       debugf("Client(%p): %s connected\n", this, addr.to_string().c_str());
     }
   } else {                         // If SSL
     initialize_SSL();              // Initialize SSL
     context= new_client_CTX();
     // NOT CODED YET
   }
   timeval tv{3, 500000};           // 3.5 second timeout
   socket->set_option(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

   // Create the ClientThread
   if( USE_PROTOCOL1A )
     operational= true;
   else
     thread= new ClientThread(this);
}

std::shared_ptr<Client>             // (New) Client
   Client::make(                    // Creator
     ClientAgent*      owner,       // Our ClientAgent
     const sockaddr_u& addr,        // Target internet address
     socklen_t         size,        // Target internet address length
     const Options*    opts)        // Client Options
{
   std::shared_ptr<Client> client=
       std::make_shared<Client>(owner, addr, size, opts);

   client->self= client;
   return client;
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Client::debug(const char* info) const  // Debugging display
{  debugf("Client(%p)::debug(%s) operational(%d)\n", this, info, operational);

   debugf("..agent(%p) context(%p) proto_id(%s) semcount(%u)\n"
         , agent.lock().get(), context, proto_id, semaphore.get_count());
// ibuffer.debug("Client.ibuffer");
// obuffer.debug("Client.obuffer");
   socket->debug("Client.socket");
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::close
//
// Purpose-
//       Close the Client
//
//----------------------------------------------------------------------------
void
   Client::close( void )            // Close the Client
{  if( HCDM ) debugh("Client(%p)::close\n", this);
   Trace::trace(".CLI", ".CLS", this);

   operational= false;              // Indicate non-operational
   // Note: Must call agent->disconnect before socket->close, peer_addr needed.
   std::shared_ptr<ClientAgent> agent= this->agent.lock();
   if( agent )                      // Report close to Agent
     agent->disconnect(this);
   socket->close();                 // Close the Socket (immediate halt)
   wait();                          // Flush the work queue
   h_close();                       // Drive the close handler
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::connection_error
//
// Purpose-
//       Handle connection error.
//
//----------------------------------------------------------------------------
void
   Client::connection_error(        // Handle connection error
     const char*       info)        // Diagnostic information
{  errorh("Client(%p)::connection_error(%s)\n", this, info);

   // TODO: PRELIMINARY
   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::enqueue
//
// Purpose-
//       Enqueue ClientItem
//
//----------------------------------------------------------------------------
void
   Client::enqueue(                 // Enqueue a Request
     ClientItem*       item)        // Embedded in this ClientItem
{  if( HCDM ) debugh("Client(%p)::enqueue(%p)\n", this, item);

   if( true ) throw "Client::enqueue deprecated";
   if( socket->get_handle() > 0 )
     task.enqueue(item);
   else
     item->post(ClientItem::CC_PURGE);
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::protocol1a
//
// Purpose-
//       Define the HTTP/1.0 and HTTP/1.1 write protocol handler
//
//----------------------------------------------------------------------------
std::function<void(ClientItem*)>
   Client::protocol1a( void )       // Define the HTTP/1 write protocol handler
{  return [this](ClientItem* item)
 { if( HCDM ) debugh("Client(%p)::protocol1a(%p)\n", this, item);

   if( item->fc == ClientItem::FC_FLUSH ) { // If FLUSH operation
// debugh("%4d %s HCDM\n", __LINE__, __FILE__);
     item->post();                  // Flush complete
     return;
   }

   // debugh("%4d %s HCDM\n", __LINE__, __FILE__);
   std::shared_ptr<ClientStream>  stream= item->stream;
   delete item;                     // (No longer needed)
   try {
     // Format the request buffer
     std::shared_ptr<ClientRequest> request= stream->get_request();
     Request& Q= *request.get();    // (Q protected by request)

     if( socket->get_handle() <= 0 ) { // If connection broken
       return;
     }

     obuffer.reset();
     obuffer.append(Q.method);
     obuffer.append(' ');
     obuffer.append(Q.path);
     obuffer.append(' ');
     obuffer.append(Q.proto_id);
     obuffer.append("\r\n");

     // Set Content-Length
     Options& opts= Q;
     opts.remove(HTTP_SIZE);
     const Data& data= request->get_data();
     size_t content_length= data.get_size();
// debugh("Method(%s) Content_Length(%zd)\n", Q.method.c_str(), content_length); // TODO: REMOVE
     if(content_length != 0 ) {
       if( Q.method != HTTP_POST && Q.method != HTTP_PUT ) {
         content_length= 0;
         if( VERBOSE )
           fprintf(stderr, "Option %s does not support data\n", Q.method.c_str());
       }
     } else if( Q.method == "POST" || Q.method == "PUT" ) {
       stream->reject(411);       // Length required
       return;
     }

     // Unpack header items
     typedef Options::const_iterator iterator;
     for(iterator i= opts.begin(); i != opts.end(); ++i) {
       obuffer.append(i->first);
       obuffer.append(':');
       obuffer.append(i->second);
       obuffer.append("\r\n");
     }

     // Add Content-Length (if required)
     if( content_length ) {
       obuffer.append(HTTP_SIZE);
       obuffer.append(':');
       obuffer.append(std::to_string(content_length));
       obuffer.append("\r\n");
     }

     // Write the request headers
     obuffer.append("\r\n");        // Add header delimiter
     ssize_t L= write(__LINE__, obuffer.addr, obuffer.length);

     // Write the POST/PUT data
     if( content_length ) {
       while( content_length > 0 ) {
         L= data.store(obuffer.addr, obuffer.size);
// debugh("%4d Client L(%zd) offset(%zd) length(%zd)\n", L, offset, content_length);
         L= write(__LINE__, obuffer.addr, L);
         content_length -= L;
       }
     }

     // Wait for the response
     size_t size= read(__LINE__, ibuffer.size - 1);
     bool cc= stream->read(ibuffer.addr, size);
     while( !cc ) {
       size= read(__LINE__, ibuffer.size - 1);
       cc= stream->read(ibuffer.addr, size);
     }

     stream->end();                 // Operation complete
   } catch(pub::Exception& X) {
     errorh("%4d %s %s\n", __LINE__, __FILE__, ((std::string)X).c_str());
     connection_error(X.what());
   } catch(io_exception& X) {
     if( IODM )
       errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
     connection_error(X.what());
   } catch(stream_error& X) {
     if( IODM )
       errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
     connection_error(X.what());
   } catch(std::exception& X) {
     errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
     connection_error(X.what());
   } catch(const char* X) {
     errorh("%4d %s catch(%s)\n", __LINE__, __FILE__, X);
     connection_error(X);
   } catch(...) {
     errorh("%4d %s catch(...)\n", __LINE__, __FILE__);
     connection_error("catch(...)");
   }
 };
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::protocol1b
//
// Purpose-
//       Define the HTTP/1.0 and HTTP/1.1 write protocol handler
//
//----------------------------------------------------------------------------
std::function<void(ClientItem*)>
   Client::protocol1b( void )       // Define the HTTP/1 write protocol handler
{  return [this](ClientItem* item)
 { if( HCDM ) debugh("Client(%p)::protocol1b(%p)\n", this, item);

   if( item->fc == ClientItem::FC_FLUSH ) { // If FLUSH operation
Trace::trace(".TXT", __LINE__, "flush wait");
     semaphore.wait();              // Wait for pending response
Trace::trace(".TXT", __LINE__, "flush ready");
     semaphore.post();              // (Allow multiple FLUSH operations)
Trace::trace(".TXT", __LINE__, "flush post");
     item->post();                  // Flush complete
     return;
   }

   // debugh("%4d %s HCDM work(%p)\n", __LINE__, __FILE__, item);
   std::shared_ptr<ClientStream>  stream= item->stream;
   delete item;                   // (No longer needed)
   try {
     // Format the request buffer
     std::shared_ptr<ClientRequest> request= stream->get_request();
     Request& Q= *request.get();    // (Q protected by request)

     if( socket->get_handle() <= 0 ) { // If connection broken
     // debugh("%4d %s HCDM\n", __LINE__, __FILE__);
       return;
     }

     obuffer.reset();
     obuffer.append(Q.method);
     obuffer.append(' ');
     obuffer.append(Q.path);
     obuffer.append(' ');
     obuffer.append(Q.proto_id);
     obuffer.append("\r\n");

     // Set Content-Length
     Options& opts= Q;
     opts.remove(HTTP_SIZE);
     const Data& data= request->get_data();
     size_t content_length= data.get_size();
     // debugh("Method(%s) Content_Length(%zd)\n", Q.method.c_str(), content_length); // TODO: REMOVE
     if(content_length != 0 ) {
       if( Q.method != HTTP_POST && Q.method != HTTP_PUT ) {
         content_length= 0;
         if( VERBOSE )
           fprintf(stderr, "Option %s does not support data\n", Q.method.c_str());
       }
     } else if( Q.method == "POST" || Q.method == "PUT" ) {
       stream->reject(411);         // Length required
       return;
     }

     // Unpack header items
     typedef Options::const_iterator iterator;
     for(iterator i= opts.begin(); i != opts.end(); ++i) {
       obuffer.append(i->first);
       obuffer.append(':');
       obuffer.append(i->second);
       obuffer.append("\r\n");
     }

     // Add Content-Length (if required)
     if( content_length ) {
       obuffer.append(HTTP_SIZE);
       obuffer.append(':');
       obuffer.append(std::to_string(content_length));
       obuffer.append("\r\n");
     }

     // Synchronize Requests and Reponses
     if( proto_id == HTTP_H1 ) {    // If HTTP/1 protocol
Trace::trace(".TXT", __LINE__, "request wait");
       semaphore.wait();            // Wait for pending response
Trace::trace(".TXT", __LINE__, "request ready");
       stream_set.insert(nullptr, stream); // Set current Stream
     }

     // Write the request headers
     obuffer.append("\r\n");        // Add header delimiter
     ssize_t L= write(__LINE__, obuffer.addr, obuffer.length);

     // Write the POST/PUT data
     if( content_length ) {
       while( content_length > 0 ) {
         L= data.store(obuffer.addr, obuffer.size);
// debugh("%4d Client L(%zd) offset(%zd) length(%zd)\n", L, offset, content_length);
         L= write(__LINE__, obuffer.addr, L);
         content_length -= L;
       }
     }
   } catch(pub::Exception& X) {
     errorh("%4d %s %s\n", __LINE__, __FILE__, ((std::string)X).c_str());
     connection_error(X.what());
   } catch(io_exception& X) {
     if( IODM )
       errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
     connection_error(X.what());
   } catch(stream_error& X) {
     if( IODM )
       errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
     connection_error(X.what());
   } catch(std::exception& X) {
     errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
     connection_error(X.what());
   } catch(const char* X) {
     errorh("%4d %s catch(%s)\n", __LINE__, __FILE__, X);
     connection_error(X);
   } catch(...) {
     errorh("%4d %s catch(...)\n", __LINE__, __FILE__);
     connection_error("catch(...)");
   }
 };
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::protocol2
//
// Purpose-
//       Define the HTTP/2 write protocol handler
//
//----------------------------------------------------------------------------
std::function<void(ClientItem*)>
   Client::protocol2( void )        // Define the HTTP/2 write protocol handler
{  return [this](ClientItem* item)
 { if( HCDM ) debugh("Client(%p)::protocol2(%p)\n", this, item);

   // NOT CODED YET
   delete item;
 };
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::request
//
// Purpose-
//       Create Client Request
//
//----------------------------------------------------------------------------
std::shared_ptr<ClientRequest>      // The associated ClientRequest
   Client::request(                 // Create a Request
     const Options*    opts)        // The associated Options
{
   if( HCDM )
     debugh("Client(%p)::request\n", this);

   if( socket->get_handle() <= 0 )  // If non-operational
     return nullptr;                // Cannot create Request

   std::shared_ptr<ClientStream> stream= ClientStream::make(this, opts);
///debugh("%4d %s shared_ptr<ClientStream>(%p)->(%p)\n", __LINE__, __FILE__, &stream, stream.get()); // TODO: REMOVE
   std::shared_ptr<ClientRequest> request= stream->get_request();
///debugh("%4d %s shared_ptr<ClientRequest>(%p)->(%p)\n", __LINE__, __FILE__, &request, request.get()); // TODO: REMOVE

   return request;
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::read
//
// Purpose-
//       Read ClientResponse data
//
//----------------------------------------------------------------------------
size_t                              // Read length
   Client::read(                    // Read data from Socket
     int               line,        // Caller's line number
     size_t            size)        // Maximum length
{  if( HCDM ) debugh("%4d Client(%p)::read(%zd)\n", line, this, size);

   if( size > BUFFER_SIZE )
     size= BUFFER_SIZE;

   ssize_t L= -1;
   while( L < 0 ) {
     errno= 0;
Trace::trace("HCDM", __LINE__, "CSocket->read");
     L= socket->read(ibuffer.addr, size);
     if( !IS_BLOCKED )
       break;
   }
   if( !operational )
     return 0;

   int ERRNO= errno;
   iodm(line, "socket.read", L);
   if( L <= 0 ) {                   // If error or EOF
     if( L == 0 )
       throw io_eof("Client::read EOF");

     string S= to_string("Client::read %d:%s", ERRNO, strerror(ERRNO));
     throw io_error(S);
   }
   iodm(line, "read", ibuffer.addr, L);
   utility::iotrace(".C<<", ibuffer.addr, L);
   return L;
}

size_t                              // Read length
   Client::read(                    // Read data from Socket
     size_t            size)        // Maximum length
{  return read(__LINE__, size); }

//----------------------------------------------------------------------------
//
// Method-
//       Client::run
//
// Purpose-
//       Read Client responses
//
//----------------------------------------------------------------------------
void
   Client::run( void )              // Read Client Responses
{  if( HCDM ) debugh("Client(%p)::run...\n", this);

   operational= true;               // Indicate operational
   try {
     while( operational ) {
       std::shared_ptr<Stream> stream; // The current Stream

       // Wait for the response
       size_t size= read(__LINE__, ibuffer.size - 1);
       if( !operational )
         break;

       if( proto_id == HTTP_H1 ) {
         stream= stream_set.get_stream(1);
       }

       bool cc= stream->read(ibuffer.addr, size);
       while( !cc ) {
         size= read(__LINE__, ibuffer.size - 1);
         cc= stream->read(ibuffer.addr, size);
       }

       if( proto_id == HTTP_H1 ) {
         stream_set.remove(stream);
         semaphore.post();
Trace::trace(".TXT", __LINE__, "response post");
       }

       stream->end();               // Operation complete
     }
   } catch(io_exception& X) {
     if( IODM )
       errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
     connection_error(X.what());
   } catch(std::exception& X) {
     errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
     connection_error(X.what());
   } catch(...) {
     errorh("%4d %s SHOULD NOT OCCUR\n", __LINE__, __FILE__);
     connection_error("catch(...)");
   }

   if( HCDM ) debugh("...Client(%p)::run\n", this);
   Trace::trace(".CLI", ".XIT", this);
   operational= false;
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::write
//
// Purpose-
//       Write to Server
//
//----------------------------------------------------------------------------
ssize_t                             // Written length
   Client::write(                   // Write to Server
     int               line,        // Caller line number
     const void*       addr,        // Data address
     size_t            size)        // Data length
{  if( HCDM )
     debugh("%4d Client(%p)::write(%p,%zd)\n", line, this, addr, size);

   utility::iotrace(".C>>", addr, size);
   errno= 0;
   ssize_t L= socket->write(addr, size);
   iodm(line, "socket.write", L);
   if( L <= 0 ) {                 // If error or EOF
// traceh("%4d %s HCDM\n", line, __FILE__);
     throw io_error(to_string("Client::write %d:%s", errno, strerror(errno)));
   }
   iodm(line, "write", addr, size);
   return L;
}

ssize_t                             // Written length
   Client::write(                   // Write to Server
     const void*       addr,        // Data address
     size_t            size)        // Data length
{  return write(__LINE__, addr, size); }

//----------------------------------------------------------------------------
//
// Method-
//       Client::writeStream
//
// Purpose-
//       Write a ClientStream Request/Response
//
//----------------------------------------------------------------------------
void
   Client::writeStream(             // Write ClientStream Request/Response
     ClientStream*     S)           // Via this ClientStream
{  if( HCDM ) debugh("Client(%p)::writeStream(%p)\n", this, S);
{{{{
char temp[16];
memset(temp, 0, sizeof(temp));
std::shared_ptr<Request> Q= S->get_request();
snprintf(temp, sizeof(temp), "%s %s %s\r\n", Q->method.c_str(), Q->path.c_str(), Q->proto_id.c_str());
Trace::trace(".CNQ", 0, temp);      // (Client eNQueue)
}}}}

   ClientItem* item= new ClientItem(S);
   if( socket->get_handle() > 0 )
     task.enqueue(item);
   else
     item->post(ClientItem::CC_PURGE);
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::wait
//
// Purpose-
//       Wait for all current outstanding Requests to complete
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Client::wait( void )             // Wait for current Requests to complete
{  if( HCDM ) debugh("Client(%p)::wait\n", this);

   dispatch::Wait done;             // Dispatch wait object
   dispatch::Item item(ClientItem::FC_FLUSH, &done);
// debugh("%4d %s HCDM\n", __LINE__, __FILE__);
// debug("Client::wait");
// ((ClientThread*)thread)->debug("Client::wait");
// task.debug(); // (Writes to trace file)

// debugh("%4d %s HCDM\n", __LINE__, __FILE__);
Trace::trace(".TXT", __LINE__, "Client::wait 1");
   task.enqueue(&item);
   done.wait();

// debugh("%4d %s HCDM\n", __LINE__, __FILE__);
// Trace::trace(".TXT", __LINE__, "Client::wait 2");
//    done.reset();
//    task.enqueue(&item);
//    done.wait();
// debugh("%4d %s HCDM\n", __LINE__, __FILE__);
Trace::trace(".TXT", __LINE__, "Client::wait end");
   return item.cc;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::ClientThread
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   ClientThread::ClientThread(      // Constructor
     Client*           client)      // The associated Client
:  Named("pub::http::Client"), Thread()
,  client(client)
{
   start();
   event.wait();
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   ClientThread::debug(             // Debugging display
     const char*       info) const  // Debugging information
{  debugh("ClientThread(%p)::debug(%s)\n", this, info);

   debugf("..operational(%d) client(%p)\n", operational, client);
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::run
//
// Purpose-
//       Drive Client::run
//
//----------------------------------------------------------------------------
void
   ClientThread::run( void )        // Read Client Responses
{  operational= true;
   event.post();                    // Indicate started
   client->run();                   // (Where the real action is)
   operational= false;
   if( HCDM ) debugh("...ClientThread.run\n");
}
}  // namespace pub::http
