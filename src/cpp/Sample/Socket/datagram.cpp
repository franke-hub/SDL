//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       datagram.cpp
//
// Purpose-
//       Sample datagram socket usage.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <com/Debug.h>
#include <com/define.h>
#include <com/Signal.h>

#include <pthread.h>                // Must be first (per doc)

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "DATAGRAM" // Source file, for debugging

/////// INET_ADDR        0xe0090008 // Group internet address
#define INET_ADDR        0x7f000001 // Local internet address

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#define SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent definitions
//----------------------------------------------------------------------------
#define SWHCDM 0
#ifdef  HCDM
#undef  SWHCDM
#define SWHCDM 1
#endif

#define SWSCDM 0
#ifdef  SCDM
#undef  SWSCDM
#define SWSCDM 1
#endif

#define SWIODM 0
#ifdef  IODM
#undef  SWIODM
#define SWIODM 1
#endif

#ifdef _OS_BSD
  #define SOCKLEN_T socklen_t
#else
  #define SOCKLET_T int
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

//----------------------------------------------------------------------------
// Internal data areas referenced by class functions
//----------------------------------------------------------------------------
static int             activeTime;  // Active time (seconds)
static int             clientPort;  // Client port number
static int             serverPort;  // Server port number

//----------------------------------------------------------------------------
//
// Class-
//       PseudoThread
//
// Purpose-
//       Pseudo-Thread Object
//
//----------------------------------------------------------------------------
class PseudoThread
{
//----------------------------------------------------------------------------
// PseudoThread::Attributes
//----------------------------------------------------------------------------
public:
pthread_t              tid;         // Thread identifier
const char*            name;        // The associated name

char                   buffer[256]; // Scratchpad
int                    swOnline;    // TRUE when connection is active

//----------------------------------------------------------------------------
// PseudoThread::Methods
//----------------------------------------------------------------------------
virtual
   ~PseudoThread( void )
{
}

