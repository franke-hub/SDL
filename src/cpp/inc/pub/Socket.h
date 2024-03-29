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
//       Socket.h
//
// Purpose-
//       Standard socket (including openssl sockets) wrapper.
//
// Last change date-
//       2022/12/06
//
// Implementation notes-
//       Error recovery is the user's responsibility.
//
//       SocketException is only thrown for usage errors and SHOULD NOT OCCUR
//       conditions. Recoverable SNO conditions result in an error message
//       which, unless described as a user error, should be reported.
//
//       Methods get_host_port, get_peer_port, set_host_port, and set_peer_port
//       apply only to family AF_INET and AF_INET6 sockets.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_SOCKET_H_INCLUDED
#define _LIBPUB_SOCKET_H_INCLUDED

#include <atomic>                   // For std::atomic
#include <functional>               // For std::function
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string
#include <fcntl.h>                  // For fcntl
#include <unistd.h>                 // For gethostname
#include <netinet/in.h>             // For struct sockaddr_ definitions
#include <openssl/ssl.h>            // For SSL, SSL_CTX
#include <sys/poll.h>               // For struct pollfd, ...
#include <sys/socket.h>             // For socket methods

#include <pub/Exception.h>          // For pub::Exception
#include <pub/Object.h>             // For base class, ...

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Select;                       // Socket select controller

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
//       Standard socket wrapper.
//
// Implementation notes-
//       Standard socket wrapper.
//
//----------------------------------------------------------------------------
class Socket : public Object {      // Standard posix socket wrapper
friend class Select;

//----------------------------------------------------------------------------
// Socket::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
static const int       CLOSED= -1;  // Closed socket handle
typedef in_port_t      Port;        // A port type
typedef std::function<void(int)>              f_select;

//----------------------------------------------------------------------------
// Socket::sockaddr_u, Socket::sockaddr_x
//----------------------------------------------------------------------------
// Extended sockaddr, currently only used for AF_UNIX
struct sockaddr_x {                 // Extended sockaddr
sa_family_t            x_family;    // Socket address family
uint16_t               _0002[10];   // (Unused)
uint16_t               x_socksize;  // Length (x_sockaddr)
sockaddr*              x_sockaddr;  // Sockaddr copy (Possibly this)
}; // struct sockaddr_x

union sockaddr_u {                  // Aligned multi-family union
uint64_t               su_align[4]; // Alignment and maximum size (32)
sa_family_t            su_af;       // Socket address family
sockaddr               sa;          // Generic socket address
sockaddr_in            su_i4;       // IPv4 internet address
sockaddr_in6           su_i6;       // IPv6 internet address
sockaddr_x             su_x;        // Extended sockaddr format

inline
   sockaddr_u( void )               // Default constructor
{  su_align[0]= su_align[1]= su_align[2]= su_align[3]= 0; }

inline
   sockaddr_u(const sockaddr_u& src) // Copy constructor
{
   su_align[0]= su_align[1]= su_align[2]= su_align[3]= 0;
   operator=(src);
}

inline
   ~sockaddr_u( void )              // Destructor
{  reset(); }

sockaddr_u&
   operator=(const sockaddr_u&);    // Assignment operator

void
   copy(const sockaddr*, socklen_t); // Replacement copy from sockaddr
void
   copy(                            // Replacement copy
     const sockaddr_storage* addr,  // From sockaddr_storage
     socklen_t         size)
{  copy((const sockaddr*)addr, size); }

void
   reset( void );                   // Reset the sockaddr_u, zeroing it

std::string                         // The display string
   to_string( void ) const;         // Get display string
}; // union sockaddr_u

//----------------------------------------------------------------------------
// Socket::Attributes
//----------------------------------------------------------------------------
protected:
std::mutex             mutex;       // Open/close mutex
std::atomic<Select*>   select= nullptr; // The associated Select
f_select               h_select;    // Selection handler

int                    handle= CLOSED; // The socket handle (handle)
short                  family= 0;   // The connection address family
short                  type= 0;     // The connection type
//                     proto= PF_UNSPEC; // The connection protocol

sockaddr_u             host_addr= {};   // The host socket address
sockaddr_u             peer_addr= {};   // The peer socket address
socklen_t              host_size= 0;   // Length of host_addr
socklen_t              peer_size= 0;   // Length of peer_addr

//----------------------------------------------------------------------------
// Socket::Constructors/Destructor/Assignment
//----------------------------------------------------------------------------
void                                // (Protected)
   copy(                            // Copy host_addr/size and peer_addr/size
     const Socket&     source);     // From this Socket

public:
   Socket( void );                  // Default constructor
   Socket(const Socket&);           // Copy constructor

virtual
   ~Socket( void );                 // Destructor

Socket& operator=(const Socket&);   // Assignment operator

//----------------------------------------------------------------------------
// Socket::debugging
//----------------------------------------------------------------------------
virtual void
   debug(                           // Debugging display
     const char*       info= "") const; // Debugging info

protected:
_LIBPUB_PRINTF(2,3)
static void
   trace(                           // Trace socket operation
     int               line,        // For this source code line
     const char*       fmt,         // Format string
                       ...);        // The PRINTF argument list

//----------------------------------------------------------------------------
// Socket::Accessors
//----------------------------------------------------------------------------
public:
/*****************************************************************************
  @brief Drive the selection handler

  @param revent: The polling revent
*****************************************************************************/
void
   do_select(                       // Drive the event handler
     int               revent)      // The polling revent
{  h_select(revent); }

int                                 // The socket flags
   get_flags( void ) const          // Get socket flags
{  return ::fcntl(handle, F_GETFL); }

int                                 // The socket handle
   get_handle( void ) const         // Get socket handle
{  return handle; }

static std::string                  // The host name
   gethostname( void )              // Get host name
{
   char host_name[256];             // The host name
   host_name[0]= '\0';              // (If error, return "")
   ::gethostname(host_name, 256);
   return host_name;
}

const sockaddr_u&                   // The host internet address
   get_host_addr( void ) const      // Get host internet address
{  return host_addr; }

Port                                // The host Port number
   get_host_port( void ) const      // Get host Port number
{
   if( host_addr.su_af != AF_INET && host_addr.su_af != AF_INET6 )
     return -1;

   return ntohs(host_addr.su_i4.sin_port);
}

socklen_t                           // The host internet address length
   get_host_size( void ) const      // Get host internet address length
{  return host_size; }

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
{
   if( peer_addr.su_af != AF_INET && peer_addr.su_af != AF_INET6 )
     return -1;

   return ntohs(peer_addr.su_i4.sin_port);
}

socklen_t                           // The peer internet address length
   get_peer_size( void ) const      // Get peer internet address length
{  return peer_size; }

Select*                             // The Select
   get_select( void )               // Get Select
{  return select.load(); }

const char*                         // The unix socket file name
   get_unix_name( void ) const;     // Get unix socket file name

bool                                // TRUE iff socket is open
   is_open( void ) const
{  return handle >= 0; }

virtual bool
   is_ssl( void ) const             // Is this an SSL socket?
{  return false; }

static bool                         // TRUE iff socket family is supported
   is_valid(sa_family_t sf)
{  return sf == AF_INET || sf == AF_INET6 || sf == AF_UNIX; }


/*****************************************************************************
  @brief Define the Socket polling selection handler.

  @param f: The lambda std::function<void(int)> function

  The lambda function is driven asynchronously (normally by Select) when
  a polling event indicates that work can be processed.

  Ususally only one thread drives polling for multiple sockets.
  In order for this thread to avoid a processing backlog, this function should
  run quickly. It may be necessary to defer processing (e.g by scheduling work)
  rather than immediately processing the polling event.
*****************************************************************************/
void
   on_select(f_select f)            // Define the event handler
{  h_select= f; }

int
   set_flags(                       // Set socket flags
     int               flags)       // To this replacement value
{  return ::fcntl(handle, F_SETFL, flags); }

void
   set_host_port(                   // Set host Port number to
     Port              port)        // This port value
{
   if( host_addr.su_af == AF_INET || host_addr.su_af == AF_INET6 )
     host_addr.su_i4.sin_port= htons(port);
}

int                                 // Return code
   set_option(                      // Set socket option (::setsockopt)
     int               level,       // Level
     int               optname,     // Name
     const void*       optval,      // Option value
     socklen_t         optlen);     // Option length

void
   set_peer_addr(                   // Set peer address
     const sockaddr*   peeraddr,    // Peer address
     socklen_t         peersize);   // Peer address length

int                                 // Return code, 0 OK
   set_peer_addr(                   // Set peer address
     std::string       nps);        // "name:port" string

void
   set_peer_port(                   // Set peer Port number to
     Port              port)        // This port value
{
   if( peer_addr.su_af == AF_INET || peer_addr.su_af == AF_INET6 )
     peer_addr.su_i4.sin_port= htons(port);
}

//----------------------------------------------------------------------------
// Socket::Methods
//----------------------------------------------------------------------------
virtual Socket*                     // The next new connection
   accept( void );                  // Accept new connections

int                                 // Return code (0 OK)
   bind(                            // Bind to address
     const sockaddr*   host_addr,   // Host address
     socklen_t         host_size);  // Host address length

int                                 // Return code (0 OK)
   bind(                            // Bind to address
     const std::string&nps);        // Host name:port string

int                                 // Return code (0 OK)
   bind(                            // Bind this socket
     Port              port)        // To this Port
{  return bind(gethostname() + ":" + std::to_string(port)); }

int                                 // Return code (0 OK)
   close( void );                   // Close the socket

virtual int                         // Return code (0 OK)
   connect(                         // Connect to server
     const sockaddr*   peeraddr,    // Peer address
     socklen_t         peersize);   // Peer address length

int                                 // Return code (0 OK)
   connect(                         // Connect to server
     const std::string&nps);        // Peer name:port string

int                                 // Return code, 0 expected
   listen( void );                  // Set Socket to listener (server)

/**
   @brief Set a socket address from a "name:port" string
   @param nps The "name:port" string.
       AF_INET and AF_INET6 family names MUST contain the ':' delimiter, and
       may be specified as ":port" to use gethostname() as the host.
       If the family name is defaulted, AF_UNIX family names MUST NOT
       contain the ':' delimiter.

   @param addr OUT: The resultant sockaddr
   @param size OUT: The resultant sockaddr length
   @param family: The preferred address family.

   @return 0 If successful,
          -1 if error with errno set,
          >0 if ::getaddrinfo failed. (See socket getaddrinfo return codes.)

   Error errno values:
     EINVAL The nps string is missing the ':' delimiter
**/
static int                          // Return code, 0 OK
   name_to_addr(                    // Convert "name:port" to socket address
     const std::string&nps,         // The "name:port" name string
     sockaddr*         addr,        // OUT: The sockaddr
     socklen_t*        size,        // INP/OUT: Length of sockaddr
     int               family= 0);  // The preferred address family

int                                 // Return code (0 OK)
   open(                            // Open the Socket
     int               family,      // Address Family
     int               type,        // Socket type
     int               protocol= 0); // Socket protocol

int                                 // Return code
   poll(                            // Poll this Socket
     struct pollfd*    pfd,         // IN/OUT The (system-defined) pollfd
     int               timeout);    // Timeout (in milliseconds)

int                                 // Return code (0 OK)
   ppoll(                           // Poll this Socket
     struct pollfd*    pfd,         // IN/OUT The (system-defined) pollfd
     const struct timespec*
                       timeout,     // Timeout
     const sigset_t*   sigmask);    // Signal set mask

virtual ssize_t                     // The number of bytes read
   read(                            // Read from the peer socket
     void*             addr,        // Data address
     size_t            size);       // Maximum data length

ssize_t                             // The number of bytes read
   recv(                            // Receive from the peer socket
     void*             addr,        // Data address
     size_t            size,        // Maximum data length
     int               flag);       // Receive options

ssize_t                             // The number of bytes read
   recvfrom(                        // Read from some socket
     void*             addr,        // Data address
     size_t            size,        // Data length
     int               flag,        // Send options
     sockaddr*         peeraddr,    // Source peer address
     socklen_t*        peersize);   // Source peer address length

ssize_t                             // The number of bytes written
   recvmsg(                         // Receive message from some socket
     msghdr*           msg,         // Message header
     int               flag);       // Recv options

ssize_t                             // The number of bytes written
   send(                            // Write to the peer socket
     const void*       addr,        // Data address
     size_t            size,        // Data length
     int               flag);       // Send options

ssize_t                             // The number of bytes written
   sendmsg(                         // Write to some socket
     const msghdr*     msg,         // Message header
     int               flag);       // Send options

ssize_t                             // The number of bytes written
   sendto(                          // Write to some socket
     const void*       addr,        // Data address
     size_t            size,        // Data length
     int               flag,        // Send options
     const sockaddr*   peeraddr,    // Target peer address
     socklen_t         peersize);   // Target peer address length

ssize_t                             // The number of bytes written
   sendto(                          // Write to the peer socket
     const void*       addr,        // Data address
     size_t            size,        // Data length
     int               flag)        // Send options
{  return sendto(addr, size, flag, (sockaddr*)&peer_addr, peer_size); }

int                                 // Return code (0 OK)
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
//       SSL_socket
//
// Purpose-
//       SSL Socket wrapper.
//
// Implementation notes-
//       Note: send and recv are not supported. When using common code for
//       Socket and SSL_socket I/O use read and write operations instead.
//       Note that error recovery may require an is_ssl() test.
//
//       We may want to make the Socket send and receive function virtual and
//       implement them here, throwing exceptions if invoked.
//
//----------------------------------------------------------------------------
class SSL_socket : public Socket {  // SSL Socket wrapper
//----------------------------------------------------------------------------
// SSL_socket::Attributes
//----------------------------------------------------------------------------
protected:
SSL_CTX*               ssl_ctx;     // The associated SSL Context
SSL*                   ssl;         // The associated SSL State

//----------------------------------------------------------------------------
// SSL_socket::Constructors/Destructor/Assignment
//----------------------------------------------------------------------------
public:
   SSL_socket(                      // Constructor
     SSL_CTX*          context);    // The associated SSL Context
   SSL_socket(const SSL_socket&);   // Copy constructor

virtual
   ~SSL_socket( void );             // Destructor

SSL_socket& operator=(const SSL_socket&); // Assignment operator

//----------------------------------------------------------------------------
// SSL_socket::debugging
//----------------------------------------------------------------------------
virtual void
   debug(                           // Debugging display
     const char*       info) const; // Debugging info

protected:
_LIBPUB_PRINTF(3,4)
virtual void
   trace(                           // Trace socket operation
     int               line,        // For this source code line
     const char*       fmt,         // Format string
                       ...) const;  // The PRINTF argument list

//----------------------------------------------------------------------------
// SSL_socket::Accessors
//----------------------------------------------------------------------------
public:
virtual bool
   is_ssl( void ) const             // Is this an SSL socket?
{  return true; }

//----------------------------------------------------------------------------
// SSL_socket::Methods
//----------------------------------------------------------------------------
virtual Socket*                     // The next new connection
   accept( void );                  // Accept connection

virtual int                         // Return code (0 OK)
   connect(                         // Connect to server
     const sockaddr*   peer_addr,   // Peer address
     socklen_t         peer_size);  // Peer address length

int                                 // Return code (0 OK)
   connect(                         // Connect to server
     const std::string&nps)         // Peer "name:port" string
{  return Socket::connect(nps); }   // Invokes connect(const sockaddr*,socklen)

virtual ssize_t                     // The number of bytes read
   read(                            // Read from the socket
     void*             addr,        // Data address
     size_t            size);       // Maximum data length

virtual ssize_t                     // The number of bytes written
   write(                           // Write to the socket
     const void*       addr,        // Data address
     size_t            size);       // Data length
}; // class SSL_socket
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_SOCKET_H_INCLUDED
