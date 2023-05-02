//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
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
//       Instantiate Socket methods.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#define FD_SETSIZE 512
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _OS_WIN
#include <io.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <com/Barrier.h>
#endif

#ifdef _OS_BSD
#define __need_timeval
#include <netdb.h>
#include <time.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#endif

#include <com/Debug.h>
#include <com/Software.h>

#include "Socket.u"
#include "com/Socket.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, I/O Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Enum-
//       SOCKET_FSM
//
// Purpose-
//       Socket::fsm states
//
//----------------------------------------------------------------------------
enum SOCKET_FSM                     // Socket FSM
{  FSM_RESET= 0                     // Reset
,  FSM_BOUND                        // Bound socket
,  FSM_CONNECTED                    // Connected socket
,  FSM_LISTENER                     // Listener socket
,  FSM_ERROR                        // Error
};


//----------------------------------------------------------------------------
// Typedefs
//----------------------------------------------------------------------------
#if defined(_OS_WIN) || defined(_OS_CYGWIN)
typedef int          socklen_t;
#endif

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif

//----------------------------------------------------------------------------
//
// Struct-
//       SockSelectHA
//
// Purpose-
//       Define the SockSelect hidden attributes.
//
//----------------------------------------------------------------------------
struct SockSelectHA {               // SockSelect hidden attributes
   int                 used;        // Number of elements used
   int                 next;        // Next index to use
   Socket*             socket[FD_SETSIZE]; // Socket* array
   fd_set              fdSet;       // fd_set
}; // struct SockSelectHA

//----------------------------------------------------------------------------
// Local data areas
//----------------------------------------------------------------------------
static char            staticBuffer[128]; // Used in addrToName

#ifdef _OS_WIN
static Barrier         barrier= BARRIER_INIT; // Thread block
static unsigned        initCounter= 0;// Non-zero if initialized
#endif

static const int       convertST[]= // ST_ conversion table
{  SOCK_ST_UNSPEC                   //  0: unspecified
,  SOCK_ST_STREAM                   //  1: stream socket
,  SOCK_ST_DGRAM                    //  2: datagram socket
,  (-1)                             //  3: (undefined)
,  (-1)                             //  4: (undefined)
,  (-1)                             //  5: (undefined)
,  (-1)                             //  6: (undefined)
,  (-1)                             //  7: (undefined)
}; // convertST

static const int       convertMO[]= // MO_ conversion table
{  SOCK_MO_OOB                      // 0x0001: process out-of-band data
,  SOCK_MO_PEEK                     // 0x0002: peek at incoming message
,  SOCK_MO_DONTROUTE                // 0x0004: send without using routing tables
,  SOCK_MO_EOR                      // 0x0008: data completes record
,  SOCK_MO_TRUNC                    // 0x0010: data discarded before delivery
,  SOCK_MO_CTRUNC                   // 0x0020: control data lost before delivery
,  SOCK_MO_WAITALL                  // 0x0040: wait for full request or error
,  SOCK_MO_MPEG2                    // 0x0080: Message contain MPEG2 data
,  (-1)                             // 0x0100: (undefined)
,  (-1)                             // 0x0200: (undefined)
,  (-1)                             // 0x0400: (undefined)
,  (-1)                             // 0x0800: (undefined)
,  (-1)                             // 0x1000: (undefined)
,  (-1)                             // 0x2000: (undefined)
,  SOCK_MO_NONBLOCK                 // 0x4000: nonblocking request
,  SOCK_MO_COMPAT                   // 0x8000: 4.3-format sockaddr
}; // convertMO

static const int       convertSO[]= // SO_ conversion table
{  SOCK_SO_UNSPEC                   // Unspecified
,  SOCK_SO_TYPE                     // Get socket type
,  SOCK_SO_ERROR                    // Get error status and clear
,  SOCK_SO_DEBUG                    // Turn on debugging info recording
,  SOCK_SO_SNDBUF                   // Send buffer size
,  SOCK_SO_RCVBUF                   // Receive buffer size
,  SOCK_SO_SNDLOWAT                 // Send low-water mark
,  SOCK_SO_RCVLOWAT                 // Receive low-water mark
,  SOCK_SO_SNDTIMEO                 // Send timeout
,  SOCK_SO_RCVTIMEO                 // Receive timeout
,  SOCK_SO_LINGER                   // Linger on close if data present
,  SOCK_SO_KEEPALIVE                // Keep connections alive
,  SOCK_SO_DONTROUTE                // Just use interface addresses
,  SOCK_SO_BROADCAST                // Permit sending of broadcast msgs
,  SOCK_SO_OOBINLINE                // Leave received OOB data in line
,  SOCK_SO_REUSEADDR                // Allow local address reuse
,  SOCK_SO_ACCEPTCONN               // Socket has had listen()
}; // convert SO

