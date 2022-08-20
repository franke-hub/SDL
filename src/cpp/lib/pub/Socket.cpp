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
//       2022/08/20
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE
#define _GNU_SOURCE                 // For ppoll
#endif
#define OPENSSL_API_COMPAT 30000    // Deprecate OSSL functions < 3.0.0

#include <new>                      // For std::bad_alloc
#include <mutex>                    // For mutex, std::lock_guard, ...
#include <stdexcept>                // For std::runtime_error

#include <assert.h>                 // For assert
#include <errno.h>                  // For errno
#include <fcntl.h>                  // For fcntl
#include <netdb.h>                  // For addrinfo, ...
#include <poll.h>                   // For poll, ...
#include <stdarg.h>                 // For va_* functions
#include <stddef.h>                 // For offsetof
#include <string.h>                 // For memset, ...
#include <unistd.h>                 // For close, ...
#include <arpa/inet.h>              // For internet address conversions
#include <openssl/err.h>            // For ERR_error_string
#include <openssl/ssl.h>            // For SSL, SSL_CTX
#include <sys/resource.h>           // For getrlimit
#include <sys/select.h>             // For select, ...
#include <sys/un.h>                 // For sockaddr_un
#include <sys/time.h>               // For timeval, ...

#include <pub/utility.h>            // For to_string(), ...
#include <pub/Debug.h>              // For debugging
#include <pub/Must.h>               // For pub::Must::malloc

#include "pub/Select.h"             // For pub::Select
#include "pub/Socket.h"             // For pub::Socket, implemented

using namespace _PUB_NAMESPACE::debugging; // For debugging

//----------------------------------------------------------------------------
// Typedefs (used by internal subroutines)
//----------------------------------------------------------------------------
typedef pub::Socket::sockaddr_u     sockaddr_u;
typedef pub::Socket::sockaddr_x     sockaddr_x;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  IODM= false                      // I/O Debug Mode?
,  IOEM= true                       // I/O error Debug Mode?

,  USE_CHECKING= true               // Use internal cross-checking?
}; // enum

