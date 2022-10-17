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
//       2022/10/16
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <cstring>                  // For memset
#include <ostream>                  // For std::ostream
#include <stdexcept>                // For std::out_of_range, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert
#include <stdio.h>                  // For fprintf
#include <stdint.h>                 // For integer types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Named.h>              // For pub::Named, base class
#include <pub/Thread.h>             // For pub::Thread, base class
#include <pub/Select.h>             // For pub::Select
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::utility::to_string, ...

#include "pub/http/Exception.h"     // For pub::http::exceptions
#include "pub/http/Ioda.h"          // For pub::http::Ioda
#include "pub/http/Listen.h"        // For pub::http::Listen
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
// Method-
//       Server::~Server
//       Server::Server
//
// Purpose-
//       Destructor
//       Constructors
//
//----------------------------------------------------------------------------
   Server::~Server( void )          // Destructor
{  if( HCDM )
     debugh("Server(%p)::~Server\n", this);

   delete socket;
   socket= nullptr;
}

   Server::Server(                  // Constructor
     Listen*           listen,      // The creating Listener
     Socket*           socket)      // The server Socket
:  Named("pub::http::Server"), Thread(), std::mutex()
,  h_close([]() {})
,  h_error([](const string&) {})
,  listen(listen->get_self())
,  size_inp(BUFFER_SIZE)
,  size_out(BUFFER_SIZE)
,  socket(socket)
{  if( HCDM )
     debugh("Server(%p)::Server(%p,%p)\n", this, listen, socket);

   timeval tv{3, 500000};           // 3.5 second timeout
   socket->set_option(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

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

   debugf("..listen(%p) socket(%p)\n", listen.lock().get(), socket);
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
//       Server::join
//
// Purpose-
//       Wait for the Server Thread to complete
//
//----------------------------------------------------------------------------
void
   Server::join( void )             // Wait for Server Thread to complete
{  if( HCDM )
     debugh("Server(%p)::join\n", this);

   Thread::join();                  // Wait for Server Thread completion
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::read
//
// Purpose-
//       (Synchronously) read data segment
//
//----------------------------------------------------------------------------
void
   Server::read(                    // Read data from Socket
     int               line)        // Caller's line number
{  if( HCDM ) debugh("%4d Server(%p)::read\n", line, this);

   for(;;) {
     Ioda ioda;
     Mesg mesg; ioda.get_rd_mesg(mesg, size_inp);

     ssize_t L= socket->recvmsg((msghdr*)&mesg, 0);
     iodm(line, "read", L);
     if( L >= 0 ) {
       if( L == 0 )
         throw io_eof("Server");

       ioda.set_used(L);
       void* addr= mesg.msg_iov[0].iov_base;
       ssize_t size= mesg.msg_iov[0].iov_len;
       if( size > L )
         size= L;
       utility::iotrace(".S<<", addr, size);
       iodm(line, "read", addr, size);
       if( stream.get() == nullptr )
         stream= ServerStream::make(this);

       if( stream->read(ioda) )
         break;
       continue;
     }

     if( !IS_RETRY && !IS_BLOCK ) { // TODO: Remove !IS_BLOCK when async
//int ERRNO= errno;
//debugf("size_inp(%d) %d:%s\n", size_inp, ERRNO, strerror(ERRNO));
//ioda.debug("I/O error"); // Invalid argument
//mesg.debug("I/O error");
//errno= ERRNO;
       throw io_error(to_string("Server::read %d:%s", errno, strerror(errno)));
     }
   }

   stream->end();
   stream= nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::run
//
// Purpose-
//       Operate the Server
//
//----------------------------------------------------------------------------
void
   Server::run( void )              // Operate the Server
{  if( HCDM ) debugh("Server(%p)::run...\n", this);

   operational= true;
   try {
     for(;;) {
       read(__LINE__);
       if( !operational )
         break;
     }
   } catch(io_eof& X) {
     if( IODM )
       debugh("%4d %s %s\n", __LINE__, __FILE__, X.what());
   } catch(io_exception& X) {
     if( IODM )
       errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
     error(X.what());
   } catch(stream_error& X) {
     if( IODM )
       errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
     stream->reject(500);
   } catch(std::length_error& X) {
     if( IODM )
       errorf("Server std::length_error(%s)\n", X.what());
     stream->reject(413);
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
   operational= false;

   std::shared_ptr<Server> server= this->self.lock(); // Keep-alive until exit
   std::shared_ptr<Listen> listen= this->listen.lock();
   if( listen)
     listen->disconnect(this);      // Must be before close()

   stream= nullptr;                 // (Destroy the Stream)
   close();
   h_close();                       // Drive the close listener

   if( HCDM ) debugh("...Server(%p)::run\n", this);
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
   Server::write(Ioda& ioda_out)   // Write to Server
{  if( HCDM )
     debugh("Server(%p)::write(*,%'zd)\n", this, ioda_out.get_used());

   size_t ioda_off= 0;
   for(;;) {
//This helps when a trace read appears before the trace write
//Trace::trace("HCDM", __LINE__, "SSocket->write");
     Mesg mesg; ioda_out.get_wr_mesg(mesg, size_out, ioda_off);
     ssize_t L= socket->sendmsg((msghdr*)&mesg, 0);
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
       return;
     }

//   if( !IS_RETRY )                // TODO: INSERT
     if( !IS_RETRY && !IS_BLOCK )   // TODO: REMOVE
       break;
   }

   if( !IS_BLOCK )
     throw io_error(to_string("Server::write %d:%s", errno, strerror(errno)));
   Select* select= socket->get_select();
   select->modify(socket, POLLIN | POLLOUT);
}

void
   Server::write(                   // Write to Server
     int               line,        // Caller line number
     const void*       addr,        // Data address
     size_t            size)        // Data length
{  if( HCDM )
     debugh("%4d Server(%p)::write(%p,%zd)\n", line, this, addr, size);

Trace::trace("HCDM", __LINE__, "SSocket->write");
   ssize_t L= socket->write(addr, size);
   iodm(line, "socket.write", L);
   if( L <= 0 ) {                 // If error or EOF
     int ERRNO= errno;
     debugh("%4d %s HCDM %d:%s\n", line, __FILE__, errno, strerror(errno));
     string S= utility::
         to_string("Server::write %d:%s", ERRNO, strerror(ERRNO));
     throw io_error(S);
   }
   utility::iotrace(".S>>", addr, size);
   iodm(line, "write", addr, size);
}
}  // namespace _LIBPUB_NAMESPACE::http
