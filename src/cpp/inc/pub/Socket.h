//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ~/pub/Socket.h
//
// Purpose-
//       Standard posix socket wrapper and openssl wrapper.
//
// Last change date-
//       2022/05/27
//
// Implementation warning-
//       THE SSL_Socket CLASS IS EXPERIMENTAL. The author expressly admits to
//       a lack of experience with openssl. He finds its documentation to be
//       either lacking or cryptic. However the documentation does make clear
//       that it's easy to misuse openssl and believe that you are using it
//       correctly when you are not. While this code "works," it probably
//       contains multiple security flaws. These are in addition to glitches
//       encountered in testing which have not been resolved.
//
//       For a usage sample, see ~/src/cpp/HTTP/SampleSSL.cpp
//       Known flaws exist.  See ~/src/cpp/HTTP/.README
//
// Implementation limitations-
//       sa_family_t: Only AF_INET and AF_INET6 are supported.
//       socket type: Only SOCK_STREAM supported.
//         protocols: Only IPPROTO_IP and IPPROTO_TCP supported. (UNCHECKED)
//       get/set_host_port currently rely on the Port field at the same offset.
//       (offsets of: sockaddr_in.in_port == sockaddr_in6.in6_port.)
//
// Implementation notes-
//       Error handling is limited, and mostly left up to the user.
//       SocketException is only thrown for usage errors and SHOULD NOT OCCUR
//       conditions.
//       (Check the library source code for details.)
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_SOCKET_H_INCLUDED
#define _LIBPUB_SOCKET_H_INCLUDED

#include <functional>               // For std::function
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string
#include <errno.h>                  // For EINVAL
#include <fcntl.h>                  // For fcntl
#include <netinet/in.h>             // For struct sockaddr_ definitions
#include <openssl/ssl.h>            // For SSL, SSL_CTX
#include <sys/poll.h>               // For struct pollfd, ...
#include <sys/socket.h>             // For socket methods

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros
#include "pub/Exception.h"          // For SocketException
#include "pub/Object.h"             // For base class, _PUB_NAMESPACE, ...

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class SocketSelect;                 // Socket select controller

//----------------------------------------------------------------------------
//
// Class-
//       SocketException
//
// Purpose-
//       Socket Exception.
//
//----------------------------------------------------------------------------
class SocketException : public Exception { using Exception::Exception;
}; // class SocketException

//----------------------------------------------------------------------------
//
// Class-
//       Socket
//
// Purpose-
//       Standard posix socket wrapper.
//
//----------------------------------------------------------------------------
class Socket : public Object {      // Standard posix socket wrapper
friend class SocketSelect;

//----------------------------------------------------------------------------
// Socket::Attributes
//----------------------------------------------------------------------------
public:
static const int       CLOSED= -1;  // Closed socket handle
typedef in_port_t      Port;        // A port type

union sockaddr_u {                  // Aligned union: sockaddr_in, sockaddr_in6
uint64_t               su_align[4]; // Alignment and maximum size (32)
sa_family_t            su_family;   // Socket address family
sockaddr_in            su_in;       // IPv4 internet address
sockaddr_in6           su_in6;      // IPv6 internet address

std::string to_string( void ) const; // Convert to display string
}; // union sockaddr_u

protected:
SocketSelect*          selector= nullptr; // The associated SocketSelector
int                    handle= CLOSED; // The socket handle (handle)
short                  family= 0;   // The connection address family
short                  type= 0;     // The connection type
//                     proto= PF_UNSPEC; // The connection protocol

sockaddr_u             host_addr= {};   // The host socket address
sockaddr_u             peer_addr= {};   // The peer socket address
socklen_t              host_size= 0;   // Length of host_addr
socklen_t              peer_size= 0;   // Length of peer_addr

int                    recv_timeo= -1; // Receive timeout
int                    send_timeo= -1; // Send timeout

//----------------------------------------------------------------------------
// Socket::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
virtual
   ~Socket( void );                 // Destructor
   Socket( void );                  // Constructor

   Socket(const Socket&);           // Copy constructor
Socket& operator=(const Socket&);   // Assignment operator

//----------------------------------------------------------------------------
// Socket::debugging
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= "") const; // Debugging info

