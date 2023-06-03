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
//       Server.cpp
//
// Purpose-
//       Implement http/Server.h
//
// Last change date-
//       2023/06/03
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <cstring>                  // For memset
#include <mutex>                    // For std::mutex, ..., base class
#include <stdexcept>                // For std::out_of_range, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <stdio.h>                  // For fprintf
#include <stdint.h>                 // For integer types
#include <sys/socket.h>             // For socket usage

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For namespace pub::dispatch objects
#include <pub/Event.h>              // For pub::Event
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Select.h>             // For pub::Select
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Statistic.h>          // For pub::Active_record
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::utility::to_string, ...

#include "pub/http/Agent.h"         // For pub::http::ListenAgent
#include "pub/http/Exception.h"     // For pub::http::exceptions
#include "pub/http/Ioda.h"          // For pub::http::Ioda
#include "pub/http/Listen.h"        // For pub::http::Listen (owner)
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Server.h"        // For pub::http::Server, implemented
#include "pub/http/Stream.h"        // For pub::http::Stream

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::utility::to_string;
using PUB::utility::visify;
using std::string;

namespace _LIBPUB_NAMESPACE::http { // Implementation namespace
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
,  IODM= false                      // I/O Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

// BUFFER_SIZE= 1'048'576           // Input buffer size
,  BUFFER_SIZE=     8'192           // Input buffer size

,  USE_ITRACE= true                 // Use internal trace?
,  USE_READ_ONCE= true              // Read once?
,  USE_REPORT= false                // Use event Reporter?
}; // enum

// Imported Options
typedef const char     CC;
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
static std::atomic_int _serialno= 2; // Serial number

//----------------------------------------------------------------------------
// Event reporting
//----------------------------------------------------------------------------
static Active_record   item_count("ServerItem"); // ServerItem count
static Active_record   server_count("Server"); // Server count

namespace {
static struct StaticGlobal {
   StaticGlobal(void)               // Constructor
{
   if( USE_REPORT ) {
     item_count.insert();
     server_count.insert();
   }
}

   ~StaticGlobal(void)              // Destructor
{
   if( USE_REPORT ) {
     item_count.remove();
     server_count.remove();
   }
}
}  staticGlobal;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Class-
//       ServerItem
//
// Purpose-
//       The Server I/O dispatch::Item
//
//----------------------------------------------------------------------------
class ServerItem : public dispatch::Item {
public:
typedef std::shared_ptr<Server>     server_ptr;

enum                                // Function codes
{  FC_CLOSE= 2                      // CLOSE
};

Ioda                   ioda;        // The Input/Output Data Area
server_ptr             server;      // The associated Server
int                    serialno;    // Server serial number
int                    sequence;    // ServerItem sequence number

   ServerItem(                      // Constructor
     server_ptr        S)           // The Server
:  dispatch::Item(), ioda()
,  server(S), serialno(S->serialno), sequence(++S->sequence)
{  if( HCDM && VERBOSE > 2 ) debugh("ServerItem(%p)!\n", this);
   if( USE_ITRACE )
     Trace::trace(".NEW", "SITM", this);

   if( USE_REPORT )
     item_count.inc();

   INS_DEBUG_OBJ("ServerItem");
}

virtual
   ~ServerItem( void )              // Destructor
{  if( HCDM && VERBOSE > 2 ) debugh("ServerItem(%p)~\n", this);
   if( USE_ITRACE )
     Trace::trace(".DEL", "SITM", this);

   if( USE_REPORT )
     item_count.dec();

   REM_DEBUG_OBJ("ServerItem");
}

virtual void
   debug(const char* info) const    // TODO: REMOVE
{  debugf("ServerItem(%p)::debug(%s) server(%p)\n", this, info, server.get());

   debugf("..serialno(%d) sequence(%d)\n", serialno, sequence);
   debugf("..fc(%d) cc(%d) done(%p)\n", fc, cc, done);
}
}; // class ServerItem

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
     debugh("%4d Server %zd= %s() %d:%s\n", line, L, op
           , errno, strerror(errno));
   else if( IODM )                  // If I/O Debug Mode active
     traceh("%4d Server %zd= %s()\n", line, L, op);

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
     int ERRNO= errno;              // (Preserving errno probably not needed)

     string V((const char*)addr, size);
     V= visify(V);
     traceh("%4d Server::%s(%p,%zd)\n%s\n", line, op, addr, size, V.c_str());

