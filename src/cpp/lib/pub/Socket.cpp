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
//       2022/06/15
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE
#define _GNU_SOURCE                 // For ppoll
#endif
#define OPENSSL_API_COMPAT 0x3'00'00'000 // Deprecate function versions < 3.0.0

#include <new>                      // For std::bad_alloc
#include <mutex>                    // For mutex, std::lock_guard, ...
#include <stdexcept>                // For std::runtime_error

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

#include "pub/Socket.h"             // The Socket Objects

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

,  USE_CROSS_CHECK= true            // Use internal cross-checking?
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
//       init_addr
//
// Purpose-
//       Initialize host or peer socket address
//
// Implementation note-
//       out_addr= &host_addr or &peer_addr. out_addr always != inp_addr.
//       The caller sets host/peer_size, normally using it as inp_size.
//
//----------------------------------------------------------------------------
static void
   init_addr(                       // Initialize host or peer socket address
     sockaddr_u*       out_addr,    // OUT: The target socket address
     const void*       inp_addr,    // INP: The source socket address
     socklen_t         inp_size)    // INP: The source socket address length
{  if( HCDM )
     debugf("Socket::init_addr(%p,%p,%d)\n", out_addr, inp_addr, inp_size);

   if( inp_size < min_sock ) {      // If sa_family won't fit in result
     errorf("socket length(%d) < minimum(%d)\n", inp_size, min_sock);
     sno_exception(__LINE__);
   }

// memset(out_addr, 0, sizeof(*outaddr)); // (Not strictly necessary)
   if( ((sockaddr*)inp_addr)->sa_family == AF_UNIX ) {
     sockaddr_x* alt_addr= (sockaddr_x*)out_addr;
     alt_addr->x_sockaddr= (sockaddr*)malloc(inp_size+1);
     if( alt_addr->x_sockaddr == nullptr )
       throw std::bad_alloc();
     memcpy(alt_addr->x_sockaddr, inp_addr, inp_size);
     ((char*)alt_addr->x_sockaddr)[inp_size]= '\0';
     alt_addr->x_family= ((sockaddr*)inp_addr)->sa_family;
   } else if( inp_size > max_sock ) { // If result won't fit in sockaddr_u
     errorf("socket length(%d) > maximum(%d)\n", inp_size, max_sock);
     sno_exception(__LINE__);
   } else {
     memcpy(out_addr, inp_addr, inp_size);
   }
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
debugf("Socket(%p)::copy(%p)\n", this, &source);
// memset(&host_addr, 0, sizeof(host_addr)); // (Not strictly necessary)
   host_size= source.host_size;
   const void* from= &source.host_addr;
   if( source.host_addr.su_family == AF_UNIX )
     from= ((sockaddr_x*)&source.host_addr)->x_sockaddr;
   init_addr(&host_addr, from, host_size);

// memset(&peer_addr, 0, sizeof(peer_addr)); // (Not strictly necessary)
   peer_size= source.peer_size;
   from= &source.peer_addr;
   if( source.peer_addr.su_family == AF_UNIX )
     from= ((sockaddr_x*)&source.peer_addr)->x_sockaddr;
   init_addr(&peer_addr, from, peer_size);
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
                       ...) const   // The PRINTF argument list
{
   int ERRNO= errno;                // (Preserve errno)
   va_list             argptr;      // Argument list pointer

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
//       Socket::gethostname
//
// Purpose-
//       Get the host name
//
//----------------------------------------------------------------------------
std::string                         // The host name
   Socket::gethostname( void )      // Get host name
{
   char host_name[HOST_NAME_MAX];   // The host name
   int rc= ::gethostname(host_name, HOST_NAME_MAX);
   if( rc )
     host_name[0]= '\0';

   return host_name;
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
     const sockaddr*   peersock,    // Peer address
     socklen_t         peersize)    // Peer address length
{
   peer_size= peersize;
   init_addr(&peer_addr, peersock, peersize);
}

int                                 // Return code, 0 OK
   Socket::set_peer_addr(           // Set peer address (See set_host_addr)
     std::string       nps)         // "name:port" string
{
   sockaddr_storage peersock;
   socklen_t peersize= sizeof(peersock);
   int rc= name_to_addr(nps, (sockaddr*)&peersock, &peersize, family);
   if( rc == 0 ) {
     peer_size= peersize;
     init_addr(&peer_addr, &peersock, peer_size);
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
   sockaddr_storage peersock;
   socklen_t peersize= sizeof(peersock);
   int client= ::accept(handle, (sockaddr*)&peersock, &peersize);
   if( IODM ) trace(__LINE__, "%d= accept", client);
   if( client < 0 )
     return nullptr;

   Socket* result= new Socket();
   result->handle= client;
   result->peer_size= peersize;
   init_addr(&result->peer_addr, &peersock, peersize);

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
     const sockaddr*   hostsock,    // Host address
     socklen_t         hostsize)    // Host address length
{  if( HCDM )
     debugh("Socket(%p)::bind(%p,%d)\n", this, hostsock, hostsize);

   int rc= ::bind(handle, hostsock, hostsize);
   if( IODM ) trace(__LINE__, "%d= bind(%d)", rc, handle);
   if( rc == 0 ) {
     host_size= hostsize;
     init_addr(&host_addr, hostsock, host_size);
   }

   return rc;
}

int                                 // Return code (0 OK)
   Socket::bind(                    // Bind to address
     const std::string&nps)         // Host name:port string
{  if( HCDM )
     debugh("Socket(%p)::bind(%s)\n", this, nps.c_str());

   sockaddr_storage hostsock;
   socklen_t hostsize= sizeof(hostsock);
   int rc= name_to_addr(nps, (sockaddr*)&hostsock, &hostsize, family);
   if( rc )
     return rc;

   return bind((sockaddr*)&hostsock, hostsize);
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
     if( selector )                 // If SocketSelect controlled
       selector->remove(this);

     // Reset host_addr/peer_addr, host_size/peer_size
     if( host_addr.su_family == AF_UNIX )
       free(((sockaddr_x*)&host_addr)->x_sockaddr);
     if( peer_addr.su_family == AF_UNIX )
       free(((sockaddr_x*)&peer_addr)->x_sockaddr);

     memset(&host_addr, 0, sizeof(host_addr));
     memset(&peer_addr, 0, sizeof(peer_addr));
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
     const sockaddr*   peersock,    // Peer socket address
     socklen_t         peersize)    // Peer address length
{  if( HCDM )
     debugh("Socket(%p)::connect(%p,%d)\n", this, peersock, peersize);

   int rc= ::connect(handle, peersock, peersize);
   if( IODM )
     trace(__LINE__, "%d= connect(%d,%p,%d)", rc, handle, peersock, peersize);

   if( rc == 0 ) {
     peer_size= peersize;
     init_addr(&peer_addr, peersock, peersize);
   }

   return rc;
}

int                                 // Return code (0 OK)
   Socket::connect(                 // Connect to address
     const std::string&nps)         // Peer name:port string
{  if( HCDM )
     debugh("Socket(%p)::connect(%s)\n", this, nps.c_str());

   sockaddr_storage peersock;
   socklen_t peersize= sizeof(peersock);
   int rc= name_to_addr(nps, (sockaddr*)&peersock, &peersize, family);
   if( rc )
     return rc;

   return connect((sockaddr*)&peersock, peersize);
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
     debugh("Socket(%p)::name_to_addr(%s,%p,%d,%d)\n", this
           , nps.c_str(), addr, *size, family);

   if( family == AF_UNIX ) {
     if( nps.size() > sizeof(sockaddr_un::sun_path )
         || size_t(*size) < (nps.size()+offsetof(sockaddr_un, sun_path)) ) {
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
   switch( su_family )
   {
     case AF_INET:                  // If IPv4
       buff= inet_ntop(AF_INET, &su_in.sin_addr, work, sizeof(work));
       if( buff )
         result= utility::to_string("%s:%d", buff, ntohs(su_in.sin_port));
       break;

     case AF_INET6:                 // If IPv6
       buff= inet_ntop(AF_INET6, &su_in6.sin6_addr, work, sizeof(work));
       if( buff )
         result= utility::to_string("[%s]:%d", buff, ntohs(su_in6.sin6_port));
       break;

     case AF_UNIX: {{{{             // If AF_UNIX
       sockaddr_un* un= (sockaddr_un*)this->su_x.x_sockaddr;
       result= std::string(un->sun_path); // (Trailing '\0' always present)
       break;
     }}}}
     default:                       // If invalid address family
       result= utility::to_string("<sa_family_t(%d)>", su_family);
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
   sockaddr_storage peersock;
   socklen_t peersize= sizeof(peersock);
   int client= ::accept(handle, (sockaddr*)&peersock, &peersize);
   if( IODM ) trace(__LINE__, "%d= accept", client);
   if( client < 0 )
     return nullptr;

   SSL_socket* result= new SSL_socket(ssl_ctx);
   result->handle= client;
   init_addr(&result->peer_addr, &peersock, peersize);
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

   // Implementation note: When the code's modified to contain an internal
   // socket, use it's file descriptor as the initial limit.
   resize(1);                  // (Start with minimum limit)
}

   SocketSelect::~SocketSelect( void )
{  if( HCDM )
     debugh("SocketSelect(%p)::~SocketSelect\n", this);

// Locking here can't be necessary. Consider that if it was, then right after
// the lock's released the waiter's going to reference deallocated storage.

// Disassociating a Socket from the SocketSelect *shouldn't* be necessary.
// If it was, then there's a good chance that whatever code was using the
// SocketSelect still thinks it exists. Note that adding lock_guard logic
// here only helps to diagnose the problem, it doesn't solve it. Each user
// needs to insure that there are no dangling SocketSelect references to a
// deleted object.
// An example might help explain why:
// SocketSelect accesses are protected by a mutex. A SocketSelect references
// each inserted Socket and each inserted Socket references the SocketSelect.
// Suppose the SocketSelect destructor is called and then an associated Socket
// is closed in a different thread before the destructor completes. The Socket
// close operation invokes SocketSelect::remove, which blocks but then resumes
// after the destructor exits. SocketSelect::remove then references the (at
// least partially) deleted SocketSelect.
// Perhaps we could gimshuckle some way of fixing this particular problem, but
// applications also need to correlate SocketSelect and Socket references and
// insure that their SocketSelect object isn't deleted while Sockets reference
// it. We can't check the application's correlation method, we can only check
// our own. So we check, and if there's a *possible* dangling reference we
// complain knowing that if a problem does exist, it will be hard to debug.

// >>>>>>>>>>>>>>>>>>>>>>>> ** USER DEBUGGING NOTE ** <<<<<<<<<<<<<<<<<<<<<<<<
// Before deleting a SocketSelect object, you should insure that no Socket
// objects still reference it. That will remove the annoying error message
// you got and quite likely also avoid some hard to debug future error.
// >>>>>>>>>>>>>>>>>>>>>>>> ** USER DEBUGGING NOTE ** <<<<<<<<<<<<<<<<<<<<<<<<
   std::lock_guard<decltype(mutex)> lock(mutex);
   for(int px= 0; px < used; ++px) {
     int fd= pollfd[px].fd;
     if( fd >= 0 && fd < size ) {
       Socket* socket= this->socket[fd];
       if( socket ) {
         #define FMT "%4d SocketSelect(%p) Socket(%p) fd(%d) User error: " \
                     "Dangling reference\n"
         errorf(FMT, __LINE__, this, socket, fd);
         sno_handled(__LINE__);     // See ** USER DEBUGGING NOTE **, above
         debug("Additional debugging information");
         socket->selector= nullptr;
       } else if( USE_CROSS_CHECK ) {
         sno_handled(__LINE__);     // (socket[fd] == nullptr)
       }
     } else if( USE_CROSS_CHECK ) {
       sno_handled(__LINE__);       // (pollfd[px].fd >= size)
     }
   }

   free(pollfd);
   free(socket);
   free(sindex);

   pollfd= nullptr;
   socket= nullptr;
   sindex= nullptr;
   size= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       SocketSelect::debug
//
// Purpose-
//       Debugging display
//
// Implementation note-
//       This may be called with or without the lock held, which is why we use
//       a recursive_mutex rather than just a mutex.
//
//----------------------------------------------------------------------------
void
   SocketSelect::debug(             // Debugging display
     const char*       info) const  // Caller information
{
   std::lock_guard<decltype(mutex)> lock(mutex);
   debugf("SocketSelect(%p)::debug(%s)\n", this, info);
   debugf("..pollfd(%p) socket(%p) sindex(%p)\n", pollfd, socket, sindex);
   debugf("..left(%u) next(%u) size(%u) used(%u)\n", left, next, size, used);
   debugf("..pollfd %d\n", used);
   for(int px= 0; px<used; ++px) {
     int fd= pollfd[px].fd;
     const Socket* socket= this->socket[fd];
     debugf("....[%3d] %p %3d:{%.4x,%.4x}\n", px, socket, fd
           , pollfd[px].events, pollfd[px].revents);
     if( socket->handle != pollfd[px].fd )
       debugf("....[%3d] %p %3d ERROR: SOCKET.HANDLE MISMATCH\n", px, socket
             , socket->handle);
     else if( px != this->sindex[fd] )
       debugf("....[%3d] %p %3d ERROR: HANDLE.SINDEX MISMATCH\n", px, socket
             , this->sindex[fd]);
   }

   debugf("..socket\n");
   for(int sx= 0; sx<size; ++sx) {
     if( socket[sx] )
       debugf("....[%3d] %p\n", sx, socket[sx]);
   }

   debugf("..sindex\n");
   for(int fd= 0; fd<size; ++fd) {
     if( sindex[fd] >= 0 )
       debugf("....[%3d] -> [%3d]\n", fd, sindex[fd]);
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
{  if( HCDM )
     debugh("SocketSelect(%p)::insert(%p,0x%.4x) fd(%d)\n", this
           , socket, events, socket->handle);

   std::lock_guard<decltype(mutex)> lock(mutex);
// debugf("\n\nSS(%p)::insert(%p) %d\n", this, socket, socket->handle);
// debug_backtrace();
   int fd= socket->handle;
   if( fd < 0 ) {
     errno= EINVAL;
     return -1;
   }
   if( fd >= size )
     resize(fd);

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
   socket->selector= this;

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
{  if( HCDM )
     debugh("SocketSelect(%p)::modify(%p,0x%.4x)\n", this, socket, events);

   std::lock_guard<decltype(mutex)> lock(mutex);
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
{  if( HCDM )
     debugh("SocketSelect(%p)::remove(%p) fd(%d)\n", this
           , socket, socket->handle);

   std::lock_guard<decltype(mutex)> lock(mutex);
// debugf("\n\nSS(%p)::remove(%p) %d %p\n", this, socket, socket->handle, socket->selector);
// debug_backtrace();
   if( socket->selector == nullptr ) { // If Socket isn't owned by a selector
     if( socket->handle < 0 ) {     // (Socket::close may have been blocked)
       errno= EINVAL;
       return -1;
     }
     if( IOEM ) {
       errorf("%4d %s remove Socket(%p) selector(nullptr) fd(%d)\n"
             , __LINE__, __FILE__, socket, socket->handle);
       debug("Additional debugging information");
     }
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
{  if( HCDM )
     debugh("SocketSelect(%p)::select(%d)\n", this, timeout);

   std::lock_guard<decltype(mutex)> lock(mutex);
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
{  if( HCDM )
     debugh("SocketSelect(%p)::select({%'ld,%'ld},%p)\n", this
           , timeout->tv_sec, timeout->tv_nsec, signals);

   std::lock_guard<decltype(mutex)> lock(mutex);
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
   // ** THIS IS AN INTERNAL LOGIC ERROR, NOT AN APPLICATION ERROR **
   errorf("%4d internal error, info(%d)\n", __LINE__, left);
   sno_handled(__LINE__);
   left= 0;
   errno= EAGAIN;
   return nullptr;
}

//----------------------------------------------------------------------------
//
// Protected method-
//       SocketSelect::resize
//
// Purpose-
//       Resize the internal tables
//
// Implementation note-
//       Caller must hold mutex lock.
//       Per fd storage: 20
//
//----------------------------------------------------------------------------
inline void
   SocketSelect::resize(            // Resize the SocketSelect tables
     int               fd)          // For this file (positive) descriptor
{  if( HCDM )
     debugh("SocketSelect(%p)::resize(%d)\n", this, fd);

   int new_size= 0;
   if( fd < 32 )
     new_size= 32;
   else if( fd < 128 )
     new_size= 128;
   else if( fd < 512 )
     new_size= 512;
   else {
     struct rlimit limits;
     int rc= getrlimit(RLIMIT_NOFILE, &limits);
     if( rc ) {
       errorf("%4d %s %d=getrlimit %d:%s\n", __LINE__, __FILE__, rc
             , errno, strerror(errno));
       limits.rlim_cur= 1024;
       limits.rlim_max= 4096;
     }

     if( size_t(fd) < size_t(limits.rlim_cur) )
       new_size= limits.rlim_cur;
     else if( size_t(fd) < size_t(limits.rlim_max) )
       new_size= limits.rlim_max;
     else {
       // Request for file descriptor index >= limits.rlim_max
       // This should not be possible.
       debugf("%4d fd(%d) >= limit(%ld)\n", __LINE__, fd, limits.rlim_max);
       sno_exception(__LINE__);
     }
   }

   struct pollfd*
   new_pollfd= (struct pollfd*)realloc(pollfd, new_size * sizeof(pollfd));
   Socket** new_socket= (Socket**)realloc(socket, new_size * sizeof(Socket));
   int* new_sindex= (int*)realloc(sindex, new_size * sizeof(int));
   if( new_pollfd == nullptr||new_socket == nullptr||new_sindex == nullptr ) {
     free(new_pollfd);
     free(new_socket);
     free(new_sindex);
     throw std::bad_alloc();
   }

   int diff= new_size - size;
   memset(new_pollfd + size, 0x00, diff * sizeof(pollfd));
   memset(new_socket + size, 0x00, diff * sizeof(Socket));
   memset(new_sindex + size, 0xff, diff * sizeof(int));

   pollfd= new_pollfd;
   socket= new_socket;
   sindex= new_sindex;
   size= new_size;
}
} // namespace _PUB_NAMESPACE