protected:
_LIBPUB_PRINTF(3,4)
void
   trace(                           // Trace socket operation
     int               line,        // For this source code line
     const char*       fmt,         // Format string
                       ...) const;  // The PRINTF argument list

//----------------------------------------------------------------------------
// Socket::Accessors
//----------------------------------------------------------------------------
public:
int                                 // The socket flags
   get_flags( void ) const          // Get socket flags
{  return ::fcntl(handle, F_GETFL); }

int                                 // The socket handle
   get_handle( void ) const         // Get socket handle
{  return handle; }

const sockaddr_u&                   // The host internet address
   get_host_addr( void ) const      // Get host internet address
{  return host_addr; }

static std::string                  // The host name
   get_host_name( void );           // Get host name

socklen_t                           // The host internet address length
   get_host_size( void ) const      // Get host internet address length
{  return host_size; }

Port                                // The host Port number
   get_host_port( void ) const      // Get host Port number
{  return ntohs(((sockaddr_in*)&host_addr)->sin_port); }

int                                 // Return code
   get_option(                      // Get socket option (::getsockopt)
     int               level,       // Level
     int               optname,     // Name
     void*             optval,      // Option value
     socklen_t*        optlen);     // Option length (IN/OUT)

const sockaddr_u&                   // The peer internet address
   get_peer_addr( void ) const      // Get peer internet address
{  return peer_addr; }

Port                                // The peer Port number
   get_peer_port( void ) const      // Get peer Port number
{  return ntohs(((sockaddr_in*)&peer_addr)->sin_port); }

socklen_t                           // The peer internet address length
   get_peer_size( void ) const      // Get peer internet address length
{  return peer_size; }

bool                                // TRUE iff socket is open
   is_open( void )
{  return handle >= 0; }

void
   set_flags(                       // Set socket flags
     int               flags)       // To this replacement value
{  ::fcntl(handle, F_SETFL, flags); }

int                                 // Return code
   set_option(                      // Set socket option (::setsockopt)
     int               level,       // Level
     int               optname,     // Name
     const void*       optval,      // Option value
     socklen_t         optlen);     // Option length

void
   set_peer_addr(                   // Set peer address
     const sockaddr*   peeraddr,    // Peer address
     socklen_t         peersize)    // Peer address length
{  memcpy(&peer_addr, peeraddr, peersize); peer_size= peersize; }

int                                 // Return code, 0 OK
   set_peer_addr(                   // Set peer address (See set_host_addr)
     std::string       name_port);  // "name:port" string

void
   set_peer_port(                   // Set peer Port number to
     Port              port)        // This port value
{  ((sockaddr_in*)&peer_addr)->sin_port= htons(port); }

//----------------------------------------------------------------------------
// Socket::Methods
//----------------------------------------------------------------------------
virtual Socket*                     // The next new connection
   accept( void );                  // Accept new connections

virtual int                         // Return code (0 OK)
   bind(                            // Bind to address
     const sockaddr*   host_addr,   // Host address
     socklen_t         host_size);  // Host address length

int                                 // Return code (0 OK)
   bind(                            // Bind to address
     const std::string&host)        // Host name:port string
{
   int rc= name_to_addr(host, &host_addr, &host_size);
   if( rc )
     return rc;

   return bind((sockaddr*)&host_addr, host_size);
}

int                                 // Return code (0 OK)
   bind(                            // Bind this socket
     Port              port)        // To this Port
{
   std::string nps= get_host_name() + ":" + std::to_string(port);
   return bind(nps);
}

virtual int                         // Return code (0 OK)
   close( void );                   // Close the socket

virtual int                         // Return code (0 OK)
   connect(                         // Connect to server
     const sockaddr*   peer_addr,   // Peer address
     socklen_t         peer_size);  // Peer address length

int                                 // Return code (0 OK)
   connect(                         // Connect to server
     const std::string&peer)        // Peer name:port string
{
   int rc= name_to_addr(peer, &peer_addr, &peer_size);
   if( rc )
     return rc;

   return connect((sockaddr*)&peer_addr, peer_size);
}

virtual int                         // Return code, 0 expected
   listen( void );                  // Set Socket to listener (server)

