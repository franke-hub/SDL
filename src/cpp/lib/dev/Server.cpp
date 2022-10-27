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
//       2022/10/23
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

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For namespace pub::dispatch objects
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
}; // enum

//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
enum FSM                            // Finite State Machine states
{  FSM_RESET= 0                     // Reset - idle
,  FSM_RD_DATA= POLLIN              // Waiting for read completion
,  FSM_WR_DATA= POLLOUT             // Write data blocked
}; // enum FSM

//----------------------------------------------------------------------------
//
// Class-
//       ServerItem
//
// Purpose-
//       The Server DispatchItem
//
//----------------------------------------------------------------------------
class ServerItem : public dispatch::Item { // Server DispatchItem
public:
Ioda                   ioda;        // The Input/Output Data Area

   ServerItem( void )               // Default constructor
:  dispatch::Item()
{  if( HCDM && VERBOSE > 0 ) debugh("ServerItem(%p)!\n", this);

   INS_DEBUG_OBJ("ServerItem");
}

virtual
   ~ServerItem( void )              // Destructor
{  if( HCDM && VERBOSE > 0 ) debugh("ServerItem(%p)~\n", this);

   REM_DEBUG_OBJ("ServerItem");
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
//       s2v
//
// Purpose-
//       Convert size_t  to void*
//
//----------------------------------------------------------------------------
static inline void* s2v(size_t s) { return (void*)intptr_t(s); }

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
,  h_close([]() {})
,  h_error([](const string&) {})
,  listen(listen)
,  size_inp(BUFFER_SIZE)
,  size_out(BUFFER_SIZE)
,  socket(socket)
,  task_inp([this](dispatch::Item* item) { inp_task((ServerItem*)item); })
,  task_out([this](dispatch::Item* item) { out_task((ServerItem*)item); })
{  if( HCDM )
     debugh("Server(%p)::Server(%p,%p)\n", this, listen, socket);

   // Initialize asynchronous operation
   socket->set_flags( socket->get_flags() | O_NONBLOCK );
   socket->on_select([this](int revent) { async(revent); });
   listen->get_agent()->select.insert(socket, POLLIN);

   // Server construction complete
   operational= true;
   INS_DEBUG_OBJ("*Server*");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Server::~Server( void )          // Destructor
{  if( HCDM )
     debugh("Server(%p)::~Server\n", this);

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
     int               revent)      // Polling revent
{  if( HCDM )
     debugh("Server(%p)::async(%.4x) fsm(%.4x)\n", this, revent, fsm);
   Trace::trace(".SRV", ".APE", this, s2v((size_t(revent) << 16) | fsm));

   if( !operational )
     return;

   // If a Socket error occurred
   if( revent & (POLLERR | POLLNVAL) ) {
     debugf("%4d HCDM Server revent(%.4x)\n", __LINE__, revent);
     error("asynch error detected");
     return;
   }

   // If Socket is readable
   if( revent & (POLLIN | POLLPRI) ) {
     read(__LINE__);
     return;
   }

   // If Socket is writable
   if( revent & POLLOUT ) {
     write(__LINE__);
     return;
   }

   // Handle unexpected event
   debugf("%4d HCDM Server revent(%.4x)\n", __LINE__, revent);
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::close
//
// Purpose-
//       Terminate the Server
//
//----------------------------------------------------------------------------
void
   Server::close( void )            // Terminate the Server
{  if( HCDM )
     debugh("Server(%p)::close\n", this);

   operational= false;
   std::shared_ptr<Server> server= this->self.lock(); // Keep-alive until exit
   listen->disconnect(this);        // Must be before close()

   socket->close();                 // Close the Server socket
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

   // TODO: PRELIMINARY
   close();
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
     ServerItem*       item)        // Input data item
{  if( HCDM )
     debugh("Server(%p)::inp_task(%p)\n", this, item);

   if( stream.get() == nullptr )
     stream= ServerStream::make(this);

   if( stream->read(item->ioda) ) {
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
     ServerItem*       item)        // Output data item
{  if( HCDM )
     debugh("Server(%p)::out_task(%p)\n", this, item);

   {{{{
     std::lock_guard<Server> lock(*this);

     ioda_out += std::move(item->ioda);
   }}}}

   write(__LINE__);
   item->post();
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::write
//
// Purpose-
//       (Synchronously) transmit data
//
//----------------------------------------------------------------------------
void
   Server::write(Ioda& ioda)       // Write to Server
{  if( HCDM )
     debugh("Server(%p)::write(*,%'zd)\n", this, ioda_out.get_used());

   if( ioda.get_used() ) {
     ServerItem* item= new ServerItem();
     item->ioda= std::move(ioda);
     task_out.enqueue(item);
   }
}

//----------------------------------------------------------------------------
//
// Method-
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

   for(;;) {
     Ioda ioda;
     Mesg mesg; ioda.get_rd_mesg(mesg, size_inp);
     ssize_t L= socket->recvmsg(&mesg, 0);
     if( L >= 0 ) {
       iodm(line, "read", L);
       if( L == 0 ) {
         if( VERBOSE > 1 )
           error("Client disconnect");
         return;
       }

       ioda.set_used(L);
       void* addr= mesg.msg_iov[0].iov_base;
       ssize_t size= mesg.msg_iov[0].iov_len;
       if( size > L )
         size= L;
       utility::iotrace(".S<<", addr, size);
       iodm(line, "read", addr, size);

       ServerItem* item= new ServerItem();
       item->ioda= std::move(ioda);
       task_inp.enqueue(item);
     } else {
       if( IS_BLOCK )
         return;
       if( !IS_RETRY ) {
         iodm(line, "read", L);
         error("I/O error");
         return;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
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

   if( ioda_out.get_used() == 0 ) {
     if( fsm & FSM_WR_DATA ) {
       fsm &= ~FSM_WR_DATA;
       Select* select= socket->get_select();
       if( select )
         select->modify(socket, POLLIN);
     }
     return;
   }

   size_t ioda_off= 0;
   for(;;) {
//This helps when a trace read appears before the trace write
//Trace::trace("HCDM", __LINE__, "SSocket->write");
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

       if( fsm & FSM_WR_DATA ) {
         fsm &= ~FSM_WR_DATA;
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

   fsm |= FSM_WR_DATA;
   Select* select= socket->get_select();
   select->modify(socket, POLLIN | POLLOUT);
}
}  // namespace _LIBPUB_NAMESPACE::http
