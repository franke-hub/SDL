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
//       2022/10/17
//
// Implementation notes-
//       TODO: Add polling flag in select, used for timeouts.
//
// Implementation notes-
//       Throughput: W: 5584.4/sec  L: 518.6/sec protocol1a (http1)
//       Throughput: W: 4088.7/sec  L: 307.5/sec protocol1b (*removed*)
//
//----------------------------------------------------------------------------
#define OPENSSL_API_COMPAT 30000    // Deprecate OSSL functions < 3.0.0

#include <cassert>                  // For assert
#include <cerrno>                   // For errno
#include <cinttypes>                // For integer types
#include <cstdio>                   // For fprintf
#include <cstring>                  // For memcmp, memset
#include <new>                      // For std::bad_alloc
#include <ostream>                  // For std::ostream
#include <stdexcept>                // For std::runtime_error, ...
#include <string>                   // For std::string

#include <arpa/inet.h>              // For inet_ntop
#include <openssl/err.h>            // For openssl error handling
#include <openssl/ssl.h>            // For openssl core library

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For pub::Dispatch objects
#include <pub/Event.h>              // For pub::Event
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Named.h>              // For pub::Named
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For namespace pub::utility

#include "pub/http/Agent.h"         // For pub::http::ClientAgent (owner)
#include "pub/http/Client.h"        // For pub::http::Client, implementated
#include "pub/http/Ioda.h"          // For pub::http::Ioda
#include "pub/http/Exception.h"     // For pub::http::exceptions
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Request.h"       // For pub::http::Request
#include "pub/http/Response.h"      // For pub::http::Response
#include "pub/http/Stream.h"        // For pub::http::Stream

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::utility::is_null;
using PUB::utility::on_exception;
using PUB::utility::to_string;
using PUB::utility::visify;
using std::string;

namespace _LIBPUB_NAMESPACE::http {  // Implementation namespace
//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#if EAGAIN == EWOULDBLOCK
#  define IS_BLOCK (errno == EAGAIN)
#else
#  define IS_BLOCK (errno == EAGAIN || errno == EWOULDBLOCK)
#endif

#define IS_RETRY (errno == EINTR)

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  IODM= false                      // Input/Output Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

// BUFFER_SIZE= 1'048'576           // Input buffer size
,  BUFFER_SIZE=     8'192           // Input buffer size
}; // enum

//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
typedef Ioda::Mesg     Mesg;
typedef Ioda::Size     Size;

enum FSM                            // Finite State Machine states
{  FSM_RESET= 0                     // Reset - idle
,  FSM_RD_DATA= POLLIN              // Waiting for read completion
,  FSM_WR_DATA= POLLOUT             // Write (data) blocked
,  FSM_WR_HEAD= POLLOUT << 1        // Write (header) blocked
}; // enum FSM

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

//----------------------------------------------------------------------------
//
// Class-
//       ClientItem
//
// Purpose-
//       The Client DispatchItem
//
//----------------------------------------------------------------------------
class ClientItem : public dispatch::Item { // Client DispatchItem
public:
typedef std::shared_ptr<ClientStream>         client_ptr;

client_ptr             stream;      // The associated ClientStream
Ioda                   ioda;        // The Input/Output Data Area

   ClientItem(                      // Constructor
     client_ptr        S)           // The ClientStream
:  dispatch::Item(), stream(S->get_self())
{  if( HCDM && VERBOSE > 0 ) debugh("ClientItem(%p)!\n", this);

   INS_DEBUG_OBJ("ClientItem");
}

virtual
   ~ClientItem( void )              // Destructor
{  if( HCDM && VERBOSE > 0 ) debugh("ClientItem(%p)~\n", this);

   REM_DEBUG_OBJ("ClientItem");
}
}; // class ClientItem