   PseudoThread(
     const char*       name)
:  name(name)
,  tid(0)
{
}

void
   start( void );                   // Start the Thread, calling operate()

virtual void*                       // Return item
   operate( void )                  // Operate the PseudoThread
{
   return 0;
}

void*
   wait( void )
{
   void*               rc;          // Return code

   pthread_join(tid, &rc);
   return rc;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       shouldNotOccur
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

   errorf("%4d: %s: ", lineno, name);
   va_start(argptr, fmt);           // Initialize va_ functions
   verrorf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions

   errorf("Error(%d): ", errno);
   perror("perror");

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       formatSockaddr
//
// Purpose-
//       Format sockaddr_in
//
//----------------------------------------------------------------------------
const char*                         // (buffer)
   formatSockaddr(                  // Format sockaddr_in
     const sockaddr_in&addr)        // The sockaddr_in to format
{
   int                 iaddr= ntohl(addr.sin_addr.s_addr);
   int                 port= ntohs(addr.sin_port);

   sprintf(buffer, "%d.%d.%d.%d:%d",
                   (iaddr >> 24) & 0x00ff,
                   (iaddr >> 16) & 0x00ff,
                   (iaddr >>  8) & 0x00ff,
                   (iaddr >>  0) & 0x00ff,
                   port);
   return buffer;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       contact
//
// Purpose-
//       Create a new connection.
//
//----------------------------------------------------------------------------
int                                 // Socket handle
   contact(                         // Create a new connection
     int               port)        // Using this port
{
   sockaddr_in         addr;        // Socket address information
   int                 talk;        // Socket handle

   int                 rc;

   // Create the socket
   talk= socket(AF_INET, SOCK_DGRAM, 0); // Create the socket
   if( SWHCDM )
     debugf("%d= socket(IF_INET, SOCK_DGRAM, 0)\n", talk);
   if( talk < 0)                    // Socket not created
     shouldNotOccur(__LINE__, "%d= socket()\n", talk);

   // Set the INET address
   memset(&addr,0,sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = INADDR_ANY;
// addr.sin_addr.s_addr = htonl(INET_ADDR);
   addr.sin_port= htons(port);      // We pick the port

if( 1 )
{{{{
   // Bind the socket
   rc= bind(talk, (sockaddr*)&addr, sizeof(addr));
   if( SWHCDM )
     debugf("%d= bind(%d,%p,%zd) %s)\n", rc, talk, &addr, sizeof(addr),
            formatSockaddr(addr));
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= bind()\n", rc);
}}}}

if( 0 )
{{{{
   // Connnect the socket
   rc= connect(talk, (sockaddr*)&addr, sizeof(addr));
   if( SWHCDM )
     debugf("%d= connect(%d,%p,%zd) %s\n",
            rc, talk, &addr, sizeof(addr),
            formatSockaddr(addr));
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= connect()\n", rc);
}}}}

   swOnline= TRUE;
   return talk;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       receive
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
   L= recvfrom(talk, buff, size, 0, (sockaddr*)&addr, &ignored);
   if( SWIODM )
     debugf("%d= recvfrom(%d,%p,%d,%d,%p,%p=%d) %s\n",
            L, talk, buff, size, 0, &addr, &ignored, ignored,
            formatSockaddr(addr));

   if( L < 0 )
     shouldNotOccur(__LINE__, "%d= recvfrom(%d,%p,%d,%d,%p,%p=%d) %s\n", L,
                              talk, buff, size, 0, &addr, &ignored, ignored,
                              formatSockaddr(addr));

   if( L > 0 )                      // If datagram present
   {
     #ifdef SCDM
       debugf("Recv: %s: %s\n", formatSockaddr(addr), buff);
     #endif
   }

   return L;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       transmit
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

   size= strlen(buff) + 1;
   L= sendto(talk, buff, size, 0, (sockaddr*)&addr, sizeof(sockaddr_in));
   if( SWIODM )
     debugf("%d= sendto(%d,%p,%d,%d,%p,%zd) %s\n",
            L, talk, buff, size, 0, &addr, sizeof(sockaddr_in),
            formatSockaddr(addr));

   #ifdef SCDM
     debugf("Send: %s: %s\n", formatSockaddr(addr), buff);
   #endif

   return L;
}
}; // class PseudoThread

//----------------------------------------------------------------------------
//
// Class-
//       ClientThread
//
// Purpose-
//       Drive the client.
//
//----------------------------------------------------------------------------
class ClientThread : public PseudoThread
{
//----------------------------------------------------------------------------
// ClientThread::Methods
//----------------------------------------------------------------------------
public:
virtual
   ~ClientThread( void )
{
}

   ClientThread(
     const char*       name)
:  PseudoThread(name)
{
}

virtual void*
   operate( void );
}; // class ClientThread

//----------------------------------------------------------------------------
//
// Class-
//       ServerThread
//
// Purpose-
//       Drive the server.
//
//----------------------------------------------------------------------------
class ServerThread : public PseudoThread
{
//----------------------------------------------------------------------------
// ServerThread::Methods
//----------------------------------------------------------------------------
public:
virtual
   ~ServerThread( void )
{
}

   ServerThread(
     const char*       name)
:  PseudoThread(name)
{
}

virtual void*
   operate( void );
}; // class ServerThread

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
// Parameters
static ClientThread    client("client"); // Client PseudoThread
static ServerThread    server("server"); // Server PseudoThread

// Switches and controls
static Signal          handler;     // (Default) signal handler
static int             swClient;    // Is this the client?
static int             swDebug;     // Is this the debug version?

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
static const char*     msgList[]=   // Message list
   { "First message of 5"
   , "Second of 5"
   , "Third of 5"
   , "Fourth of 5"
   , "Final of 5"
   , NULL
   };

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
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       threadStarter
//
// Purpose-
//       The function that actually starts a thread
//
//----------------------------------------------------------------------------
static void*                        // Return value
   threadStarter(                   // Sample thread
     void*             parm)        // -> PseudoThread
{
   return ((PseudoThread*)parm)->operate();
}

//----------------------------------------------------------------------------
//
// Method-
//       PseudoThread::start
//
// Purpose-
//       Start a PseudoThread
//
//----------------------------------------------------------------------------
void
   PseudoThread::start( void )
{
   pthread_attr_t      attrs;       // Thread attributes
   int                 rc;

   rc= pthread_attr_init(&attrs);   // Initialize the attributes
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= pthread_attr_init\n", rc);

   pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE);
   rc= pthread_create(&tid, &attrs, threadStarter, this);
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= pthread_create\n", rc);

   pthread_attr_destroy(&attrs);    // Destroy the attributes
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
   fprintf(stderr, "{client|server}\n");
   fprintf(stderr, "runtime (seconds)\n");
   fprintf(stderr, "client port number\n");
   fprintf(stderr, "server port number\n");
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
   clientPort= 12345;
   serverPort= 54321;
   swClient= (-1);
   swDebug= FALSE;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp(argv[j], "-help") == 0 )
         error= TRUE;

       else if( strcmp(argv[j], "--") == 0 )
         break;

       else
       {
         for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'd':              // -d (debug)
               swDebug= TRUE;
               break;

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
           if( strcmp(argv[j], "client") == 0 )
             swClient= TRUE;

           else if( strcmp(argv[j], "server") == 0 )
             swClient= FALSE;

           else
           {
             error= TRUE;
             fprintf(stderr, "Not client or server: '%s'\n", argv[j]);
           }
           break;

         case 1:
           activeTime= atol(argv[j]);
           break;

         case 2:
           clientPort= atol(argv[j]);
           break;

         case 3:
           serverPort= atol(argv[j]);
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
   if( swClient < 0 )
   {
     error= TRUE;
     fprintf(stderr, "Must specify client or server\n");
   }

   if( error )
     info(argv[0]);

   if( verify )
   {
     fprintf(stderr, "%s\n", swClient ? "Client" : "Server");
     fprintf(stderr, "clientPort: %d\n", clientPort);
     fprintf(stderr, "serverPort: %d\n", serverPort);
     fprintf(stderr, "activeTime: %d\n", activeTime);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::operate
//
// Purpose-
//       Client driver.
//
//----------------------------------------------------------------------------
void*                               // Return item
   ClientThread::operate( void )    // Client driver
{
   char                buffer[32];  // Response buffer
   int                 L;           // Response length
   const char*         message;     // -> Message
   sockaddr_in         recvFr;      // Partner's socket address information
   sockaddr_in         sendTo;      // Partner's socket address information
   int                 talk;        // Socket handle

   int                 i;

   // Hello
   debugf("%4d: Client started\n", __LINE__);
   talk= contact(clientPort);
   debugf("Connection(%d) opened\n", talk);

   memset(&sendTo, 0, sizeof(sendTo));
   sendTo.sin_family = AF_INET;
   sendTo.sin_addr.s_addr = htonl(INET_ADDR);
   sendTo.sin_port= htons(serverPort);

   // Operate
   for(i= 0; ; i++)
   {
     message= msgList[i];
     if( message == NULL )
       break;

     transmit(talk, sendTo, message);
     if( !swOnline )
       break;

     for(;;)
     {
       L= receive(talk, recvFr, buffer, sizeof(buffer));
       if( L > 0 || !swOnline )
         break;
       sleep(0);
     }
   }

   // Goodbye
   debugf("Connection(%d) closed\n", talk);
   close(talk);
   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::operate
//
// Purpose-
//       Server driver.
//
//----------------------------------------------------------------------------
void*                               // Return item
   ServerThread::operate( void )    // Server driver
{
   char                buffer[512]; // Message buffer
   int                 L;           // Response length
   sockaddr_in         recvFr;      // Partner's socket address information
// sockaddr_in         sendTo;      // Partner's socket address information
   int                 talk;        // Socket handle

   // Hello
   debugf("%4d: Server started\n", __LINE__);
   talk= contact(serverPort);
   debugf("Connection(%d) opened\n", talk);

   // Operate
   for(;;)
   {
     for(;;)
     {
       L= receive(talk, recvFr, buffer, sizeof(buffer));
       if( L > 0 || !swOnline )
         break;
       sleep(1);
     }
     if( !swOnline )
       break;

     transmit(talk, recvFr, "OK");
   }

   // Goodbye
   debugf("Connection(%d) closed\n", talk);
   close(talk);
   return nullptr;
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
   //-------------------------------------------------------------------------
   // Parameter validation
   //-------------------------------------------------------------------------
   init();
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Process the function
   //-------------------------------------------------------------------------
   if( swClient )
     client.operate();
   else
     server.operate();

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   term();
   return 0;
}

