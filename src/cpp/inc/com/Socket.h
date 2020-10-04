//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
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
//       Socket descriptor.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#ifndef SOCKET_H_INCLUDED
#define SOCKET_H_INCLUDED

#include <stdint.h>

#ifndef DEFINE_H_INCLUDED
#include "define.h"
#endif

#ifndef MEDIA_H_INCLUDED
#include "Media.h"                  // For MediaType
#endif

//----------------------------------------------------------------------------
//
// Class-
//       SocketType
//
// Purpose-
//       Define the types used by Socket objects.
//
// Notes-
//       The number of bits used in Addr and Port are connection dependent.
//
//----------------------------------------------------------------------------
class SocketType : public MediaType { // Socket types
//----------------------------------------------------------------------------
// Socket::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef uint64_t       Addr;        // Internet address
typedef const char*    Name;        // Internet name (string)
typedef uint32_t       Port;        // Port number
typedef int            SocketEC;    // Socket error code (Software::SystemEC)

enum SocketST                       // Socket type
{  ST_UNSPEC=          0            // Unspecified
,  ST_STREAM=          1            // Stream socket
,  ST_DGRAM=           2            // Datagram socket

,  ST_MAX=             8
}; // enum SocketST

enum SocketMO                       // Message option
{  MO_UNSPEC=          0x0000       // Unspecified
,  MO_OOB=             0x0001       // Process out-of-band data
,  MO_PEEK=            0x0002       // Peek at incoming message
,  MO_DONTROUTE=       0x0004       // Send without using routing tables
,  MO_EOR=             0x0008       // Data completes record
,  MO_TRUNC=           0x0010       // Data discarded before delivery
,  MO_CTRUNC=          0x0020       // Control data lost before delivery
,  MO_WAITALL=         0x0040       // Wait for full request or error
,  MO_MPEG2=           0x0080       // Message contain MPEG2 data
,  MO_NONBLOCK=        0x4000       // Nonblocking request
,  MO_COMPAT=          0x8000       // 4.3-format sockaddr
}; // enum SocketMO

//----------------------------------------------------------------------------
// *ALL* times are in milliseconds (Implementation may round up to second.)
enum SocketSO                       // Socket option
{  SO_UNSPEC=          0            // Unspecified
,  SO_TYPE                          // Get socket type (integer)
,  SO_ERROR                         // Get/clear error status (integer)
,  SO_DEBUG                         // Turn on debugging info recording (boolean)
,  SO_SNDBUF                        // Send buffer size (bytes)
,  SO_RCVBUF                        // Receive buffer size (bytes)
,  SO_SNDLOWAT                      // Send low-water mark (bytes)
,  SO_RCVLOWAT                      // Receive low-water mark (bytes)
,  SO_SNDTIMEO                      // Send timeout (milliseconds)
,  SO_RCVTIMEO                      // Receive timeout (milliseconds)
,  SO_LINGER                        // Linger time for close (seconds)
,  SO_KEEPALIVE                     // Keep connections alive (boolean)
,  SO_DONTROUTE                     // Just use interface addresses (boolean)
,  SO_BROADCAST                     // Permit sending of broadcast msgs (boolean)
,  SO_OOBINLINE                     // Leave received OOB data in line (boolean)
,  SO_REUSEADDR                     // Allow local address reuse (boolean)
,  SO_ACCEPTCONN                    // Socket has had listen() (boolean)

,  SO_MAX                           // Number of SocketOption values
}; // enum SocketSO
}; // class SocketType

//----------------------------------------------------------------------------
//
// Class-
//       Socket
//
// Purpose-
//       Socket descriptor.
//
//----------------------------------------------------------------------------
class Socket : public SocketType {  // Socket descriptor
friend class SockSelect;

//----------------------------------------------------------------------------
// Socket::Attributes
//----------------------------------------------------------------------------
protected:
int                    fsm;         // Finite State Machine
int                    handle;      // Socket handle
SocketST               st;          // Socket type
SocketEC               ec;          // Last error code

Size_t                 hSize;       // Host internet address size
Size_t                 pSize;       // Peer internet address size
char                   hInet[32];   // Host internet address
char                   pInet[32];   // Peer internet address
char                   hName[64];   // Host name (work/result area)

//----------------------------------------------------------------------------
// Socket::Constructors
//----------------------------------------------------------------------------
public:
   ~Socket( void );                 // Destructor

   Socket( void );                  // Default constructor