//----------------------------------------------------------------------------
//
// Subroutine-
//       ctx_error
//
// Purpose-
//       Handle CTX creation error
//
//----------------------------------------------------------------------------
static void
   ctx_error(                       // Handle CTX creation error
     const char*       fmt)         // Format string (with one %s)
{
   char buffer[256];                // Working buffer
   long E= ERR_get_error();         // Get last error code
   ERR_error_string(E, buffer);
   std::string S= to_string(fmt, buffer);
   throw SocketException(S);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ctx_password_cb
//
// Purpose-
//       Our pem_password_cb
//
//----------------------------------------------------------------------------
static int                          // Actual password length
   ctx_password_cb(                 // Our pem password callback
    char*              buff,        // Return buffer address (for password)
    int                size,        // Return buffer length  (for password)
    int                rwflag,      // FALSE:decryption, TRUE:encryption
    void*              userdata)    // User data
{
   if( false )                      // Note: only usage of userdata parameter
     debugf("%4d HCDM(%p,%d,%d,%p)\n", __LINE__, buff, size, rwflag, userdata);

   if( rwflag ) {                   // If encryption
     debugf("%4d HCDM SHOULD NOT OCCUR\n", __LINE__);
     return -1;                     // (NOT SUPPORTED)
   }

   const char* result= "xxyyz";     // Our (not so secret) password
   int L= strlen(result);           // Resultant length
   if( L > size ) L= size;          // (Cannot exceed maximum)

   memcpy(buff, result, L);         // Set the resultant
   return L;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       initialize_SSL
//
// Purpose-
//       Initialize SSL
//
//----------------------------------------------------------------------------
static void
   initialize_SSL( void )           // Initialize SSL
{
static std::mutex      mutex;       // Latch protecting initialized
static bool            initialized= false; // TRUE when initialized

   std::lock_guard<decltype(mutex)> lock(mutex);

   if( !initialized ) {
     SSL_library_init();
     SSL_load_error_strings();
//   ERR_load_BIO_strings();        // Deprecated in OSSL version 3.0
     OpenSSL_add_all_algorithms();

     initialized= true;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       iodm
//
// Purpose-
//       I/O debug mode message
//
//----------------------------------------------------------------------------
static void                         // NOTE: Preserves errno
   iodm(                            // I/O Debug Mode message
     int               line,        // Source code line number
     const char*       op,          // Operation
     ssize_t           L)           // Return code/length
{
   int ERRNO= errno;

   if( L < 0 )                      // If I/O error
     debugh("%4d Client %zd= %s() %d:%s\n", line, L, op
           , errno, strerror(errno));
   else if( IODM )                  // If I/O Debug Mode active
     traceh("%4d Client %zd= %s()\n", line, L, op);

   errno= ERRNO;
}

static void
   iodm(                            // I/O Debug Mode trace message
     int               line,        // Source code line number
     const char*       op,          // Operation
     const void*       addr,        // Data address
     ssize_t           size)        // Data length
{
   if( IODM && VERBOSE > 0 ) {
     string V((const char*)addr, size);
     V= visify(V);
     debugh("%4d Client::%s(%p,%zd)\n%s\n", line, op, addr, size, V.c_str());
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       i2v
//       s2v
//
// Purpose-
//       Convert integer to void*
//       Convert size_t  to void*
//
//----------------------------------------------------------------------------
static inline void* i2v(int i)    { return (void*)intptr_t(i); }
static inline void* s2v(size_t s) { return (void*)intptr_t(s); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       new_client_CTX
//
// Purpose-
//       Create a client SSL_CTX
//
//----------------------------------------------------------------------------
static SSL_CTX*
   new_client_CTX( void )           // Create a client SSL_CTX
{
   const SSL_METHOD* method= TLS_client_method();
   SSL_CTX* context= SSL_CTX_new(method);
   if( context == nullptr )
     ctx_error("SSL_CTX_new: %s");

   SSL_CTX_set_mode(context, SSL_MODE_AUTO_RETRY);
   SSL_CTX_set_default_passwd_cb(context, ctx_password_cb);

   return context;
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::Client    (Constructor)
//       Client::~Client   (Destructor)
//       Client::make      (Creator)
//
//----------------------------------------------------------------------------
   Client::Client(                  // Constructor
     ClientAgent*      owner,       // Our ClientAgent
     const sockaddr_u& addr,        // Target internet address
     socklen_t         size,        // Target internet address length
     const Options*    opts)        // Client Options
:  std::mutex()
,  h_close([]() {})
,  agent(owner->get_self())
,  proto_id(HTTP_H1)
,  sem_rd(0), sem_wr(1)
,  size_inp(BUFFER_SIZE)
,  size_out(BUFFER_SIZE)
,  task_inp([this](dispatch::Item* item)
   { if( HCDM ) debugh("Client(%p)::task_inp(%p)\n", this, item);
     h_iptask((ClientItem*)item);
   })
,  task_out([this](dispatch::Item* item)
   { if( HCDM ) debugh("Client(%p)::task_out(%p)\n", this, item);
     h_optask((ClientItem*)item);
   })
{  if( HCDM )
     debugh("Client(%p)!(%p)\n>> %s\n", this, owner
           , addr.to_string().c_str());

   // Internal consistency check
// assert( size_t(size) <= sizeof(sockaddr_u)); // (Checked in Socket::connect)

   // Handle Options
   bool encrypt= false;             // Default, not encrypted
   if( opts ) {
     const char* type= opts->locate(OPT_PROTO); // Get specified protocol
     if( type ) {                   // If protocol specified
       if( strcmp(type, HTTP_H2) == 0 || strcmp(type, HTTP_S2) == 0 ) {
         proto_id= HTTP_H2;
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
   if( proto_id == HTTP_H1 )  {
     http1();
   } else {
     http2();
   }

   // Create connection
   if( !encrypt ) {
     socket= new Socket();
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
       if( HCDM ) {
         errno= ERRNO;
         utility::report_error(__LINE__, __FILE__, "connect");
       }
       socket->close();
       return;
     }
     if( HCDM ) {
       debugf("Client(%p): %s connected\n", this, addr.to_string().c_str());
     }
   } else {                         // If SSL
     initialize_SSL();              // Initialize SSL
     context= new_client_CTX();
     // NOT CODED YET
   }

   // Initialize asynchronous operation
   socket->set_flags( socket->get_flags() | O_NONBLOCK );
   socket->on_select([this](int revent) { async(revent); });
   owner->select.insert(socket, POLLIN);

   // Client construction complete.
   operational= true;
   INS_DEBUG_OBJ("*Client*");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Client::~Client( void )          // Destructor
{  if( HCDM )
     debugh("Client(%p)~\n", this);

   // Disconnect the Client
   on_close([]() {});               // Disconnect the close handler
   close();                         // Close the Client, disconnecting it

   // Release allocated storage
   if( context )                    // If context exists
     SSL_CTX_free(context);
   delete socket;
   REM_DEBUG_OBJ("*Client*");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
{  debugf("Client(%p)::debug(%s) operational(%d) fsm(%.2x)\n", this
         , info, operational, fsm);

   debugf("..agent(%p) context(%p) proto_id(%s) sem_rd(%u) sem_wr(%u)\n"
         , agent.lock().get(), context, proto_id
         , sem_rd.get_count(), sem_wr.get_count());
   socket->debug("Client.socket");
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::async
//
// Purpose-
//       Handle Asynchronous Polling Event
//
//----------------------------------------------------------------------------
void
   Client::async(                   // Handle Asynchronous Polling Event
     int               revent)      // Polling revent
{  if( HCDM )
     debugh("Client(%p)::async(%.4x) fsm(%.4x)\n", this, revent, fsm);
   Trace::trace(".CLI", ".APE", this, s2v((size_t(revent) << 32) | fsm));

   // If a Socket error occurred
   if( revent & (POLLERR | POLLNVAL) ) {
     debugf("%4d HCDM Client revent(%.4x) fsm(%.4x)\n", __LINE__, revent, fsm);
     error("async error detected");
     return;
   }

   // If Socket is readable
   if( revent & (POLLIN | POLLPRI) ) {
     if( fsm & FSM_RD_DATA ) {
       h_reader();
     } else {
//     traceh("%4d HCDM Client revent(%.4x) fsm(%.4x)\n", __LINE__
//           , revent, fsm);
     }
     return;
   }

   // If Socket is writable
   if( revent & POLLOUT ) {
     if( fsm & (FSM_WR_HEAD | FSM_WR_DATA) ) {
       h_writer();
     } else {
       Select* select= socket->get_select();
       select->modify(socket, POLLIN);
     }
     return;
   }

   // If unexpected event TODO: Add recovery, considering revent, fsm
   // TODO: Might need to keep track of event in FSM
   debugf("%4d HCDM Client revent(%.4x) fsm(%.4x)\n", __LINE__, revent, fsm);
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
   wait();                          // Flush the work queue
   // Note: Must call agent->disconnect before socket->close.
   // The Socket's peer_addr is needed for the Agent's map lookup.
   std::shared_ptr<ClientAgent> agent= this->agent.lock();
   if( agent ) {                    // Report close to Agent
     agent->disconnect(this);
   }
   socket->close();                 // Close the Socket (immediate halt)
   h_close();                       // Drive the close handler
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::error
//
// Purpose-
//       Handle connection error.
//
//----------------------------------------------------------------------------
void
   Client::error(                   // Handle connection error
     const char*       info)        // Diagnostic information
{  errorh("Client(%p)::error(%s)\n", this, info);

   // TODO: PRELIMINARY
   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::http1
//
// Purpose-
//       Initialize the HTTP/1.0 and HTTP/1.1 write protocol handlers
//
//----------------------------------------------------------------------------
void
   Client::http1( void )            // Initialize the HTTP/1 protocol handlers
{
   // h_iptask - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   h_iptask= [this](ClientItem* item) // Input task
   { if( HCDM ) debugh("Client(%p)::h_iptask(%p)\n", this, item);

     if( item->stream->read(item->ioda) ) // If operation complete
       sem_rd.post();
     item->post();                  // (Omitted line found using Diagnostic.h)
   }; // h_iptask=

   // h_optask - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   h_optask= [this](ClientItem* item) // Output task
   { if( HCDM ) debugh("Client(%p)::h_optask(%p)\n", this, item);

     // debugh("%4d %s HCDM\n", __LINE__, __FILE__);
     stream_item= item;
     stream= item->stream;
     try {
       // Format the request buffer
       std::shared_ptr<ClientRequest> request= stream->get_request();
       Request& Q= *request.get();    // (Q protected by request)

       if( socket->get_handle() <= 0 ) // If connection broken
         throw stream_error("disconnected");

       // TODO: VERIFY method, path, and proto_id (avoiding connection error)
       ioda_off= 0;
       ioda_out.reset();
       ioda_out.put(Q.method);
       ioda_out.put(' ');
       ioda_out.put(Q.path);
       ioda_out.put(' ');
       ioda_out.put(Q.proto_id);
       ioda_out.put("\r\n");

       // Set Content-Length
       Options& opts= Q;
       opts.remove(HTTP_SIZE);
       Ioda& ioda= Q.get_ioda();
       size_t content_length= ioda.get_used();
// debugh("Method(%s) Content_Length(%zd)\n", Q.method.c_str(), content_length); // TODO: REMOVE
       if(content_length != 0 ) {
         if( Q.method != HTTP_POST && Q.method != HTTP_PUT ) {
           if( VERBOSE )
             fprintf(stderr, "Method(%s) does not permit content\n"
                    , Q.method.c_str());
           stream->reject(400);     // Bad request
           throw stream_error("content-length disallowed");
         }
       } else if( Q.method == "POST" || Q.method == "PUT" ) {
         // ??TODO: MOVE TO SERVER??
         stream->reject(411);       // Length required
         throw stream_error("content-length required");
       }

       // Unpack header items
       typedef Options::const_iterator iterator;
       for(iterator i= opts.begin(); i != opts.end(); ++i) {
         ioda_out.put(i->first);
         ioda_out.put(':');
         ioda_out.put(i->second);
         ioda_out.put("\r\n");
       }

       // Add Content-Length (if required)
       if( content_length ) {
         ioda_out.put(HTTP_SIZE);
         ioda_out.put(':');
         ioda_out.put(std::to_string(content_length));
         ioda_out.put("\r\n");
       }
       ioda_out.put("\r\n");      // Add header delimiter
// ioda_out.debug("h_optask");

       // Write the request headers
// assert( fsm == 0 );
// assert( sem_rd.get_count() == 0 );
       fsm= FSM_WR_HEAD;            // Update state
       if( content_length )
         fsm |= FSM_WR_DATA;
       h_writer();
       sem_rd.wait();               // Wait for response
     } catch(Exception& X) {
       errorh("%4d %s %s\n", __LINE__, __FILE__, ((std::string)X).c_str());
       error(X.what());
     } catch(io_exception& X) {
       if( IODM )
         errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
       error(X.what());
     } catch(stream_error& X) {
       if( IODM )
         errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
       error(X.what());
     } catch(std::exception& X) {
       errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
       error(X.what());
     } catch(const char* X) {
       errorh("%4d %s catch(%s)\n", __LINE__, __FILE__, X);
       error(X);
     } catch(...) {
       errorh("%4d %s catch(...)\n", __LINE__, __FILE__);
       error("catch(...)");
     }

     // Stream processing is complete
     stream= nullptr;
     stream_item= nullptr;
     item->post();
   }; // h_optask=

   // h_reader - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   h_reader= [this](void)           // (Asynchronous) input data available
   { if( HCDM ) debugh("Client(%p)::h_reader\n", this);

     if( (fsm & FSM_RD_DATA) == 0 ) { // If unexpected data
       // This SHOULD NOT OCCUR.
       // We set FSM_RD_DATA *BEFORE* writing the last piece of data.
       debugf("%4d Client::h_reader fsm(%.2x)\n", __LINE__, fsm);
       return;
     }

     // Read the response, passing it to Stream
     read(__LINE__);                // (Exception if error)
   }; // h_reader=

   // h_writer - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   h_writer= [this](void)           // The output writer
   { if( HCDM ) debugh("Client(%p)::h_writer\n", this);

     std::shared_ptr<ClientStream> stream= stream_item->stream;
     if( fsm & FSM_WR_HEAD ) {      // Write the request header?
       if( (fsm & FSM_WR_DATA) == 0 )
         fsm |= FSM_RD_DATA;

       ssize_t L= write(__LINE__);
       if( L <= 0 ) {               // If blocked (else io_error exception)
         fsm &= ~FSM_RD_DATA;       // (write() updated select event)
         return;
       }

       // Implementation note: if there's no data, the server could have
       // alreaday received the request and sent the response.
       fsm &= ~FSM_WR_HEAD;
       if( fsm & FSM_WR_DATA ) {    // If there's data to be sent
         std::shared_ptr<ClientRequest> request= stream->get_request();
         Request& Q= *request.get();  // (Q protected by request)
         ioda_out= std::move(Q.get_ioda());
       } else {
         ioda_out.reset();
       }
       ioda_off= 0;
     }

     if( fsm & FSM_WR_DATA ) {      // Write the request data?
       fsm |= FSM_RD_DATA;
       ssize_t L= write(__LINE__);
       if( L <= 0 ) {               // If blocked (else io_error exception)
         fsm &= ~FSM_RD_DATA;       // (write() updated select event)
         return;
       }

       fsm &= ~FSM_WR_DATA;
     }
   }; // h_writer=
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::http2
//
// Purpose-
//       Initialize the HTTP/2 protocol handlers
//
//----------------------------------------------------------------------------
void
   Client::http2( void )            // Initialize the HTTP/2 protocol handler
{
   throw std::runtime_error("NOT CODED YET");
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::request
//
// Purpose-
//       Create Client Request
//
//---------------------------------------------------------------------------
std::shared_ptr<ClientRequest>      // The associated ClientRequest
   Client::request(                 // Create a Request
     const Options*    opts)        // The associated Options
{  if( HCDM ) debugh("Client(%p)::request\n", this);

   if( socket->get_handle() <= 0 )  // If non-operational
     return nullptr;                // Cannot create Request

   std::shared_ptr<ClientStream> stream= ClientStream::make(this, opts);
   std::shared_ptr<ClientRequest> request= stream->get_request();

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
// Implementation notes:
//       An exception is thrown on any error condition.
//       Only called from async; IS_BLOCK is an eror condition.
//       TODO: Consider multiple reads, moving get_rd_mesg inside for loop.
//
//----------------------------------------------------------------------------
void
   Client::read(                    // Read data from Socket
     int               line)        // Caller's line number
{  if( HCDM ) debugh("%4d Client(%p)::read\n", line, this);

   Ioda ioda;
   Mesg mesg;
   ioda.get_rd_mesg(mesg, size_inp);

   for(;;) {
// This helps when a trace read appears before the trace write
// Trace::trace("HCDM", __LINE__, "CSocket->read");
     ssize_t L= socket->recvmsg((msghdr*)&mesg, 0);
     iodm(line, "read", L);
     if( L > 0 ) {
       ioda.set_used(L);
       void* addr= mesg.msg_iov[0].iov_base;
       ssize_t size= mesg.msg_iov[0].iov_len;
       if( size > L )
         size= L;
       utility::iotrace(".C<<", addr, size);
       iodm(line, "read", addr, size);
       ClientItem* item= new ClientItem(stream);
       item->ioda= std::move(ioda);
       task_inp.enqueue(item);
       return;
     }
     if( !IS_RETRY )
       throw io_error(to_string("Client::read %d:%s", errno, strerror(errno)));
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::write
//
// Purpose-
//       (Synchronously) transmit data
//
//----------------------------------------------------------------------------
ssize_t                             // Written length
   Client::write(                   // Write to Server
     int               line)        // Caller line number
{  if( HCDM ) debugh("%4d Client(%p)::write\n", line, this);

   for(;;) {
Trace::trace("HCDM", __LINE__, "CSocket->write");
     Mesg mesg; ioda_out.get_wr_mesg(mesg, size_out, ioda_off);
     ssize_t L= socket->sendmsg((msghdr*)&mesg, 0);
     iodm(line, "sendmsg", L);
     if( L > 0 ) {
       void* addr= mesg.msg_iov[0].iov_base;
       ssize_t size= mesg.msg_iov[0].iov_len;
       if( size > L )
         size= L;
       utility::iotrace(".C>>", addr, size);
       iodm(line, "sendmsg", addr, size);

       size_t want= ioda_out.get_used() - ioda_off;
       if( size_t(L) < want ) {
         ioda_off += L;
         continue;
       }
       return ioda_out.get_used();
     }

     if( !IS_RETRY )
       break;
   }

   if( !IS_BLOCK )
     throw io_error(to_string("Client::write %d:%s", errno, strerror(errno)));
   Select* select= socket->get_select();
   if( select )
     select->modify(socket, POLLIN | POLLOUT);
   return -1;
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::write
//
// Purpose-
//       Write a ClientStream Request
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Client::write(                   // Write ClientStream Request
     ClientStream*     S)           // Via this ClientStream
{  if( HCDM ) debugh("Client(%p)::write(Stream* %p)\n", this, S);
{{{{
char temp[16];
memset(temp, 0, sizeof(temp));
std::shared_ptr<Request> Q= S->get_request();
snprintf(temp, sizeof(temp), "%s %s %s\r\n", Q->method.c_str(), Q->path.c_str()
        , Q->proto_id.c_str());
Trace::trace(".CNQ", 0, temp);      // (Client eNQueue)
}}}}

   int rc= ClientItem::CC_PURGE;    // Default, not sent
   if( socket->get_handle() >= 0 ) {
     ClientItem* item= new ClientItem(S->get_self());
     task_out.enqueue(item);
     rc= 0;
   }

   return rc;
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
   dispatch::Item item(dispatch::Item::FC_CHASE, &done);
   task_out.enqueue(&item);
// Trace::trace(".CLI", __LINE__, "wait...");
   done.wait();
// Trace::trace(".CLI", __LINE__, "...wait");
   return item.cc;
}
}  // namespace _LIBPUB_NAMESPACE::http