virtual int                         // Return code (0 OK)
   open(                            // Open the Socket
     int               family,      // Address Family
     int               type,        // Socket type
     int               protocol= 0); // Socket protocol

// Return code: <0 if error; 0 if timeout; >0 if fds->revents valid
virtual int                         // Return code
   poll(                            // Poll this Socket
     struct pollfd*    fds,         // IN/OUT The (system-defined) pollfd
     int               timeout)     // Timeout (in milliseconds)
{
   if( fds->fd != handle )
     fds->fd= handle;

   return ::poll(fds, 1, timeout);
}

#ifdef _GNU_SOURCE                  // (Only available with _GNU_SOURCE)
virtual int                         // Return code (0 OK)
   ppoll(                           // Poll this Socket
     struct pollfd*    fds,         // The (system) pollfd
     const struct timespec*
                       timeout,     // Timeout
     const sigset_t*   sigmask)     // Signal set mask
{
   if( fds->fd != handle )
     fds->fd= handle;

   return ::ppoll(fds, 1, timeout, sigmask);
}
#endif

virtual ssize_t                     // The number of bytes read
   read(                            // Read from the socket
     void*             addr,        // Data address
     size_t            size);       // Maximum data length

virtual ssize_t                     // The number of bytes read
   recv(                            // Recieve socket data
     void*             addr,        // Data address
     size_t            size,        // Maximum data length
     int               flag);       // Receive options

virtual ssize_t                     // The number of bytes written
   send(                            // Write to the socket
     const void*       addr,        // Data address
     size_t            size,        // Data length
     int               flag);       // Send options

virtual int                         // Return code (0 OK)
   shutdown(                        // Shutdown the socket
     int               how);        // Shutdown control flags

virtual ssize_t                     // The number of bytes written
   write(                           // Write to the socket
     const void*       addr,        // Data address
     size_t            size);       // Data length

protected:                          // (Currently) internal use only
/**
   @brief Set a socket address from a "name:port" string
   @param nps The "name:port" string, which can also be specified as
          ":port" to use get_host_name() as the host name.
   @return 0 If successful,
          -1 if error with errno set,
          >0 if ::getaddrinfo failed. (See socket getaddrinfo return codes.)

   Lookup fails if the address family does not match the socket address family
   specfied in open.

   Error errno values:
     EINVAL The nps string is missing the ':' delimiter
**/
int                                 // Return code, 0 OK
   name_to_addr(                    // Convert "name:port" to socket address
     const std::string nps,         // The "name:port" name string
     sockaddr_u*       addr,        // OUT: The sockaddr_u
     socklen_t*        size);       // OUT: Length of sockaddr_u
}; // class Socket

//----------------------------------------------------------------------------
//
// Class-
//       SSL_Socket
//
// Purpose-
//       SSL Socket wrapper.
//
//----------------------------------------------------------------------------
class SSL_Socket : public Socket {  // SSL Socket wrapper
//----------------------------------------------------------------------------
// SSL_Socket::Attributes
//----------------------------------------------------------------------------
protected:
SSL_CTX*               ssl_ctx;     // The associated SSL Context
SSL*                   ssl;         // The associated SSL State

//----------------------------------------------------------------------------
// SSL_Socket::Destructor/Constructor/Assignment
//----------------------------------------------------------------------------
public:
virtual
   ~SSL_Socket( void );             // Destructor
   SSL_Socket(                      // Constructor
     SSL_CTX*          context);    // The associated SSL Context

   SSL_Socket(const SSL_Socket&);   // Copy constructor
SSL_Socket& operator=(const SSL_Socket&); // Assignment operator

//----------------------------------------------------------------------------
// SSL_Socket::debugging
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info) const; // Debugging info

protected:
_LIBPUB_PRINTF(3,4)
void
   trace(                           // Trace socket operation
     int               line,        // For this source code line
     const char*       fmt,         // Format string
                       ...) const;  // The PRINTF argument list

//----------------------------------------------------------------------------
// SSL_Socket::Methods
//----------------------------------------------------------------------------
public:
virtual Socket*                     // The next new connection
   accept( void );                  // Accept connection

