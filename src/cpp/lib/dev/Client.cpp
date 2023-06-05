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
//       Client.cpp
//
// Purpose-
//       Implement http/Client.h
//
// Last change date-
//       2023/06/03
//
// Implmentation note-
//       TODO: Test _read() disconnect (close processing)
//
//----------------------------------------------------------------------------
#define OPENSSL_API_COMPAT 30000    // Deprecate OSSL functions < 3.0.0

#include <atomic>                   // For std::atomic<int>
#include <cassert>                  // For assert
#include <cerrno>                   // For errno
#include <cinttypes>                // For integer types
#include <cstdio>                   // For fprintf
#include <cstring>                  // For memcmp, memset
#include <new>                      // For std::bad_alloc
#include <stdexcept>                // For std::runtime_error, ...
#include <string>                   // For std::string

#include <arpa/inet.h>              // For inet_ntop
#include <openssl/err.h>            // For openssl error handling
#include <openssl/ssl.h>            // For openssl core library
#include <time.h>                   // For clock_gettime

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For pub::Dispatch objects
#include <pub/Event.h>              // For pub::Event
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Named.h>              // For pub::Named
#include <pub/Statistic.h>          // For pub::Active_record
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
// Forward references
//----------------------------------------------------------------------------
static inline void* i2v(intptr_t);

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  IODM= false                      // Input/Output Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

// BUFFER_SIZE= 1'048'576           // Input buffer size
,  BUFFER_SIZE=     8'192           // Input buffer size

,  USE_ITRACE= true                 // Use internal trace?
,  USE_READ_ONCE= true              // Read once?
,  USE_REPORT= false                // Use event Reporter?
}; // enum

//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
typedef Ioda::Mesg     Mesg;

enum EVT                            // Event states
{  EVT_RESET= 0                     // Reset - idle
,  EVT_RD_DATA= POLLIN              // Waiting for read completion
,  EVT_WR_DATA= POLLOUT             // Write (data) blocked
,  EVT_WR_HEAD= POLLOUT << 1        // Write (header) blocked
}; // enum EVT

// Imported Options
typedef const char     CC;
static constexpr CC*   HTTP_SIZE= Options::HTTP_HEADER_LENGTH;

static constexpr CC*   HTTP_POST= Options::HTTP_METHOD_POST;
static constexpr CC*   HTTP_PUT=  Options::HTTP_METHOD_PUT;

static constexpr CC*   OPT_PROTO= Options::HTTP_OPT_PROTOCOL; // Protocol type

//----------------------------------------------------------------------------
// Constant data
//----------------------------------------------------------------------------
enum HTTP_PROTO
{  HTTP_H1                          // HTTP/1.1
,  HTTP_H2                          // HTTP/2
,  HTTP_S1                          // HTTPS/1.1
,  HTTP_S2                          // HTTPS/2
,  HTTP_PROTO_LENGTH
}; // enum HTTP_PROTO

static const char*     proto[HTTP_PROTO_LENGTH]=
{  Options::HTTP_PROTOCOL_H1        // HTTP/1.1
,  Options::HTTP_PROTOCOL_H2        // HTTP/2
,  Options::HTTP_PROTOCOL_S1        // HTTPS/1.1
,  Options::HTTP_PROTOCOL_S2        // HTTPS/2
}; // proto[]

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::atomic_int _serialno= 1; // Serial number

//----------------------------------------------------------------------------
// Event reporting
//----------------------------------------------------------------------------
static Active_record   client_count("Client"); // Client counter
static Active_record   item_count("ClientItem"); // ClientItem counter
static Active_record   socket_count("ClientSocket"); // Client Socket counter

namespace {
static struct StaticGlobal {
   StaticGlobal(void)               // Constructor
{
   if( USE_REPORT ) {
     client_count.insert();
     item_count.insert();
     socket_count.insert();
   }
}

   ~StaticGlobal(void)              // Destructor
{
   if( USE_REPORT ) {
     client_count.remove();
     item_count.remove();
     socket_count.remove();
   }
}
}  staticGlobal;
}  // Anonymous namespace

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
typedef std::shared_ptr<Client>               client_ptr;
typedef std::shared_ptr<ClientStream>         stream_ptr;

enum                                // Function codes
{  FC_CLOSE= 2                      // CLOSE
};

client_ptr             client;      // The associated Client
int                    serialno;    // Client serial number
int                    sequence;    // ClientItem sequence number
stream_ptr             stream;      // The associated ClientStream
Ioda                   ioda;        // The Input/Output Data Area

   ClientItem(                      // Constructor
     client_ptr        C,           // The Client
     stream_ptr        S)           // The ClientStream
