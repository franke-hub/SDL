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
//       sample.cpp
//
// Purpose-
//       Sample socket usage.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <com/Clock.h>
#include <com/Debug.h>
#include <com/define.h>
#include <com/Memory.h>
#include <com/Signal.h>
#include <com/Thread.h>

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/timeb.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "SAMPLE  " // Source file, for debugging

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
// Constants for parameterization
//----------------------------------------------------------------------------
#define FC_XMIT              "XMIT" // Function: transmit file
#define MAX_XFER         0x00010000 // Maximum transfer size

//----------------------------------------------------------------------------
//
// Class-
//       CommonThread
//
// Purpose-
//       Common Thread attributes
//
//----------------------------------------------------------------------------
class CommonThread : public Thread
{
//----------------------------------------------------------------------------
// CommonThread::Attributes
//----------------------------------------------------------------------------
public:
const char*            name;        // The associated name
}; // class CommonThread

//----------------------------------------------------------------------------
//
// Class-
//       ClientThread
//
// Purpose-
//       Drive the client.
//
//----------------------------------------------------------------------------
class ClientThread : public CommonThread
{
//----------------------------------------------------------------------------
// ClientThread::Methods
//----------------------------------------------------------------------------
public:
virtual long
   run();
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
class ServerThread : public CommonThread
{
//----------------------------------------------------------------------------
// ServerThread::Methods
//----------------------------------------------------------------------------
public:
virtual long
   run();
}; // class ServerThread

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
// Parameters
static int             port;        // Connection port number

// Switches and controls
static Signal          signalHandler; // Signal handler
static ClientThread    client;      // Client thread
static ServerThread    server;      // Server thread

// Switches and controls
static int             debug;       // Debugging control
static int             verbose;     // Verbosity control
static int             swOnline;    // TRUE when connection is active

//----------------------------------------------------------------------------
//
// Subroutine-
//       logf
//
// Purpose-
//       Write message to log.
//
//----------------------------------------------------------------------------
static void
   vlogf(                           // Write log message
     const char*       fmt,         // PRINTF format descriptor
     va_list           argptr)      // PRINTF arguments
{
   char                buffer[512]; // Accumulator buffer
   Thread*             thread;      // Current thread
   const char*         threadName;  // Thread name

   thread= Thread::current();
   threadName= __SOURCE__;
   if( thread == &client || thread == &server )
     threadName= ((CommonThread*)thread)->name;

   vsprintf(buffer, fmt, argptr);   // Format the message
   fprintf(stderr, "%s: %s", threadName, buffer);
   fflush(stderr);
}

static void
   logf(                            // Write log message
     const char*       fmt,         // PRINTF format descriptor
                       ...)         // PRINTF argruments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vlogf(fmt, argptr);              // Write to log
   va_end(argptr);                  // Close va_ functions
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
static void
   shouldNotOccur(                  // Write log message and exit
     int               lineno,      // Line number
     const char*       fmt,         // PRINTF format descriptor
                       ...)         // PRINTF argruments
{
   va_list             argptr;      // Argument list pointer
   char                buffer[512]; // Accumulator buffer

   va_start(argptr, fmt);           // Initialize va_ functions
   vsprintf(buffer, fmt, argptr);   // Write to buffer
   va_end(argptr);                  // Close va_ functions

   logf("%4d: %s: ", lineno, buffer);
   perror("should not occur");

   exit(EXIT_FAILURE);
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
   fprintf(stderr, "%s function <options>\n", sourceName);
   fprintf(stderr, "\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "-d\tDebugging\n");
   fprintf(stderr, "-verbose:number\tDebugging verbosity\n");
   fprintf(stderr, "-port:number\tPort number (default 65025)\n");
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
   int                 error;       // TRUE if error encountered
   int                 verify;      // TRUE if verify required

   int                 i, j;

   //-------------------------------------------------------------------------
   // Set defaults
   //-------------------------------------------------------------------------
   error= FALSE;                    // Default, no error found
   verify= FALSE;

   port= 65025;
   debug= FALSE;
   verbose= 0;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp(argv[j], "-help") == 0 )
         error= TRUE;

       else if( memcmp(argv[j], "-port:", 6) == 0 )
         port= atol(argv[j]+6);

       else if( memcmp(argv[j], "-verbose:", 9) == 0 )
         verbose= atol(argv[j]+9);

       else
       {
         for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'd':              // -d (debug)
               debug= TRUE;
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
       error= TRUE;
       fprintf(stderr, "Invalid parameter: '%s'\n", argv[j]);
     }
   }

   //-------------------------------------------------------------------------
   // Check and validate
   //-------------------------------------------------------------------------
   if( debug && verbose == 0 )
     verbose= 1;
   if( verbose )
     debug= TRUE;

