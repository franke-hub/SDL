//----------------------------------------------------------------------------
//
//       Copyright (C) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Socket.h
//
// Purpose-
//       Document standard posix implementations.
//
// Last change date-
//       2019/03/27
//
// Usage notes-
//       See Socket.cpp for sample usage information.
//       This exposition contains code extracted from /usr/include, which
//       can only be distributed under the GNU GPL.
//
//----------------------------------------------------------------------------
#include <netdb.h>                  // For gethostbyname, struct hostent
#include <arpa/inet.h>              // For inet methods
#include <netinet/in.h>             // For struct sockaddr_ definitions
#include <sys/socket.h>             // For socket methods
#include <sys/time.h>               // For timeval
#include <sys/types.h>              // For completeness, included by above

//----------------------------------------------------------------------------
//
// Class-
//       Socket
//
// Purpose-
//       Socket implmentation details, extracted from library.
//
//----------------------------------------------------------------------------
class Socket {
//----------------------------------------------------------------------------
// Enumerations and typedefs
//----------------------------------------------------------------------------
public:
static const int       CLOSED= -1;  // Closed socket handle
typedef int            Handle;      // A socket handle (file descriptor)

// Extracted from library, subject to GPL
typedef int            socklen_t;   // (Usual implementation)
typedef uint16_t       sa_family_t; // (Usual implementation)
typedef uint32_t       in_addr_t;   // (Usual implementation)
typedef uint16_t       in_port_t;   // (Usual implementation)

// The IPV4 in_addr
struct in_addr
{
  in_addr_t            s_addr;
};

// The IPV6 in_addr
struct in6_addr
{
  union
  {
    uint8_t            u6_addr8[16];
    uint16_t           u6_addr16[8];
    uint32_t           u6_addr32[4];
  } in6_u;
// The following would be redefinitions
// #define s6_addr     in6_u.u6_addr8
// #define s6_addr16   in6_u.u6_addr16
// #define s6_addr32   in6_u.u6_addr32
};


// Various forms of sockaddr
struct sockaddr {
  sa_family_t          sa_family;   // address family, AF_xxx
  char                 sa_data[14]; // 14 bytes of protocol address
};

struct sockaddr_in
{
  sa_family_t          sin_family; // AF_INET
  in_port_t            sin_port;   // (Big endian) Port number
  struct in_addr       sin_addr;   // (Big endian) Internet address

  // Pad to size of `struct sockaddr'
  unsigned char        __pad[sizeof(struct sockaddr) - sizeof(sin_family)
                             - sizeof(sin_port) - sizeof(struct in_addr)];
};

struct sockaddr_in6
{
  sa_family_t          sin6_family; // AF_INET6
  in_port_t            sin6_port;     // (Big endian) Port number.
  uint32_t             sin6_flowinfo; // (Big endian) Traffic class and flow inf.
  struct in6_addr      sin6_addr;     // (Big endian) IPv6 address.
  uint32_t             sin6_scope_id; // Set of interfaces for a scope.
};

struct sockaddr_storage {           // 128= sizeof(sockaddr_storage);
  sa_family_t          ss_family;   // address family, AF_xxx
  char                 ss_pad1[6];  // (Alignment padding)
  int64_t              ss_align;    // Alignment
  char                 ss_pad2[112]; // (More padding)
};

// getaddrinfo result type
/* These are #defined in the library *****************************************
enum AI_FLAGS                       // addrinfo.ai_flags definitions
{  AI_PASSIVE=     0x0001           // Intend socket address for bind.
,  AI_CANONNAME=   0x0002           // Return canonical node name.
,  AI_NUMERICHOST= 0x0004           // Input is address, don't resolve.
,  AI_V4MAPPED=    0x0008           // IPv4 mapped addresses are acceptable.
,  AI_ALL=         0x0010           // Return v4-mapped and v6 addresses.
,  AI_ADDRCONFIG=  0x0020           // Only return address types available on
                                    // this host.
,  AI_NUMERICSERV= 0x0400           // Input is port number, don't resolve.
}; // enum AI_FLAGS  ********************************************************/

struct addrinfo {
  int                  ai_flags;    // input flags
  int                  ai_family;   // address family of socket
  int                  ai_socktype; // socket type
  int                  ai_protocol; // ai_protocol
  socklen_t            ai_addrlen;  // length of socket address
  char*                ai_canonname; // canonical name of service location
  struct sockaddr*     ai_addr;     // socket address of socket
  struct addrinfo*     ai_next;     // pointer to next in list
};

// gethostbyname result type
struct hostent {
   const char*         h_name;      // official name of host
   char**              h_aliases;   // alias list
   short               h_addrtype;  // host address type
   short               h_length;    // length of address
   char**              h_addr_list; // list of addresses from name server
// The following would be a redefinition
// #define h_addr h_addr_list[0]    // address, for backward compatiblity
};

// struct timeval (Used in timeout socket options)
typedef long           time_t;      // (Usual implementation)
typedef long           suseconds_t; // (Usual implementation)

struct timeval {
  time_t               tv_sec;      // seconds
  suseconds_t          tv_usec;     // and microseconds
};

//----------------------------------------------------------------------------
// Attributes (Socket simply wraps fd)
//----------------------------------------------------------------------------
public:
Handle                 fd;          // The socket handle (file descriptor)

//----------------------------------------------------------------------------
// Destructor, Constructors, Assignment operator
//----------------------------------------------------------------------------
   ~Socket( void ) {}               // Destructor

   Socket( void )                   // Constructor
:  fd(CLOSED)
{  }

   Socket(const Socket&) = default; // Copy constructor
Socket& operator=(const Socket&) = default; // Assignment operator

//----------------------------------------------------------------------------
// Methods
//----------------------------------------------------------------------------
// NONE DEFINED. Just use socket methods and fd (file descriptor) attribute.
}; // class Socket
