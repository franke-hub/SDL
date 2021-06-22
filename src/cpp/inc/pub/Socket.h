//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2021 Frank Eskesen.
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
//       2021/06/10
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
#ifndef _PUB_SOCKET_H_INCLUDED
#define _PUB_SOCKET_H_INCLUDED

#include <string>                   // For std::string
#include <netinet/in.h>             // For struct sockaddr_ definitions
#include <openssl/ssl.h>            // For SSL, SSL_CTX
#include <sys/socket.h>             // For socket methods

#include "pub/Exception.h"          // For SocketException
#include "pub/Object.h"             // For base class, _PUB_NAMESPACE, ...

namespace _PUB_NAMESPACE {
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
typedef int            Port;        // A port type

protected:
int                    handle;      // The socket handle

sockaddr_storage       host_addr;   // The host socket address
sockaddr_storage       peer_addr;   // The peer socket address
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
// Socket::Accessors
//----------------------------------------------------------------------------
public:
int                                 // The socket handle
   get_handle( void ) const         // Get socket handle
{  return handle; }

Port                                // The host Port number
   get_host_port( void ) const      // Get host Port number
{  return ntohs(((sockaddr_in*)&host_addr)->sin_port); }

int                                 // Return code
   get_option(                      // Get socket option (::getsockopt)
     int               level,       // Level
     int               optname,     // Name
     void*             optval,      // Option value
     socklen_t*        optlen);     // Option length (IN/OUT)

Port                                // The peer Port number
   get_peer_port( void ) const      // Get peer Port number
{  return ntohs(((sockaddr_in*)&peer_addr)->sin_port); }

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
public:
virtual int                         // Return code (0 OK)
   bind(                            // Bind this socket
     Port              port);       // To this Port

virtual int                         // Return code (0 OK)
   close( void );                   // Close the socket

virtual int                         // Return code (0 OK)
   connect(                         // Connect to server
     sockaddr*         peer_addr,   // Peer address
     socklen_t         peer_size);  // Peer address length

virtual Socket*                     // The next new connection
   listen( void );                  // Listen for and accept new connections

virtual int                         // Return code (0 OK)
   open(                            // Open the Socket
     int               family,      // Address Family
     int               type,        // Socket type
     int               protocol);   // Socket protocol

virtual int                         // The number of bytes read
   read(                            // Read from the socket
     void*             addr,        // Data address
     int               size);       // Maximum data length

virtual int                         // The number of bytes written
   write(                           // Write to the socket
     const void*       addr,        // Data address
     int               size);       // Data length
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
// SSL_Socket::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 OK)
   connect(                         // Connect to server
     sockaddr*         peer_addr,   // Peer address
     socklen_t         peer_size);  // Peer address length

virtual Socket*                     // The next new connection
   listen( void );                  // Listen for new connections

virtual int                         // The number of bytes read
   read(                            // Read from the socket
     void*             addr,        // Data address
     int               size);       // Maximum data length

virtual int                         // The number of bytes written
   write(                           // Write to the socket
     const void*       addr,        // Data address
     int               size);       // Data length
}; // class SSL_Socket
}  // namespace _PUB_NAMESPACE
#endif // _PUB_SOCKET_H_INCLUDED