   if( error )
     info(argv[0]);

   if( verify || debug )
   {
     fprintf(stderr, "  -debug: %d\n", debug);
     fprintf(stderr, "-verbose: %d\n", verbose);
     fprintf(stderr, "   -port: %d\n", port);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       setFlush
//
// Purpose-
//       Flush socket option
//
//----------------------------------------------------------------------------
extern void
   setFlush(                        // Flush a Socket option
     int               talk)        // The socket handle
{
#if 0
   FILE*               file;
   int                 rc;

   file= &_iob[talk];

logf("%p= FILE(%d)\n", file, talk);

   rc= fflush(file);
logf("%d= fflush()\n", rc);
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       setOption
//
// Purpose-
//       Set a socket option
//
//----------------------------------------------------------------------------
extern void
   setOption(                       // Set a Socket option
     int               talk,        // The socket handle
     int               so,          // The option to set
     int               value)       // The value to set
{
   struct linger       linger;      // Linger option value
   int                 optval;      // Other option value

   int                 rc;

   if( so == SO_LINGER )
   {
     linger.l_onoff=  TRUE;
     linger.l_linger= value;

     rc= ::setsockopt(talk, SOL_SOCKET, SO_LINGER,
                      (const char*)&linger, sizeof(linger));
   }
   else
   {
     optval= value;
     rc= ::setsockopt(talk, SOL_SOCKET, so,
                      (const char*)&optval, sizeof(optval));
   }

   logf("%d= ::setsockopt(%d, SOL_SOCKET, 0x%x, %d)\n",
        rc, talk, so, value);

   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= ::setsockopt(%d, SOL_SOCKET, 0x%x, %d)\n",
                              rc, talk, so, value);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       recvLine
//
// Purpose-
//       Receive a string message.
//
//----------------------------------------------------------------------------
int                                 // Number of bytes read
   recvLine(                        // Receive a string message
     int             talk,          // Socket handle
     char*           addr,          // Message address
     unsigned        size)          // Maximum message length
{
   int               L;             // Read length
   int               used;          // Number of bytes read

   used= 0;
   while( used < size )             // Read the string
   {
     L= recv(talk, &addr[used], 1, 0); // Read the next character
     #ifdef IODM
       debugf("%d= recv(%d)\n", L, talk);
     #endif
     if( L < 0 )
       shouldNotOccur(__LINE__, "%d= recv(%d)", L, talk);

     if( L < 1 )
     {
       logf("Connection(%d) closed\n", talk);
       swOnline= FALSE;
       addr[0]= '\0';
       return 0;
     }

     if( addr[used] == '\0' )       // If string terminator
     {
       if( verbose > 1 ) logf("recvLine: '%s'\n", addr);
       return used;                 // Exit, function complete
     }

     used++;
   }

   shouldNotOccur(__LINE__, "recvLine error: String(%d) overflow", used);
   return used;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sendLine
//
// Purpose-
//       Send a string message.
//
//----------------------------------------------------------------------------
int                                 // Number of bytes sent
   sendLine(                        // Send a message
     int               talk,        // Socket handle
     const char*       addr)        // Message address
{
   int                 L;           // Write length
   int                 size;        // Message length
   int                 sent;        // Number of bytes sent

   if( verbose > 1 ) logf("sendLine: '%s'\n", addr);

   size= strlen(addr) + 1;
   sent= 0;
   while( sent < size )             // Write the string
   {
     L= send(talk, addr+sent, size-sent, 0); // Write the string
     #ifdef IODM
       debugf("%d= send(%d,%d)\n", L, talk, size-sent);
     #endif
     if( L < 0 )
       shouldNotOccur(__LINE__, "%d= send(%d)", L, talk);

     if( L < 1 )
       shouldNotOccur(__LINE__, "%d= send(%d)", L, talk);

     addr+= L;
     sent+= L;
   }

   return sent;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::run
//
// Purpose-
//       Client driver.
//
//----------------------------------------------------------------------------
long                                // Return code
   ClientThread::run( void )        // Client driver
{
   sockaddr_in         addr;        // Socket address information
   char*               buffer;      // Working buffer
   double              now;         // Ending time
   int                 left;        // Remaining transfer length
   int                 talk;        // Socket handle
   double              then;        // Start time
   struct stat         s;           // File stats

   int                 rc;

   // Transfer buffer
   buffer= (char*)Memory::allocate(MAX_XFER);

   // Create the talk socket
   talk= socket(AF_INET, SOCK_STREAM, 0); // Create the socket
   if( talk < 0)                    // Socket not created
     shouldNotOccur(__LINE__, "%d= socket()", talk);

   memset(&addr,0,sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(0x7f010101);
   addr.sin_port= htons(port);

   rc= connect(talk, (sockaddr*)&addr, sizeof(addr));
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= connect()", rc);
   logf("Connection(%d) opened\n", talk);

   // Drive the socket
   then= Clock::current();
   for(int i= 0; i<1000; i++)
   {
     rc= lstat(name, &s);
     if( rc != 0 )
       shouldNotOccur(__LINE__, "%d= lstat(%s) failure", rc);

     left= s.st_size;
     sprintf(buffer, "%s %d", FC_XMIT, left);
     sendLine(talk, buffer);
     setFlush(talk);
     recvLine(talk, buffer, MAX_XFER);
     if( strcmp(buffer, "OK") != 0 )
       shouldNotOccur(__LINE__, "Response: %s", buffer);

     FILE* file= fopen(name, "rb");
     if( file == NULL )
       shouldNotOccur(__LINE__, "fopen(%s)", name);

     while( left > 0 )
     {
       int size= left;
       if( size > MAX_XFER )
         size= MAX_XFER;
       int L= fread(buffer, 1, size, file);
       if( L != size )
         shouldNotOccur(__LINE__, "%d= fread(%s)", L, name);

       L= send(talk, buffer, size, 0); // Write the buffer
       if( L != size )
         shouldNotOccur(__LINE__, "%d= send(%d)", L, size);

       left -= L;
     }

     fclose(file);
     setFlush(talk);
   }
   now= Clock::current();
   logf("Elapsed: %.3f\n", now-then);

   Thread::sleep(1);
   close(talk);
   logf("Connection(%d) closed\n", talk);
   Memory::release(buffer, MAX_XFER);
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::run
//
// Purpose-
//       Server driver.
//
//----------------------------------------------------------------------------
long                                // Return code
   ServerThread::run( void )        // Server driver
{
   sockaddr_in         addr;        // Socket address information
   char*               buffer;      // Message buffer
   socklen_t           L;           // Generic int
   int                 left;        // Number of bytes remaining
   int                 list;        // Socket handle
   int                 talk;        // Socket handle

   int                 rc;

   // Transfer buffer
   buffer= (char*)Memory::allocate(MAX_XFER);

   // Create the listener socket
   list= socket(AF_INET, SOCK_STREAM, 0); // Create the listener socket
   if( list < 0)                    // Socket not created
     shouldNotOccur(__LINE__, "%d= socket()", list);

   memset(&addr,0,sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = INADDR_ANY;
// addr.sin_port= 0;                // Let system pick the port
   addr.sin_port= htons(port);      // We pick the port

   rc= bind(list, (sockaddr*)&addr, sizeof(addr));
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= bind()", rc);

   L=sizeof(addr);
   rc= getsockname(list, (sockaddr*)&addr, &L);
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= getsockname()", rc);

// port= ntohs(addr.sin_port);      // Save host form of port

   // Start listening
   rc= listen(list, 1);
   if( rc < 0 )
     shouldNotOccur(__LINE__, "%d= listen()", rc);

   // Accept the next connection
   swOnline= TRUE;
   logf("Ready\n");
   L= sizeof(addr);
   talk= accept(list, (sockaddr*)&addr, &L);
   if( talk < 0 )
     shouldNotOccur(__LINE__, "%d= listen(%d)", talk, list);

   logf("Connection(%d) opened\n", talk);

   // Set large buffer size
// setOption(talk, SO_RCVBUF, MAX_XFER); // ** DOES NOT WORK AS EXPECTED **

   // Handle messages
   for(;;)
   {
     recvLine(talk, buffer, MAX_XFER);
     if( !swOnline )
       break;

     if( memcmp(buffer,FC_XMIT " ",sizeof(FC_XMIT)) != 0 )
       shouldNotOccur(__LINE__, "Command: '%s'", buffer);
     sendLine(talk, "OK");
     left= atoi(buffer+sizeof(FC_XMIT));

     while( left > 0 )
     {
       int size= left;
       if( size > MAX_XFER )
         size= MAX_XFER;

       L= recv(talk, buffer, size, 0); // Read the buffer
       if( L <= 0 )
         shouldNotOccur(__LINE__, "%d= recv(%d)", L, size);

       if( verbose > 5 ) logf("%d= recv(%d)\n", L, size);

       left -= L;
     }
   }

   Memory::release(buffer, MAX_XFER);
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
   //-------------------------------------------------------------------------
   // Parameter validation
   //-------------------------------------------------------------------------
   init();
   parm(argc, argv);

   //-------------------------------------------------------------------------
   // Process the function
   //-------------------------------------------------------------------------
   client.name= "Client";
   server.name= "Server";

   server.start();
   Thread::sleep(1);
   client.start();

   client.wait();
   server.wait();

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   term();
   return 0;
}

