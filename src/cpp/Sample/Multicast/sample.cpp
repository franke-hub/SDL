//----------------------------------------------------------------------------
//
//       Copyright (c) 2016 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       sample.cpp
//
// Purpose-
//       Sample multicast socket usage.
//
// Last change date-
//       2016/01/01
//
// Notes (port number)-
//       In order to send a datagram, both the address and port number must be
//       specified.  Only recipients who have matching datagram ports open will
//       see group messages sent to a port.  In that respect, each port is like
//       a separate group.
//
//       For this sample, the client port and server port need not differ.  If
//       they are the same the server sees all the messages it writes and the
//       client sees all the join messages.
//
// Notes (multicast)-
//       There is no restriction on running multiple "clients" or "servers"
//       on one or more machines.  More applications result in more messages.
//
//----------------------------------------------------------------------------
#include <com/Logger.h>             // Use logger for append
#include <com/define.h>
#include <com/Signal.h>

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#if defined(CLIENT) && defined(SERVER)
  #error "Only define either CLIENT or SERVER, not both"
#elif defined(CLIENT)
  #define CONFIG "client"
  #define USE_CLIENT 1
  #define USE_SERVER 0
#elif defined(SERVER)
  #define CONFIG "server"
  #define USE_CLIENT 0
  #define USE_SERVER 1
#else
  #error "Must define either CLIENT or SERVER"
#endif

#define SERVER_REPLIES   5          // Number of server replies
#define CLIENT_PORT 0               // Default client port, may be 0
#define SERVER_PORT 12345           // Default server port, should be >= 1024
#define VERSION_ID "1.0.1-12.06"    // Version identifier

#define INET_ADDR  "225.0.0.37"     // Group internet address
/////// INET_ADDR  "239.9.0.8"      // Group internet address
/////// INET_ADDR  "127.0.0.1"      // Local internet address

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

#ifndef SCDM
#define SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef USE_ADDRINFO
#define USE_ADDRINFO 1              // Use GET_ADDRINFO to select interfaces?
#endif

//----------------------------------------------------------------------------
// Dependent definitions
//----------------------------------------------------------------------------
#ifdef _OS_BSD
  #define SOCKLEN_T socklen_t
#else
  #define SOCKLET_T int
#endif

//----------------------------------------------------------------------------
//
// Class-
//       SignalIgnore
//
// Purpose-
//       Local signal handler.
//
//----------------------------------------------------------------------------
class SignalIgnore : public Signal {
//----------------------------------------------------------------------------
// SignalIgnore::Attributes
//----------------------------------------------------------------------------
public:
SignalCode             handled;     // TRUE when signal handled

//----------------------------------------------------------------------------
// SignalIgnore::Constructors
//----------------------------------------------------------------------------
public:
   SignalIgnore( void )             // Default constructor
:  Signal(), handled((SignalCode)0) {}

//----------------------------------------------------------------------------
// SignalIgnore::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (0 iff COMPLETELY handled)
   handle(                          // Handle a signal
     SignalCode        signal)      // The signal to handle
{
   handled= signal;
   return 0;                        // (Signals are ignored)
}
}; // class SignalIgnore

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static SignalIgnore    handler;     // Signal handler
static long            hostaddr;    // (One of) Our host addresses
static char            hostname[256]; // This host's name
static int             ifCount= 0;  // Number of interfaces
static int             ifIndex= 0;  // Selected interface index
static struct addrinfo*ifTable= NULL; // The interface table
static int             activeTime;  // Active time (seconds)
static int             clientPort;  // Client port number
static int             serverPort;  // Server port number

static int             SWIODM;      // IODM switch
static int             SWHCDM;      // HCDM switch
static int             SWSCDM;      // SCDM switch

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
// Switches and controls
static int             config;      // Test configuration control