virtual int                         // Return code (0 OK)
   connect(                         // Connect to server
     const sockaddr*   peer_addr,   // Peer address
     socklen_t         peer_size);  // Peer address length

int                                 // Return code (0 OK)
   connect(                         // Connect to server
     const std::string&name_port);  // Peer name:port string

virtual ssize_t                     // The number of bytes read
   read(                            // Read from the socket
     void*             addr,        // Data address
     size_t            size);       // Maximum data length

virtual ssize_t                     // The number of bytes written
   write(                           // Write to the socket
     const void*       addr,        // Data address
     size_t            size);       // Data length
}; // class SSL_Socket

//----------------------------------------------------------------------------
//
// Class-
//       SocketSelect
//
// Purpose-
//       Socket selector
//
// Implementation notes-
//       ** THREAD SAFE, BUT NOT CURRENTLY MULTI-THREAD CAPABLE **
//         Polling operations can hold the SocketSelector mutex for long
//         intervals, blocking socket inserts, modification, and removal.
//
//       Sockets may only be associated with one SocketSelector object.
//       Sockets are automatically removed from a SocketSelector whenever they
//       are opened, closed, or deleted.
//
//       The SocketSelector is intended for use with a large number of sockets.
//       It contains element arrays indexed by the file descriptor, each
//       allocated large enough to contain *all* file descriptors.
//
//----------------------------------------------------------------------------
class SocketSelect {                // Socket selector
public:
typedef std::function<void(void)>             v_func;

protected:
mutable std::mutex     mutex;       // Internal mutex
struct pollfd*         pollfd= nullptr; // Array of pollfd's
Socket**               socket= nullptr; // Array of Socket's
int*                   sindex= nullptr; // File descriptor to pollfd index

int                    left= 0;     // Number of remaining selections
int                    next= 0;     // The next selection index
int                    size= 0;     // Number of result elements available
int                    used= 0;     // Number of result elements used

public:
   SocketSelect();
   ~SocketSelect();

//----------------------------------------------------------------------------
// SocketSelect::methods
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= "");   // Caller information

void
   with_lock(v_func f)              // Run function holding SocketSelect mutex
{  std::lock_guard<decltype(mutex)> lock(mutex); f(); }

const struct pollfd*                // The associated pollfd
   get_pollfd(                      // Extract pollfd
     const Socket*     socket) const // For this Socket
{  std::lock_guard<decltype(mutex)> lock(mutex);

   int fd= socket->handle;
   if( fd < 0 || fd >= size ) {     // (Not valid, most likely closed)
     errno= EBADF;
     return nullptr;
   }

   fd= sindex[fd];
   if( fd < 0 || fd >= size ) {     // (Not mapped, most likely user error)
     errno= EINVAL;
     return nullptr;
   }

   return &pollfd[fd];
}

const Socket*                       // The associated Socket*
   get_socket(                      // Extract Socket
     int               fd) const    // For this file descriptor
{  std::lock_guard<decltype(mutex)> lock(mutex);

   if( fd < 0 || fd >= size ) {
     errno= EINVAL;
     return nullptr;
   }

   return socket[fd];
}

int                                 // Return code, 0 expected
   insert(                          // Insert a Socket onto the list
     Socket*           socket,      // The associated Socket
     int               events);     // The associated poll events

int                                 // Return code, 0 expected
   modify(                          // Replace the Socket's poll events mask
     const Socket*     socket,      // The associated Socket
     int               events);     // The associated poll events

int                                 // Return code, 0 expected
   remove(                          // Remove the Selector
     Socket*           socket);     // For this Socket

Socket*                             // The next selected Socket, or nullptr
   select(                          // Select next Socket
     int               timeout);    // Timeout, in milliseconds

Socket*                             // The next selected Socket, or nullptr
   select(                          // Select next Socket
     const struct timespec*         // (tv_sec, tv_nsec)
                       timeout,     // Timeout, infinite if omitted
     const sigset_t*   signals);    // Timeout, in milliseconds

//============================================================================
protected:
Socket*                             // The next selected Socket
   remain( void );                  // Select next remaining Socket
}; // class SocketSelect
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_SOCKET_H_INCLUDED
