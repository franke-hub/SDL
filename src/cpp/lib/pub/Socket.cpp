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
//       2022/05/18
//
// Implementation notes-
//       SocketSelect:
//         Do we want a Selector map rather than a List? NO. Too complex.
//         Do we want a sorted socket array? Maybe, slightly more complex.
//           Insert slower. Locate faster. (Implementation deferred)
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE
#define _GNU_SOURCE                 // For ppoll
#endif
#include <errno.h>                  // For errno
#include <mutex>                    // For mutex, std::lock_guard, ...

#include <fcntl.h>                  // For fcntl
#include <netdb.h>                  // For addrinfo, ...
#include <poll.h>                   // For poll, ...
#include <stdarg.h>                 // For va_* functions
#include <string.h>                 // For memset, ...
#include <unistd.h>                 // For close, ...
#include <arpa/inet.h>              // For internet address conversions
#include <openssl/err.h>            // For ERR_error_string
#include <sys/select.h>             // For select, ...
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

static std::string
   connect_mess(std::string name_port, std::string message)
{
   std::string S= "Socket::connect(";
   S += name_port;
   S += ") ";
   S += message;
   return S;
}

int                                 // Return code (0 OK)
   Socket::connect(                 // Connect to peer
     const std::string&name_port)   // Peer name:port
{  if( HCDM )
     debugh("Socket(%p)::connect(%s)\n", this, name_port.c_str());

   size_t x= name_port.find(':');
   if( x == std::string::npos )
     throw SocketException(connect_mess(name_port, "missing ':' delimiter"));
   std::string peer_name;
   if( x == 0 )
     peer_name= get_host_name();
   else
     peer_name= name_port.substr(0, x);

   std::string peer_port= name_port.substr(x+1);
   if( peer_port == "" )
     throw SocketException(connect_mess(name_port, "missing port number"));

   addrinfo hint{};                 // Hints
   hint.ai_family= family;
   hint.ai_socktype= type;
   hint.ai_protocol= PF_UNSPEC;

   addrinfo* info= nullptr;         // Resultant info
   int rc= getaddrinfo(peer_name.c_str(), peer_port.c_str(), &hint, &info);
   if( rc ) {                       // If unable to get addrinfo
     errorp("%d= %s", rc, connect_mess(name_port, "[getaddrinfo]").c_str());
   } else {
     rc= connect(info->ai_addr, info->ai_addrlen);
     int ERRNO= errno;
     freeaddrinfo(info);
     errno= ERRNO;
     if( rc && IODM )
       errorp("%d= %s", rc, connect_mess(name_port, "connect").c_str());
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
{  if( HCDM )
     debugh("Socket(%p)::listen handle(%d)\n", this, handle);

   int rc= ::listen(handle, SOMAXCONN); // Wait for new connection
   if( rc != 0 ) {                  // If listen failure
     if( IODM ) {
       trace(__LINE__, "%d= listen()", rc);
       display_ERR();
     }
     return nullptr;
   }

   // Accept the next connection
   int client;
   for(;;) {
     /* IMPLEMENTATION NOTE: *************************************************
     A problem occurs in Test/TestSock --stress, where clients only use
     connections for one HTTP operation. After about 30K operations clients
     fail to connect and the server ::accept operation does not complete.

     Problem 1) The server accept operation blocks.
     It's only this part of the problem that we can address here. The options
     tried are coded below. Options 0 and 1 rely on the client to fix the
     problem and options 2 and 3 prevent the accept from blocking. We don't
     want to leave Socket operations in an unrecoverable blocked state if it's
     reasonably avoidable. Option 3 only has about a 0.5% overhead over the
     entire HTTP operation sequence, and is the implementation chosen.

     Problem 2) Clients fail to connect.
     This occurs because sockets are left in the TIME_WAIT state after close,
     and the rapid re-use of ports exhausts the port space. In a server, this
     can only be fixed using SO_LINGER with linger l_onoff=1 and l_linger=0
     to immediately close its half of the socket. It only needs to do this
     when it detects a client close or a transmission error, so there's no
     associated client recovery required.

     We implement this SO_LINGER logic in TestSock's StreamServer::serve
     method. With that logic and StreamServer::stop's normal recovery logic,
     TestSock --stress runs properly with any of the options below.

     Implementation options are coded below.
     ************************************************************************/

// ===========================================================================
#define ACCEPT_OPTION 3
#define LISTEN_HCDM false

#if false                           // (Used for option verification)
static int once= true;
     if( once ) {
       once= false;
       debugf("%4d HCDM ACCEPT_OPTION(%d)\n", __LINE__, ACCEPT_OPTION);
     }
#endif

#if ACCEPT_OPTION == 0
     // Do nothing...
     //
     // The client fails to connect after about 30K operations and the ::accept
     // operation hangs.
     //
     // 6025 ops/second Timing w/TestSock USE_LINGER == true

#elif ACCEPT_OPTION == 1
     // Add a short time delay
     //
     // The client fails to connect after about 30K operations and the ::accept
     // operation hangs.
     //
     // 3200 ops/second Timing w/TestSock setting SO_LINGER option

     usleep(125);

#elif ACCEPT_OPTION == 2
     // Use select to insure that the accept won't block.
     //
     // The client fails to connect after about 30K operations.
     // The server does not see the client's failing connection attempts.
     // With TestSock's StreamServer::stop method disabled, the select times
     // out, and the "::accept would block" path is driven.
     //
     // With the stop method enabled, the listener socket is closed well before
     // the select timeout. In this instance (for some unknown reason) select
     // returns 1, so the accept fails with "Bad file descriptor" because it's
     // using the CLOSED handle.
     //
     // 5825 ops/second Timing w/TestSock USE_LINGER == true

     struct timeval tv= {};
     tv.tv_usec= 1000000;

     fd_set rd_set;
     FD_ZERO(&rd_set);
     FD_SET(handle, &rd_set);
     int rc= select(handle+1, &rd_set, nullptr, nullptr, &tv);
     if( LISTEN_HCDM )
       traceh("%4d %d=select(%d) tv(%zd,%zd) %d:%s\n", __LINE__, rc, handle+1
             , tv.tv_sec, tv.tv_usec, errno, strerror(errno));
     if( rc == 0 ) {                // If timeout
       if( IODM )
         debugh("%4d %s ::accept would block\n", __LINE__, __FILE__);
       return nullptr;
     }

#elif ACCEPT_OPTION == 3
     // Use poll to insure that the accept won't block.
     //
     // The client fails to connect after about 30K operations.
     // The server does not see the client's failing connection attempts.
     // When the poll times out, (pfd.revents&POLLIN) == 0 whether or not
     // TestSock's StreamServer::stop method is disabled, thus always driving
     // the "::accept would block" path.
     //
     // 6000 ops/second Timing w/TestSock USE_LINGER == true

     pollfd pfd= {};
     pfd.fd= handle;
     pfd.events= POLLIN;
     int rc= poll(&pfd, 1, 1000);   // 1 second timeout (1000 ms)
     if( LISTEN_HCDM )
       traceh("%4d %d=poll() {%.4x,%.4x}\n", __LINE__, rc
             , pfd.events, pfd.revents);
     if( (pfd.revents & POLLIN) == 0 ) { // If timeout
       if( IODM )
         debugh("%4d %s ::accept would block\n", __LINE__, __FILE__);
       return nullptr;
     }
#endif // ====================================================================

     if( LISTEN_HCDM )
       traceh("%4d HCDM accept\n", __LINE__);
     peer_size= sizeof(peer_addr);
     client= ::accept(handle, (sockaddr*)&peer_addr, &peer_size);
     if( LISTEN_HCDM )
       traceh("%4d HCDM(%d) %d %d,%d accepted %d %d:%s\n", __LINE__, handle
             , ACCEPT_OPTION, get_host_port(), get_peer_port(), client
             , errno, strerror(errno));

     if( client >= 0 )              // If valid handle
       break;

     if( handle < 0 )               // If socket is currently closed
       return nullptr;              // (Expected)

     if( errno != EINTR ) {         // If not interrupted
       if( IODM )
         errorp("listen [accept]");

       return nullptr;
     }
   }

   // NOTE: Copy constructor only copies host_addr/size and peer_addr/size.
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

   protocol= PF_UNSPEC;             // (Always specified as unspecified)
   this->family= family;
   this->type= type;
// this->protocol= protocol;        // (this->protocol undefined)

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
{  }

   SocketSelect::~SocketSelect( void )
{  free(sarray); }

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
   debugf("..sarray(%p) result(%p)\n", sarray, result);
   debugf("..left(%u) next(%u) size(%u) used(%u)\n", left, next, size, used);
   for(unsigned i= 0; i<used; ++i) {
     const Socket* socket= sarray[i].socket;
     debugf("..[%3d] %p %3d {%.4x,%.4x}\n", i, socket, result[i].fd
           , result[i].events, result[i].revents);
     if( socket->get_handle() != result[i].fd )
       debugf("..[%3d] %p %3d ERROR: SOCKET.HANDLE MISMATCH\n", i, socket
             , socket->get_handle());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       SocketSelect::insert
//
// Purpose-
//       Insert a Socket onto sarray
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   SocketSelect::insert(            // Insert Socket
     const Socket*     socket,      // The associated Socket
     int               events)      // The associated poll events
{  std::lock_guard<decltype(mutex)> lock(mutex);

   ssize_t j= locate(socket);
   if( j >= 0 )                     // If extant, treat operation as modify
     return modify(socket, events);

   if( used >= size ) {             // If expansion required
     if( (size+SIZE_INC) < size ) { // If arithmetic overflow
       errno= ENOMEM;
       return -1;
     }

     size_t combo_size= sizeof(*sarray) + sizeof(*result);
     Selector* rep_sarray= (Selector*)malloc((size+SIZE_INC) * combo_size);
     if( rep_sarray == nullptr ) {
       errno= ENOMEM;
       return -1;
     }
     for(unsigned i= 0; i<size; ++i)
       rep_sarray[i]= sarray[i];

     struct pollfd* rep_result= (struct pollfd*)(&rep_sarray[size+SIZE_INC]);
     for(unsigned i= 0; i<size; ++i)
       rep_result[i]= result[i];

     size += SIZE_INC;
     free(sarray);
     sarray= rep_sarray;
     result= rep_result;
   }

   sarray[used].socket= socket;
   result[used].fd= socket->get_handle();
   result[used].events= events;
   result[used].revents= 0;
   ++used;
// left= next= 0;                   // Not needed

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

   ssize_t i= locate(socket);
   if( i < 0 )                      // If not extant, treat operation as insert
     return insert(socket, events);

   result[i].fd= socket->get_handle();
   result[i].events= events;
   result[i].revents= 0;
   left= next= 0;
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       SocketSelect::remove
//
// Purpose-
//       Remove a Socket from sarray
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   SocketSelect::remove(            // Remove Socket
     const Socket*     socket)      // The associated Socket
{  std::lock_guard<decltype(mutex)> lock(mutex);

   ssize_t i= locate(socket);
   if( i < 0 ) {
     errno= EINVAL;
     return -1;
   }

   if( next == used )
     next= 0;
   if( result[i].revents )
     --left;

   for(unsigned j= (unsigned)i + 1; j<used; ++j) {
     sarray[j-1]= sarray[j];
     result[j-1]= result[j];
   }

   --used;
   // TODO: Shrink array if (size-used) > 1.5 SIZE_INC

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       SocketSelect::select
//
// Purpose-
//       Select the next available Socket from sarray
//
//----------------------------------------------------------------------------
Socket*                             // The next selected Socket, or nullptr
   SocketSelect::select(            // Select next Socket
     int               timeout)     // Timeout, in milliseconds
{  std::lock_guard<decltype(mutex)> lock(mutex);

   if( used == 0 ) {                // USAGE ERROR
     if( IODM )
       debugf("SocketSelect(%p)::select Empty Socket array\n", this);
     errno= EINVAL;
     return nullptr;
   }

   if( left )
     return remain();

   for(unsigned i= 0; i<used; ++i)
     result[i].revents= 0;

   left= poll(result, used, timeout);
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

   for(unsigned i= 0; i<used; ++i)
     result[i].revents= 0;

   left= ppoll(result, used, timeout, signals);
   if( left == 0 ) {
     errno= EAGAIN;
     return nullptr;
   }

   return remain();
}

//----------------------------------------------------------------------------
//
// Protected method-
//       SocketSelect::locate
//
// Purpose-
//       Locate a Socket in the sarray
//
// Implementation note-
//       Caller must hold mutex lock.
//
//----------------------------------------------------------------------------
ssize_t                             // The Selector index, -1 if not found
   SocketSelect::locate(            // Locate the Selector index
     const Socket*     socket) const // For this Socket
{
   for(unsigned i= next; i<used; ++i) {
     if( sarray[i].socket == socket )
       return i;
   }

   for(unsigned i= 0; i<next; ++i) {
     if( sarray[i].socket == socket )
       return i;
   }

   errno= EINVAL;
   return -1;
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
   for(unsigned i= next; i<used; ++i) {
     if( result[i].revents != 0 ) {
       --left;
       next= i + 1;
       return const_cast<Socket*>(sarray[i].socket);
     }
   }

   for(unsigned i= 0; i<next; ++i) {
     if( result[i].revents != 0 ) {
       --left;
       next= i + 1;
       return const_cast<Socket*>(sarray[i].socket);
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