//----------------------------------------------------------------------------
//
// Class-
//       Common
//
// Purpose-
//       Common Client/Server Object
//
//----------------------------------------------------------------------------
class Common {
//----------------------------------------------------------------------------
// Common::Attributes
//----------------------------------------------------------------------------
public:
enum FSM                            // States
{  FSM_RESET                        // Reset
,  FSM_GROUP                        // At least one multicast group joined
,  FSM_READY                        // Fully operational
,  FSM_CLOSE                        // Shutting down
}; // enum FSM

int                    fsm;         // Finite State Machine
const char*            name;        // The associated name
int                    talkHandle;  // Common talk handle

char                   buffer[256]; // Scratchpad

//----------------------------------------------------------------------------
// Common::Methods
//----------------------------------------------------------------------------
virtual
   ~Common( void )
{
   if( talkHandle >= 0 )
     sclose(talkHandle);
}

   Common(
     const char*       name)
:  fsm(FSM_RESET), name(name), talkHandle(-1)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::shouldNotOccur
//
// Purpose-
//       Write message to log and exit.
//
//----------------------------------------------------------------------------
void
   shouldNotOccur(                  // Write log message and exit
     int               lineno,      // Line number
     const char*       fmt,         // PRINTF format descriptor
                       ...)         // PRINTF argruments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   debugf("%4d: %s: ", lineno, name);
   vdebugf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions

   errorf("Error(%d): ", errno);
   perror("perror");
   errorf("\n");

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Common::formatAddrinfo
//
// Purpose-
//       Format addrinfo entry
//
//----------------------------------------------------------------------------
const char*                         // (buffer)
   formatAddrinfo(                  // Format Addrinfo
     struct addrinfo*  ifEntry)     // The addrinfo
{
   return inet_ntop(ifEntry->ai_family, &((sockaddr_in*)ifEntry->ai_addr)->sin_addr,
                    buffer, sizeof(buffer));
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::formatHostaddr
//
// Purpose-
//       Format host address
//
//----------------------------------------------------------------------------
const char*                         // (buffer)
   formatHostaddr(                  // Format host address
     long              addr)        // The address
{
   sprintf(buffer, "%ld.%ld.%ld.%ld",
                   (addr >> 24) & 0x00ff,
                   (addr >> 16) & 0x00ff,
                   (addr >>  8) & 0x00ff,
                   (addr >>  0) & 0x00ff);
   return buffer;
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::formatSockaddr
//
// Purpose-
//       Format sockaddr_in
//
//----------------------------------------------------------------------------
const char*                         // (buffer)
   formatSockaddr(                  // Format sockaddr_in
     const sockaddr_in&addr)        // The sockaddr_in to format
{
   int                 host= ntohl(addr.sin_addr.s_addr);
   int                 port= ntohs(addr.sin_port);

   sprintf(buffer, "%d.%d.%d.%d:%d",
                   (host >> 24) & 0x00ff,
                   (host >> 16) & 0x00ff,
                   (host >>  8) & 0x00ff,
                   (host >>  0) & 0x00ff,
                   port);
   return buffer;
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::sopen
//
// Purpose-
//       Create a new connection.
//
//----------------------------------------------------------------------------
int                                 // Socket handle
   sopen(                           // Create a new connection
     int&              port)        // Using this port
{
   sockaddr_in         addr;        // Socket address information
   ip_mreq             imr;         // Multicast group option
   int                 intopt;      // Generic integer option
   int                 talk;        // Socket handle

   int                 rc;

   // Get the host name
   gethostname(hostname, sizeof(hostname));

   // Create the socket
   talk= socket(AF_INET, SOCK_DGRAM, 0); // Create the socket
   if( SWHCDM )
     debugf("%4d: %d= socket(IF_INET, SOCK_DGRAM, 0)\n", __LINE__, talk);
   if( talk < 0)                    // Socket not created
     shouldNotOccur(__LINE__, "%d= socket()\n", talk);
   talkHandle= talk;                // Set recovery handle

   // Bind the socket
   memset(&addr,0,sizeof(addr));
   addr.sin_family = AF_INET;
   if( inet_pton(AF_INET, INET_ADDR, &addr.sin_addr) != 1 ) // WRONG! (CYGWIN bind fails)
     perror("inet_pton");
   addr.sin_addr.s_addr = htonl(INADDR_ANY); // RIGHT!
   addr.sin_port= htons(port);      // We pick the port

if( config & 1 )
{{{{
   // It makes no difference whether this section is included or not
   // Enable reuseaddr option
   intopt= TRUE;
   rc= setsockopt(talk, SOL_SOCKET, SO_REUSEADDR, &intopt, sizeof(intopt));
   if( SWHCDM )
     debugf("%4d: %d= setsockopt(%d,%d,%d,%p=%d,%ld) REUSEADDR\n", __LINE__, rc, talk,
            SOL_SOCKET, SO_REUSEADDR, &intopt, intopt, (long)sizeof(intopt));
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= setsockopt()\n", rc);
}}}}

   rc= bind(talk, (sockaddr*)&addr, sizeof(addr));
   if( SWHCDM )
     debugf("%4d: %d= bind(%d,%p,%ld) %s)\n", __LINE__, rc, talk, &addr,
            (long)sizeof(addr), formatSockaddr(addr));
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= bind()\n", rc);

if( config & 2 )
{{{{
   // It makes no difference whether this section is included or not
   // Enable multicast option
   intopt= TRUE;
   rc= setsockopt(talk, SOL_SOCKET, SO_BROADCAST, &intopt, sizeof(intopt));
   if( SWHCDM )
     debugf("%4d: %d= setsockopt(%d,%d,%d,%p=%d,%ld) BROADCAST\n", __LINE__, rc, talk,
            SOL_SOCKET, SO_BROADCAST, &intopt, intopt, (long)sizeof(intopt));
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= setsockopt()\n", rc);
}}}}

   if( inet_pton(AF_INET, INET_ADDR, &imr.imr_multiaddr.s_addr) != 1 )
     shouldNotOccur(__LINE__, "inet_pton");


#if(USE_ADDRINFO)
   // Join the multicast group (on each interface)
   struct addrinfo* ifEntry= ifTable;
   for(int index= 1; ifEntry != NULL; index++)
   {
     if( ifIndex == 0 || ifIndex == index )
     {
       imr.imr_interface.s_addr= ((sockaddr_in*)ifEntry->ai_addr)->sin_addr.s_addr;
       rc= setsockopt(talk, SOL_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr));
       if( SWHCDM )
         debugf("%4d: %d= setsockopt(%d,%d,%d,%p,%ld)\n"
                "      ADD_MEMBERSHIP(%s,%s)\n", __LINE__,
                rc, talk, SOL_IP, IP_ADD_MEMBERSHIP, &imr, (long)sizeof(imr),
                INET_ADDR, formatAddrinfo(ifEntry));
       if( rc != 0 )
         shouldNotOccur(__LINE__, "%d= setsockopt()\n", rc);

       fsm= FSM_GROUP;              // Set if ANY group membership added
     }

     ifEntry= ifEntry->ai_next;
   }

#else
   // Join the multicast group (on each interface)
   hostent* entry= gethostbyname(hostname);
   if( SWHCDM )
     debugf("%4d: %p= gethostbyname(%s)\n", __LINE__, entry, hostname);
   if( entry == NULL || entry->h_addr_list[0] == NULL  )
     shouldNotOccur(__LINE__, "%p= gethostbyname(%s)\n", entry, hostname);

   for(int i= 0; entry->h_addr_list[i] != NULL; i++)
   {
     hostaddr= ntohl(*(long*)entry->h_addr_list[i]);
     imr.imr_interface.s_addr= htonl(hostaddr); // Better

     rc= setsockopt(talk, SOL_IP, IP_ADD_MEMBERSHIP, &imr, sizeof(imr));
     if( SWHCDM )
       debugf("%4d: %d= setsockopt(%d,%d,%d,%p,%ld)\n"
              "      ADD_MEMBERSHIP(%s,%s)\n", __LINE__,
              rc, talk, SOL_IP, IP_ADD_MEMBERSHIP, &imr, (long)sizeof(imr),
              INET_ADDR, formatHostaddr(hostaddr));
     if( rc != 0 )
       shouldNotOccur(__LINE__, "%d= setsockopt()\n", rc);

     fsm= FSM_GROUP;                // Set if ANY group membership added
   }
#endif

   // Set TTL option
   intopt= 3;
   rc= setsockopt(talk, SOL_IP, IP_MULTICAST_TTL, &intopt, sizeof(intopt));
   if( SWHCDM )
     debugf("%4d: %d= setsockopt(%d,%d,%d,%p=%d,%ld) TTL\n", __LINE__, rc, talk,
            SOL_IP, IP_MULTICAST_TTL, &intopt, intopt, (long)sizeof(intopt));
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= setsockopt()\n", rc);

   fsm= FSM_READY;                  // Fully ooperational
   return talk;
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::sclose
//
// Purpose-
//       Terminate a connection.
//
//----------------------------------------------------------------------------
void
   sclose(                          // Terminate a connection
     int               talk)        // For this handle
{
   ip_mreq             imr;         // Multicast group option

   int                 rc;

   if( fsm >= FSM_GROUP ) {
     // Leave the multicast group
     hostent* entry= gethostbyname(hostname);
     if( SWHCDM )
       debugf("%4d: %p= gethostbyname(%s)\n", __LINE__, entry, hostname);
     if( entry == NULL )
       shouldNotOccur(__LINE__, "%p= gethostbyname(%s)\n", entry, hostname);

     if( inet_pton(AF_INET, INET_ADDR, &imr.imr_multiaddr.s_addr) != 1 )
       shouldNotOccur(__LINE__, "inet_pton");

#if( USE_ADDRINFO)
     // Drop the multicast group (on each interface)
     struct addrinfo* ifEntry= ifTable;
     for(int index= 1; ifEntry != NULL; index++)
     {
       if( ifIndex == 0 || ifIndex == index )
       {
         imr.imr_interface.s_addr= ((sockaddr_in*)ifEntry->ai_addr)->sin_addr.s_addr;
         rc= setsockopt(talk, SOL_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(imr));
         if( SWHCDM )
           debugf("%4d: %d= setsockopt(%d,%d,%d,%p,%ld)\n"
                  "      DROP_MEMBERSHIP(%s,%s)\n", __LINE__,
                  rc, talk, SOL_IP, IP_DROP_MEMBERSHIP, &imr, (long)sizeof(imr),
                  INET_ADDR, formatAddrinfo(ifEntry));
         if( rc != 0 )
           shouldNotOccur(__LINE__, "%d= setsockopt()\n", rc);

         fsm= FSM_GROUP;              // Set if ANY group membership added
       }

       ifEntry= ifEntry->ai_next;
     }

#else
     // Drop the multicast group (on each interface)
     for(int i= 0; entry->h_addr_list[i] != NULL; i++)
     {
       hostaddr= ntohl(*(long*)entry->h_addr_list[i]);
       imr.imr_interface.s_addr= htonl(hostaddr); // Better

       rc= setsockopt(talk, SOL_IP, IP_DROP_MEMBERSHIP, &imr, sizeof(imr));
       if( SWHCDM )
         debugf("%4d: %d= setsockopt(%d,%d,%d,%p,%ld)\n"
                "      DROP_MEMBERSHIP(%s,%s)\n", __LINE__,
                rc, talk, SOL_IP, IP_DROP_MEMBERSHIP, &imr, (long)sizeof(imr),
                INET_ADDR, formatHostaddr(hostaddr));
       if( rc != 0 )
         shouldNotOccur(__LINE__, "%d= setsockopt()\n", rc);
     }
#endif
   }

   if( talk >= 0 ) {
     rc= close(talk);
     if( SWHCDM )
       debugf("%4d: %d= close(%d)\n", __LINE__, rc, talk);
   }
   talkHandle= (-1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::receive
//
// Purpose-
//       Receive a datagram string message.
//
//----------------------------------------------------------------------------
int                                 // Number of bytes read
   receive(                         // Receive a message
     int               talk,        // Socket handle
     sockaddr_in&      addr,        // Address information
     char*             buff,        // Message address
     unsigned          size)        // Maximum message length
{
   int                 L;           // Read length
   SOCKLEN_T           ignored= sizeof(sockaddr_in);

   memset(&addr, 0, sizeof(sockaddr_in));
   addr.sin_family = AF_INET;
   handler.handled= (Signal::SignalCode)FALSE;
   L= recvfrom(talk, buff, size-1, 0, (sockaddr*)&addr, &ignored);
   if( SWIODM )
     debugf("%4d: %d= recvfrom(%d,%p,%d,%d,%p,%p=%d) %s\n", __LINE__,
            L, talk, buff, size, 0, &addr, &ignored, ignored,
            formatSockaddr(addr));

   if( L < 0 )
   {
     Signal::SignalCode signaled= handler.handled;
     if( !signaled )
       shouldNotOccur(__LINE__, "%d= recvfrom(%d,%p,%d,%d,%p,%p=%d) %s\n", L,
                                talk, buff, size, 0, &addr, &ignored, ignored,
                                formatSockaddr(addr));

     if( !SWIODM )
       debugf("%4d: %d= recvfrom(%d,%p,%d,%d,%p,%p=%d) %s\n", __LINE__, L,
              talk, buff, size, 0, &addr, &ignored, ignored,
              formatSockaddr(addr));
     debugf("Signal(%d) %s\n", signaled, handler.getSignalName(signaled));
     fsm= FSM_CLOSE;
     return L;
   }

   buff[L]= '\0';
   if( L > 0 )                      // If datagram present
   {
     if( SWSCDM )
       debugf("Recv: %s: %s\n", formatSockaddr(addr), buff);
   }

   return L;
}

//----------------------------------------------------------------------------
//
// Method-
//       Common::transmit
//
// Purpose-
//       Send a datagram string message.
//
//----------------------------------------------------------------------------
int                                 // Number of bytes sent
   transmit(                        // Send a message
     int               talk,        // Socket handle
     const sockaddr_in&
                       addr,        // Address information
     const char*       buff)        // Message buffer
{
   int                 L;           // Write length
   int                 size;        // Message length

   size= strlen(buff);
   L= sendto(talk, buff, size, 0, (sockaddr*)&addr, sizeof(sockaddr_in));
   if( SWIODM )
     debugf("%4d: %d= sendto(%d,%p,%d,%d,%p,%ld) %s\n", __LINE__,
            L, talk, buff, size, 0, &addr, (long)sizeof(sockaddr_in),
            formatSockaddr(addr));

   if( SWSCDM )
     debugf("Send: %s: %s\n", formatSockaddr(addr), buff);

   return L;
}
}; // class Common

//----------------------------------------------------------------------------
//
// Class-
//       Client
//
// Purpose-
//       Drive the client.
//
//----------------------------------------------------------------------------
class Client : public Common {
//----------------------------------------------------------------------------
// Client::Methods
//----------------------------------------------------------------------------
public:
virtual
   ~Client( void )
{
}

   Client(
     const char*       name)
:  Common(name)
{
}

virtual int
   operate( void );
}; // class Client

//----------------------------------------------------------------------------
//
// Class-
//       Server
//
// Purpose-
//       Drive the server.
//
//----------------------------------------------------------------------------
class Server : public Common {
//----------------------------------------------------------------------------
// Server::Methods
//----------------------------------------------------------------------------
public:
virtual
   ~Server( void )
{
}

   Server(
     const char*       name)
:  Common(name)
{
}

virtual int
   operate( void );
}; // class Server

//----------------------------------------------------------------------------
//
// Subroutine-
//       list
//
// Purpose-
//       List the interfaces
//
//----------------------------------------------------------------------------
void
   list( void )                     // List the interfaces
{
   Common common("common");         // For formatAddrinfo

   struct addrinfo* ifEntry= ifTable;
   for(int index= 1; ifEntry != NULL; index++)
   {
     debugf("IF[%2d] %s\n", ifCount, common.formatAddrinfo(ifEntry));
     if( ifEntry->ai_canonname != NULL )
       debugf("Host name: '%s'\n", ifEntry->ai_canonname);

     ifEntry= ifEntry->ai_next;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialization processing
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   // Set up debugging
   char filename[32];
   sprintf(filename, "%s.out", CONFIG);
   Debug::set(new Logger(filename));

   time_t rawtime;
   struct tm* timeinfo;

   time(&rawtime);
   timeinfo= localtime(&rawtime);
   tracef("\n\n\n======== %s========\n", asctime(timeinfo));

   // Get our host name
   hostname[0]= '\0';
   gethostname(hostname, sizeof(hostname));

   // Get list of host interfaces
   struct addrinfo hints=
       { AI_CANONNAME               // ai_flags
       , AF_UNSPEC // AF_INET       // ai_family
       , SOCK_DGRAM                 // ai_socktype
       , 0                          // ai_protocol (ANY)
       , 0                          // ai_addrlen
       , NULL, NULL, NULL};         // ai_addr, ai_canonname, ai_next

   int rc= getaddrinfo(hostname, NULL, &hints, &ifTable);
   if( rc != 0 ) {
     errorf("Internal error(%d) %d %s\n", __LINE__,
             rc, gai_strerror(rc));
     exit(1);
   }

   // Count the interfaces
   struct addrinfo* ifEntry= ifTable;
   hostaddr= ntohl((in_addr_t)((sockaddr_in*)ifEntry->ai_addr)->sin_addr.s_addr);
   for(ifCount= 0; ifEntry != NULL; ifCount++)
   {
     ifEntry= ifEntry->ai_next;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Termination processing
//
//----------------------------------------------------------------------------
static void
   term( void )                     // Terminate
{
   if( ifTable != NULL )
   {
     freeaddrinfo(ifTable);
     ifTable= NULL;
   }

   // Terminate debugging
   delete Debug::set(NULL);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Informational exit
//
//----------------------------------------------------------------------------
static void
   info(                            // Informational exit
     const char*       sourceName)  // The source fileName
{
   fprintf(stderr, "%s\n", sourceName);
   fprintf(stderr, "runtime (seconds)\n");
   fprintf(stderr, "server port number\n");
   fprintf(stderr, "client port number\n");
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Analyze parameters
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 argx;        // Argument index
   int                 error;       // TRUE if error encountered
   int                 verify;      // TRUE if verify required

   int                 i, j;

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   verify= FALSE;

   argx= 0;
   activeTime= 0;
   ifIndex= 0;                      // All indexes
   clientPort= CLIENT_PORT;
   serverPort= SERVER_PORT;
   config= 4;                       // Best default
   SWIODM= FALSE;
   SWHCDM= FALSE;
   SWSCDM= FALSE;
   #if defined(IODM)
     SWIODM= TRUE;
   #endif
   #if defined(HCDM)
     SWHCDM= TRUE;
   #endif
   #if defined(SCDM)
     SWSCDM= TRUE;
   #endif

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp(argv[j], "-help") == 0 )
         error= TRUE;

       else if( strcmp(argv[j], "-iodm") == 0 )
         SWIODM= TRUE;

       else if( strcmp(argv[j], "-hcdm") == 0 )
         SWHCDM= TRUE;

       else if( strcmp(argv[j], "-scdm") == 0 )
         SWSCDM= TRUE;

       else if( strcmp(argv[j], "-list") == 0 ) {
         list();
       }

       else if( strcmp(argv[j], "-if") == 0 ) {
         j++;
         if( j >= argc ) {
           error= TRUE;
           fprintf(stderr, "Missing interface index\n");
         } else
           ifIndex= atol(argv[j]);
           if( ifIndex < 0 || ifIndex > ifCount )
           {
             error= TRUE;
             fprintf(stderr, "Invalid interface index(%d) of(%d)\n",
                     ifIndex, ifCount);
           }
       }

       else if( strcmp(argv[j], "-test") == 0 ) {
         j++;
         if( j >= argc ) {
           error= TRUE;
           fprintf(stderr, "Missing test number\n");
         } else
           config= atol(argv[j]);
       }
       else if( strcmp(argv[j], "--") == 0 )
         break;

       else
       {
         for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'h':              // -h (help)
               error= TRUE;
               break;

             case 'v':              // -v (verify)
               verify= TRUE;
               break;

             default:               // If invalid switch
               error= TRUE;
               fprintf(stderr, "Invalid switch '%c'\n", (int)argv[j][i]);
               break;
           }
         }
       }
     }
     else                           // Argument
     {
       switch( argx )
       {
         case 0:
           activeTime= atol(argv[j]);
           break;

         case 1:
           serverPort= atol(argv[j]);
           break;

         case 2:
           clientPort= atol(argv[j]);
           break;

         default:
           error= TRUE;
           fprintf(stderr, "Unexpected argument: '%s'\n", argv[j]);
           break;
       }
       argx++;
     }
   }

   //-------------------------------------------------------------------------
   // Check and validate
   //-------------------------------------------------------------------------
   if( SWHCDM )                     // HCDM implies IODM and SCDM
     SWIODM= TRUE;
   if( SWIODM )                     // IODM implies SCDM
     SWSCDM= TRUE;

   if( error )
     info(argv[0]);

   if( verify )
   {
     debugf("%s\n", CONFIG);
     debugf("clientPort: %d\n", clientPort);
     debugf("serverPort: %d\n", serverPort);
     debugf("activeTime: %d\n", activeTime);
     if( ifIndex == 0 )
       debugf("interfaces: ALL(%d)\n", ifCount);
     else
       debugf(" interface: %d of %d\n", ifIndex, ifCount);
     debugf("HCDM: %d\n", SWHCDM);
     debugf("IODM: %d\n", SWIODM);
     debugf("SCDM: %d\n", SWSCDM);
//// debugf("TEST: %d\n", config); // Always displayed
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Client::operate
//
// Purpose-
//       Client driver.
//
//----------------------------------------------------------------------------
int                                 // Return code
   Client::operate( void )          // Client driver
{
   char                buffer[4096]; // Message buffer
   sockaddr_in         addr;        // Our socket address information
   socklen_t           alen= sizeof(addr); // Our socket address length
   int                 L;           // Response length
   sockaddr_in         recvFr;      // Partner's socket address information
   sockaddr_in         sendTo;      // Partner's socket address information
   time_t              start;       // Starting time
   int                 talk;        // Socket handle
   time_t              tod;         // Current time

   int                 i;

   // Initialize
   talk= sopen(clientPort);
   getsockname(talk, (sockaddr*)&addr, &alen);
   clientPort= ntohs(addr.sin_port);

   debugf("Connection(%d) opened\n", talk);
   debugf(" CONFIG: %d Client-%s\n", config, VERSION_ID);
   debugf("   HOST: %s/%s\n", hostname, formatHostaddr(hostaddr));
   debugf(" CLIENT: %s:%d\n", INET_ADDR, clientPort);
   debugf(" SERVER: %s:%d\n", INET_ADDR, serverPort);
   if( activeTime == 0 )
     activeTime= 30;
   start= time(NULL);

   // Join the conversation
   memset(&sendTo, 0, sizeof(sendTo));
   sendTo.sin_family = AF_INET;
   if( inet_pton(AF_INET, INET_ADDR, &sendTo.sin_addr) != 1 )
     perror("inet_pton");
   sendTo.sin_port= htons(serverPort);

   sprintf(buffer, "JOIN: %s/%s", hostname, formatHostaddr(hostaddr));
   transmit(talk, sendTo, buffer);

   // Operate
   for(i= 0; ; i++)
   {
     for(;;)
     {
       tod= time(NULL);
       if( (tod - start) >= activeTime )
       {
         fsm= FSM_CLOSE;
         break;
       }

       alarm(activeTime - (tod - start));
       L= receive(talk, recvFr, buffer, sizeof(buffer));
       alarm(0);

       if( L > 0 || fsm != FSM_READY )
         break;
       sleep(1);
     }
     if( fsm != FSM_READY )
       break;
   }

   // Goodbye
   debugf("Connection(%d) closed\n", talk);
   sclose(talk);
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::operate
//
// Purpose-
//       Server driver.
//
//----------------------------------------------------------------------------
int                                 // Return code
   Server::operate( void )          // Server driver
{
   char                buffer[512]; // Message buffer
   int                 L;           // Response length
   sockaddr_in         recvFr;      // Partner's socket address information
   sockaddr_in         sendTo;      // Partner's socket address information
   time_t              start;       // Starting time
   int                 talk;        // Socket handle
   time_t              tod;         // Current time

   int                 i;

   // Initialize
   talk= sopen(serverPort);

   debugf("Connection(%d) opened\n", talk);
   debugf(" CONFIG: %d Server-%s\n", config, VERSION_ID);
   debugf("   HOST: %s/%s\n", hostname, formatHostaddr(hostaddr));
   debugf(" SERVER: %s:%d\n", INET_ADDR, serverPort);
   debugf(" CLIENT: %s:%d\n", INET_ADDR, clientPort);
   if( activeTime == 0 )
     activeTime= 300;
   start= time(NULL);

   // Global talk address
   memset(&sendTo, 0, sizeof(sendTo));
   sendTo.sin_family = AF_INET;
   if( inet_pton(AF_INET, INET_ADDR, &sendTo.sin_addr) != 1 )
     perror("inet_pton");
   sendTo.sin_port= htons(clientPort);

   // Operate
   for(;;)
   {
     for(;;)
     {
       tod= time(NULL);
       if( (tod - start) >= activeTime )
       {
         fsm= FSM_CLOSE;
         break;
       }

       alarm(activeTime - (tod - start));
       buffer[0]= '\0';
       L= receive(talk, recvFr, buffer, sizeof(buffer));
       alarm(0);

       if( L > 0 || fsm != FSM_READY )
         break;
       sleep(1);
     }
     if( fsm != FSM_READY )
       break;

if( config & 4 )
{{{{
     // Clients on different machines gets reply when used
     sendTo.sin_addr= recvFr.sin_addr;
}}}}
     sendTo.sin_port= recvFr.sin_port;
     if( memcmp("JOIN:", buffer, 5) == 0 )
     {
       for(i= 1; i <= SERVER_REPLIES; i++)
       {
         sprintf(buffer, "Message %d of %d", i, SERVER_REPLIES);
         transmit(talk, sendTo, buffer);
       }
     }
   }

   // Goodbye
   debugf("Connection(%d) closed\n", talk);
   sclose(talk);
   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
static Client client("client");     // Create the Client
static Server server("server");     // Create the Server

   //-------------------------------------------------------------------------
   // Parameter validation
   //-------------------------------------------------------------------------
   init();
   atexit(&term);
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Process the function
   //-------------------------------------------------------------------------
   try {
     if( USE_CLIENT )
       client.operate();
     else if( USE_SERVER )
       server.operate();
   } catch(char* X) {
     debugf("Exception(%s)\n", X);
   } catch(...) {
     debugf("Exception(...)\n");
   }

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   return 0;
}

