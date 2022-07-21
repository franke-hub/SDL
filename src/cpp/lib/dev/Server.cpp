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
//       2022/07/16
//
// Implementation notes-
//       TODO: Combine the two iodm subroutines. (Also in Client.cpp)
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
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::utility::to_string

#include "pub/http/Data.h"          // For pub::http::Buffer
#include "pub/http/Exception.h"     // For pub::http::exceptions
#include "pub/http/Listen.h"        // For pub::http::Listen
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Server.h"        // The pub::http::Server, implemented
#include "pub/http/Stream.h"        // For pub::http::Stream
#include "pub/http/utility.h"       // For namespace pub::http::utility

using namespace _PUB_NAMESPACE;
using namespace _PUB_NAMESPACE::debugging;
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
,  IODM= false                      // I/O Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  BUFFER_SIZE= 1'048'576           // Input buffer size
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
     debugh("%4d Server %zd= %s()\n", line, L, op);
// traceh("%4d %s %4d %zd=%s() HCDM\n", __LINE__, __FILE__, line, L, op);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-zero-length"
// traceh("");
#pragma GCC diagnostic pop

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
     debugh("%4d Server::%s(addr,%zd)\n%s\n", line, op, size, V.c_str());
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
,  h_close(utility::f_void())
,  h_error(utility::f_error())
,  listen(listen->get_self())
,  buffer(BUFFER_SIZE)
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

   debugf("..buffer(%p) listen(%p) socket(%p)\n", buffer.addr
         , listen.lock().get(), socket);
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
//       Server::connection_error
//
// Purpose-
//       Handle connection error.
//
//----------------------------------------------------------------------------
void
   Server::connection_error(        // Handle connection error
     const char*       info)        // Diagnostic information
{  errorh("Server(%p)::connection_error(%s)\n", this, info);

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
size_t                              // Read length
   Server::read(                    // Read data from Socket
     int               line,        // Caller's line number
     size_t            size)        // Maximum length
{  if( HCDM ) debugh("%4d Server(%p)::read(%zd)\n", line, this, size);

   if( size > BUFFER_SIZE )
     size= BUFFER_SIZE;

   ssize_t L= -1;
   while( L < 0 ) {
     errno= 0;
Trace::trace("HCDM", __LINE__, "SSocket->read");
     L= socket->read(buffer.addr, size);
     if( !IS_BLOCKED )
       break;
   }
   if( !operational )
     return 0;

   int ERRNO= errno;
   iodm(line, "socket.read", L);
   if( L <= 0 ) {                   // If error or EOF
     if( L == 0 )
       throw io_eof("Server::read EOF");

     string S= ::pub::utility::
         to_string("Server::read %d:%s", ERRNO, strerror(ERRNO));
     throw io_error(S);
   }
   iodm(line, "read", buffer.addr, L);
   utility::iotrace(".S<<", buffer.addr, L);
   return L;
}

size_t                              // Read length
   Server::read(                    // Read data from Socket
     size_t            size)        // Maximum length
{  return read(__LINE__, size); }

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
   std::shared_ptr<ServerStream> stream;
   try {
     for(;;) {
       size_t size= read(__LINE__, BUFFER_SIZE);
       if( !operational )
         break;

       stream= ServerStream::make(this);
       try {
// traceh("%4d %s HCDM\n", __LINE__, __FILE__);
         bool cc= stream->read(buffer.addr, size);
         while( !cc ) {
// traceh("%4d %s HCDM\n", __LINE__, __FILE__);
           size= read(__LINE__, BUFFER_SIZE);
           cc= stream->read(buffer.addr, size);
         }

// traceh("%4d %s HCDM\n", __LINE__, __FILE__);
         stream->end();
       } catch(stream_error& X) {
         errorf("pub::http::Server http::stream_error(%s)\n", X.what());
         connection_error(X.what());
       } catch(std::length_error& X) {
         errorf("pub::http::Server std::length_error(%s)\n", X.what());
         stream->reject(413);
       } catch(std::exception& X) {
         errorf("pub::http::Server std::exception(%s)\n", X.what());
         connection_error(X.what());
       } catch(...) {
         errorf("%4d pub::http::Server SHOULD NOT OCCUR\n", __LINE__);
         connection_error("catch(...)");
         stream->reject(501);
       }

       stream= nullptr;             // (Destroy the Stream)
     }
   } catch(pub::Exception& X) {
     errorh("%4d %s %s\n", __LINE__, __FILE__, ((std::string)X).c_str());
     connection_error(X.what());
   } catch(io_eof& X) {
     if( IODM )
       debugh("%4d %s %s\n", __LINE__, __FILE__, X.what());
   } catch(io_exception& X) {
     if( IODM )
       errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
     connection_error(X.what());
   } catch(stream_error& X) {
     if( IODM )
       errorh("%4d %s %s\n", __LINE__, __FILE__, X.what());
     stream->reject(599);
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
//       (Synchronously) transmit data segment
//
//----------------------------------------------------------------------------
ssize_t                             // Written length
   Server::write(                   // Write to Server
     int               line,        // Caller line number
     const void*       addr,        // Data address
     size_t            size)        // Data length
{  if( HCDM )
     debugh("%4d Server(%p)::write(%p,%zd)\n", line, this, addr, size);

   utility::iotrace(".S>>", addr, size);
   errno= 0;
   ssize_t L= socket->write(addr, size);
   iodm(line, "socket.write", L);
   if( L <= 0 ) {                 // If error or EOF
// traceh("%4d %s HCDM\n", line, __FILE__);
     string S= ::pub::utility::
         to_string("Client::write %d:%s", errno, strerror(errno));
     throw io_error(S);
   }
   iodm(line, "write", addr, size);
   return 0;
}

ssize_t                             // Written length
   Server::write(                   // Write to Server
     const void*       addr,        // Data address
     size_t            size)        // Data length
{  return write(__LINE__, addr, size); }
}  // namespace pub::http
