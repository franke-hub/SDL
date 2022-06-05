//----------------------------------------------------------------------------
//
//       Copyright (C) 2019-2022 Frank Eskesen.
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
//       2022/06/04
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE
#define _GNU_SOURCE                 // For ppoll
#endif
#include <new>                      // For std::bad_alloc
#include <mutex>                    // For mutex, std::lock_guard, ...

#include <errno.h>                  // For errno
#include <fcntl.h>                  // For fcntl
#include <netdb.h>                  // For addrinfo, ...
#include <poll.h>                   // For poll, ...
#include <stdarg.h>                 // For va_* functions
#include <string.h>                 // For memset, ...
#include <unistd.h>                 // For close, ...
#include <arpa/inet.h>              // For internet address conversions
#include <openssl/err.h>            // For ERR_error_string
#include <sys/resource.h>           // For getrlimit
#include <sys/select.h>             // For select, ...
#include <sys/time.h>               // For timeval, ...

#include <pub/utility.h>            // For to_string()
#include <pub/Debug.h>              // For debugging

#include "pub/Socket.h"             // The Socket Objects

using namespace _PUB_NAMESPACE::debugging; // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  IODM= false                      // I/O Debug Mode

,  USE_CROSS_CHECK= true            // Use internal cross-checking?
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
// Subroutine-
//       sno_handled
//
// Purpose-
//       Message: A should not occur situation has been handled
//
//----------------------------------------------------------------------------
static int
   sno_handled(int line)
{  errorf("%4d %s Should not occur (but handled)\n", line, __FILE__);
   return 0;
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
{  if( HCDM )
     debugh("Socket(%p)::~Socket()\n", this);

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
:  Object()
{  if( HCDM )
     debugh("Socket(%p)::Socket()\n", this);

   memset(&host_addr, 0, sizeof(host_addr));
   memset(&peer_addr, 0, sizeof(peer_addr));
   host_size= sizeof(host_addr);
   peer_size= sizeof(peer_addr);
}

   Socket::Socket(                  // Copy constructor
     const Socket&     source)      // Source Socket
:  Object()
{  if( HCDM )
     debugh("Socket(%p)::Socket(%p)\n", this, &source);

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

   close();

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
//       Socket::set_peer_addr
//
// Purpose-
//       Get the host name
//       Set the peer internet address/length
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

void
   Socket::set_peer_addr(           // Set peer address
     const sockaddr*   peeraddr,    // Peer address
     socklen_t         peersize)    // Peer address length
{
   if( (size_t)peersize > sizeof(peer_addr) )
     peersize= sizeof(peer_addr);

   memcpy(&peer_addr, peeraddr, peersize);
   peer_size= peersize;
}

int                                 // Return code, 0 OK
   Socket::set_peer_addr(           // Set peer address (See set_host_addr)
     std::string       nps)         // "name:port" string
{
   peer_size= sizeof(peer_addr);
   return name_to_addr(nps, (sockaddr*)&peer_addr, &peer_size);
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
//       Socket::accept
//
// Purpose-
//       Accept the next available connection
//
//----------------------------------------------------------------------------
Socket*                             // The new connection Socket
   Socket::accept( void )           // Get new connection Socket
{  if( HCDM )
     debugh("Socket(%p)::accept handle(%d)\n", this, handle);

   // Accept the next connection
   int client;
   for(;;) {
     peer_size= sizeof(peer_addr);
     client= ::accept(handle, (sockaddr*)&peer_addr, &peer_size);
     if( IODM )
       trace(__LINE__, "%d= accept", client);

     if( client >= 0 )              // If valid handle
       break;

     if( handle < 0 )               // If Socket is currently closed
       return nullptr;              // (Expected)

     if( errno != EINTR ) {         // If not interrupted
       if( IODM )
         errorp("accept");

       return nullptr;
     }
   }

   Socket* result= new Socket();
   result->handle= client;
   result->host_addr= this->host_addr;
   result->peer_addr= this->peer_addr;
   result->host_size= this->host_size;
   result->peer_size= this->peer_size;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::bind
//
// Purpose-
//       Bind this Socket to a connection
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Socket::bind(                    // Bind to connection
     const sockaddr*   hostaddr,    // Host address
     socklen_t         hostsize)    // Host address length
{  if( HCDM )
     debugh("Socket(%p)::bind(%p,%d)\n", this, hostaddr, hostsize);

   if( size_t(host_size) > sizeof(this->host_addr) ) {
     errno= EINVAL;                 // Buffer overflow detected
     return -1;
   }
   int rc= ::bind(handle, hostaddr, hostsize);
   if( IODM )
     trace(__LINE__, "%d= bind(%d)", rc, handle);
   if( rc == 0 ) {
     if( (const sockaddr*)&host_addr != hostaddr )
       memcpy(&host_addr, hostaddr, hostsize);
     this->host_size= hostsize;
   }

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
//       Calling close when aready closed allowed, and returns 0.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Socket::close( void )            // Close the Socket
{  if( HCDM ) debugh("Socket(%p)::close() handle(%d)\n", this, handle);

   int rc= 0;
   if( handle >= 0 ) {
     if( selector )                 // If SocketSelect controlled
       selector->remove(this);

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
//       Connect this Socket to a remote peer
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Socket::connect(                 // Connect to remote peer
     const sockaddr*   peeraddr,    // Peer socket address
     socklen_t         peersize)    // Peer address length
{  if( HCDM )
     debugh("Socket(%p)::connect(%p,%d)\n", this, peeraddr, peersize);

   if( size_t(peer_size) > sizeof(this->peer_addr) ) {
     errno= EINVAL;                 // Buffer overflow detected
     return -1;
   }
   int rc= ::connect(handle, peeraddr, peersize);
   if( IODM )
     trace(__LINE__, "%d= connect(%d)", rc, handle);
   if( rc == 0 ) {
     if( (const sockaddr*)&peer_addr != peeraddr )
       memcpy(&peer_addr, peeraddr, peersize);
     this->peer_size= peersize;
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::listen
//
// Purpose-
//       Indicate socket is a listener
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   Socket::listen( void )           // Indicate listener Socket
{  if( HCDM )
     debugh("Socket(%p)::listen handle(%d)\n", this, handle);

   int rc= ::listen(handle, SOMAXCONN); // Wait for new connection
   if( rc && IODM ) {               // If listen failure
     trace(__LINE__, "%d= listen()", rc);
     display_ERR();
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::name_to_addr
//
// Purpose-
//       Convert a "host:port" name to a sockaddr/length pair
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Socket::name_to_addr(            // Convert "host:port" to sockaddr_u
     const std::string&nps,         // The "host:port" name string
     sockaddr*         addr,        // OUT: The sockaddr*
     socklen_t*        size)        // INP/OUT: The addr length
{
   size_t x= nps.find(':');
   if( x == std::string::npos ) {
     if( IODM )
       traceh("'%s name:port missing ':' delimiter\n", nps.c_str());
     errno= EINVAL;
     return -1;
   }
   std::string name;
   if( x )
     name= nps.substr(0, x);
   else
     name= Socket::get_host_name();

   std::string port= nps.substr(x+1);
   if( port == "" )
     port= "0";

   addrinfo hint{};                 // Hints
   hint.ai_family= family;
   hint.ai_socktype= type;
   hint.ai_protocol= PF_UNSPEC;

   addrinfo* info= nullptr;         // Resultant info
   int rc= getaddrinfo(name.c_str(), port.c_str(), &hint, &info);
   if( rc && IODM ) {               // If unable to get addrinfo
     errorp("%d= getaddrinfo(%s,%s)", rc, name.c_str(), port.c_str());
     *size= 0;
   } else {
     memcpy(addr, info->ai_addr, info->ai_addrlen);
     *size= info->ai_addrlen;
     freeaddrinfo(info);
   }
   return rc;
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

   protocol= PF_UNSPEC;             // (Always specified as unspecified)
   this->family= family;
   this->type= type;
// this->protocol= protocol;        // (this->protocol undefined)

   memset(&host_addr, 0, sizeof(host_addr));
   memset(&peer_addr, 0, sizeof(peer_addr));
   host_size= 0;
   peer_size= 0;

   handle= ::socket(family, type, protocol);
   if( handle < 0 )                 // (Errors unexpected)
     return handle;

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::poll
//       Socket::ppoll
//
// Purpose-
//       Poll *this* Socket
//
//----------------------------------------------------------------------------
int                                 // Return code
   Socket::poll(                    // Poll this Socket
     struct pollfd*    pfd,         // IN/OUT The (system-defined) pollfd
     int               timeout)     // Timeout (in milliseconds)
{
   if( pfd->fd != handle )
     pfd->fd= handle;

   return ::poll(pfd, 1, timeout);
}

int                                 // Return code
   Socket::ppoll(                   // Poll this Socket
     struct pollfd*    pfd,         // The (system) pollfd
     const struct timespec*
                       timeout,     // Timeout
     const sigset_t*   sigmask)     // Signal set mask
{
   if( pfd->fd != handle )
     pfd->fd= handle;

   return ::ppoll(pfd, 1, timeout, sigmask);
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
//       Socket::recvfrom
//       Socket::recvmsg
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
   ssize_t L= ::recv(handle, addr, size, flag);
   if( IODM ) trace(__LINE__, "%zd= recv()", L);
   return L;
}

ssize_t                             // The number of bytes read
   Socket::recvfrom(                // Read from the socket
     void*             addr,        // Data address
     size_t            size,        // Data length
     int               flag,        // Send options
     sockaddr*         peer_addr,   // Source peer address
     socklen_t*        peer_size)   // Source peer address length
{
   ssize_t L= ::recvfrom(handle, addr, size, flag, peer_addr, peer_size);
   if( IODM ) trace(__LINE__, "%zd= recvfrom()", L);
   return L;
}

ssize_t                             // The number of bytes written
   Socket::recvmsg(                 // Receive message from the peer
     msghdr*           msg,         // Message header
     int               flag)        // Send options
{
   ssize_t L= ::recvmsg(handle, msg, flag);
   if( IODM ) trace(__LINE__, "%zd= recvmsg()", L);
   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::send
//       Socket::sendmsg
//       Socket::sendto
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
   ssize_t L= ::send(handle, addr, size, flag);
   if( IODM ) trace(__LINE__, "%zd= send()", L);
   return L;
}

ssize_t                             // The number of bytes written
   Socket::sendmsg(                 // Write to the socket
     const msghdr*     msg,         // Message header
     int               flag)        // Send options
{
   ssize_t L= ::sendmsg(handle, msg, flag);
   if( IODM ) trace(__LINE__, "%zd= sendmsg()", L);
   return L;
}

ssize_t                             // The number of bytes written
   Socket::sendto(                  // Write to the socket
     const void*       addr,        // Data address
     size_t            size,        // Data length
     int               flag,        // Send options
     const sockaddr*   peer_addr,   // Target peer address
     socklen_t         peer_size)   // Target peer address length
{
   ssize_t L= ::sendto(handle, addr, size, flag, peer_addr, peer_size);
   if( IODM ) trace(__LINE__, "%zd= sendto()", L);
   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::shutdown
//
// Purpose-
//       Shutdown the Socket
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Socket::shutdown(                // Shutdown the Socket
     int               how)         // Shutdown options
{  if( HCDM )
     debugh("Socket(%p)::shutdown(%d) handle(%d)\n", this, handle, how);

   int rc= -1;
   if( handle < 0 )
     errno= EBADF;
   else
     rc= ::shutdown(handle, how);

   return rc;
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
   debugf("..ssl_ctx(%p) ssl(%p)\n", ssl_ctx, ssl);
   Socket::debug(info);
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
//       SSL_Socket::accept
//
// Purpose-
//       Accept next connection
//
//----------------------------------------------------------------------------
Socket*                             // The new connection SSL_Socket
   SSL_Socket::accept( void )       // Get new connection SSL_Socket
{  if( HCDM )
     debugh("SSL_Socket(%p)::accept handle(%d)\n", this, handle);

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
       errorf("Warning: SSL_Socket::accept failure(%s)\n",
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
       errorf("Warning: SSL_Socket::accept failure\n");
     SSL_free(ssl);
     return nullptr;
   }

   SSL_Socket* result= new SSL_Socket(*this);
   result->handle= client;
   result->ssl= ssl;
   if( IODM )
     trace(__LINE__, "%p[%d]= accept()", result, client);

   return result;
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

int                                 // Return code (0 OK)
   SSL_Socket::connect(             // Connect to peer
     const std::string&nps)         // Peer "name:port" string
{  if( HCDM )
     debugh("SSL_Socket(%p)::connect(%s)\n", this, nps.c_str());

   return Socket::connect(nps);
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

//----------------------------------------------------------------------------
//
// Method-
//       SocketSelect::SocketSelect
//       SocketSelect::~SocketSelect
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   SocketSelect::SocketSelect( void )
{  if( HCDM )
     debugh("SocketSelect(%p)::SocketSelect\n", this);

   struct rlimit limits;
   int rc= getrlimit(RLIMIT_NOFILE, &limits);
   if( rc ) {
     errorf("%4d %s %d=getrlimit %d:%s\n", __LINE__, __FILE__, rc
           , errno, strerror(errno));
     limits.rlim_cur= 1024;
     limits.rlim_max= 4096;
   }

   size= limits.rlim_max;
   pollfd= (struct pollfd*)malloc(size * sizeof(pollfd));
   socket= (Socket**)malloc(size * sizeof(Socket));
   sindex= (int*)malloc(size * sizeof(int));
   if( pollfd == nullptr || socket == nullptr || sindex == nullptr )
     throw std::bad_alloc();

   memset(pollfd, 0, size * sizeof(pollfd));
   memset(socket, 0, size * sizeof(Socket));
   memset(sindex, 0xff, size * sizeof(int));
}

   SocketSelect::~SocketSelect( void )
{  if( HCDM )
     debugh("SocketSelect(%p)::~SocketSelect\n", this);

   std::lock_guard<decltype(mutex)> lock(mutex);
   for(int px= 0; px < used; ++px) {
     int fd= pollfd[px].fd;
     if( fd >= 0 && fd < size ) {
       Socket* socket= this->socket[fd];
       if( socket )
         socket->selector= nullptr;
       else if( USE_CROSS_CHECK )
         sno_handled(__LINE__);
     } else if( USE_CROSS_CHECK ) {
       sno_handled(__LINE__);
     }
   }

   free(pollfd);
   free(socket);
   free(sindex);
}

//----------------------------------------------------------------------------
//
// Method-
//       SocketSelect::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   SocketSelect::debug(             // Debugging display
     const char*       info)        // Caller information
{
   debugf("SocketSelect(%p)::debug(%s)\n", this, info);
   debugf("..pollfd(%p) socket(%p) sindex(%p)\n", pollfd, socket, sindex);
   debugf("..left(%u) next(%u) size(%u) used(%u)\n", left, next, size, used);
   debugf("..pollfd %d\n", used);
   for(int px= 0; px<used; ++px) {
     int fd= pollfd[px].fd;
     const Socket* socket= this->socket[fd];
     debugf("..[%3d] %p %3d:{%.4x,%.4x}\n", px, socket, fd
           , pollfd[px].events, pollfd[px].revents);
     if( socket->handle != pollfd[px].fd )
       debugf("..[%3d] %p %3d ERROR: SOCKET.HANDLE MISMATCH\n", px, socket
             , socket->handle);
     else if( px != this->sindex[fd] )
       debugf("..[%3d] %p %3d ERROR: HANDLE.SINDEX MISMATCH\n", px, socket
             , this->sindex[fd]);

   }

   debugf("..socket\n");
   for(int sx= 0; sx<size; ++sx) {
     if( socket[sx] )
       debugf("[%3d] %p\n", sx, socket[sx]);
   }

   debugf("..sindex\n");
   for(int fd= 0; fd<size; ++fd) {
     if( sindex[fd] >= 0 )
       debugf("[%3d] -> [%3d]\n", fd, sindex[fd]);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       SocketSelect::insert
//
// Purpose-
//       Insert a Socket onto the Socket array
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   SocketSelect::insert(            // Insert Socket
     Socket*           socket,      // The associated Socket
     int               events)      // The associated poll events
{  std::lock_guard<decltype(mutex)> lock(mutex);

   int fd= socket->handle;
   if( fd < 0 || fd >= size ) {     // (Probably closed socket)
     errorf("SocketSelect(%p)::insert(%p) invalid socket handle(%d)\n", this
           , socket, fd);
     errno= EINVAL;
     return -1;
   }

   if( socket->selector ) {
     errorf("SocketSelect(%p)::insert(%p) already inserted(%p)\n", this
           , socket, socket->selector);
     errno= EINVAL;
     return -1;
   }

   if( USE_CROSS_CHECK ) {
     if( used >= size ) {           // More FD's used than hard limit
       errorf("SocketSelect(%p)::insert(%p) Should not occur\n", this, socket);
       errno= EINVAL;
       return -1;
     }

     // If the socket is already assigned, something is wrong with our table.
     // Perhaps the socket got closed and opened, and we weren't informed.
     if( this->socket[fd] ) {       // Socket already assigned
       errorf("SocketSelect(%p)::insert(%p) Should not occur\n", this, socket);
       errno= EINVAL;
       return -1;
     }
   }

   struct pollfd* pollfd= this->pollfd + used;
   pollfd->fd= fd;
   pollfd->events= events;
   pollfd->revents= 0;
   this->sindex[fd]= used;
   this->socket[fd]= socket;

   ++used;
// left= next= 0;                   // Not needed, revents == 0

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       SocketSelect::modify
//
// Purpose-
//       Modify a Socket's events mask
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   SocketSelect::modify(            // Modify Socket
     const Socket*     socket,      // The associated Socket
     int               events)      // The associated poll events
{  std::lock_guard<decltype(mutex)> lock(mutex);

   int fd= socket->handle;
   if( fd < 0 || fd >= size ) {     // (Probably closed socket)
     errorf("SocketSelect(%p)::modify(%p) invalid socket handle(%d)\n", this
           , socket, fd);
     errno= EINVAL;
     return -1;
   }

   if( socket->selector != this ) { // If owned by a different selector
     sno_handled(__LINE__);
     return socket->selector->modify(socket, events);
   }

   int px= sindex[fd];
   pollfd[px].events= events;
   pollfd[px].revents= 0;

   left= next= 0;
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       SocketSelect::remove
//
// Purpose-
//       Remove a Socket from the Socket array
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   SocketSelect::remove(            // Remove Socket
     Socket*           socket)      // The associated Socket
{  std::lock_guard<decltype(mutex)> lock(mutex);

   if( socket->selector == nullptr ) { // If Socket isn't owned by a selector
     errorf("%4d %s remove(%p) but not active\n", __LINE__, __FILE__, socket);
     errno= EINVAL;
     return -1;
   }

   if( socket->selector != this ) { // If owned by a different selector
     sno_handled(__LINE__);
     return socket->selector->remove(socket);
   }

   socket->selector= nullptr;
   int fd= socket->handle;
   if( USE_CROSS_CHECK && (fd < 0 || fd >= size) ) // If invalid socket handle
     return sno_handled(__LINE__);

   int px= sindex[fd];              // Get the poll index
   if( USE_CROSS_CHECK && (px < 0 || px >= size) ) // If invalid poll index
     return sno_handled(__LINE__);

   for(int i= px; i<(used-1); ++i)
     pollfd[i]= pollfd[i+1];

   this->socket[fd]= nullptr;
   this->sindex[fd]= -1;
   --used;

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       SocketSelect::select
//
// Purpose-
//       Select the next available Socket from the Socket array
//
//----------------------------------------------------------------------------
Socket*                             // The next selected Socket, or nullptr
   SocketSelect::select(            // Select next Socket
     int               timeout)     // Timeout, in milliseconds
{  std::lock_guard<decltype(mutex)> lock(mutex);

   if( used == 0 ) {                // If true, usage error
     if( IODM )
       debugf("SocketSelect(%p)::select Empty Socket array\n", this);
     errno= EINVAL;
     return nullptr;
   }

   if( left )
     return remain();

   for(int px= 0; px<used; ++px)
     pollfd[px].revents= 0;

   left= poll(pollfd, used, timeout);
   if( left == 0 ) {
     errno= EAGAIN;
     return nullptr;
   }

   return remain();
}

Socket*                             // The next selected Socket, or nullptr
   SocketSelect::select(            // Select next Socket
     const struct timespec*         // (tv_sec, tv_nsec)
                       timeout,     // Timeout, infinite if omitted
     const sigset_t*   signals)     // Timeout, in milliseconds
{  std::lock_guard<decltype(mutex)> lock(mutex);

   if( used == 0 ) {                // USAGE ERROR
     if( IODM )
       debugf("SocketSelect(%p)::select Empty Socket array\n", this);
     errno= EINVAL;
     return nullptr;
   }

   if( left )
     return remain();

   for(int px= 0; px<used; ++px)
     pollfd[px].revents= 0;

   left= ppoll(pollfd, used, timeout, signals);
   if( left == 0 ) {
     errno= EAGAIN;
     return nullptr;
   }

   return remain();
}

//----------------------------------------------------------------------------
//
// Protected method-
//       SocketSelect::remain
//
// Purpose-
//       Select the next remaining Socket
//
// Implementation note-
//       Caller must hold mutex lock.
//
//----------------------------------------------------------------------------
Socket*                             // The next selected Socket
   SocketSelect::remain( void )     // Select next remaining Socket
{
   for(int px= next; px<used; ++px) {
     if( pollfd[px].revents != 0 ) {
       --left;
       next= px + 1;
       int fd= pollfd[px].fd;
       return socket[fd];
     }
   }

   for(int px= 0; px<next; ++px) {
     if( pollfd[px].revents != 0 ) {
       --left;
       next= px + 1;
       int fd= pollfd[px].fd;
       return socket[fd];
     }
   }

   // ERROR: The number of elements set < number of elements found
   // ** THIS IS AN INTERNAL ERROR, NOT AN APPLICATION ERROR **
   debugf("%4d %s Should not occur(%d), internal correctable error\n"
         , __LINE__, __FILE__, left);
   left= 0;
   errno= EAGAIN;
   return nullptr;
}
} // namespace _PUB_NAMESPACE
