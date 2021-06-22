//----------------------------------------------------------------------------
//
//       Copyright (C) 2019-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Socket.cpp
//
// Purpose-
//       Socket method implementations.
//
// Last change date-
//       2021/06/10
//
//----------------------------------------------------------------------------
#include <errno.h>                  // For errno
#include <mutex>                    // For mutex, std::lock_guard, ...
#include <poll.h>                   // For poll, ...
#include <string.h>                 // For memset, ...
#include <unistd.h>                 // For close, ...
#include <arpa/inet.h>              // [[ REMINDER: inet address conversions ]]
#include <openssl/err.h>            // For ERR_error_string
#include <sys/time.h>               // For timeval, ...

#include <pub/utility.h>            // For to_string()
#include <pub/Debug.h>              // For debugging

#include "pub/Socket.h"             // The Socket Object

using namespace _PUB_NAMESPACE::debugging; // For debugging
using _PUB_NAMESPACE::utility::to_string; // For to_string

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, I/O Debug Mode
#endif

#ifndef SCDM                        // ** NOT IMPLEMENTED **
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef TRACE                       // ** NOT IMPLEMENTED **
#undef  TRACE                       // If defined, Use memory trace
#endif

#include <pub/ifmacro.h>

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
//
// Subroutine-
//       display_ERR
//
// Purpose-
//       Display all pending OPENSSL errors
//
//----------------------------------------------------------------------------
static void
   display_ERR( void )              // Display all pending BIO errors
{
   char buffer[256];                // Error buffer string

   int ERRNO= errno;                // Preserve errno
   long ec= ERR_get_error();        // The first error
   while( ec ) {                    // Display all errors
     ERR_error_string(ec, buffer);  // Format this error
     errorf("%s\n", buffer);

     ec= ERR_get_error();
   }
   errno= ERRNO;                    // Restore errno
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::~Socket
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Socket::~Socket( void )          // Destructor
{  IFHCDM( debugh("Socket(%p)::~Socket()\n", this); )

   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::Socket
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Socket::Socket( void )           // Constructor
:  Object(), handle(CLOSED), recv_timeo(0), send_timeo(0)
{  IFHCDM( debugh("Socket(%p)::Socket()\n", this); )

   memset(&host_addr, 0, sizeof(host_addr));
   memset(&peer_addr, 0, sizeof(peer_addr));
   host_size= sizeof(host_addr);
   peer_size= sizeof(peer_addr);
}

   Socket::Socket(                  // Copy constructor
     const Socket&     source)      // Source Socket
:  Object(), handle(CLOSED)
{  IFHCDM( debugh("Socket(%p)::Socket(%p)\n", this, &source); )

   this->host_addr= source.host_addr;
   this->peer_addr= source.peer_addr;
   this->host_size= source.host_size;
   this->peer_size= source.peer_size;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::operator=
//
// Purpose-
//       Assignment operator
//
//----------------------------------------------------------------------------
Socket&                             // (Always *this)
   Socket::operator=(               // Assignment operator
     const Socket&     source)      // Source Socket
{  IFHCDM( debugh("Socket(%p)::operator=(%p)\n", this, &source); )

   this->handle= CLOSED;

   this->host_addr= source.host_addr;
   this->peer_addr= source.peer_addr;
   this->host_size= source.host_size;
   this->peer_size= source.peer_size;

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::get_option
//
// Purpose-
//       Get socket option
//
//----------------------------------------------------------------------------
int                                 // Return code
   Socket::get_option(              // Get socket option
     int               optlevel,    // Level
     int               optname,     // Name
     void*             optval,      // Option value
     socklen_t*        optlen)      // Option length (IN/OUT)
{  return ::getsockopt(handle, optlevel, optname, optval, optlen); }

//----------------------------------------------------------------------------
//
// Method-
//       Socket::set_option
//
// Purpose-
//       Set socket option
//
//----------------------------------------------------------------------------
int                                 // Return code
   Socket::set_option(              // Set socket option
     int               optlevel,    // Level
     int               optname,     // Name
     const void*       optval,      // Option value
     socklen_t         optlen)      // Option length (IN/OUT)
{
   int CC= ::setsockopt(handle, optlevel, optname, optval, optlen);

   // Intercept SOL_SOCKET+SO_RCVTIMEO/SO_SNDTIMEO
   if( CC == 0 && optlevel == SOL_SOCKET ) {
     if( optname == SO_RCVTIMEO ) {
       struct timeval* tv= (struct timeval*)optval;
       recv_timeo= tv->tv_sec * 1000 + tv->tv_usec / 1000;
     } else if( optname == SO_SNDTIMEO ) {
       struct timeval* tv= (struct timeval*)optval;
       send_timeo= tv->tv_sec * 1000 + tv->tv_usec / 1000;
     }
   }

   return CC;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::bind
//
// Purpose-
//       Bind this Socket to a port
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Socket::bind(                    // Bind this Socket
     Port              port)        // To this Port
{  IFHCDM( debugh("Socket(%p)::bind(%d)\n", this, port); )

   set_host_port(port);             // Set the host port

   int rc= ::bind(handle, (sockaddr*)&host_addr, host_size);
   IFIODM(
     int ERRNO= errno;
     traceh("%d= Socket(%p)::bind(...)\n", rc, this);
     errno= ERRNO;
   )

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::close
//
// Purpose-
//       Close the Socket
//
// Implementation note-
//       Duplicate close() function calls allowed.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Socket::close( void )            // Close the Socket
{  IFHCDM( debugh("Socket(%p)::close() handle(%d)\n", this, handle); )

   int rc= 0;
   if( handle >= 0 ) {
     rc= ::close(handle);
     handle= CLOSED;
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::connect
//
// Purpose-
//       Connect to peer
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Socket::connect(                 // Connect to peer
     sockaddr*         peer_addr,   // Peer address
     socklen_t         peer_size)   // Peer address length
{  IFHCDM( debugh("Socket(%p)::connect(%p,%d)\n", this, peer_addr, peer_size); )

   if( size_t(peer_size) > sizeof(this->peer_addr) )
     throw SocketException("Socket::connect peer_size");
   int rc= ::connect(handle, peer_addr, peer_size);
   IFIODM(
     int ERRNO= errno;
     traceh("%d= Socket(%p)::connect(...)\n", rc, this);
     errno= ERRNO;
   )
   if( rc == 0 ) {
     memcpy(&this->peer_addr, peer_addr, peer_size);
     this->peer_size= peer_size;
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::listen
//
// Purpose-
//       Listen for and accept new connections
//
//----------------------------------------------------------------------------
Socket*                             // The new connection Socket
   Socket::listen( void )           // Get new connection Socket
{
   int rc= ::listen(handle, SOMAXCONN); // Wait for new connection
   if( rc != 0 ) {                  // If listen failure
     int ERRNO= errno;
     traceh("%d= Socket(%p)::listen() error(%d,%s)\n", rc, this,
            ERRNO, strerror(ERRNO));
     display_ERR();
     errno= ERRNO;
     return nullptr;
   }

   // Accept the next connection
   int client;
   for(;;) {
     peer_size= sizeof(peer_addr);
     client= ::accept(handle, (sockaddr*)&peer_addr, &peer_size);

     if( client >= 0 )              // If valid handle
       break;

     if( handle < 0 )               // If closed
       return nullptr;              // (Expected)

     if( errno != EINTR ) {         // If not interrupted
       errorf("Warning: Socket::listen, accept failure(%s)\n",
              strerror(errno));
       return nullptr;
     }
   }

   Socket* result= new Socket(*this);
   result->handle= client;
   IFIODM( traceh("%p[%d]= Socket(%p)::listen()\n", result, client, this); )

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::open
//
// Purpose-
//       Open a Socket
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Socket::open(                    // Open a Socket
     int               family,      // Address Family
     int               type,        // Socket type
     int               protocol)    // Socket protocol
{  IFHCDM(
     debugh("Socket(%p)::open(%d,%d,%d)\n", this, family, type, protocol);
   )

   if( handle >= 0 )
     throw SocketException("Socket already open"); // This is a usage error

   memset(&host_addr, 0, sizeof(host_addr));
   memset(&peer_addr, 0, sizeof(peer_addr));
   host_size= sizeof(host_addr);
   peer_size= sizeof(peer_addr);

   // (Current) implementation restrictions
   if( family == AF_INET )
     host_size= sizeof(sockaddr_in);
   else if( family == AF_INET6 )
     host_size= sizeof(sockaddr_in6);
   else {
     char temp[64];
     sprintf(temp, "sa_family_t(%d) not supported", family);
     throw SocketException(temp);
   }

   if( type != SOCK_STREAM ) {
     char temp[64];
     sprintf(temp, "socket type(%d) not supported", type);
     throw SocketException(temp);
   }

   handle= ::socket(family, type, protocol);
   if( handle < 0 )
     throw SocketException("Error creating socket"); // (SHOULD NOT OCCUR)

   // Open accepted
   host_addr.ss_family= family;
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::read
//
// Purpose-
//       Read from the Socket
//
//----------------------------------------------------------------------------
int                                 // Number of bytes read
   Socket::read(                    // Read from the Socket
     void*             addr,        // Data address
     int               size)        // Data length
{
   int L= ::recv(handle, (char*)addr, size, 0);
   IFIODM( traceh("%d= Socket(%p)::read(...)\n", L, this); )
   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::write
//
// Purpose-
//       Write to the Socket
//
//----------------------------------------------------------------------------
int                                 // Number of bytes sent
   Socket::write(                   // Write to the Socket
     const void*       addr,        // Data address
     int               size)        // Data length
{
   int L= ::send(handle, (char*)addr, size, 0);
   IFIODM( traceh("%d= Socket(%p)::write(...)\n", L, this); )
   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_Socket::~SSL_Socket
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   SSL_Socket::~SSL_Socket( void )  // Destructor
{  IFHCDM( debugh("SSL_Socket(%p)::~SSL_Socket() ssl(%p)\n", this, ssl); )

   if( ssl )                        // If SSL state exists
     SSL_free(ssl);                 // Delete it
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_Socket::SSL_Socket
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   SSL_Socket::SSL_Socket(          // Constructor
     SSL_CTX*          context)     // The associated SSL Context
:  Socket(), ssl_ctx(context), ssl(nullptr)
{  IFHCDM( debugh("SSL_Socket(%p)::SSL_Socket(%p,%d,%d,%d)\n", this, context,
                  family, type, protocol); )
}

   SSL_Socket::SSL_Socket(          // Copy constructor
     const SSL_Socket& source)      // Source SSL_Socket
:  Socket(source), ssl_ctx(source.ssl_ctx), ssl(nullptr)
{  }

//----------------------------------------------------------------------------
//
// Method-
//       SSL_Socket::operator=
//
// Purpose-
//       Assignment operator
//
//----------------------------------------------------------------------------
SSL_Socket&                         // (Always *this)
   SSL_Socket::operator=(           // Assignment operator
     const SSL_Socket& source)      // Source SSL_Socket
{
   Socket::operator=(source);

   this->ssl_ctx= source.ssl_ctx;
   this->ssl= nullptr;

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_Socket::connect
//
// Purpose-
//       Connect to peer
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   SSL_Socket::connect(             // Connect to peer
     sockaddr*         peer_addr,   // Peer address
     socklen_t         peer_size)   // Peer address length
{  IFHCDM(
     debugh("SSL_Socket(%p)::connect(%p,%d)\n", this, peer_addr, peer_size);
   )

   int rc= Socket::connect(peer_addr, peer_size); // Create the connection
   if( rc == 0 ) {
     ssl= SSL_new(ssl_ctx);
     if( ssl == nullptr ) {
       display_ERR();
       throw SocketException("SSL_new failure"); // (SHOULD NOT OCCUR)
     }
     SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

     SSL_set_fd(ssl, handle);
     if( SSL_connect(ssl) < 0 ) {
       display_ERR();
       throw SocketException("SSL_connect failure"); // (SHOULD NOT OCCUR)
     }
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_Socket::listen
//
// Purpose-
//       Listen for new connections
//
//----------------------------------------------------------------------------
Socket*                             // The new connection SSL_Socket
   SSL_Socket::listen( void )       // Get new connection SSL_Socket
{
   int rc= ::listen(handle, SOMAXCONN); // Wait for new connection
   if( rc != 0 ) {                  // If listen failure
     int ERRNO= errno;
     traceh("%d= SSL_Socket(%p)::listen() error(%d,%s)\n", rc, this,
            ERRNO, strerror(ERRNO));
     display_ERR();
     errno= ERRNO;
     return nullptr;
   }

   // Accept the next connection
   int client;
   for(;;) {
     peer_size= sizeof(peer_addr);
     client= ::accept(handle, (sockaddr*)&peer_addr, &peer_size);

     if( client >= 0 )              // If valid handle
       break;

     if( handle < 0 )               // If closed
       return nullptr;

     if( errno != EINTR ) {         // If not interrupted
       errorf("Warning: SSL_Socket::listen, accept failure(%s)\n",
              strerror(errno));
       return nullptr;
     }
   }

   SSL* ssl= SSL_new(ssl_ctx);
   if( ssl == nullptr ) {
     display_ERR();
     throw SocketException("SSL_new failure"); // (SHOULD NOT OCCUR)
   }
   SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

   SSL_set_fd(ssl, client);
   if( SSL_accept(ssl) < 0 ) {
     if( handle >= 0 )
       errorf("Warning: SSL_Socket::listen, SSL_accept failure\n");
     SSL_free(ssl);
     return nullptr;
   }

   SSL_Socket* result= new SSL_Socket(*this);
   result->handle= client;
   result->ssl= ssl;
   IFIODM( traceh("%p[%d]=SSL_Socket(%p)::listen()\n", result, client, this); )

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_Socket::read
//
// Purpose-
//       Read from the SSL_Socket
//
//----------------------------------------------------------------------------
int                                 // Number of bytes read
   SSL_Socket::read(                // Read from the SSL_Socket
     void*             addr,        // Data address
     int               size)        // Data length
{
   int ERRNO;                       // Preserved errno
   int L;                           // Number of bytes read

   for(;;) {
     errno= 0;
     if( recv_timeo ) {
       pollfd fd= { handle, POLLIN, 0 }; // Our pollfd
       L= poll(&fd, 1, recv_timeo);
       ERRNO= errno;
       IFIODM(
         traceh("%d= poll(%x,%x) %d\n", L, fd.events, fd.revents, recv_timeo);
       )
       if( L <= 0 ) {                 // If interrupt or timeout
         if( ERRNO == EINTR )
           continue;

         break;
       }
     }

     L= SSL_read(ssl, addr, size);
     ERRNO= errno;
     if( L > 0 )
       break;

     int X= SSL_get_error(ssl, L);
     IFIODM(
       traceh("%d= SSL_Socket(%p)::read() %d,%d,%s\n", L, this,
              X, ERRNO, strerror(ERRNO));
       display_ERR();
     )

     if( X != SSL_ERROR_WANT_READ && X != SSL_ERROR_WANT_WRITE )
       break;
   }

   IFIODM( traceh("%d= SSL_Socket(%p)::read(...)\n", L, this); )
   errno= ERRNO;                    // (Return actual errno)
   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_Socket::write
//
// Purpose-
//       Write to the SSL_Socket
//
//----------------------------------------------------------------------------
int                                 // Number of bytes sent
   SSL_Socket::write(               // Write to the SSL_Socket
     const void*       addr,        // Data address
     int               size)        // Data length
{
   int ERRNO;                       // Preserved errno
   int L;                           // Number of bytes read

   for(;;) {
     L= SSL_write(ssl, addr, size);
     ERRNO= errno;
     if( L > 0 )
       break;

     int X= SSL_get_error(ssl, L);
     if( X == SSL_ERROR_ZERO_RETURN || X == SSL_ERROR_NONE )
       break;

     if( X != SSL_ERROR_WANT_READ && X != SSL_ERROR_WANT_WRITE )
       break;
   }

   IFIODM( traceh("%d= SSL_Socket(%p)::write(...)\n", L, this); )
   errno= ERRNO;
   return L;
}
} // namespace _PUB_NAMESPACE
