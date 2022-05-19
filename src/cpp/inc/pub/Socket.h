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
//       2022/05/15
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
//       Library error handling is limited, and mostly left up to the user.
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
#include <netinet/in.h>             // For struct sockaddr_ definitions
#include <openssl/ssl.h>            // For SSL, SSL_CTX
#include <sys/poll.h>               // For struct pollfd, ...
#include <sys/socket.h>             // For socket methods

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros
#include "pub/Exception.h"          // For SocketException
#include "pub/Object.h"             // For base class, _PUB_NAMESPACE, ...

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
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
int                    handle;      // The socket handle
short                  family;      // The connection address family
short                  type;        // The connection type
//                     proto;       // The connection protocol, PF_UNSPEC

sockaddr_u             host_addr;   // The host socket address
sockaddr_u             peer_addr;   // The peer socket address
socklen_t              host_size;   // Length of host_addr
socklen_t              peer_size;   // Length of peer_addr

int                    recv_timeo;  // Receive timeout
int                    send_timeo;  // Send timeout

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

void
   set_host_port(                   // Set host Port number to
     Port              port)        // This port value
{  ((sockaddr_in*)&host_addr)->sin_port= htons(port); }

int                                 // Return code
   set_option(                      // Set socket option (::setsockopt)
     int               level,       // Level
     int               optname,     // Name
     const void*       optval,      // Option value
     socklen_t         optlen);     // Option length

void
   set_peer_port(                   // Set peer Port number to
     Port              port)        // This port value
{  ((sockaddr_in*)&peer_addr)->sin_port= htons(port); }

//----------------------------------------------------------------------------
// Socket::Methods
//----------------------------------------------------------------------------
virtual int                         // Return code (0 OK)
   bind(                            // Bind this socket
     Port              port);       // To this Port

virtual int                         // Return code (0 OK)
   close( void );                   // Close the socket

virtual int                         // Return code (0 OK)
   connect(                         // Connect to server
     const sockaddr*   peer_addr,   // Peer address
     socklen_t         peer_size);  // Peer address length

virtual int                         // Return code (0 OK)
   connect(                         // Connect to server
     const std::string&name_port);  // Peer name:port string

virtual Socket*                     // The next new connection
   listen( void );                  // Listen for and accept new connections

virtual int                         // Return code (0 OK)
   open(                            // Open the Socket
     int               family,      // Address Family
     int               type,        // Socket type
     int               protocol= 0); // Socket protocol

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
virtual int                         // Return code (0 OK)
   connect(                         // Connect to server
     const sockaddr*   peer_addr,   // Peer address
     socklen_t         peer_size);  // Peer address length

virtual Socket*                     // The next new connection
   listen( void );                  // Listen for new connections

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
//       ** NOT THREAD SAFE **
//         Similar operation to epoll, but a SocketSelect object cannot be
//         shared among threads. A mutex is used to enforce this restriction.
//
//       It it is the user's reponsibility to ensure that inserted Sockets
//       remain consistent: They must not be deleted, opened or closed.
//       Sockets should be only be inserted in one SocketSelect.
//       Unpredictable results enforce these usage requirements.
//
//----------------------------------------------------------------------------
class SocketSelect {                // Socket selector
public:
typedef std::function<void(void)>             v_func;

enum { SIZE_INC= 32 };              // Array increment size TODO: MOVE TO .cpp

struct Selector {
const Socket*          socket;      // The associated Socket
}; // struct Selector

protected:
mutable std::mutex     mutex;       // Internal mutex
Selector*              sarray= nullptr; // Array of Selectors
struct pollfd*         result= nullptr; // Array of pollfd's

uint32_t               left= 0;     // Number of remaining selections
uint32_t               next= 0;     // The next selection index
uint32_t               size= 0;     // Array size(s)
uint32_t               used= 0;     // Number of element used

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

   ssize_t i= locate(socket);
   if( i < 0 ) {
     errno= EINVAL;
     return nullptr;
   }

   return &result[i];
}

int                                 // Return code, 0 expected
   insert(                          // Insert a Socket onto the list
     const Socket*     socket,      // The associated Socket
     int               events);     // The associated poll events

int                                 // Return code, 0 expected
   modify(                          // Replace the Socket's poll events mask
     const Socket*     socket,      // The associated Socket
     int               events);     // The associated poll events

int                                 // Return code, 0 expected
   remove(                          // Remove the Selector
     const Socket*     socket);     // For this Socket

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
ssize_t                             // The Selector index, -1 if not found
   locate(                          // Locate the Selector index
     const Socket*     socket) const; // For this Socket

Socket*                             // The next selected Socket
   remain( void );                  // Select next remaining Socket
}; // class SocketSelect
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_SOCKET_H_INCLUDED
