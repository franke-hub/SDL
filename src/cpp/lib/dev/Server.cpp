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
//       Server.cpp
//
// Purpose-
//       Implement http/Server.h
//
// Last change date-
//       2022/11/27
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <cstring>                  // For memset
#include <mutex>                    // For std::mutex, ..., base class
#include <ostream>                  // For std::ostream
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

,  USE_XTRACE= true                 // Use extended trace? // TODO: false
}; // enum

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
Ioda                   ioda;        // The Input/Output Data Area

   ServerItem( void )               // Default constructor
:  dispatch::Item(), ioda()
{  if( HCDM && VERBOSE > 0 ) debugh("ServerItem(%p)!\n", this);

   if( USE_XTRACE )
     Trace::trace(".NEW", "SITM", this);

   INS_DEBUG_OBJ("ServerItem");
}

virtual
   ~ServerItem( void )              // Destructor
{  if( HCDM && VERBOSE > 0 ) debugh("ServerItem(%p)~\n", this);

   if( USE_XTRACE )
     Trace::trace(".DEL", "SITM", this);

   REM_DEBUG_OBJ("ServerItem");
}

virtual void
   debug(const char* info) const    // TODO: REMOVE
{  debugf("ServerItem(%p)::debug(%s)\n", this, info); }
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
,  task_inp([this](dispatch::Item* it) { inp_task(it); })
,  task_out([this](dispatch::Item* it) { out_task(it); })
{  if( HCDM )
     debugh("Server(%p)::Server(%p,%p)\n", this, listen, socket);

   if( USE_XTRACE )
     Trace::trace(".NEW", "HSRV", this);

   // Initialize asynchronous operation
   socket->set_flags( socket->get_flags() | O_NONBLOCK );
   socket->on_select([this](int revents) { async(revents); });
   listen->get_agent()->select.insert(socket, POLLIN);

   // Allow immediate port re-use on close
   struct linger optval;
   optval.l_onoff= 1;
   optval.l_linger= 0;
   socket->set_option(SOL_SOCKET, SO_LINGER, &optval, sizeof(optval));

   // Server construction complete
   operational= true;
   INS_DEBUG_OBJ("*Server*");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Server::~Server( void )          // Destructor
{  if( HCDM )
     debugh("Server(%p)::~Server\n", this);

   if( USE_XTRACE )
     Trace::trace(".DEL", "HSRV", this);

   // Close the socket, insuring task completion
   close();                         // Not needed, but we do it anyway
// wait();                          // Not needed, already in ~Task()

   delete socket;
   socket= nullptr;
   REM_DEBUG_OBJ("*Server*");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::shared_ptr<Server>             // The Server
   Server::make(                    // Create Server
     Listen*           listen,      // The creating Listener
     Socket*           socket)      // The server Socket
{  if( HCDM )
     debugh("Server::make(%p,%p)\n", listen, socket);

   std::shared_ptr<Server> server=
      std::make_shared<Server>(listen, socket);

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
{  debugf("Server(%p)::debug(%s) %s %soperational\n", this, info
         , get_peer_addr().to_string().c_str()
         , operational ? "" : "non-");

   debugf("..listen(%p) socket(%p)\n", listen, socket);
   debugf("..size_inp(%'zd) size_out(%'zd)\n", size_inp, size_out);
   debugf("task_inp:\n"); task_inp.debug(info);
   debugf("task_out:\n"); task_out.debug(info);
   socket->debug("Server.socket");
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
     debugh("Server(%p)::async(%.4x) events(%.4x)\n", this, revents, events);
   Trace::trace(".SRV", ".APE", this, a2v(events, revents, get_handle()));

   if( !operational )               // Ignore event if non-operational
     return;

   // If a Socket error occurred
   if( revents & (POLLERR | POLLNVAL) ) {
     debugf("%4d HCDM Server revents(%.4x)\n", __LINE__, revents);
     error("asynch error detected");
     return;
   }

   // If Socket is readable
   if( revents & (POLLIN | POLLPRI) ) {
     read(__LINE__);
     return;
   }

   // If Socket is writable
   if( revents & POLLOUT ) {
     write(__LINE__);
     return;
   }

   // Handle unexpected event
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
// Implementation notes-
//       If wait != nullptr, the close is synchronous.
//
//----------------------------------------------------------------------------
void
   Server::close( void )            // Terminate the Server
{  if( HCDM )
     debugh("Server(%p)::close() %d\n", this, operational);
   Trace::trace(".CLI", ".CLS", this);

   // The Listenter might contain the last active shared_ptr<Server>
   std::shared_ptr<Server> server= get_self(); // Keep-alive

   {{{{
     std::lock_guard<Server> lock(*this);

     if( operational ) {
       operational= false;
       listen->disconnect(this);    // (Only called once)
       socket->close();             // (Only called once)
     }
   }}}}
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

   close();                         // Close the Server
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::inp_task
//
// Purpose-
//       Handle input (reader) item
//
//----------------------------------------------------------------------------
void
   Server::inp_task(                // Handle input data
     dispatch::Item*   it)          // Input data item
{  if( HCDM )
     debugh("Server(%p)::inp_task(%p) fc(%d)\n", this, it, it->fc);
   if( USE_XTRACE )
     Trace::trace(".DEQ", "SINP", this, it);

   if( !operational ) {
     it->post(it->CC_PURGE);
     return;
   }

   ServerItem* item= static_cast<ServerItem*>(it);
   if( stream.get() == nullptr )
     stream= ServerStream::make(this);

   if( stream && stream->read(item->ioda) ) {
     stream->end();
     stream= nullptr;
   }

   item->post();
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::out_task
//
// Purpose-
//       Handle input (writer) item
//
//----------------------------------------------------------------------------
void
   Server::out_task(                // Handle output data
     dispatch::Item*   it)          // Output data item
{  if( HCDM ) debugh("Server(%p)::out_task(%p)\n", this, it);
   if( USE_XTRACE )
     Trace::trace(".DEQ", "SOUT", this, it);

   if( !operational ) {
     it->post(it->CC_PURGE);
     return;
   }

   ServerItem* item= static_cast<ServerItem*>(it);
   ioda_out += std::move(item->ioda);
   write(__LINE__);
   item->post();
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
{  if( HCDM )
     debugh("Server(%p)::write(*,%'zd)\n", this, ioda.get_used());

   if( ioda.get_used() ) {
     ServerItem* item= new ServerItem();
     item->ioda= std::move(ioda);
     if( USE_XTRACE )
       Trace::trace(".ENQ", "SOUT", this, item);
     task_out.enqueue(item);
   }
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Server::read
//
// Purpose-
//       Read data segments
//
//----------------------------------------------------------------------------
void
   Server::read(                    // Read data from Socket
     int               line)        // Caller's line number
{  if( HCDM ) debugh("%4d Server(%p)::read\n", line, this);

   ssize_t L;
   for(;;) {
     Ioda ioda;
     Mesg mesg; ioda.get_rd_mesg(mesg, size_inp);
     L= socket->recvmsg(&mesg, 0);
     if( L >= 0 ) {
       iodm(line, "read", L);
       if( L == 0 ) {
         close();
         return;
       }
       ioda.set_used(L);

       // Trace read
       void* addr= mesg.msg_iov[0].iov_base;
       ssize_t size= mesg.msg_iov[0].iov_len;
       if( size > L )
         size= L;
       utility::iotrace(".S<<", addr, size);
       iodm(line, "read", addr, size);

       ServerItem* item= new ServerItem();
       item->ioda= std::move(ioda);
       if( USE_XTRACE )
         Trace::trace(".ENQ", "SINP", this, item);
       task_inp.enqueue(item);
     } else {
       if( IS_BLOCK )
         return;
       if( !IS_RETRY )
         break;
     }
   }

   // Handle I/O error
   if( L == 0 || (L < 0 && errno == ECONNRESET) ) { // If connection reset
     close();
     return;
   }

   iodm(line, "read", L);
   error("I/O error");
}

//----------------------------------------------------------------------------
//
// Protected method-
//       Server::write
//
// Purpose-
//       Write data segments
//
// Implementation notes-
//       This can be called from out_task and async.
//       Since these are separate tasks, locking is required.
//
//----------------------------------------------------------------------------
void
   Server::write(                   // Write data into Socket
     int               line)        // Caller's line number
{  if( HCDM ) debugh("%4d Server(%p)::write\n", line, this);

   std::lock_guard<Server> lock(*this);

   if( !operational ) {
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
     if( USE_XTRACE )
       Trace::trace(".INF", __LINE__, "SSocket->write");

     Mesg mesg; ioda_out.get_wr_mesg(mesg, size_out, ioda_off);
     ssize_t L= socket->sendmsg(&mesg, 0);
     iodm(__LINE__, "sendmsg", L);
     if( L > 0 ) {
       void* addr= mesg.msg_iov[0].iov_base;
       ssize_t size= mesg.msg_iov[0].iov_len;
       if( size > L )
         size= L;
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