// Maximum/minimum sockaddr_u lengths
static int constexpr   max_sock= (int)sizeof(sockaddr_u);
static int constexpr   min_sock= (int)sizeof(sockaddr::sa_family);

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
[[noreturn]] static void sno_exception(int line);

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
//       sno_exception
//
// Purpose-
//       Message: A should not occur situation occured.
//
//----------------------------------------------------------------------------
[[noreturn]]
static void
   sno_exception(int line)
{
   errorf("%4d %s Should not occur (but did)\n", line, __FILE__);
   throw std::runtime_error("Should not occur");
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::copy
//
// Purpose-
//       Copy host_addr/size and peer_addr_size
//
//----------------------------------------------------------------------------
void
   Socket::copy(                    // Copy host_addr/size and peer_addr/size
     const Socket&     source)      // From this source Socket
{
   host_addr.copy(source.host_addr);
   peer_addr.copy(source.peer_addr);
   host_size= source.host_size;
   peer_size= source.peer_size;
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
,  selected([](int) {})             // Default (NOP) event handler
,  host_addr(), peer_addr()
{  if( HCDM )
     debugh("Socket(%p)::Socket()\n", this);

   host_size= sizeof(host_addr);
   peer_size= sizeof(peer_addr);
}

   Socket::Socket(                  // Copy constructor
     const Socket&     source)      // Source Socket
:  Object()
{  if( HCDM )
     debugh("Socket(%p)::Socket(%p)\n", this, &source);

   copy(source);
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

   copy(source);
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

   debugf("..selector(%p) handle(%d) family(%d) type(%d)\n", selector
         , handle, family, type);
   debugf("..host_addr: %s\n", host_addr.to_string().c_str());
   debugf("..peer_addr: %s\n", peer_addr.to_string().c_str());
   debugf("..host_size(%d), peer_size(%d)\n", host_size, peer_size);
}

void
   Socket::trace(                   // Trace socket operation
     int               line,        // For this source code line
     const char*       fmt,         // Format string
                       ...)         // The PRINTF argument list
{
   int ERRNO= errno;                // (Preserve errno)
   va_list             argptr;      // Argument list pointer

   std::lock_guard<decltype(*Debug::get())> lock(*Debug::get());

   traceh("%4d Socket: ", line);    // (Heading)

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
//       Socket::get_option
//
// Purpose-
//       Get socket option (not traced)
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
//       Socket::get_unix_name
//
// Purpose-
//       Get AF_UNIX socket file name
//
//----------------------------------------------------------------------------
const char*                         // The unix socket file name
   Socket::get_unix_name( void ) const // Get unix socket file name
{
   if( family != AF_UNIX )          // If not an AF_UNIX socket
     return nullptr;                // There is no socket file name

   return ((sockaddr_un*)host_addr.su_x.x_sockaddr)->sun_path;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::set_option
//
// Purpose-
//       Set socket option (not traced)
//
//----------------------------------------------------------------------------
int                                 // Return code
   Socket::set_option(              // Set socket option
     int               optlevel,    // Level
     int               optname,     // Name
     const void*       optval,      // Option value
     socklen_t         optlen)      // Option length (IN/OUT)
{  return ::setsockopt(handle, optlevel, optname, optval, optlen); }

//----------------------------------------------------------------------------
//
// Method-
//       Socket::set_peer_addr
//
// Purpose-
//       Set the peer internet address/length
//
//----------------------------------------------------------------------------
void
   Socket::set_peer_addr(           // Set peer address
     const sockaddr*   peeraddr,    // Peer address
     socklen_t         peersize)    // Peer address length
{
   peer_size= peersize;
   peer_addr.copy(peeraddr, peersize);
}

int                                 // Return code, 0 OK
   Socket::set_peer_addr(           // Set peer address (See set_host_addr)
     std::string       nps)         // "name:port" string
{
   sockaddr_storage peeraddr;
   socklen_t peersize= sizeof(peeraddr);
   int rc= name_to_addr(nps, (sockaddr*)&peeraddr, &peersize, family);
   if( rc == 0 ) {
     peer_size= peersize;
     peer_addr.copy((sockaddr*)&peeraddr, peersize);
   }
   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::accept
//
// Purpose-
//       Accept the next available connection
//
// Implementation notes-
//       Retries errno == EINTR
//
//----------------------------------------------------------------------------
Socket*                             // The new connection Socket
   Socket::accept( void )           // Get new connection Socket
{  if( HCDM )
     debugh("Socket(%p)::accept handle(%d)\n", this, handle);

   // Accept the next connection
   sockaddr_storage peeraddr;
   socklen_t peersize= sizeof(peeraddr);
   int client= ::accept(handle, (sockaddr*)&peeraddr, &peersize);
   if( IODM ) trace(__LINE__, "%d= accept", client);
   if( client < 0 )
     return nullptr;

   Socket* result= new Socket();
   result->handle= client;
   result->host_size= host_size;
   result->host_addr.copy(host_addr);
   result->peer_size= peersize;
   result->peer_addr.copy((sockaddr*)&peeraddr, peersize);

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

   int rc= ::bind(handle, hostaddr, hostsize);
   if( IODM ) trace(__LINE__, "%d= bind(%d)", rc, handle);
   if( rc == 0 ) {
     host_size= hostsize;
     host_addr.copy(hostaddr, hostsize);
   }

   int port= get_host_port();
   if( port == 0 ) {                // If IPV4/6 port not assigned
     sockaddr_storage haveaddr;
     socklen_t havesize= sizeof(haveaddr);
     if( ::getsockname(handle, (sockaddr*)&haveaddr, &havesize) == 0 )
       host_addr.su_i4.sin_port= ((sockaddr_in*)&haveaddr)->sin_port;
   }

   return rc;
}

int                                 // Return code (0 OK)
   Socket::bind(                    // Bind to address
     const std::string&nps)         // Host name:port string
{  if( HCDM )
     debugh("Socket(%p)::bind(%s)\n", this, nps.c_str());

   sockaddr_storage hostaddr;
   socklen_t hostsize= sizeof(hostaddr);
   int rc= name_to_addr(nps, (sockaddr*)&hostaddr, &hostsize, family);
   if( rc )
     return rc;

   return bind((sockaddr*)&hostaddr, hostsize);
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
//       Calling close when aready closed is not an error.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   Socket::close( void )            // Close the Socket
{  if( HCDM )
     debugh("Socket(%p)::close() handle(%d)\n", this, handle);

   int rc= 0;
   if( handle >= 0 ) {
     Select* selector= this->selector;
     if( selector ) {               // If Select controlled
       selector->with_lock([this]() {
         // We may have been blocked by close() running on another thread.
         // If so, selector->remove() will have set selector == nullptr
         // and closed the socket.
         if( this->selector )
           this->selector->remove(this);

         on_select([](int) {});     // Reset default (NOP) select handler
       }); // selector->with_lock(...)
     }

     // Reset host_addr/peer_addr, host_size/peer_size, and handle
     host_addr.reset();
     peer_addr.reset();
     host_size= 0;
     peer_size= 0;

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

   int rc= ::connect(handle, peeraddr, peersize);
   if( IODM )
     trace(__LINE__, "%d= connect(%d,%p,%d)", rc, handle, peeraddr, peersize);

   if( rc == 0 ) {
     peer_size= peersize;
     peer_addr.copy(peeraddr, peersize);
   }

   return rc;
}

int                                 // Return code (0 OK)
   Socket::connect(                 // Connect to address
     const std::string&nps)         // Peer name:port string
{  if( HCDM )
     debugh("Socket(%p)::connect(%s)\n", this, nps.c_str());

   sockaddr_storage peeraddr;
   socklen_t peersize= sizeof(peeraddr);
   int rc= name_to_addr(nps, (sockaddr*)&peeraddr, &peersize, family);
   if( rc )
     return rc;

   return connect((sockaddr*)&peeraddr, peersize);
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
     sockaddr*         addr,        // OUT: The sockaddr
     socklen_t*        size,        // INP/OUT: The addr length
     int               family)      // The preferred address family
{  if( HCDM )
     debugh("Socket::name_to_addr(%s,%p,%d,%d)\n"
           , nps.c_str(), addr, *size, family);

   if( family == AF_UNIX ) {
     if( nps.size() >= sizeof(sockaddr_un::sun_path)
         || size_t(*size) <= (nps.size()+offsetof(sockaddr_un, sun_path)) ) {
       errno= EINVAL;
       if( IOEM )
         trace(__LINE__, "'%s' AF_UNIX name too long", nps.c_str());
       return -1;
     }

     *size= offsetof(sockaddr_un, sun_path) + nps.size();
     addr->sa_family= AF_UNIX;
     strcpy(((sockaddr_un*)addr)->sun_path, nps.c_str());
     return 0;
   }

   size_t x= nps.find(':');
   if( x == std::string::npos ) {
     errno= EINVAL;
     if( IOEM )
       trace(__LINE__, "'%s' name:port missing ':' delimiter", nps.c_str());
     return -1;
   }

   // Generate IpV4 or IpV6 address, preferring the specified address family
   std::string name;
   if( x )
     name= nps.substr(0, x);
   else
     name= Socket::gethostname();

   std::string port= nps.substr(x+1);
   if( port == "" )
     port= "0";

   addrinfo* info= nullptr;         // Resultant info
   int rc= getaddrinfo(name.c_str(), port.c_str(), nullptr, &info);
   if( IODM ) trace(__LINE__, "%d= getaddrinfo(%s)", rc, nps.c_str());
   if( rc ) {                       // If unable to get addrinfo
     if( IOEM )
       trace(__LINE__, "'%s' name:port invalid/unknown", nps.c_str());
     *size= 0;
     errno= EINVAL;
     rc= -1;
   } else {
     addrinfo* used= info;
     if( family != AF_UNSPEC ) {
       while( used ) {
         if( family == used->ai_family )
           break;

         used= used->ai_next;
       }
       if( used == nullptr )
         used= info;
     }
     memcpy(addr, used->ai_addr, used->ai_addrlen);
     *size= used->ai_addrlen;
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
// Implementation notes-
//       A SocketException is thrown if the Socket's already open
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

   host_addr.reset();
   peer_addr.reset();
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
   ssize_t L= ::read(handle, (char*)addr, size);
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
     const sockaddr*   peeraddr,    // Target peer address
     socklen_t         peersize)    // Target peer address length
{
   if( family == AF_UNIX && (void*)peeraddr == (void*)&peer_addr ) {
     peeraddr= ((sockaddr_u*)peeraddr)->su_x.x_sockaddr;
     if( peeraddr == nullptr ) {
       errorf("Socket::sendto peer_addr not initialized");
       sno_exception(__LINE__);
     }
   }
   ssize_t L= ::sendto(handle, addr, size, flag, peeraddr, peersize);
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
   else {
     rc= ::shutdown(handle, how);
     if( IODM ) trace(__LINE__, "%d= shutdown(%d)", rc, how);
   }

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
   ssize_t L= ::write(handle, (char*)addr, size);
   if( IODM ) trace(__LINE__, "%zd= write()", L);
   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::sockaddr_u::copy
//
// Purpose-
//       Replace content from source
//
//----------------------------------------------------------------------------
void
   Socket::sockaddr_u::copy(        // Replace this sockaddr_u
     const sockaddr_u& source)      // From this sockaddr_u
{
   reset();                         // Release allocated storage

   *this= source;
   if( source.su_af == AF_UNIX ) {
     socklen_t size= source.su_x.x_socksize;
     assert( size_t(size) <= sizeof(sockaddr_un) );
     if( source.su_x.x_sockaddr == &source.sa ) {
       assert( size_t(size) <  (offsetof(sockaddr_x, x_socksize) - 1) );
       su_x.x_sockaddr= &this->sa;
     } else {
       assert( size_t(size) >= (offsetof(sockaddr_x, x_socksize) - 1) );
       su_x.x_sockaddr= (sockaddr*)Must::malloc(size+1);
       memcpy(su_x.x_sockaddr, source.su_x.x_sockaddr, size+1);
     }
   }
}

void
   Socket::sockaddr_u::copy(        // Replace this sockaddr_u
     const sockaddr*   addr,        // From this sockaddr
     socklen_t         size)        // Of this length
{
   reset();                         // Release allocated storage

   if( addr->sa_family == AF_UNIX ) {
     char* copy= (char*)this;
     if( size_t(size) >= (offsetof(sockaddr_x, x_socksize) - 1) ) {
       assert( size_t(size) <= sizeof(sockaddr_un) );
       if( size_t(size) < offsetof(sockaddr_un, sun_path) )
         size= offsetof(sockaddr_un, sun_path);
       copy= (char*)Must::malloc(size+1); // (+1 for trailing '\0')
     }

     memcpy(copy, addr, size);
     copy[size]= '\0';
     su_x.x_family= AF_UNIX;
     su_x.x_socksize= size;
     su_x.x_sockaddr= (sockaddr*)copy;
   } else {
     if( size_t(size) > sizeof(sockaddr_u) ) {
       errorf("socket length(%d) > maximum(%d)\n", size, max_sock);
       sno_exception(__LINE__);
     }

     memcpy(this, addr, size);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::sockaddr_u::reset
//
// Purpose-
//       Reset the sockaddr_u, zeroing it
//
//----------------------------------------------------------------------------
void
   Socket::sockaddr_u::reset( void ) // Reset the sockaddr_u
{
   if( su_af == AF_UNIX && su_x.x_sockaddr != &this->sa )
     free(su_x.x_sockaddr);

   su_align[0]= su_align[1]= su_align[2]= su_align[3]= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::sockaddr_u::to_string
//
// Purpose-
//       Convert (sockaddr_u) to string
//
// Implementation notes-
//       Families AF_INET, AF_INET6, and IF_UNIX are currently supported.
//       Others can be added if desired.
//
//----------------------------------------------------------------------------
std::string
   Socket::sockaddr_u::to_string( void ) const // Convert to string
{
   std::string         result= "<inet_ntop error>"; // Default resultant

   const char*         buff= nullptr; // inet_ntop resultant
   char                work[256];   // (Address string buffer)

   errno= 0;
   switch( su_af )
   {
     case AF_INET:                  // If IPv4
       buff= inet_ntop(AF_INET, &su_i4.sin_addr, work, sizeof(work));
       if( buff )
         result= utility::to_string("%s:%d", buff, ntohs(su_i4.sin_port));
       break;

     case AF_INET6:                 // If IPv6
       buff= inet_ntop(AF_INET6, &su_i6.sin6_addr, work, sizeof(work));
       if( buff )
         result= utility::to_string("[%s]:%d", buff, ntohs(su_i6.sin6_port));
       break;

     case AF_UNIX: {{{{             // If AF_UNIX
       sockaddr_un* un= (sockaddr_un*)this->su_x.x_sockaddr;
       result= std::string(un->sun_path); // (Trailing '\0' always present)
       break;
     }}}}
     default:                       // If invalid address family
       result= utility::to_string("<sa_family_t(%d)>", su_af);
       errno= EINVAL;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_socket::SSL_socket
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   SSL_socket::SSL_socket(          // Constructor
     SSL_CTX*          context)     // The associated SSL Context
:  Socket(), ssl_ctx(context), ssl(nullptr)
{  if( HCDM ) debugh("SSL_socket(%p)::SSL_socket(%p)\n", this, context); }

   SSL_socket::SSL_socket(          // Copy constructor
     const SSL_socket& source)      // Source SSL_socket
:  Socket(source), ssl_ctx(source.ssl_ctx), ssl(nullptr)
{  }

//----------------------------------------------------------------------------
//
// Method-
//       SSL_socket::~SSL_socket
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   SSL_socket::~SSL_socket( void )  // Destructor
{  if( HCDM ) debugh("SSL_socket(%p)::~SSL_socket() ssl(%p)\n", this, ssl);

   if( ssl )                        // If SSL state exists
     SSL_free(ssl);                 // Delete it
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_socket::operator=
//
// Purpose-
//       Assignment operator
//
//----------------------------------------------------------------------------
SSL_socket&                         // (Always *this)
   SSL_socket::operator=(           // Assignment operator
     const SSL_socket& source)      // Source SSL_socket
{
   Socket::operator=(source);

   this->ssl_ctx= source.ssl_ctx;
   this->ssl= nullptr;

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_socket::debug
//       SSL_socket::trace
//
// Purpose-
//       Diagnostic display
//       Trace socket operation message
//
//----------------------------------------------------------------------------
void
   SSL_socket::debug(               // Diagnostic display
     const char*       info) const  // Diagnostic info
{
   debugf("SSL_socket(%p)::debug(%s)\n", this, info);
   debugf("..ssl_ctx(%p) ssl(%p)\n", ssl_ctx, ssl);
   Socket::debug(info);
}

void
   SSL_socket::trace(               // Trace SSL_socket operation
     int               line,        // For this source code line
     const char*       fmt,         // Format string
                       ...) const   // The PRINTF argument list
{
   va_list             argptr;      // Argument list pointer

   int ERRNO= errno;                // (Preserve errno)
   std::lock_guard<decltype(*Debug::get())> lock(*Debug::get());

   traceh("%4d SSL_socket(%p) ", line, this); // (Heading)

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
//       SSL_socket::accept
//
// Purpose-
//       Accept next connection
//
//----------------------------------------------------------------------------
Socket*                             // The new connection SSL_socket
   SSL_socket::accept( void )       // Get new connection SSL_socket
{  if( HCDM )
     debugh("SSL_socket(%p)::accept handle(%d)\n", this, handle);

   // Accept the next connection
   sockaddr_storage peeraddr;
   socklen_t peersize= sizeof(peeraddr);
   int client= ::accept(handle, (sockaddr*)&peeraddr, &peersize);
   if( IODM ) trace(__LINE__, "%d= accept", client);
   if( client < 0 )
     return nullptr;

   SSL_socket* result= new SSL_socket(ssl_ctx);
   result->handle= client;
   peer_addr.copy((sockaddr*)&peeraddr, peersize);
   peer_size= peersize;

   result->ssl= SSL_new(ssl_ctx);
   if( IODM ) trace(__LINE__, "%p= SSL_new", result->ssl);
   if( result->ssl == nullptr ) {
     display_ERR();
     delete result;
     return nullptr;
   }
   SSL_set_fd(result->ssl, client);
   SSL_set_mode(result->ssl, SSL_MODE_AUTO_RETRY);

   int rc= SSL_accept(result->ssl);
   if( IODM ) trace(__LINE__, "%d= SSL_accept", rc);
   if( rc != 1 ) {
     if( IOEM ) {                   // (May need to pass error info to user)
       char buff[256];
       int ec= SSL_get_error(result->ssl, rc);
       ERR_error_string(ec, buff);
       fprintf(stderr, "%d= SSL_socket::accept '%s'\n", rc, buff);
     }

     delete result;
     return nullptr;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_socket::connect
//
// Purpose-
//       Connect to peer
//
// Implementation notes-
//       Currently, SocketException is thrown if SSL_new or SSL_connect fails.
//       We may need to instead provide error recovery information.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   SSL_socket::connect(             // Connect to peer
     const sockaddr*   peer_addr,   // Peer address
     socklen_t         peer_size)   // Peer address length
{  if( HCDM )
     debugh("SSL_socket(%p)::connect(%p,%d)\n", this, peer_addr, peer_size);

   int rc= Socket::connect(peer_addr, peer_size); // Create the connection
   if( rc == 0 ) {
     ssl= SSL_new(ssl_ctx);
     if( IODM ) trace(__LINE__, "%p= SSL_new", ssl);
       if( ssl == nullptr ) {
       display_ERR();
       throw SocketException("SSL_new failure"); // (SHOULD NOT OCCUR)
     }
     SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

     SSL_set_fd(ssl, handle);
     rc= SSL_connect(ssl);
     if( IODM ) trace(__LINE__, "%d= SSL_connect(%p)", rc, ssl);
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
//       SSL_socket::read
//
// Purpose-
//       Read from the SSL_socket
//
//----------------------------------------------------------------------------
ssize_t                             // Number of bytes read
   SSL_socket::read(                // Read from the SSL_socket
     void*             addr,        // Data address
     size_t            size)        // Data length
{
   ssize_t L= SSL_read(ssl, addr, size);
   if( IODM ) trace(__LINE__, "%zd= SSL_read()", L);

   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_socket::write
//
// Purpose-
//       Write to the SSL_socket
//
//----------------------------------------------------------------------------
ssize_t                             // Number of bytes sent
   SSL_socket::write(               // Write to the SSL_socket
     const void*       addr,        // Data address
     size_t            size)        // Data length
{
   ssize_t L= SSL_write(ssl, addr, size);
   if( IODM ) trace(__LINE__, "%zd= SSL_write()", L);

   return L;
}
} // namespace _PUB_NAMESPACE