   Socket(                          // Constructor
     SocketST            st);       // Socket type

private:                            // Bitwise copy prohibited
   Socket(const Socket&);           // Copy constructor
Socket&
   operator=(const Socket&);        // Assignment operator

//----------------------------------------------------------------------------
// Socket::Static methods
//----------------------------------------------------------------------------
public:
static Addr                         // Host Addr, 0 if error
   getAddr(                         // Get host Addr
     int               alias= 0);   // For this alias

static Name                         // Host Name, NULL if error
   getName(                         // Get host Name
     int               alias= 0);   // For this alias

static const char*                  // Resultant string
   getSocketEI(                     // Convert SocketEC to string
     SocketEC          socketEC);   // The SocketEC

static const char*                  // String representaton of address
   addrToChar(                      // Convert address to string
     Addr              addr);       // The network address

static Name                         // Associated Name, NULL if error
   addrToName(                      // Convert address to name
     Addr              addr,        // The network address
     char*             result= NULL,// Resultant (NULL value not reentrant)
     unsigned          length= 0);  // Resultant length

static Addr                         // Addr, 0 if error
   nameToAddr(                      // Convert name to Address
     Name              name);       // The host name

//----------------------------------------------------------------------------
// Socket::Accessors
//----------------------------------------------------------------------------
public:
void
   debug( void ) const;             // Display debugging information

Addr                                // The host Addr
   getHostAddr( void ) const;       // Get host Addr

Name                                // The host network name
   getHostName( void );             // Get host network name

Port                                // The host Port
   getHostPort( void ) const;       // Get host Port

Addr                                // The peer Addr
   getPeerAddr( void ) const;       // Get the peer Addr

Name                                // The peer Name
   getPeerName( void );             // Get the peer Name

Port                                // The peer Port
   getPeerPort( void ) const;       // Get the peer Port

SocketEC                            // The SocketEC
   getSocketEC( void ) const;       // Get SocketEC

void
   setSocketEC(                     // Set SocketEC
     SocketEC          ec);         // The SocketEC

const char*                         // Get SocketEI (Converted StocketEC)
   getSocketEI( void ) const;       // Get SocketEI

int                                 // The SocketSO value
   getSocketSO(                     // Get SocketSO
     SocketSO          so);         // The SocketSO to get

int                                 // Return code, 0 OK
   setSocketSO(                     // Set a SocketSO
     SocketSO          so,          // The SocketSO to set
     int               value);      // The value to set it to

SocketST                            // The SocketST
   getSocketST( void ) const;       // Get SocketST

int                                 // TRUE iff open
   isOpen( void ) const;            // Get open state

//----------------------------------------------------------------------------
// Socket::Accessors (Connection socket)
//----------------------------------------------------------------------------
public:
int                                 // Return code (0 OK)
   setHost(                         // Set the host Addr and Port
     Addr              addr= 0,     // To this Addr and
     Port              port= 0);    // To this Port

int                                 // Return code (0 OK)
   setHostAddr(                     // Set the host Addr
     Addr              addr= 0);    // To this Addr

int                                 // Return code (0 OK)
   setHostPort(                     // Set the host Port
     Port              port= 0);    // Host port

//----------------------------------------------------------------------------
// Socket::Accessors (Datagram socket)
//----------------------------------------------------------------------------
public:
int                                 // Return code (0 OK)
   setPeer(                         // Set the peer Addr and Port
     Addr              addr,        // The peer Addr
     Port              port);       // The peer Port

int                                 // Return code (0 OK)
   setPeerAddr(                     // Set the peer Addr
     Addr              addr);       // The peer Addr

int                                 // Return code (0 OK)
   setPeerPort(                     // Set the peer Port
     Port              port);       // The peer Port

//----------------------------------------------------------------------------
// Socket::Methods
//----------------------------------------------------------------------------
public:
Size_t                              // Data length actually received
   recv(                            // Receive data
     Byte*             addr,        // Data address
     Size_t            size,        // Data length
     SocketMO          opts= MO_UNSPEC); // Options

Size_t                              // Data length actually sent
   send(                            // Send data
     const Byte*       addr,        // Data address
     Size_t            size,        // Data length
     SocketMO          opts= MO_UNSPEC); // Options

//----------------------------------------------------------------------------
// Socket::Methods
//----------------------------------------------------------------------------
public:
int                                 // Return code (0 OK)
   close( void );                   // Close the Socket

int                                 // Return code (0 OK)
   connect(                         // Connect to peer
     Addr              addr,        // Peer network address
     Port              port);       // Peer port

Socket*
   listen(                          // Wait for connection
     Port              port= 0);    // On this port

protected:
void
   logError(                        // Diagnostic error message
     int               line,        // Line number
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
   _ATTRIBUTE_PRINTF(3,4);

void
   logEvent(                        // Diagnostic event message
     int               line,        // Line number
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
   _ATTRIBUTE_PRINTF(3,4);

static void
   logOpError(                      // Diagnostic error message
     int               line,        // Line number
     const char*       op,          // Operation name
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
   _ATTRIBUTE_PRINTF(3,4);

int                                 // Return code (0 OK)
   setHandle( void );               // Set the handle

int                                 // Resultant
   verifyMO(                        // MO_ conversion
     SocketMO          value);      // MessageOption value

int                                 // Resultant (>0 OK)
   verifySO(                        // SO_ conversion
     SocketSO          so);         // SocketOption value
}; // class Socket

//----------------------------------------------------------------------------
//
// Class-
//       SockSelect
//
// Purpose-
//       The SockSelect is used to select a ready Socket from a list.
//
//----------------------------------------------------------------------------
class SockSelect {                  // SockSelect
//----------------------------------------------------------------------------
// SockSelect::Attributes
//----------------------------------------------------------------------------
protected:
   void*               object;      // Hidden attributes

//----------------------------------------------------------------------------
// SockSelect::Constructors
//----------------------------------------------------------------------------
public:
   ~SockSelect( void );             // Destructor
   SockSelect( void );              // Default constructor

private:                            // Bitwise copy prohibited
   SockSelect(const SockSelect&);   // Copy constructor
SockSelect&
   operator=(const SockSelect&);    // Assignment operator

//----------------------------------------------------------------------------
// SockSelect::Methods
//----------------------------------------------------------------------------
public:
int                                 // Return code (0 OK)
   insert(                          // Insert Socket
     Socket*           socket);     // -> Socket

int                                 // Return code (0 OK)
   remove(                          // Remove Socket
     Socket*           socket);     // -> Socket

Socket*                             // -> Selected Socket
   selectInp(                       // Select Socket for input
     unsigned long     timeout);    // Timeout (in milliseconds)

Socket*                             // -> Selected Socket
   selectOut(                       // Select Socket for output
     unsigned long     timeout);    // Timeout (in milliseconds)
}; // class SockSelect

#endif // SOCKET_H_INCLUDED
