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
//       2021/12/06
//
//----------------------------------------------------------------------------
#include <errno.h>                  // For errno
#include <mutex>                    // For mutex, std::lock_guard, ...

#include <netdb.h>                  // Form addrinfo, ...
#include <poll.h>                   // For poll, ...
#include <stdarg.h>                 // For va_* functions
#include <string.h>                 // For memset, ...
#include <unistd.h>                 // For close, ...
#include <arpa/inet.h>              // For internet address conversions
#include <openssl/err.h>            // For ERR_error_string
#include <sys/time.h>               // For timeval, ...

#include <pub/utility.h>            // For to_string()
#include <pub/Debug.h>              // For debugging

#include "pub/Socket.h"             // The Socket Object

using namespace _PUB_NAMESPACE::debugging; // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  IODM= false                      // I/O Debug Mode
}; // enum

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
{  if( HCDM ) debugh("Socket(%p)::~Socket()\n", this);

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
{  if( HCDM ) debugh("Socket(%p)::Socket()\n", this);

   memset(&host_addr, 0, sizeof(host_addr));
   memset(&peer_addr, 0, sizeof(peer_addr));
   host_size= sizeof(host_addr);
   peer_size= sizeof(peer_addr);
}

   Socket::Socket(                  // Copy constructor
     const Socket&     source)      // Source Socket
:  Object(), handle(CLOSED)
{  if( HCDM ) debugh("Socket(%p)::Socket(%p)\n", this, &source);

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
{  if( HCDM ) debugh("Socket(%p)::operator=(%p)\n", this, &source);

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
//       Socket::debug
//       Socket::trace
//
// Purpose-
//       Diagnostic display
//       Trace socket operation message
//
//----------------------------------------------------------------------------
void
   Socket::debug(                   // Diagnostic display
     const char*       info) const  // Diagnostic info
{
   debugf("Socket(%p)::debug(%s) handle(%d)\n", this, info, handle);

   std::string S= host_addr.to_string();
   debugf("..host_addr: %s\n", host_addr.to_string().c_str());
   debugf("..peer_addr: %s\n", peer_addr.to_string().c_str());
   debugf("..host_size(%d), peer_size(%d), recv_timeo(%d), send_timeo(%d)\n"
         , host_size, peer_size, recv_timeo, send_timeo);
}

void
   Socket::trace(                   // Trace socket operation
     int               line,        // For this source code line
     const char*       fmt,         // Format string
                       ...) const   // The PRINTF argument list
{
   va_list             argptr;      // Argument list pointer

   int ERRNO= errno;                // (Preserve errno)
   std::lock_guard<decltype(*Debug::get())> lock(*Debug::get());

   traceh("%4d Socket(%p): ", line, this); // (Heading)

   va_start(argptr, fmt);           // Initialize va_ functions
   vtracef(fmt, argptr);            // (User error message)
   va_end(argptr);                  // Close va_ functions

   if( ERRNO ) {                    // If error
     tracef(" %d:%s\n", ERRNO, strerror(ERRNO)); // (errno information)
   } else {
     tracef("\n");
   }

   errno= ERRNO;                    // (Restore errno)
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::get_host_name
//
// Purpose-
//       Get the host name
//
//----------------------------------------------------------------------------
std::string                         // The host name
   Socket::get_host_name( void )    // Get host name
{
   char host_name[HOST_NAME_MAX];   // The host name
   int rc= gethostname(host_name, HOST_NAME_MAX);
   if( rc ) {
     int ERRNO= errno;
     traceh("%d Socket::get_host_name %d:%s\n", __LINE__
           , ERRNO, strerror(ERRNO));
     errno= ERRNO;
     host_name[0]= '\0';
   }
   return host_name;
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
{  if( HCDM ) debugh("Socket(%p)::bind(%d)\n", this, port);

   set_host_port(port);             // Set the host port

   int rc= ::bind(handle, (sockaddr*)&host_addr, host_size);
   if( IODM )
     trace(__LINE__, "%d= bind()", rc);

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
{  if( HCDM ) debugh("Socket(%p)::close() handle(%d)\n", this, handle);

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
     const sockaddr*   peer_addr,   // Peer address
     socklen_t         peer_size)   // Peer address length
{  if( HCDM )
     debugh("Socket(%p)::connect(%p,%d)\n", this, peer_addr, peer_size);

   if( size_t(peer_size) > sizeof(this->peer_addr) )
     throw SocketException("Socket::connect peer_size");
   int rc= ::connect(handle, peer_addr, peer_size);
   if( IODM )
     trace(__LINE__, "%d= connect(%d)", rc, handle);
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
     trace(__LINE__, "%d= listen()", rc);
     display_ERR();
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
       if( IODM )                   // (This shouldn't occur, but does)
         trace(__LINE__, "listen [accept]");
       return nullptr;
     }
   }

   Socket* result= new Socket(*this);
   result->handle= client;
   if( IODM )
     trace(__LINE__, "%p[%d]= listen", result, client);

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
{  if( HCDM )
     debugh("Socket(%p)::open(%d,%d,%d)\n", this, family, type, protocol);

   if( handle >= 0 )
     throw SocketException("Socket already open"); // This is a usage error

   memset(&host_addr, 0, sizeof(host_addr));
   memset(&peer_addr, 0, sizeof(peer_addr));
   host_size= sizeof(host_addr);
   peer_size= sizeof(peer_addr);

   handle= ::socket(family, type, protocol);
   if( handle < 0 )                 // (Errors unexpected)
     return handle;

   // Open accepted
   addrinfo hint{};                 // Hints
   hint.ai_family= family;
   hint.ai_socktype= type;
   hint.ai_protocol= protocol;

   addrinfo* info= nullptr;         // Resultant info
   int rc= getaddrinfo(get_host_name().c_str(), nullptr, &hint, &info);
   if( rc ) {                       // If unable to get addrinfo
     if( IODM )
       trace(__LINE__, "open [getaddrinfo]");
   } else {
     memcpy(&host_addr, info->ai_addr, info->ai_addrlen);
     host_size= info->ai_addrlen;
     freeaddrinfo(info);
   }

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
ssize_t                             // Number of bytes read
   Socket::read(                    // Read from the Socket
     void*             addr,        // Data address
     size_t            size)        // Data length
{
   ssize_t L= ::recv(handle, (char*)addr, size, 0);
   if( IODM ) trace(__LINE__, "%zd= read()", L);
   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::recv
//
// Purpose-
//       Receive from the Socket
//
//----------------------------------------------------------------------------
ssize_t                             // Number of bytes read
   Socket::recv(                    // Receive from the Socket
     void*             addr,        // Data address
     size_t            size,        // Data length
     int               flag)        // Receive options
{
   ssize_t L= ::recv(handle, (char*)addr, size, flag);
   if( IODM ) trace(__LINE__, "%zd= recv()", L);
   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::send
//
// Purpose-
//       Transmit to the Socket
//
//----------------------------------------------------------------------------
ssize_t                             // Number of bytes sent
   Socket::send(                    // Write to the Socket
     const void*       addr,        // Data address
     size_t            size,        // Data length
     int               flag)        // Transmit options
{
   ssize_t L= ::send(handle, (char*)addr, size, flag);
   if( IODM ) trace(__LINE__, "%zd= send()", L);
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
ssize_t                             // Number of bytes sent
   Socket::write(                   // Write to the Socket
     const void*       addr,        // Data address
     size_t            size)        // Data length
{
   ssize_t L= ::send(handle, (char*)addr, size, 0);
   if( IODM ) trace(__LINE__, "%zd= write()", L);
   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::sockaddr_u::to_string
//
// Purpose-
//       Convert (sockaddr_u) to string
//
//----------------------------------------------------------------------------
std::string
   Socket::sockaddr_u::to_string( void ) const // Convert to string
{
   std::string result;              // Resultant string
   char work[INET6_ADDRSTRLEN];     // (Address string buffer)
   const char* buff= nullptr;       // inet_ntop resultant

   errno= 0;
   if( su_family == AF_INET ) {    // If IPv4
     buff= inet_ntop(AF_INET, &su_in.sin_addr, work, sizeof(work));
     if( buff )
       result= utility::to_string("%s:%d", buff, ntohs(su_in.sin_port));

   } else if( su_family == AF_INET6) { // If IPv6
     buff= inet_ntop(AF_INET6, &su_in6.sin6_addr, work, sizeof(work));
     if( buff )
       result= utility::to_string("[%s]:%d\n", buff, ntohs(su_in6.sin6_port));

   } else {                         // If invalid address family
     buff= work;                    // (Indicate result valid)
     result= utility::to_string("<undefined(%d)>", su_family);
     errno= EINVAL;
   }

   if( buff == nullptr )
     result= "<inet_ntop error>";   // (errno set by inet_ntop)

   return result;
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
{  if( HCDM ) debugh("SSL_Socket(%p)::~SSL_Socket() ssl(%p)\n", this, ssl);

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
{  if( HCDM ) debugh("SSL_Socket(%p)::SSL_Socket(%p)\n", this, context);
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
//       SSL_Socket::debug
//       SSL_Socket::trace
//
// Purpose-
//       Diagnostic display
//       Trace socket operation message
//
//----------------------------------------------------------------------------
void
   SSL_Socket::debug(               // Diagnostic display
     const char*       info) const  // Diagnostic info
{
   debugf("SSL_Socket(%p)::debug(%s)\n", this, info);

   // NOT CODED TET
}

void
   SSL_Socket::trace(               // Trace SSL_Socket operation
     int               line,        // For this source code line
     const char*       fmt,         // Format string
                       ...) const   // The PRINTF argument list
{
   va_list             argptr;      // Argument list pointer

   int ERRNO= errno;                // (Preserve errno)
   std::lock_guard<decltype(*Debug::get())> lock(*Debug::get());

   traceh("%4d SSL_Socket(%p) ", line, this); // (Heading)

   va_start(argptr, fmt);           // Initialize va_ functions
   vtracef(fmt, argptr);            // (User error message)
   va_end(argptr);                  // Close va_ functions

   if( ERRNO ) {                    // If error
     tracef(" %d:%s\n", ERRNO, strerror(ERRNO)); // (errno information)
   } else {
     tracef("\n");
   }

   errno= ERRNO;                    // (Restore errno)
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
     const sockaddr*   peer_addr,   // Peer address
     socklen_t         peer_size)   // Peer address length
{  if( HCDM )
     debugh("SSL_Socket(%p)::connect(%p,%d)\n", this, peer_addr, peer_size);

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
     trace(__LINE__, "%d= listen()", rc);
     display_ERR();
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
   if( IODM )
     trace(__LINE__, "%p[%d]= listen()", result, client);

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
ssize_t                             // Number of bytes read
   SSL_Socket::read(                // Read from the SSL_Socket
     void*             addr,        // Data address
     size_t            size)        // Data length
{
   ssize_t L;                       // Number of bytes read

   for(;;) {
     errno= 0;
     if( recv_timeo ) {
       pollfd fd= { handle, POLLIN, 0 }; // Our pollfd
       L= poll(&fd, 1, recv_timeo);
       if( IODM )
         trace(__LINE__, "%zd= poll(%x,%x) %d", L
                       , fd.events, fd.revents, recv_timeo);
       if( L <= 0 ) {               // If interrupt or timeout
         if( errno == EINTR )
           continue;

         break;
       }
     }

     L= SSL_read(ssl, addr, size);
     if( L > 0 )
       break;

     int X= SSL_get_error(ssl, L);
     if( IODM )  {
       trace(__LINE__, "%zd= read() %d", L, X);
       display_ERR();
     }

     if( X != SSL_ERROR_WANT_READ && X != SSL_ERROR_WANT_WRITE )
       break;
   }

   if( IODM )
     trace(__LINE__, "%zd= read()", L);
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
ssize_t                             // Number of bytes sent
   SSL_Socket::write(               // Write to the SSL_Socket
     const void*       addr,        // Data address
     size_t            size)        // Data length
{
   ssize_t L;                       // Number of bytes read

   for(;;) {
     L= SSL_write(ssl, addr, size);
     if( L > 0 )
       break;

     int X= SSL_get_error(ssl, L);
     if( X == SSL_ERROR_ZERO_RETURN || X == SSL_ERROR_NONE )
       break;

     if( X != SSL_ERROR_WANT_READ && X != SSL_ERROR_WANT_WRITE )
       break;
   }

   if( IODM )
     trace(__LINE__, "%zd= write()", L);
   return L;
}
} // namespace _PUB_NAMESPACE