:  dispatch::Item(), client(C), serialno(C->serialno), sequence(++C->sequence)
,  stream(S), ioda()
{  if( HCDM && VERBOSE > 0 ) debugh("ClientItem(%p)!\n", this);

   if( USE_ITRACE )
     Trace::trace(".NEW", "CITM", this);

   if( USE_REPORT )
     item_count.inc();

   INS_DEBUG_OBJ("ClientItem");
}

virtual
   ~ClientItem( void )              // Destructor
{  if( HCDM && VERBOSE > 0 ) debugh("ClientItem(%p)~\n", this);

   if( USE_ITRACE )
     Trace::trace(".DEL", "CITM", this, i2v(fc));

   if( USE_REPORT )
     item_count.dec();

   REM_DEBUG_OBJ("ClientItem");
}

virtual void
   debug(const char* info) const
{  debugf("ClientItem(%p)::debug(%s) client(%p) stream(%p)\n", this, info
         , client.get(), stream.get());

   debugf("..serialno(%d) sequence(%d)\n", serialno, sequence);
   debugf("..fc(%d) cc(%d) done(%p)\n", fc, cc, done);
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
static inline void
   initialize_SSL( void )           // Initialize SSL
{
static std::mutex      mutex;       // Latch protecting initialized
static bool            initialized= false; // TRUE when initialized

   std::lock_guard<decltype(mutex)> lock(mutex);

   if( !initialized ) {
     SSL_library_init();
     SSL_load_error_strings();
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
//       a2v
//
// Purpose-
//       Convert asynchronous event information to void*
//
//----------------------------------------------------------------------------
static inline void*
   a2v(int events, int revents, int fd)
{  return (void*)(intptr_t(events)  << 48 | intptr_t(revents) << 32 | fd); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       i2v
//
// Purpose-
//       Convert intptr_t  to void*
//
//----------------------------------------------------------------------------
static inline void* i2v(intptr_t i) { return (void*)i; }

//----------------------------------------------------------------------------
//
// Subroutine-
//       new_client_CTX
//
// Purpose-
//       Create a client SSL_CTX
//
//----------------------------------------------------------------------------
static inline SSL_CTX*
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
     ClientAgent*      owner)       // Our ClientAgent
:  std::mutex()
,  agent(owner)
,  proto_id(proto[HTTP_H1])
,  size_inp(BUFFER_SIZE)
,  size_out(BUFFER_SIZE)
// stream_set(&root)
,  task_inp([this](dispatch::Item* it) { inp_task(it); })
,  task_out([this](dispatch::Item* it) { out_task(it); })
{  if( HCDM || VERBOSE > 1 ) debugh("Client(%p)!(%p)\n", this, owner);
   if( USE_ITRACE )
     Trace::trace(".NEW", "HCLI", this);

   serialno= (_serialno += 10);

   if( USE_REPORT )
     client_count.inc();

   INS_DEBUG_OBJ("*Client*");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Client::~Client( void )          // Destructor
{  if( HCDM || VERBOSE > 1 ) debugh("Client(%p)~\n", this);
   if( USE_ITRACE )
     Trace::trace(".DEL", "HCLI", this, stream.get());

   // Delete the socket
   if( socket ) {
     Select* select= socket->get_select();
     if( select )
       select->flush();

     delete socket;
     socket= nullptr;
     if( USE_REPORT )
       socket_count.dec();
   }

   if( context )                    // If context exists
     SSL_CTX_free(context);

   if( USE_REPORT )
     client_count.dec();

   REM_DEBUG_OBJ("*Client*");

   // Implementation note:
   // After return, C++ invokes task_inp and task_out destructors
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::shared_ptr<Client>             // (New) Client
   Client::make(                    // Creator
     ClientAgent*      owner)       // Our ClientAgent
{  if( HCDM ) debugh("Client::make(%p)\n", owner);

   std::shared_ptr<Client> client= std::make_shared<Client>(owner);
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
{  debugf("Client(%p)::debug(%s) fsm(%d) events(0x%.2x)\n"
         , this, info, fsm, events);

   debugf("..serialno(%d), sequence(%d)\n", serialno, sequence);
   debugf("..agent(%p) context(%p) proto_id(%s) rd_complete(%u)\n"
         , agent, context, proto_id, rd_complete.is_post());
   debugf("..size_inp(%'zd) size_out(%'zd)\n", size_inp, size_out);
   socket->debug("Client.socket");
   debugf("task_inp:\n"); task_inp.debug(info);
   debugf("task_out:\n"); task_out.debug(info);
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
     int               revents)     // Polling revents
{  if( HCDM )
     debugh("Client(%p)::async(%.4x) events(%.4x)\n", this, revents, events);
   if( USE_ITRACE )
     Trace::trace(".CLI", ".APE", this, a2v(events, revents, get_handle()));

   if( fsm != FSM_READY )           // Ignore event if non-operational
     return;

   // If a Socket error occurred
   if( revents & (POLLERR | POLLNVAL) ) {
     debugf("%4d HCDM Client revents(%.4x)\n", __LINE__, revents);
     error("async error detected");
     return;
   }

   // If Socket is readable
   if( revents & (POLLIN | POLLPRI) ) {
     if( events & EVT_RD_DATA )
       h_reader();

     return;
   }

   // If Socket is writable
   if( revents & POLLOUT ) {
     if( events & (EVT_WR_HEAD | EVT_WR_DATA) ) {
       h_writer();
     } else {
       Select* select= socket->get_select();
       select->modify(socket, POLLIN);
     }
     return;
   }

   // If unexpected event TODO: Add recovery, considering revents, events
   // TODO: Might need to keep track of event in events
   debugf("%4d HCDM Client revents(%.4x) events(%.4x)\n", __LINE__
         , revents, events);
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::close
//
// Purpose-
//       Close the Client, making it inoperative.
//
//----------------------------------------------------------------------------
void
   Client::close( void )            // Close the Client
{  if( HCDM ) debugh("Client(%p)::close() fsm(%d)\n", this, fsm);
   if( USE_ITRACE )
     Trace::trace(".CLI", ".CLS", this, i2v(get_handle()));

   {{{{
     std::lock_guard<Client> lock(*this);

     if( fsm != FSM_RESET ) {
       fsm= FSM_RESET;
       // Note: Agent::disconnect uses socket->get_peer_addr(), therefore
       // agent->disconnect() must precede socket->close()
       agent->disconnect(this);     // (Only called once)
       socket->close();             // (Only called once)
     }
   }}}}

   if( !rd_complete.is_post() )     // Post out_task wait
     rd_complete.post(dispatch::Item::CC_PURGE);
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::close_enq
//
// Purpose-
//       Schedule a close operation
//
//----------------------------------------------------------------------------
void
   Client::close_enq( void )        // Schedule Client close
{  if( HCDM ) debugh("Client(%p)::close_enq() fsm(%d)\n", this, fsm);
   if( USE_ITRACE )
     Trace::trace(".CLI", "CLSQ", this, i2v(get_handle()));

   if( fsm == FSM_READY ) {
     fsm= FSM_CLOSE;                // Close in progress
     Select* select= socket->get_select();
     if( select )
       select->modify(socket, 0);   // Remove from poll list

     ClientItem* item= new ClientItem(get_self(), stream);
     item->fc= item->FC_CLOSE;
     task_inp.enqueue(item);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::connect
//
// Purpose-
//       Connect to server
//
//----------------------------------------------------------------------------
Socket*                             // Resultant Socket (nullptr if failure)
   Client::connect(                 // Connect to Server
     const sockaddr*   addr,        // Server internet address
     socklen_t         size,        // Server internet address length
     const Options*    opts)        // Client Options
{
   // Handle Options
   bool encrypt= false;             // Default, not encrypted
   int proto_ix= HTTP_H1;
   if( opts ) {
     const char* type= opts->locate(OPT_PROTO); // Get specified protocol
     if( type ) {                   // If protocol specified
       int proto_ix= -1;
       for(int i= 0; i<HTTP_PROTO_LENGTH; ++i) {
         if( strcmp(type, proto[i]) == 0 ) {
           proto_ix= i;
           break;
         }
       }

       if( proto_ix < 0 ) {         // If invalid protocol specified
         errno= EINVAL;             // Invalid argument
         return nullptr;
       }

       proto_id= proto[proto_ix];
       if( proto_ix == HTTP_S1 || proto_ix == HTTP_S2 )
         encrypt= true;
     }
   }
   if( proto_ix == HTTP_H1 )  {
     _http1();
   } else {
     _http2();
   }

   // Create connection
   if( !encrypt ) {
     socket= new Socket();
     if( USE_REPORT )
       socket_count.inc();

     int rc= socket->open(addr->sa_family, SOCK_STREAM, PF_UNSPEC);
     if( IODM ) {
       int ERRNO= errno;
       traceh("%4d Client %d= open(%d,%d,%d)\n", __LINE__, rc
             , addr->sa_family, SOCK_STREAM, PF_UNSPEC);
       errno= ERRNO;
     }
     if( rc ) {                       // If unable to open
       utility::report_error(__LINE__, __FILE__, "open");
       return nullptr;
     }

     const sockaddr_u* addr_u= (sockaddr_u*)addr;
     rc= socket->connect(addr, size); // Connect
     if( IODM ) {
       int ERRNO= errno;
       traceh("%4d Client %d= connect(%s)\n", __LINE__, rc
             , addr_u->to_string().c_str());
       errno= ERRNO;
     }
     if( rc ) {                       // If unable to connect
       if( HCDM )
         utility::report_error(__LINE__, __FILE__, "connect");

       delete socket;
       if( USE_REPORT )
         socket_count.dec();
       socket= nullptr;
       return nullptr;
     }
     if( HCDM )
       debugf("Client(%p): %s connected\n", this, addr_u->to_string().c_str());
   } else {                         // If SSL
     initialize_SSL();              // Initialize SSL
     context= new_client_CTX();
     delete socket;
     if( USE_REPORT )
       socket_count.dec();
     socket= nullptr;

     // NOT CODED YET
     errno= EINVAL;
     return nullptr;
   }

   // Initialize asynchronous operation
   fsm= FSM_READY;
   socket->set_flags( socket->get_flags() | O_NONBLOCK );
   socket->on_select([this](int revents) { async(revents); });
   agent->select.insert(socket, POLLIN);

   // Client connected.
   if( USE_ITRACE )
     Trace::trace(".CLI", "CONN", this, nullptr
                 , socket, i2v(socket->get_handle()));

   return socket;
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

   close_enq();
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::make_stream
//
// Purpose-
//       Create Client Stream
//
//---------------------------------------------------------------------------
std::shared_ptr<ClientStream>       // The ClientStream
   Client::make_stream(             // Create a ClientStream
     const Options*    opts)        // The associated Options
{  if( HCDM ) debugh("Client(%p)::make_stream(%p)\n", this, opts);

   if( socket->get_handle() <= 0 )  // If non-operational
     return nullptr;                // Cannot create Request

   std::shared_ptr<ClientStream> stream= ClientStream::make(this, opts);
   return stream;
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::wait
//
// Purpose-
//       Wait until idle
//
//----------------------------------------------------------------------------
void
   Client::wait( void )             // Wait for current Requests to complete
{  if( HCDM ) debugh("Client(%p)::wait\n", this);

   dispatch::Wait wait;
   dispatch::Item item(item.FC_CHASE, &wait);
   if( USE_ITRACE )
     Trace::trace(".ENQ", "WOUT", this, &item);
   task_out.enqueue(&item);
   wait.wait();
   wait.reset();

   if( USE_ITRACE )
     Trace::trace(".ENQ", "WINP", this, &item);
   task_inp.enqueue(&item);
   wait.wait();
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
   Client::write(                   // Write using
     ClientStream*     S)           // This ClientStream
{  if( HCDM ) debugh("Client(%p)::write(Stream* %p)\n", this, S);

   int rc= ClientItem::CC_PURGE;    // Default, not sent

   std::lock_guard<Client> lock(*this);

   if( fsm == FSM_READY ) {
     ClientItem* item= new ClientItem(get_self(), S->get_self());
     if( USE_ITRACE )
       Trace::trace(".ENQ", "COUT", this, item);
     task_out.enqueue(item);
     rc= 0;
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Client::_http1
//
// Purpose-
//       Initialize the HTTP/1.0 and HTTP/1.1 protocol handlers
//
// Implementation notes (Issue #3: close operation deadlock)-
//       We can't post ClientItems (which contain a shared_ptr<Client>)
//       anywhere in inp/out_task because Client destruction might occur.
//       If it does, the incomplete inp/out_task blocks the FC_CHASE wait
//       operation that dispatch::Task::~Task would require.
//
// Implementation notes-
//       Defines: inp_task  (Handles responses)
//       Defines: out_task  (Handles requests)
//       Defines: h_reader  (Handles asynchronous data available)
//       Defines: h_writer  (Data writer, synchronous and asynchronous)
//
//----------------------------------------------------------------------------
void
   Client::_http1( void )           // Initialize the HTTP/1 protocol handlers
{
   // inp_task - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   inp_task= [this](dispatch::Item* it) // Input task
   { if( HCDM ) debugh("Client(%p)::inp_task(%p)\n", this, it);
     if( USE_ITRACE )
       Trace::trace(".DEQ", "CINP", this, it);

     ClientItem* item= static_cast<ClientItem*>(it);
     if( item->serialno != serialno )
       utility::checkstop(__LINE__, __FILE__, "inp_task");

     if( fsm != FSM_READY ) {
       if( item->fc == item->FC_CLOSE )
         close();

       item->cc= item->CC_PURGE;
       dispatch::Disp::defer(item);
       return;
     }

     if( item->stream->read(item->ioda) ) { // If response complete
       rd_complete.post();          // Indicate HTTP/1 operation complete
     }

     dispatch::Disp::defer(item);
   }; // inp_task=

   // out_task - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   out_task= [this](dispatch::Item* it) // Output task
   { if( HCDM ) debugh("Client(%p)::out_task(%p)\n", this, it);
     if( USE_ITRACE )
       Trace::trace(".DEQ", "COUT", this, it);

     ClientItem* item= static_cast<ClientItem*>(it);
     if( item->serialno != serialno )
       utility::checkstop(__LINE__, __FILE__, "out_task");

     if( fsm != FSM_READY ) {
       item->cc= item->CC_PURGE;
       dispatch::Disp::defer(item);
       return;
     }

     stream_item= item;
     stream= item->stream;
     try {
       // Format the request buffer
       std::shared_ptr<ClientRequest> request= stream->get_request();
       Request& Q= *request.get();    // (Q protected by request)

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
       Q.remove(HTTP_SIZE);
       Ioda& ioda= Q.get_ioda();
       size_t content_length= ioda.get_used();
       if(content_length != 0 ) {
         if( Q.method != HTTP_POST && Q.method != HTTP_PUT ) {
           if( VERBOSE > 0 )
             fprintf(stderr, "Method(%s) does not permit content\n"
                    , Q.method.c_str());
           item->post(-400);
           return;
         }
       } else if( Q.method == "POST" || Q.method == "PUT" ) {
         item->post(-411);
         return;
       }

       // Unpack header items
       typedef Options::const_iterator iterator;
       Options& opts= Q.get_opts();
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

       // Write the request headers
       events= EVT_WR_HEAD;       // Update state
       if( content_length )
         events |= EVT_WR_DATA;
       h_writer();
       rd_complete.wait();          // Wait for HTTP/1 operation completion
       rd_complete.reset();
     } catch(io_exception& X) {
       close_enq();
       if( IODM )
         errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
       error(X.what());
     } catch(Exception& X) {
       close_enq();
       errorh("%4d %s %s\n", __LINE__, __FILE__, ((std::string)X).c_str());
       error(X.what());
     } catch(std::exception& X) {
       close_enq();
       errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
       error(X.what());
     } catch(const char* X) {
       close_enq();
       errorh("%4d %s catch(%s)\n", __LINE__, __FILE__, X);
       error(X);
     } catch(...) {
       close_enq();
       errorh("%4d %s catch(...)\n", __LINE__, __FILE__);
       error("catch(...)");
     }

     // Stream processing is complete
     stream->end();
     stream= nullptr;
     stream_item= nullptr;

     if( USE_ITRACE )
       Trace::trace(".XIT", "COUT", this, it);
     dispatch::Disp::defer(item);
   }; // out_task=

   // h_reader - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   h_reader= [this](void)           // (Asynchronous) input data available
   { if( HCDM ) debugh("Client(%p)::h_reader\n", this);

     if( (events & EVT_RD_DATA) == 0 ) { // If unexpected data
       // This SHOULD NOT OCCUR.
       // We set EVT_RD_DATA *BEFORE* writing the last piece of data.
       debugf("%4d Client::h_reader events(%.2x)\n", __LINE__, events);
       return;
     }

     // Read the response, passing it to Stream
     _read(__LINE__);               // (Exception if error)
   }; // h_reader=

   // h_writer - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   h_writer= [this](void)           // The output writer
   { if( HCDM ) debugh("Client(%p)::h_writer\n", this);

     std::shared_ptr<ClientStream> stream= stream_item->stream;
     if( events & EVT_WR_HEAD ) {      // Write the request header?
       if( (events & EVT_WR_DATA) == 0 )
         events |= EVT_RD_DATA;

       ssize_t L= _write(__LINE__);
       if( L <= 0 ) {               // If blocked (else io_error exception)
         events &= ~EVT_RD_DATA;    // (_write() updated select event)
         return;
       }

       // Implementation note: if there's no data, the server could have
       // alreaday received the request and sent the response.
       events &= ~EVT_WR_HEAD;
       if( events & EVT_WR_DATA ) { // If there's data to be sent
         std::shared_ptr<ClientRequest> request= stream->get_request();
         Request& Q= *request.get(); // (Q protected by request)
         ioda_out= std::move(Q.get_ioda());
       } else {
         ioda_out.reset();
       }
       ioda_off= 0;
     }

     if( events & EVT_WR_DATA ) {   // Write the request data?
       events |= EVT_RD_DATA;
       ssize_t L= _write(__LINE__);
       if( L <= 0 ) {               // If blocked (else io_error exception)
         events &= ~EVT_RD_DATA;    // (_write() updated select event)
         return;
       }

       events &= ~EVT_WR_DATA;
     }
   }; // h_writer=
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Client::_http2
//
// Purpose-
//       Initialize the HTTP/2 protocol handlers
//
//----------------------------------------------------------------------------
void
   Client::_http2( void )           // Initialize the HTTP/2 protocol handler
{
   throw std::runtime_error("NOT READY YET");
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Client::_read
//
// Purpose-
//       Read Client response
//
//----------------------------------------------------------------------------
void
   Client::_read(                   // Read data from Socket
     int               line)        // Caller's line number
{  if( HCDM ) debugh("%4d Client(%p)::_read\n", line, this);

   ssize_t L;
   for(;;) {
     Ioda ioda;
     Mesg mesg;
     ioda.get_rd_mesg(mesg, size_inp);
     L= socket->recvmsg(&mesg, 0);
     iodm(line, "read", L);
     if( L > 0 ) {
       ioda.set_used(L);

       // Trace read operation
       void* addr= mesg.msg_iov[0].iov_base;
       ssize_t size= mesg.msg_iov[0].iov_len;
       if( size > L )
         size= L;
       if( USE_ITRACE )
         utility::iotrace(".C<<", addr, size);
       iodm(line, "read", addr, size);

       // Enqueue IODA to input task
       ClientItem* item= new ClientItem(get_self(), stream);
       item->ioda= std::move(ioda);
       if( USE_ITRACE )
         Trace::trace(".ENQ", "CINP", this, item);
       task_inp.enqueue(item);
       if( USE_READ_ONCE )
         return;
     } else {
       if( L == 0 )
         break;
       if( !USE_READ_ONCE && IS_BLOCK )
         return;
       if( !IS_RETRY )
         break;
       debugf("%4d %s HCDM read retry\n", __LINE__, __FILE__);
     }
   }

   // Handle disconnect
if( L < 0 && IS_BLOCK ) {
  debugf("Client IS_BLOCK ignored\n");
  return;
}

   if( L == 0 || (L < 0 && errno == ECONNRESET) ) { // If connection reset
     close();                       // Schedule Client close
     return;
   }

   // Report I/O error
   string S= to_string("Client::read %d:%s", errno, strerror(errno));
   error(S.c_str());
   throw io_error(S);
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Client::_write
//
// Purpose-
//       (Synchronously) transmit data
//
//----------------------------------------------------------------------------
ssize_t                             // Written length
   Client::_write(                  // Write to Server
     int               line)        // Caller line number
{  if( HCDM ) debugh("%4d Client(%p)::_write\n", line, this);

   for(;;) {
     // This helps when a trace read appears before the trace write
     if( USE_ITRACE )
       Trace::trace(".INF", __LINE__, "CSocket->write");

     Mesg mesg; ioda_out.get_wr_mesg(mesg, size_out, ioda_off);
     ssize_t L= socket->sendmsg(&mesg, 0);
     iodm(line, "sendmsg", L);
     if( L > 0 ) {
       void* addr= mesg.msg_iov[0].iov_base;
       ssize_t size= mesg.msg_iov[0].iov_len;
       if( size > L )
         size= L;
       if( USE_ITRACE )
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
     debugf("%4d %s HCDM write retry\n", __LINE__, __FILE__);
   }

   if( !IS_BLOCK )
     throw io_error(to_string("Client::write %d:%s", errno, strerror(errno)));
   Select* select= socket->get_select();
   if( select )
     select->modify(socket, POLLIN | POLLOUT);
   return -1;
}
}  // namespace _LIBPUB_NAMESPACE::http