//----------------------------------------------------------------------------
//
// Subroutine-
//       getDebugBarrier
//
// Purpose-
//       Obtain Debug::Barrier
//
//----------------------------------------------------------------------------
static int                          // Non-zero if latch held by thread
   getDebugBarrier( void )          // Obtain Debug::Barrier
{
   int cc= Debug::obtain();
   if( cc != 0 )
     tracef("\n");

   return cc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       relDebugBarrier
//
// Purpose-
//       Release Debug::Barrier
//
//----------------------------------------------------------------------------
static inline void
   relDebugBarrier(                 // Release Debug::Barrier
     int               cc)          // Return code from getDebugBarrier
{
   if( cc == 0 )
     Debug::release();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::initSockets
//
// Purpose-
//       Sockets initialization.
//
//----------------------------------------------------------------------------
static void
   initSockets( void )              // Initialize sockets
{
#ifdef _OS_WIN
   WORD                parmWord;    // Parameter word
   WSADATA             wsaData;     // Data area

   int                 rc= 0;

   AutoBarrier lock(barrier);
   {{{{
     if( initCounter == 0 )         // if not initialized
     {
       parmWord= MAKEWORD(2,0);
       rc= WSAStartup(parmWord, &wsaData);
       if( rc != 0 )
       {
         fprintf(stderr, "No version 1.1 winsock.dll\n");
         exit(EXIT_FAILURE);
       }
     }

     initCounter++;
   }}}}

#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::termSockets
//
// Purpose-
//       Sockets termination.
//
//----------------------------------------------------------------------------
static void
   termSockets( void )              // Initialize sockets
{
#ifdef _OS_WIN
   AutoBarrier lock(barrier);
   {{{{
     initCounter--;                 // Decrement counter

     if( initCounter == 0 )         // If last user
       WSACleanup();                // Clean up
   }}}}
#endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::throwf
//
// Purpose-
//       Write a diagnostic error message and abort.
//
//----------------------------------------------------------------------------
static void
   throwf(                          // Abort with error message
     int               line,        // Line number
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
   _ATTRIBUTE_PRINTF(2,3)
   _ATTRIBUTE_NORETURN;

static void
   throwf(                          // Abort with error message
     int               line,        // Line number
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer

   fprintf(stderr, "%4d %s: ABORT: ", line, __FILE__);

   va_start(argptr, fmt);           // Initialize va_ functions
   vthrowf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::closesocket
//
// Purpose-
//       BSD only: close socket handle
//
//----------------------------------------------------------------------------
#ifndef _OS_WIN
static int                          // Resultant
   closesocket(                     // Close a socket
     int               handle)      // The socket handle
{
   return ::close(handle);
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::toAddr
//
// Purpose-
//       Convert sockaddr to Addr.
//
//----------------------------------------------------------------------------
static Socket::Addr                 // The Addr
   toAddr(                          // Convert sockaddr to Addr
     const char*       addr)        // The raw sockaddr
{
   Socket::Addr        result= 0;   // Resultant

   switch( ((sockaddr*)addr)->sa_family )
   {
     case SOCK_AF_INET:
       result= Socket::Addr(ntohl(((sockaddr_in*)addr)->sin_addr.s_addr));
       break;

     default:
       break;
   }
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ::toPort
//
// Purpose-
//       Convert sockaddr to Port.
//
//----------------------------------------------------------------------------
static Socket::Port                 // The Port
   toPort(                          // Convert sockaddr to Port
     const char*       addr)        // The raw sockaddr
{
   Socket::Port        result= 0;   // Resultant

   switch( ((sockaddr*)addr)->sa_family )
   {
     case SOCK_AF_INET:
       result= Socket::Port(ntohs(((sockaddr_in*)addr)->sin_port));
       break;

     default:
       break;
   }
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       SockSelect::~SockSelect
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   SockSelect::~SockSelect( void )  // Destructor
{
   IFHCDM( debugf("SockSelect(%p)::~SockSelect()\n", this); )

   if( object != NULL )
   {
     delete (SockSelectHA*)object;
     object= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       SockSelect::SockSelect
//
// Purpose-
//       Default constructor
//
//----------------------------------------------------------------------------
   SockSelect::SockSelect( void )   // Default constructor
:  object(NULL)
{
   SockSelectHA*       object;

   IFHCDM( debugf("SockSelect(%p)::SockSelect()\n", this); )

   this->object= object= new SockSelectHA();
   memset(object, 0, sizeof(*object));
}

//----------------------------------------------------------------------------
//
// Method-
//       SockSelect::insert
//
// Purpose-
//       Insert Socket onto list.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   SockSelect::insert(              // Insert Socket onto list
     Socket*           socket)      // -> Socket
{
   IFHCDM( debugf("SockSelect(%p)::insert(%p)\n", this, socket); )

   int                 result= (-1);// Resultant
   SockSelectHA*       object= (SockSelectHA*)this->object;

   if( object->used < FD_SETSIZE && socket != NULL
       && socket->handle >= 0 && socket->handle < FD_SETSIZE )
   {
     result= 0;
     object->socket[object->used++]= socket;
     object->next= 0;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       SockSelect::remove
//
// Purpose-
//       Remove Socket from list.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   SockSelect::remove(              // Remove Socket from list
     Socket*           socket)      // -> Socket
{
   IFHCDM( debugf("SockSelect(%p)::remove(%p)\n", this, socket); )

   int                 result= (-1);
   SockSelectHA*       object= (SockSelectHA*)this->object;

   int                 i;

   for(i= 0; i<object->used; i++)
   {
     if( object->socket[i] == socket )
       break;
   }

   if( i < object->used )
   {
     result= 0;

     for(++i; i<object->used; i++)
       object->socket[i-1]= object->socket[i];

     object->used--;
     object->next= 0;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       SockSelect::selectInp
//
// Purpose-
//       Select next Socket with input available
//
//----------------------------------------------------------------------------
Socket*                             // Socket*, NULL if timeout
   SockSelect::selectInp(           // Select input Socket
     unsigned long     timeout)     // Timeout (in milliseconds)
{
   IFHCDM( debugf("SockSelect(%p)::selectInp(%lu)\n", this, timeout); )

   SockSelectHA*       object= (SockSelectHA*)this->object;
   Socket*             result= NULL;// Resultant
   const int           next= object->next;
   const int           used= object->used;

   int                 handle;      // The current socket handle
   int                 maxHandle;   // The highest socket handle
   timeval             tmo;         // The selection timeout
   timeval*            ptrtmo= &tmo;// ->  selection timeout

   int                 rc;          // Called routine return code
   int                 i;
   int                 j;

   if( used > 0 )
   {
     FD_ZERO(&object->fdSet);

     maxHandle= (-1);
     for(i=0; i<used; i++)
     {
       handle= object->socket[i]->handle;
       if( handle >= 0 && handle < FD_SETSIZE )
       {
         FD_SET(handle, &object->fdSet);
         if( handle > maxHandle )
           maxHandle= handle;
       }
     }

     tmo.tv_sec= timeout / 1000;      // Timeout (seconds)
     tmo.tv_usec= (timeout - tmo.tv_sec*1000)*1000; // Timeout (microseconds)
     if( timeout == ULONG_MAX )
       ptrtmo= NULL;

     rc= 0;
     if( maxHandle >= 0 )
       rc= select(maxHandle+1, &object->fdSet, NULL, NULL, ptrtmo);
     if( rc > 0 )
     {
       for(i=0; i<used; i++)
       {
         j= i + next;
         j %= used;
         handle= object->socket[j]->handle;
         if( handle >= 0 && handle < FD_SETSIZE )
         {
           if( FD_ISSET(handle, &object->fdSet) )
           {
             object->next= j + 1;
             result= object->socket[j];
             break;
           }
         }
       }
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       SockSelect::selectOut
//
// Purpose-
//       Select next Socket that can be written
//
//----------------------------------------------------------------------------
Socket*                             // Socket*, NULL if timeout
   SockSelect::selectOut(           // Select output Socket
     unsigned long     timeout)     // Timeout (in milliseconds)
{
   IFHCDM( debugf("SockSelect(%p)::selectInp(%lu)\n", this, timeout); )

   SockSelectHA*       object= (SockSelectHA*)this->object;
   Socket*             result= NULL;// Resultant
   const int           next= object->next;
   const int           used= object->used;

   int                 handle;      // The current socket handle
   int                 maxHandle;   // The highest socket handle
   timeval             tmo;         // The selection timeout
   timeval*            ptrtmo= &tmo;// ->  selection timeout

   int                 rc;          // Called routine return code
   int                 i;
   int                 j;

   if( used > 0 )
   {
     FD_ZERO(&object->fdSet);

     maxHandle= (-1);
     for(i=0; i<used; i++)
     {
       handle= object->socket[i]->handle;
       if( handle >= 0 && handle < FD_SETSIZE )
       {
         FD_SET(handle, &object->fdSet);
         if( handle > maxHandle )
           maxHandle= handle;
       }
     }

     tmo.tv_sec= timeout / 1000;      // Timeout (seconds)
     tmo.tv_usec= (timeout - tmo.tv_sec*1000)*1000; // Timeout (microseconds)
     if( timeout == ULONG_MAX )
       ptrtmo= NULL;

     rc= 0;
     if( maxHandle >= 0 )
       rc= select(maxHandle+1, NULL, &object->fdSet, NULL, ptrtmo);
     if( rc > 0 )
     {
       for(i=0; i<used; i++)
       {
         j= i + next;
         j %= used;
         handle= object->socket[j]->handle;
         if( handle >= 0 && handle < FD_SETSIZE )
         {
           if( FD_ISSET(handle, &object->fdSet) )
           {
             object->next= j + 1;
             result= object->socket[j];
             break;
           }
         }
       }
     }
   }

   return result;
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
{
   IFHCDM( debugf("Socket(%p)::~Socket()\n", this); )

   close();                         // Close the connection
   ::termSockets();
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::Socket
//
// Purpose-
//       Default constructor
//
//----------------------------------------------------------------------------
   Socket::Socket( void )           // Default constructor
:  fsm(FSM_RESET)
,  handle(-1)
,  st(ST_UNSPEC)
,  ec(0)
,  hSize(0)
,  pSize(0)
{
   ::initSockets();

   IFHCDM( debugf("Socket(%p)::Socket()\n", this); )

   memset(&hInet, 0, sizeof(hInet));
   memset(&pInet, 0, sizeof(pInet));

   hSize= sizeof hInet;
   ((sockaddr_in*)hInet)->sin_family= SOCK_AF_INET;
   ((sockaddr_in*)hInet)->sin_addr.s_addr= INADDR_ANY;

   pSize= sizeof pInet;
   ((sockaddr_in*)pInet)->sin_family= SOCK_AF_INET;
   ((sockaddr_in*)pInet)->sin_addr.s_addr= INADDR_ANY;
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
   Socket::Socket(                  // Constructor
     SocketST          st)          // Socket type
:  fsm(FSM_RESET)
,  handle(-1)
,  st(ST_UNSPEC)
,  ec(0)
,  hSize(0)
,  pSize(0)
{
   ::initSockets();

   int                 sysST;

   IFHCDM( debugf("Socket(%p)::Socket(%d)\n", this, st); )

   sysST= (-1);
   if( st >= 0 && st < Socket::ST_MAX )
     sysST= convertST[st];

   if( sysST < 0 )
     ec= Software::EC_INVAL;
   else
     Socket::st= st;

   memset(&hInet, 0, sizeof(hInet));
   memset(&pInet, 0, sizeof(pInet));

   hSize= sizeof hInet;
   ((sockaddr_in*)hInet)->sin_family= SOCK_AF_INET;
   ((sockaddr_in*)hInet)->sin_addr.s_addr= INADDR_ANY;

   pSize= sizeof pInet;
   ((sockaddr_in*)pInet)->sin_family= SOCK_AF_INET;
   ((sockaddr_in*)pInet)->sin_addr.s_addr= INADDR_ANY;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getAddr
//
// Purpose-
//       Get the associated host network address
//
//----------------------------------------------------------------------------
Socket::Addr                        // The associated Addr, 0 if error
   Socket::getAddr(                 // Get host internet address
     int               alias)       // For this alias
{
   Addr                result= 0;   // Resultant

   unsigned char*      C;           // -> name
   hostent*            hostEntry;   // -> hostent
   char                hostName[512]; // Working string
   int                 rc;
   int                 i;
   int                 j;

   ::initSockets();

   rc= ::gethostname(hostName, sizeof(hostName));
   if( rc != 0 )
     logOpError(__LINE__, "gethostname", "rc(%d)\n", rc);
   else
   {
     hostEntry= ::gethostbyname(hostName);
     if( hostEntry == NULL )
       logOpError(__LINE__, "gethostbyname", "NULL= gethostbyname(%s)\n",
                            hostName);
     else
     {
       for(i= 0; hostEntry->h_addr_list[i] != NULL; i++)
       {
         if( i == alias )
         {
           C= (unsigned char*)hostEntry->h_addr_list[i];
           for(j= 0; j<hostEntry->h_length; j++)
           {
             result <<= 8;
             result  |= C[j];
           }
           break;
         }
       }
     }
   }

   ::termSockets();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getName
//
// Purpose-
//       Get the associated host network name
//
//----------------------------------------------------------------------------
Socket::Name                        // The associated Name, NULL if error
   Socket::getName(                 // Get host internet name
     int               alias)       // For this alias
{
   const char*         result= NULL;// Resultant

   hostent*            hostEntry;   // -> hostent
   char                hostName[512]; // Working string
   int                 rc;
   int                 i;

   ::initSockets();

   rc= ::gethostname(hostName, sizeof(hostName));
   if( rc != 0 )
     logOpError(__LINE__, "gethostname", "rc(%d)\n", rc);
   else
   {
     hostEntry= ::gethostbyname(hostName);
     if( hostEntry == NULL )
       logOpError(__LINE__, "gethostbyname", "NULL= gethostbyname(%s)\n",
                            hostName);
     else
     {
       if( alias == 0 )
         result= hostEntry->h_name;
       else
       {
         for(i= 0; hostEntry->h_aliases[i] != NULL; i++)
         {
           if( i == alias-1 )
           {
             result= hostEntry->h_aliases[i];
             break;
           }
         }
       }
     }
   }

   ::termSockets();

   return Name(result);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getSocketEI
//
// Purpose-
//       Convert error code to error text.
//
//----------------------------------------------------------------------------
const char*                         // Resultant
   Socket::getSocketEI(             // Convert error code to error text
     SocketEC          ec)          // Error Code
{
   return Software::getSystemEI(Software::SystemEC(ec));
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::addrToChar
//
// Purpose-
//       Convert address to constant string.
//
//----------------------------------------------------------------------------
const char*                         // Constant name representation
   Socket::addrToChar(              // Convert network address to string
     Addr              addr)        // The network address
{
   in_addr             temp;        // The network address

   temp.s_addr= htonl((int)addr);
   return inet_ntoa(temp);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::addrToName
//
// Purpose-
//       Convert network address to network name
//
//----------------------------------------------------------------------------
Socket::Name                        // The associated Name
   Socket::addrToName(              // Convert Addr to Name
     Addr              addr,        // The network address
     char*             target,      // Resultant (NULL value not reentrant)
     unsigned          length)      // Resultant length
{
   Name                result= NULL;

   ::initSockets();

#if 0
   hostent*            hostEntry;   // -> hostent
   char                string[8];   // Converted address

   int                 i;

   string[0]= addr >> 56;
   string[1]= addr >> 48;
   string[2]= addr >> 40;
   string[3]= addr >> 32;
   string[4]= addr >> 24;
   string[5]= addr >> 16;
   string[6]= addr >>  8;
   string[7]= addr;

   for(i= 0; i<sizeof(string); i++)
   {
     if( string[i] != 0 )
       break;
   }

   if( i > 4 )
     i= 4;

   hostEntry= ::gethostbyaddr(&string[8-i], 8-i, SOCK_PF_INET);
   if( hostEntry != NULL )
     result= hostEntry->h_name;
#else

   if( target == NULL )
   {
     target= staticBuffer;
     length= sizeof(staticBuffer);
   }

   sockaddr_in sockinfo;            // Socket information
   sockinfo.sin_family= SOCK_PF_INET; // Set socket family
   sockinfo.sin_port= 0;
   sockinfo.sin_addr.s_addr= htonl((int)addr);

   int rc= ::getnameinfo((const struct sockaddr*)&sockinfo, sizeof(sockinfo),
                         target, length, NULL, 0, NI_NOFQDN);
   if( rc == 0 )
     result= target;
#endif

   ::termSockets();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::nameToAddr
//
// Purpose-
//       Convert network name to network address
//
//----------------------------------------------------------------------------
Socket::Addr                        // Resultant host address
   Socket::nameToAddr(              // Convert Name to Addr
     const Name        name)        // Host name or address
{
   Addr                result= 0;   // Resultant
   hostent*            hostEntry;   // -> hostent

   int                 dotCount;    // Number of '.' characters encountered
   int                 isConstant;  // TRUE if name is a constant

   int                 i;

   ::initSockets();

   isConstant= TRUE;                // Default, name is a constant
   dotCount= 0;                     // Default, no '.' characters encountered
   for(i=0; name[i] != '\0'; i++)
   {
     if( name[i] == '.' )
       dotCount++;
     else if( name[i] < '0' || name[i] > '9' )
     {
       isConstant= FALSE;
       break;
     }
   }

   if( isConstant == TRUE && dotCount == 3 )
     result= Addr(ntohl(inet_addr(name)));
   else
   {
     hostEntry= ::gethostbyname(name);// Get the target host entry
     if( hostEntry != NULL )
       result= Addr(ntohl(*(long*)hostEntry->h_addr));
   }

   ::termSockets();

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::debug
//
// Purpose-
//       Display debugging information
//
//----------------------------------------------------------------------------
void
   Socket::debug( void ) const      // Display debugging information
{
   debugf("Socket(%p)::debug()\n", this);
   debugf(">>fsm(%d) handle(%d) st(%d) ec(%d)\n", fsm, handle, st, ec);
   debugf(">>host: %ld: %s:%d\n", hSize,
          inet_ntoa(((sockaddr_in*)hInet)->sin_addr),
          ntohs(((sockaddr_in*)hInet)->sin_port));
   debugf(">>peer: %ld: %s:%d\n", pSize,
          inet_ntoa(((sockaddr_in*)pInet)->sin_addr),
          ntohs(((sockaddr_in*)pInet)->sin_port));
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getHostAddr
//
// Purpose-
//       Get the socket network address
//
//----------------------------------------------------------------------------
Socket::Addr                        // The host network address
   Socket::getHostAddr( void ) const// Get host network address
{
   return toAddr(hInet);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getHostName
//
// Purpose-
//       Get the socket network name
//
//----------------------------------------------------------------------------
Socket::Name                        // The host network name
   Socket::getHostName( void )      // Get host network name
{
   Name                result= NULL;

#if 0
   hostent*            hostEntry;   // -> hostent
   hostEntry= ::gethostbyaddr((char*)&((sockaddr_in*)hInet)->sin_addr.s_addr,
                              sizeof(((sockaddr_in*)hInet)->sin_addr.s_addr),
                              SOCK_PF_INET);
   if( hostEntry != NULL )
     result= (char*)hostEntry->h_name;
#else

   int rc= ::gethostname(hName, sizeof(hName));
   if( rc == 0 )
     result= hName;
#endif

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getHostPort
//
// Purpose-
//       Get the socket port number
//
//----------------------------------------------------------------------------
Socket::Port                        // The host port number
   Socket::getHostPort( void ) const// Get host port number
{
   return toPort(hInet);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getPeerAddr
//
// Purpose-
//       Get the peer network address
//
//----------------------------------------------------------------------------
Socket::Addr                        // The peer network address
   Socket::getPeerAddr( void ) const// Get peer network address
{
   return toAddr(pInet);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getPeerName
//
// Purpose-
//       Get the peer network name
//
//----------------------------------------------------------------------------
Socket::Name                        // The peer network name
   Socket::getPeerName( void )      // Get peer network ame
{
   Name                result= NULL;

#if 0
   hostent*            hostEntry;   // -> hostent
   hostEntry= ::gethostbyaddr((char*)&((sockaddr_in*)pInet)->sin_addr.s_addr,
                              sizeof(((sockaddr_in*)pInet)->sin_addr.s_addr),
                              SOCK_PF_INET);
   if( hostEntry != NULL )
     result= (char*)hostEntry->h_name;
#else
   int rc= ::getnameinfo((const struct sockaddr*)pInet, sizeof(pInet),
                         hName, sizeof(hName), NULL, 0, NI_NOFQDN);
   if( rc == 0 )
     result= hName;
#endif

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getPeerPort
//
// Purpose-
//       Get the peer port number
//
//----------------------------------------------------------------------------
Socket::Port                        // The peer port number
   Socket::getPeerPort( void ) const// Get peer port number
{
   return toPort(pInet);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getSocketEC
//
// Purpose-
//       Get the last error code.
//
//----------------------------------------------------------------------------
Socket::SocketEC                    // Resultant
   Socket::getSocketEC( void ) const// Get last SocketEC
{
   return ec;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::setSocketEC
//
// Purpose-
//       Set the error code.
//
//----------------------------------------------------------------------------
void
   Socket::setSocketEC(             // Set the SocketEC
     SocketEC          ec)          // To this SocketEC
{
   Socket::ec= ec;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getSocketEI
//
// Purpose-
//       Get the error information relating to the current error.
//
//----------------------------------------------------------------------------
const char*                         // Associated error information
   Socket::getSocketEI( void ) const// Convert SocketEC to text
{
   return getSocketEI(ec);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getSocketSO
//
// Purpose-
//       Get a socket option
//
//----------------------------------------------------------------------------
int                                 // The SocketOption value
   Socket::getSocketSO(             // Get a SocketOption
     SocketSO          so)          // The SocketOption to get
{
   int                 result= (-1);// Resultant
   int                 rc= (-1);    // Return code

   struct linger       linger;      // Linger option value
   int                 option;      // Converted option control
   int                 optval;      // Other option value
   socklen_t           optlen;      // Option length

   IFHCDM( debugf("Socket::getSocketSO(%d)\n", so); )

   ec= 0;                           // (No error)
   option= verifySO(so);
   switch(so)
   {
     case SO_TYPE:
       rc= 0;
       result= st;
       break;

     case SO_LINGER:
       optlen= sizeof(linger);
       rc= ::getsockopt(handle, SOL_SOCKET, SOCK_SO_LINGER,
                        (char*)&linger, &optlen);
       if( rc == 0 )
       {
         result= 0;
         if( linger.l_onoff != 0 )
           result= linger.l_linger;
       }
       break;

     case SO_SNDTIMEO:
     case SO_RCVTIMEO:
       timeval tmo;
       tmo.tv_sec= 0;
       tmo.tv_usec= 0;
       optlen= sizeof(tmo);
       rc= ::getsockopt(handle, SOL_SOCKET, option,
                        (char*)&tmo, &optlen);
///////debugf("GET %d {%ld,%ld} %d\n", rc, (long)tmo.tv_sec, (long)tmo.tv_usec, optlen);
       if( rc == 0 )
         result= tmo.tv_sec * 1000 + tmo.tv_usec / 1000;
       break;

     case SO_ERROR:
     case SO_DEBUG:
     case SO_SNDBUF:
     case SO_RCVBUF:
     case SO_SNDLOWAT:
     case SO_RCVLOWAT:
     case SO_KEEPALIVE:
     case SO_DONTROUTE:
     case SO_BROADCAST:
     case SO_OOBINLINE:
     case SO_REUSEADDR:
     case SO_ACCEPTCONN:
       optval= 0;
       optlen= sizeof(optval);
       rc= ::getsockopt(handle, SOL_SOCKET, option,
                        (char*)&optval, &optlen);
       if( rc == 0 )
         result= optval;
       break;

     default:
       ec= Software::EC_INVAL;
       break;
   }

   IFIODM( debugf("%d= ::getsockopt(%d, SOL_SOCKET, %d) %d\n",
                  rc, handle, so, result); )
   if( rc != 0 )                    // If error
   {
     if( ec != 0 )                  // If not parameter error
       ec= Software::getSystemEC();

     IFHCDM( debugf("%d= ::getsockopt(%d, SOL_SOCKET, %d) %d, %d:%s\n",
                    rc, handle, so, result, ec, getSocketEI()); )
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::setSocketSO
//
// Purpose-
//       Set a socket option
//
//----------------------------------------------------------------------------
int                                 // Resultant, 0 OK
   Socket::setSocketSO(             // Set a SocketOption
     SocketSO          so,          // The SocketOption to set
     int               value)       // The value to set
{
   struct linger       linger;      // Linger option value
   int                 option;      // Converted option control
   int                 optval;      // Other option value

   int                 rc= (-1);    // Resultant

   IFHCDM( debugf("Socket::setSocketSO(%d, 0x%x)\n", so, value); )

   ec= 0;                           // (No error)
   if( setHandle() != 0 )           // Create handle (if required)
     return rc;

   option= verifySO(so);
   switch(so)
   {
     case SO_TYPE:
       optval= convertST[value];
       if( value < 0 || value >= ST_MAX || optval < 0 )
         ec= Software::EC_INVAL;
       else
       {
         rc= ::setsockopt(handle, SOL_SOCKET, SOCK_SO_TYPE,
                          (const char*)&optval, sizeof(optval));
         if( rc == 0 )
           st= SocketST(value);
       }
       break;

     case SO_LINGER:
       linger.l_onoff= (value > 0);
       linger.l_linger= value;
       rc= ::setsockopt(handle, SOL_SOCKET, SOCK_SO_LINGER,
                        (const char*)&linger, sizeof(linger));
       break;

     case SO_SNDTIMEO:
     case SO_RCVTIMEO:
       timeval tmo;
       tmo.tv_sec= value / 1000;
       tmo.tv_usec= (value%1000) * 1000;
       rc= ::setsockopt(handle, SOL_SOCKET, option,
                        (const char*)&tmo, sizeof(tmo));
///////debugf("SET %d {%ld,%ld} %d\n", rc, (long)tmo.tv_sec, (long)tmo.tv_usec, sizeof(tmo));
       break;

     case SO_ERROR:
     case SO_DEBUG:
     case SO_SNDBUF:
     case SO_RCVBUF:
     case SO_SNDLOWAT:
     case SO_RCVLOWAT:
     case SO_KEEPALIVE:
     case SO_DONTROUTE:
     case SO_BROADCAST:
     case SO_OOBINLINE:
     case SO_REUSEADDR:
     case SO_ACCEPTCONN:
       optval= value;
       rc= ::setsockopt(handle, SOL_SOCKET, option,
                        (const char*)&optval, sizeof(optval));
       break;

     default:
       ec= Software::EC_INVAL;
       break;
   }

   IFIODM( debugf("%d= ::setsockopt(%d, SOL_SOCKET, %d, %d)\n",
                  rc, handle, so, value); )
   if( rc != 0 )                    // If error
   {
     if( ec != 0 )                  // If not parameter error
       ec= Software::getSystemEC();

     IFHCDM( debugf("%d= ::setsockopt(%d, SOL_SOCKET, %d, %d) %d:%s\n",
                    rc, handle, so, value, ec, getSocketEI()); )
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::getSocketST
//
// Purpose-
//       Get the SocketST
//
//----------------------------------------------------------------------------
Socket::SocketST                    // Resultant
   Socket::getSocketST( void ) const// Get SocketST
{
   return st;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::isOpen
//
// Purpose-
//       Get the OPEN state
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Socket::isOpen( void ) const     // Get OPEN state
{
   return (fsm == FSM_CONNECTED);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::setHost
//
// Purpose-
//       Set the host network address and port
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Socket::setHost(                 // Set the host network address and port
     Addr              addr,        // The network address
     Port              port)        // The port
{
   int                 result= (-1);// Resultant

   int                 rc;

   IFHCDM( debugf("Socket(%p)::setHost(%s,%d)\n",
                  this, addrToChar(addr), port); )

   ec= 0;                           // (No error)
   if( fsm == FSM_CONNECTED )       // If invalid state
   {
     ec= Software::EC_ISCONN;
     logError(__LINE__, "Socket::setHost(%s,%d) fsm(%d)\n",
                        addrToChar(addr), port, fsm);
     return result;
   }

   if( setHandle() != 0 )
     return result;

   hSize= sizeof hInet;
   if( addr == 0 )
     addr= getAddr();
   ((sockaddr_in*)hInet)->sin_addr.s_addr= htonl((int)addr);
   ((sockaddr_in*)hInet)->sin_port= htons(port);
   rc= ::bind(handle, (sockaddr*)&hInet, (socklen_t)hSize);
   IFIODM( logEvent(__LINE__, "%d= ::bind(%d,...)\n", rc, handle); )
   if( rc != 0 )                    // If bind failure
   {
     ec= Software::getSystemEC();
     logError(__LINE__,
              "%d= ::bind(%d,...) %d:%s\n", rc, handle, ec, getSocketEI());
     return result;
   }

   result= 0;
   ::getsockname(handle, (sockaddr*)hInet, (socklen_t*)&hSize);
   fsm= FSM_BOUND;
   IFIODM(
     logEvent(__LINE__, "setHost: %s:%d\n",
                        addrToChar(getHostAddr()), getHostPort());
   )

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::setHostAddr
//
// Purpose-
//       Get the associated host network address
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Socket::setHostAddr(             // Set the host network address
     Addr              addr)        // The network address
{
   return setHost(addr, getHostPort());
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::setHostPort
//
// Purpose-
//       Set the associated Port
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Socket::setHostPort(             // Set the host Port
     Port              port)        // The Port
{
   return setHost(getHostAddr(), port);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::setPeer
//
// Purpose-
//       Get the peer network address and port
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Socket::setPeer(                 // Set the network address and port
     Addr              addr,        // The network address
     Port              port)        // The port
{
   int                 result= (-1);// Resultant

   ec= 0;                           // (No error)
   if( fsm != FSM_RESET && fsm != FSM_BOUND )
     ec= Software::EC_ISCONN;
   else
   {
     result= 0;

     ((sockaddr_in*)pInet)->sin_addr.s_addr= htonl((int)addr);
     ((sockaddr_in*)pInet)->sin_port= htons(port);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::setPeerAddr
//
// Purpose-
//       Get the associated host network address
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Socket::setPeerAddr(             // Set the associated Addr
     Addr              addr)        // The network address
{
   return setPeer(addr, getPeerPort());
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::setPeerPort
//
// Purpose-
//       Set the associated Port
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Socket::setPeerPort(             // Set the associated Port
     Port              port)        // The Port
{
   return setPeer(getPeerAddr(), port);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::recv
//
// Purpose-
//       Receive on socket.
//
//----------------------------------------------------------------------------
Socket::Size_t                      // Transfer length
   Socket::recv(                    // Receive data
     Byte*             addr,        // Data address
     Size_t            size,        // Data length
     SocketMO          opts)        // Message options
{
   long                L;           // Transfer length

   ec= 0;                           // (No error)
   switch( st )
   {
     case ST_STREAM:
       L= ::recv(handle, (char*)addr, size, verifyMO(opts));
       break;

     case ST_DGRAM:
       pSize= sizeof(pInet);
       L= ::recvfrom(handle, (char*)addr, size, verifyMO(opts),
               (sockaddr*)pInet, (socklen_t*)&pSize);
       break;

     default:
       throwf(__LINE__, "send, SocketST(%d)", st);
       break;
   }

   if( L == SOCKET_ERROR )
   {
     fsm= FSM_ERROR;
     ec= Software::getSystemEC();
   }

   IFIODM(
     logEvent(__LINE__, "%ld= Socket(%p)::recv(%p,%lu,%x)\n",
                        L, this, addr, size, opts);
     if( L == SOCKET_ERROR )
       perror("..I/O error");
   )
   IFSCDM(
     if( L == SOCKET_ERROR )
     {
       debugf("%ld= Socket(%p)::recv(%p,%lu,%x)\n",
               L, this, addr, size, opts);
       perror("..I/O error");
     }
   )

   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::send
//
// Purpose-
//       Transmit on socket.
//
//----------------------------------------------------------------------------
Socket::Size_t                      // Transfer length
   Socket::send(                    // Transmit data
     const Byte*       addr,        // Data address
     Size_t            size,        // Data length
     SocketMO          opts)        // Message options
{
   long                L;           // Transfer length

   ec= 0;                           // (No error)
   switch( st )
   {
     case ST_STREAM:
       L= ::send(handle, (char*)addr, size, verifyMO(opts));
       break;

     case ST_DGRAM:
       L= ::sendto(handle, (char*)addr, size, verifyMO(opts),
               (sockaddr*)pInet, pSize);
       break;

     default:
       throwf(__LINE__, "send, SocketST(%d)", st);
       break;
   }

   if( L == SOCKET_ERROR )
   {
     fsm= FSM_ERROR;
     ec= Software::getSystemEC();
   }

   IFIODM(
     logEvent(__LINE__, "%ld= Socket(%p)::send(%p,%lu,%x)\n",
                        L, this, addr, size, opts);
     if( L == SOCKET_ERROR )
       perror("..I/O error");
   )
   IFSCDM(
     if( L == SOCKET_ERROR )
     {
       debugf("%ld= Socket(%p)::send(%p,%lu,%x)\n",
               L, this, addr, size, opts);
       perror("..I/O error");
     }
   )

   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::close
//
// Purpose-
//       Close the Socket
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Socket::close( void )            // Close the Socket
{
   int                 result= 0;   // Resultant

   IFHCDM( debugf("Socket(%p)::close(%d)\n", this, handle); )

   ec= 0;                           // (No error)
   if( handle >= 0 )                // If open
   {
     result= ::closesocket(handle); // Close the Socket
     IFIODM( logEvent(__LINE__, "%d= ::close(%d)\n", result, handle); )
     handle= (-1);                  // Indicate closed
   }

   fsm= FSM_RESET;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::connect
//
// Purpose-
//       Connect to peer.
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Socket::connect(                 // Connect to peer
     Addr              addr,        // Peer network address
     Port              port)        // Peer port
{
   int                 result= (-1);// Resultant

   int                 rc;

   IFHCDM( debugf("Socket(%p)::connect(%s,%d)\n",
                  this, addrToChar(addr), port); )

   ec= 0;                           // (No error)
   if( fsm != FSM_RESET )           // If not in reset state
   {
     ec= Software::EC_ISCONN;
     logError(__LINE__, "Socket::connect(%s,%d) fsm(%d)\n",
                        addrToChar(addr), port, fsm);
     return result;
   }

   if( st == ST_UNSPEC )
     st= ST_STREAM;

   if( st != ST_STREAM )
   {
     ec= Software::EC_PROTOTYPE;
     return result;
   }

   if( setHandle() != 0 )
     return result;

   pSize= sizeof pInet;
   ((sockaddr_in*)pInet)->sin_family= ((sockaddr_in*)hInet)->sin_family;
   ((sockaddr_in*)pInet)->sin_addr.s_addr= htonl((int)addr);
   ((sockaddr_in*)pInet)->sin_port= htons(port);
   rc= ::connect(handle, (sockaddr*)pInet, (socklen_t)pSize);
   IFIODM( logEvent(__LINE__, "%d= ::connect(%d,...)\n", rc, handle); )
   if( rc != 0 )                  // If connection not made
   {
     ec= Software::getSystemEC();
     logError(__LINE__,
              "%d= connect(%d) %d:%s\n", rc, handle, ec, getSocketEI());
     return result;
   }

   result= 0;
   fsm= FSM_CONNECTED;

   hSize= sizeof hInet;
   ::getsockname(handle, (sockaddr*)hInet, (socklen_t*)&hSize);
   ::getpeername(handle, (sockaddr*)pInet, (socklen_t*)&pSize);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::listen
//
// Purpose-
//       Wait for connection on port.
//
//----------------------------------------------------------------------------
Socket*                             // -> new Socket
   Socket::listen(                  // Listen for connection
     Port              port)        // Source host port
{
   int                 newHandle;   // New handle

   int                 rc;

   IFHCDM( debugf("Socket(%p)::listen(%d)\n", this, port); )

   ec= 0;                           // (No error)
   switch( fsm )
   {
     case FSM_RESET:
     case FSM_BOUND:
       break;

     case FSM_LISTENER:
       if( port != 0 && port != ntohs(((sockaddr_in*)hInet)->sin_port) )
       {
         if( setHostPort(port) != 0 )
           return NULL;
       }
       break;

     case FSM_CONNECTED:
     case FSM_ERROR:
       ec= Software::EC_ISCONN;
       IFIODM(
         logEvent(__LINE__, "Socket(%p)::listen() fsm(%d)\n", this, fsm);
       )
       return NULL;
       break;

     default:
       throwf(__LINE__, "fsm(%d)", fsm);
   }

   if( fsm != FSM_BOUND && fsm != FSM_LISTENER )
   {
     if( setHostPort(port) != 0 )
       return NULL;
   }
   fsm= FSM_LISTENER;

   // Wait for connection
   rc= ::listen(handle, SOMAXCONN); // Begin listening
   IFIODM( logEvent(__LINE__, "%d= listen(%d,%x)\n", rc, handle, SOMAXCONN); )
   if( rc < 0 )                     // If listen failure
   {
     ec= Software::getSystemEC();
     logError(__LINE__, "%d= listen(%d,%d)\n", rc, handle, SOMAXCONN);
     return NULL;
   }

   // Accept the next connection
   for(;;)
   {
     pSize= sizeof pInet;
     newHandle= accept(handle, (sockaddr*)&pInet, (socklen_t*)&pSize);
     IFIODM(
       logEvent(__LINE__, "%d= accept(LISTEN) %d:%s\n",
                          newHandle, getSocketEC(), getSocketEI());
       )
     if( newHandle >= 0 )           // If valid handle
       break;

     if( errno == EINTR )           // If interrupted
       continue;

     ec= Software::getSystemEC();
     IFHCDM( logEvent(__LINE__, "accept(LISTEN) %d:%s\n", ec, getSocketEI()); )
     return NULL;
   }

   // Create new Socket
   Socket* result= new Socket();
   result->fsm= FSM_CONNECTED;
   result->handle= newHandle;
   result->st= st;
   result->hSize= hSize;
   result->pSize= pSize;
   memcpy(result->hInet, hInet, sizeof hInet);
   memcpy(result->pInet, pInet, sizeof pInet);

   IFHCDM( debugf("Socket(%p) %d= accept(%p)\n", result, newHandle, this); )

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::logError
//
// Purpose-
//       Write a diagnostic log message.
//
// Implementation notes-
//       DO NOT write to trace file. This results in unwanted debug.out files.
//
//----------------------------------------------------------------------------
void
   Socket::logError(                // Diagnostic error message
     int               line,        // Line number
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer

   int cc= getDebugBarrier();

   fprintf(stderr, "%4d Socket(%p) ERROR: ", line, this);

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);
   va_end(argptr);                  // Close va_ functions

   relDebugBarrier(cc);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::logEvent
//
// Purpose-
//       Write a diagnostic log message.
//
//----------------------------------------------------------------------------
void
   Socket::logEvent(                // Diagnostic event message
     int               line,        // Line number
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer

   int cc= getDebugBarrier();

   traceh("%4d Socket(%p) ", line, this);

   va_start(argptr, fmt);           // Initialize va_ functions
   vtracef(fmt, argptr);
   va_end(argptr);                  // Close va_ functions

   relDebugBarrier(cc);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::logOpError
//
// Purpose-
//       Write a diagnostic log message.
//
//----------------------------------------------------------------------------
void
   Socket::logOpError(              // Diagnostic error message
     int               line,        // Line number
     const char*       op,          // Operation name
     const char*       fmt,         // Error message
                       ...)         // PRINTF arguments
{
   va_list             argptr;      // Argument list pointer

   int cc= getDebugBarrier();

   traceh("%4d Socket(*) ERROR: OP(%s) ", line, op);

   va_start(argptr, fmt);           // Initialize va_ functions
   vtracef(fmt, argptr);
   va_end(argptr);                  // Close va_ functions

   IFHCDM( perror(op); )

   relDebugBarrier(cc);
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::setHandle
//
// Purpose-
//       Update the handle
//
//----------------------------------------------------------------------------
int                                 // Return code (0 OK)
   Socket::setHandle( void )        // Update the handle
{
   ec= 0;                           // (No error)
   if( handle < 0 )
   {
     if( st == ST_UNSPEC )
       st= ST_STREAM;

     handle= ::socket(SOCK_AF_INET, convertST[st], SOCK_PF_UNSPEC);
     IFIODM( logEvent(__LINE__, "%d= socket(%d,%d,%d)\n", handle,
                                SOCK_AF_INET, convertST[st], SOCK_PF_UNSPEC); )
     if( handle < 0 )               // If socket not created
     {
       ec= Software::getSystemEC();
       logError(__LINE__, "%d= socket(%d,%d,%d) %d:%s\n",
                          handle, SOCK_AF_INET, convertST[st], SOCK_PF_UNSPEC,
                          ec, getSocketEI());
       return (-1);
     }

     // Set default options
     int optval= TRUE;
     ::setsockopt(handle, SOL_SOCKET, SOCK_SO_REUSEADDR,
                  (const char*)&optval, sizeof(optval));

     IFHCDM( debugf("Socket(%p) %d= setHandle()\n", this, handle); )
   }

   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::verifyMO
//
// Purpose-
//       Convert Socket.h MO_ value to system value.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   Socket::verifyMO(                // MO_ conversion
     SocketMO          value)       // MessageOption value
{
   int                 resultant= 0;// Resultant

   unsigned const      mo= value;
   unsigned            mask;
   unsigned            i;

   if( value != 0 )                 // FastPath
   {
     mask= 1;
     for(i=0; i<16; i++)
     {
       if( (mo&mask) != 0 )
       {
         if( convertMO[i] == (-1) )
           logError(__LINE__, "SocketMO(%.4X) %.4X not supported\n",
                              mo, mask);
         else
           resultant |= convertMO[i];
       }
       mask <<= 1;
     }
   }

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       Socket::verifySO
//
// Purpose-
//       Verify and convert a SocketOption.
//
//----------------------------------------------------------------------------
int                                 // Resultant (>0 OK)
   Socket::verifySO(                // SO_ conversion
     SocketSO          so)          // SocketOption value
{
   int                 result= (-1);// Resultant

   if( so < 0 || so >= Socket::SO_MAX )
     logError(__LINE__, "Invalid SocketSO(%d)\n", so);
   else
   {
     result= convertSO[so];
     if( result < 0 )
       logError(__LINE__, "SocketSO(%d) not supported\n", so);
   }

   return result;
}