     errno= ERRNO;
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
// Method-
//       Server::Server
//       Server::~Server
//       Server::make
//
// Purpose-
//       Constructors
//       Destructor
//       Creator
//
//----------------------------------------------------------------------------
   Server::Server(                  // Constructor
     Listen*           listen,      // The creating Listener
     Socket*           socket)      // The server Socket
:  std::mutex()
,  listen(listen)
,  size_inp(BUFFER_SIZE)
,  size_out(BUFFER_SIZE)
,  socket(socket)
// stream_set(&root)
,  task_inp([this](dispatch::Item* it) { inp_task(it); })
,  task_out([this](dispatch::Item* it) { out_task(it); })
{  if( HCDM || VERBOSE > 1 )
     debugh("Server(%p)!(%p,%p)\n", this, listen, socket);
   if( USE_ITRACE )
     Trace::trace(".NEW", "HSRV", this, socket);

   serialno= (_serialno += 10);

   // Initialize protocol
   int proto_ix= HTTP_H1;
   const char* ptype= listen->get_option(OPT_PROTO); // Get specified protocol
   if( ptype ) {                    // If protocol specified
     int proto_ix= -1;
     for(int i= 0; i<HTTP_PROTO_LENGTH; ++i) {
       if( strcmp(ptype, proto[i]) == 0 ) {
         proto_ix= i;
         break;
       }
     }

     if( proto_ix < 0 ) {         // If invalid protocol specified
       errorh("Server(%p) invalid protocol '%s'\n", this, ptype);
       errorf("Prococol '%s' selected\n", proto[HTTP_H1]);
     } else {
       proto_id= proto[proto_ix];
     }
   }
   if( proto_ix == HTTP_H1 )  {
     _http1();
   } else {
     _http2();
   }

   // Allow immediate port re-use on close
   struct linger optval;
   optval.l_onoff= 1;
   optval.l_linger= 0;
   socket->set_option(SOL_SOCKET, SO_LINGER, &optval, sizeof(optval));

   // Initialize asynchronous operation
   fsm= FSM_READY;
   events= POLLIN;
   socket->set_flags( socket->get_flags() | O_NONBLOCK );
   socket->on_select([this](int revents) { async(revents); });
   listen->get_agent()->select.insert(socket, POLLIN);

   if( USE_REPORT )
     server_count.inc();

   INS_DEBUG_OBJ("*Server*");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Server::~Server( void )          // Destructor
{  if( HCDM || VERBOSE > 1 ) debugh("Server(%p)~\n", this);
   if( USE_ITRACE )
     Trace::trace(".DEL", "HSRV", this, stream.get());

   // Close and delete the socket
   close();
   delete socket;

   if( USE_REPORT )
     server_count.dec();

   REM_DEBUG_OBJ("*Server*");

   // Implementation note:
   // After return, C++ invokes task_inp and task_out destructors
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::shared_ptr<Server>             // The Server
   Server::make(                    // Create Server
     Listen*           listen,      // The creating Listener
     Socket*           socket)      // The server Socket
{  if( HCDM ) debugh("Server::make(%p,%p)\n", listen, socket);

   std::shared_ptr<Server> server=
      std::make_shared<Server>(listen, socket);
//debugf("%4d Server make %p\n", __LINE__, &server);

   server->self= server;
   return server;
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Server::debug(const char* info) const  // Debugging display
{  debugf("Server(%p)::debug(%s) fsm(%d) %s\n", this, info
         , fsm, get_peer_addr().to_string().c_str());

   debugf("..serialno(%d), sequence(%d)\n", serialno, sequence);
   debugf("..listen(%p) socket(%p)\n", listen, socket);
   debugf("..size_inp(%'zd) size_out(%'zd)\n", size_inp, size_out);
   socket->debug("Server::debug");
   debugf("task_inp:\n"); task_inp.debug(info);
   debugf("task_out:\n"); task_out.debug(info);
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::async
//
// Purpose-
//       Handle Asynchronous Polling Event
//
//----------------------------------------------------------------------------
void
   Server::async(                   // Handle Asynchronous Polling Event
     int               revents)     // Polling revents
{  if( HCDM )
     debugh("Server(%p)::async(%.4x) events(%.4x) fsm(%d)\n", this
           , revents, events, fsm);
   if( USE_ITRACE )
     Trace::trace(".SRV", ".APE", this, a2v(fsm, revents, get_handle()));

   if( fsm != FSM_READY )           // Ignore event if non-operational
     return;

   // If a Socket error occurred
   if( revents & (POLLERR | POLLNVAL) ) {
     debugf("%4d HCDM Server revents(%.4x)\n", __LINE__, revents);
     error("async error detected");
     return;
   }

   // If Socket is readable
   if( revents & (POLLIN | POLLPRI) ) {
     h_reader();
     return;
   }

   // If Socket is writable
   if( revents & POLLOUT ) {
     h_writer();
     return;
   }

   // If unexpected event TODO: Add recovery, considering revents
   debugf("%4d HCDM Server revents(%.4x)\n", __LINE__, revents);
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::close
//
// Purpose-
//       Terminate the Server
//
// Implementation notes (Issue #3)-
//       See task_inp close() method call.
//
//----------------------------------------------------------------------------
void
   Server::close( void )            // Terminate the Server
{  if( HCDM ) debugh("Server(%p)::close() fsm(%d)\n", this, fsm);
   if( USE_ITRACE )
     Trace::trace(".SRV", ".CLS", this, i2v(get_handle()));

   {{{{
     std::lock_guard<Server> lock(*this);

     if( fsm != FSM_RESET ) {
       fsm= FSM_RESET;
       // Note: Listen::disconnect uses socket->get_peer_addr(), therefore
       // listen->disconnet() must precede socket->close()
       listen->disconnect(this);    // (Only called once)
       socket->close();             // (Only called once)
     }
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::close_enq
//
// Purpose-
//       Schedule a close operation
//
//----------------------------------------------------------------------------
void
   Server::close_enq( void )        // Schedule Server close
{  if( HCDM ) debugh("Server(%p)::close_enq() fsm(%d)\n", this, fsm);
   if( USE_ITRACE )
     Trace::trace(".SRV", "QCLS", this, i2v(get_handle()));

   if( fsm == FSM_READY ) {
     fsm= FSM_CLOSE;                // Close in progress
     Select* select= socket->get_select();
     if( select )
       select->modify(socket, 0);   // Remove from poll list

     ServerItem* item= new ServerItem(get_self());
     item->fc= item->FC_CLOSE;
     task_inp.enqueue(item);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::error
//
// Purpose-
//       Handle connection error.
//
//----------------------------------------------------------------------------
void
   Server::error(                   // Handle connection error
     const char*       info)        // Diagnostic information
{  errorh("Server(%p)::error(%s)\n", this, info);

   close_enq();                     // Schedule Server close
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::wait
//
// Purpose-
//       Wait until idle
//
//----------------------------------------------------------------------------
void
   Server::wait( void )             // Wait for current Requests to complete
{  if( HCDM ) debugh("Server(%p)::wait\n", this);

   dispatch::Wait wait;
   dispatch::Item item(item.FC_CHASE, &wait);
   task_out.enqueue(&item);
   wait.wait();
   wait.reset();

   task_inp.enqueue(&item);
   wait.wait();
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::write
//
// Purpose-
//       Queue Iota to output task
//
//----------------------------------------------------------------------------
void
   Server::write(Ioda& ioda)       // Write to Server
{  if( HCDM ) debugh("Server(%p)::write(*,%'zd)\n", this, ioda.get_used());

   if( ioda.get_used() ) {
     ServerItem* item= new ServerItem(get_self());
     item->ioda= std::move(ioda);
     if( USE_ITRACE )
       Trace::trace(".ENQ", "SOUT", this, item);
     task_out.enqueue(item);
   }
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Server::_http1
//
// Purpose-
//       Initialize the HTTP/1.0 and HTTP/1.1 protocol handlers
//
//----------------------------------------------------------------------------
void
   Server::_http1( void )           // Initialize the HTTP/1 protocol handlers
{
   // debugh("%4d %s HCDM1n", __LINE__, __FILE__);

   // inp_task - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   inp_task= [this](dispatch::Item* it) // Input task
   { if( HCDM ) debugh("Server(%p)::inp_task(%p)\n", this, it);
     if( USE_ITRACE )
       Trace::trace(".DEQ", "SINP", this, it);

     ServerItem* item= static_cast<ServerItem*>(it);
     if( item->serialno != serialno )
       utility::checkstop(__LINE__, __FILE__, "inp_task");

     if( fsm != FSM_READY ) {
       if( item->fc == item->FC_CLOSE )
         close();

       // Issue #3: close operation deadlock:
       // We need to post the work item (which contains a shared_ptr<Server>)
       // under a different Task so that Server (and inp_task) destruction
       // won't occur under inp_task control.
       item->cc= item->CC_PURGE;
       dispatch::Disp::defer(item);
       return;
     }

     if( stream.get() == nullptr )
       stream= ServerStream::make(this);

     if( stream && stream->read(item->ioda) ) {
       stream->end();
       stream= nullptr;
     }

     dispatch::Disp::defer(item);
   }; // inp_task=

   // out_task - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   out_task= [this](dispatch::Item* it) // Output task
   { if( HCDM ) debugh("Server(%p)::out_task(%p)\n", this, it);
     if( USE_ITRACE )
       Trace::trace(".DEQ", "SOUT", this, it);

     ServerItem* item= static_cast<ServerItem*>(it);
     if( item->serialno != serialno )
       utility::checkstop(__LINE__, __FILE__, "out_task");

     if( fsm != FSM_READY ) {
       item->post(item->CC_PURGE);
       dispatch::Disp::defer(item);
       return;
     }

     ioda_out += std::move(item->ioda);
     _write(__LINE__);

     if( USE_ITRACE )
       Trace::trace(".XIT", "SOUT", this, it);

     // Issue #3: write deadlock:
     // The output task can be delayed here until the Server is closed.
     // We need to post the work item (which contains a shared_ptr<Server>)
     // under a different Task so that Server (and out_task) destruction
     // won't occur under out_task control.
     dispatch::Disp::defer(item);
   }; // out_task=

   // h_reader - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   h_reader= [this](void)           // (Asynchronous) input data available
   { if( HCDM ) debugh("Server(%p)::h_reader\n", this);

     // Read the request, passing it to Stream
     _read(__LINE__);               // (Exception if error)
   }; // h_reader=

   // h_writer - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   h_writer= [this](void)           // The output writer
   { if( HCDM ) debugh("Server(%p)::h_writer\n", this);

     _write(__LINE__);              // (Exception if error)
   }; // h_writer=
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Server::_http2
//
// Purpose-
//       Initialize the HTTP/2 protocol handlers
//
//----------------------------------------------------------------------------
void
   Server::_http2( void )           // Initialize the HTTP/2 protocol handler
{
   throw std::runtime_error("NOT READY YET");
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Server::_read
//
// Purpose-
//       Read Server request
//
//----------------------------------------------------------------------------
void
   Server::_read(                   // Read data from Socket
     int               line)        // Caller's line number
{  if( HCDM ) debugh("%4d Server(%p)::read\n", line, this);

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
         utility::iotrace(".S<<", addr, size);
       iodm(line, "read", addr, size);

       // Enqueue IODA to input task
       ServerItem* item= new ServerItem(get_self());
       item->ioda= std::move(ioda);
       if( USE_ITRACE )
         Trace::trace(".ENQ", "SINP", this, item);
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
  debugf("Server IS_BLOCK ignored\n");
  return;
}

   if( L == 0 || (L < 0 && errno == ECONNRESET) ) { // If connection reset
     close_enq();                   // Schedule Client close
     return;
   }

   // Report I/O error
   string S= to_string("Server::read %d:%s", errno, strerror(errno));
   error(S.c_str());
   throw io_error(S);
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Server::_write
//
// Purpose-
//       Write data segments
//
// Implementation notes-
//       This can be called from out_task via enqueue or asynch.
//       Since these are separate tasks, locking is required.
//
//----------------------------------------------------------------------------
void
   Server::_write(                  // Write data into Socket
     int               line)        // Caller's line number
{  if( HCDM ) debugh("%4d Server(%p)::write\n", line, this);

   std::lock_guard<Server> lock(*this);

   if( fsm != FSM_READY ) {
     ioda_out.reset();
     return;
   }

   if( ioda_out.get_used() == 0 ) {
     if( events & POLLOUT ) {
       events &= ~POLLOUT;
       Select* select= socket->get_select();
       if( select )
         select->modify(socket, POLLIN);
     }
     return;
   }

   size_t ioda_off= 0;
   for(;;) {
     // This helps when a trace read appears before the trace write
     if( USE_ITRACE )
       Trace::trace(".INF", __LINE__, "SSocket->write");

     Mesg mesg; ioda_out.get_wr_mesg(mesg, size_out, ioda_off);
     ssize_t L= socket->sendmsg(&mesg, 0);
     iodm(__LINE__, "sendmsg", L);
     if( L > 0 ) {
       void* addr= mesg.msg_iov[0].iov_base;
       ssize_t size= mesg.msg_iov[0].iov_len;
       if( size > L )
         size= L;
       if( USE_ITRACE )
         utility::iotrace(".S>>", addr, size);
       iodm(__LINE__, "sendmsg", addr, size);

       size_t want= ioda_out.get_used() - ioda_off;
       if( size_t(L) < want ) {
         ioda_off += L;
         continue;
       }
       ioda_out.reset();

       if( events & POLLOUT ) {
         events &= ~POLLOUT;
         Select* select= socket->get_select();
         if( select )
           select->modify(socket, POLLIN);
       }
       return;
     }

     if( !IS_RETRY )
       break;
     debugf("%4d %s HCDM write retry\n", __LINE__, __FILE__);
   }

   if( !IS_BLOCK ) {
     debugf("%4d Server::write I/O error %d:%s", __LINE__
           , errno, strerror(errno));
     error("I/O error");
   }
   ioda_out.discard(ioda_off);

   events |= POLLOUT;
   Select* select= socket->get_select();
   select->modify(socket, POLLIN | POLLOUT);
}
}  // namespace _LIBPUB_NAMESPACE::http
