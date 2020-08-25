//----------------------------------------------------------------------------
//
//       Copyright (C) 2019 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       SpeedTest.cpp
//
// Purpose-
//       Network speed test.
//
// Last change date-
//       2019/04/27
//
// Implementation notes-
//       Setting SO_SNDBUF and SO_RCVBUF significantly slows down Linux.
//
// Timings-
//       907.4 MB/sec Linux   [Internal]
//        85.6 MB/sec Windows [Internal]
//
//----------------------------------------------------------------------------
#include <atomic>

#include <assert.h>
#include <errno.h>
#include <netdb.h>
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

#include <com/Random.h>
#include <pub/Clock.h>
#include <pub/Debug.h>
#include <pub/Named.h>
#include <pub/Semaphore.h>
#include <pub/Thread.h>

using namespace _PUB_NAMESPACE;     // For pub:: classes
using namespace _PUB_NAMESPACE::debugging; // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

#include <pub/ifmacro.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       logf
//
// Purpose-
//       Write message to log.
//
//----------------------------------------------------------------------------
static inline void
   vlogf(                           // Write log message
     const char*       fmt,         // PRINTF format descriptor
     va_list           argptr)      // PRINTF arguments
{
   char                buffer[512]; // Accumulator buffer
   Thread*             thread;      // Current thread
   const char*         threadName;  // Thread name

   thread= Thread::current();
   threadName= "*Main*";
   Named* named= dynamic_cast<Named*>(thread);
   if( named != nullptr )
     threadName= named->get_name().c_str();

   vsprintf(buffer, fmt, argptr);   // Format the message
   fprintf(stderr, "%s: %s", threadName, buffer);
   fflush(stderr);
}

static inline void
   logf(                            // Write log message
     const char*       fmt,         // PRINTF format descriptor
                       ...)         // PRINTF argruments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vtraceh(fmt, argptr);
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

   fprintf(stderr, "%4d: %s: ", lineno, buffer);
   perror("should not occur");

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Class-
//       BufferPool
//
// Purpose-
//       Maintain a list of transmit/receive buffers
//
//----------------------------------------------------------------------------
class BufferPool {
//----------------------------------------------------------------------------
// BufferPool::Attributes
//----------------------------------------------------------------------------
enum { DIM= 16, SIZE= 10000 }; // Number of buffers in the pool and their size
Semaphore              freeSem;     // Counts number of free buffers
Semaphore              initSem;     // Counts number of initialized buffers
char                   pool[DIM][SIZE]; // The pool of buffers
std::atomic<uint32_t>  freeUF;      // The free Used/Free buffer indexes
std::atomic<uint32_t>  initUF;      // Initialized Used/Free buffer indexes

//----------------------------------------------------------------------------
// BufferPool::Destructor/Constructor
//----------------------------------------------------------------------------
public:
   ~BufferPool( void )
{  }

   BufferPool( void )
:  freeSem(DIM), initSem(0), freeUF(DIM), initUF(0)
{  }

//----------------------------------------------------------------------------
// BufferPool::Methods
//----------------------------------------------------------------------------
public:
static int                          // The number of buffer pool elements
   get_DIM( void )                  // DIM accessor
{  return DIM; }

static int                          // The size of each buffer pool element
   get_SIZE( void )                 // SIZE accessor
{  return SIZE; }

char*                               // The next available buffer
   free_get( void )                 // Allocate next free buffer
{
   freeSem.wait();

   uint32_t oldV= freeUF.load();    // Current used index, free count
   uint32_t newV;
   for(;;) {
     if( (oldV & 0x0000ffff) == 0 ) // If just posted, no data
       return nullptr;              // Return no data

     newV= oldV + 0x0000ffff;       // Increment used, decrement free
     if( (newV >> 16) >= DIM )      // If overflow
       newV &= 0x0000ffff;          // Set used index to 0

     if( freeUF.compare_exchange_weak(oldV, newV) )
       break;
   }

   newV >>= 16;                     // Get used index (+1)
   if( newV == 0 )
     newV= DIM;
   IFHCDM( logf("Pool(%p).free_get: oldV(0x%.8x) newV(0x%.8x)\n",
                this, oldV, newV); )
   return pool[newV - 1];           // Return buffer
}

void
   free_put( void )                 // Make a free buffer available
{
   uint32_t oldV= freeUF.load();    // Current used index, free count
   for(;;) {
     if( (oldV & 0x0000ffff) >= DIM ) {
       debugh("%4d ERROR oldV(%d)\n", __LINE__, oldV);
       return;
     }

     if( freeUF.compare_exchange_weak(oldV, oldV+1) )
       break;
   }

   freeSem.post();
   IFHCDM( logf("Pool(%p).free_put: oldV(0x%.8x) newV(0x%.8x)\n",
                this, oldV, oldV+1); )
}

char*                               // The next available buffer
   init_get( void )                 // Allocate next initialized buffer
{
   initSem.wait();

   uint32_t oldV= initUF.load();    // Current used index, free count
   uint32_t newV;
   for(;;) {
     if( (oldV & 0x0000ffff) == 0 ) // If just posted, no data
       return nullptr;              // Return no data

     newV= oldV + 0x0000ffff;       // Increment used, decrement free
     if( (newV >> 16) >= DIM )      // If overflow
       newV &= 0x0000ffff;          // Set used index to 0

     if( initUF.compare_exchange_weak(oldV, newV) )
       break;
   }

   newV >>= 16;                     // Get used index (+1)
   if( newV == 0 )
     newV= DIM;
   IFHCDM( logf("Pool(%p).init_get: oldV(0x%.8x) newV(0x%.8x)\n",
                this, oldV, newV); )
   return pool[newV - 1];           // Return buffer
}

void
   init_put( void )                 // Make a free buffer available
{
   uint32_t oldV= initUF.load();    // Current used index, free count
   for(;;) {
     assert( (oldV & 0x0000ffff) < DIM );
     if( initUF.compare_exchange_weak(oldV, oldV+1) )
       break;
   }

   initSem.post();
   IFHCDM( logf("Pool(%p).init_put: oldV(0x%.8x) newV(0x%.8x)\n",
                this, oldV, oldV+1); )
}

void
   post( void )                     // Post the semaphore(s)
{  IFHCDM( traceh("Pool(%p).post\n", this); )
   freeSem.post(); initSem.post(); }
}; // class BufferPool

//----------------------------------------------------------------------------
//
// Class-
//       NamedThread
//
// Purpose-
//       Named Thread attributes
//
//----------------------------------------------------------------------------
class NamedThread : public Thread, public Named {
//----------------------------------------------------------------------------
// NamedThread::Constructors
//----------------------------------------------------------------------------
public:
   NamedThread(                     // A Named Thread
     const char*       name)        // The associated name
:  Thread(), Named(name) {}
}; // class NamedThread

//----------------------------------------------------------------------------
//
// Class-
//       Producer
//
// Purpose-
//       The thread that produces the buffers
//
//----------------------------------------------------------------------------
class Producer : public NamedThread {
//----------------------------------------------------------------------------
// Producer::Attributes
//----------------------------------------------------------------------------
public:
BufferPool             pool;        // The buffer pool, kept full
Random                 rand;        // Random number generator

//----------------------------------------------------------------------------
// Producer::Constructors
//----------------------------------------------------------------------------
public:
   Producer( void )
:  NamedThread("*Prod*")
,  pool(), rand()
{
   for(int i= 0; i<pool.get_DIM(); i++) {
     char* buffer= pool.free_get();
     produce(buffer);
     pool.init_put();
   }
}

//----------------------------------------------------------------------------
// Producer::Methods
//----------------------------------------------------------------------------
public:
void
   produce(                         // Fill a buffer with random data
     char*             buffer)      // (The fill buffer)
{
   int size= pool.get_SIZE() >> 2;  // Buffer size / 4
   uint32_t* iBuffer= (uint32_t*)buffer; // Use integer filler
   for(int i= 0; i<size; i++)
     iBuffer[i]= rand.get();
}

virtual void
   run( void );

virtual void
   stop( void )
{  IFHCDM( traceh("Producer(%p).stop\n", this); )
   pool.post();
}
}; // class Producer

//----------------------------------------------------------------------------
//
// Class-
//       Verifier
//
// Purpose-
//       The thread that verifies the buffers
//
//----------------------------------------------------------------------------
class Verifier : public NamedThread {
//----------------------------------------------------------------------------
// Verifier::Attributes
//----------------------------------------------------------------------------
public:
BufferPool             pool;        // The buffer pool, kept full
Random                 rand;        // Random number generator
size_t                 count;       // Number of buffers received

//----------------------------------------------------------------------------
// Verifier::Constructors
//----------------------------------------------------------------------------
public:
   Verifier( void )
:  NamedThread("*Veri*")
,  pool(), rand(), count(0)
{  }

//----------------------------------------------------------------------------
// Verifier::Methods
//----------------------------------------------------------------------------
public:
int                                 // Return code, 0 OK
   verify(                          // Verify buffer data
     char*             buffer)      // (The fill buffer)
{
   int size= pool.get_SIZE() >> 2; // Buffer size / 4
   uint32_t* iBuffer= (uint32_t*)buffer; // Use integer filler
   for(int i= 0; i<size; i++) {
     uint32_t next= rand.get();   // The next random number
     if( iBuffer[i] != next ) {
       fprintf(stderr, "Buffer[%zd][%d] expected(0x%.8x) got(0x%.8x)\n",
                       count, i, next, iBuffer[i]);
       return 1;
     }
   }

   count++;
   return 0;
}

virtual void
   run( void );

virtual void
   stop( void )
{  IFHCDM( traceh("Verifier(%p).stop\n", this); )
   pool.post();
}
}; // class Verifier

//----------------------------------------------------------------------------
//
// Class-
//       RecvThread
//
// Purpose-
//       Drive the receiver Thread
//
//----------------------------------------------------------------------------
class RecvThread : public NamedThread {
//----------------------------------------------------------------------------
// RecvThread::Attributes
//----------------------------------------------------------------------------
public:
size_t                 count;       // Number of buffers received
int                    fsm;         // Finite State Machine

//----------------------------------------------------------------------------
// RecvThread::Constructors
//----------------------------------------------------------------------------
public:
   RecvThread( void )
:  NamedThread("*Recv*"), count(0), fsm(0) {}

//----------------------------------------------------------------------------
// RecvThread::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debug( void )
{
   debugf("RecvThread(%p)::debug() fsm(%d)\n", this, fsm);
}

virtual void
   run( void );

virtual void
   stop( void );
}; // class RecvThread

//----------------------------------------------------------------------------
//
// Class-
//       XmitThread
//
// Purpose-
//       Drive the transmit client.
//
//----------------------------------------------------------------------------
class XmitThread : public NamedThread {
//----------------------------------------------------------------------------
// XmitThread::Attributes
//----------------------------------------------------------------------------
public:
size_t                 count;       // Number of buffers transmitted
int                    fsm;         // Finite State Machine

//----------------------------------------------------------------------------
// XmitThread::Constructors
//----------------------------------------------------------------------------
public:
   XmitThread( void )
:  NamedThread("*Xmit*"), count(0), fsm(0) {}

//----------------------------------------------------------------------------
// XmitThread::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debug( void )
{
   debugf("XmitThread(%p)::debug() fsm(%d)\n", this, fsm);
}

virtual void
   run( void );

virtual void
   stop( void );
}; // class XmitThread

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
// Parameters
static int             port;        // Connection port number

// Switches and controls
static Verifier        verifier;    // Verifier Thread
static Producer        producer;    // Producer Thread
static RecvThread      rThread;     // Receiver Thread
static XmitThread      xThread;     // Transmitter Thread

// Switches and controls
static char*           command= nullptr; // "xmit | recv", default both
static int             debug;       // Debugging control
static int             verbose;     // Verbosity control
static int             swOnline;    // TRUE when connection is active

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
     linger.l_onoff=  true;
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

   IFIODM( logf("%d= ::setsockopt(%d, SOL_SOCKET, 0x%x, %d)\n",
                rc, talk, so, value); )

   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= ::setsockopt(%d, SOL_SOCKET, 0x%x, %d)\n",
                              rc, talk, so, value);
}

//----------------------------------------------------------------------------
//
// Method-
//       Verifier::run
//
// Purpose-
//       Drive the Verify Thread
//
//----------------------------------------------------------------------------
void
   Verifier::run( void )
{
   while( swOnline ) {
     char* buffer= pool.init_get();
     if( swOnline ) {
       if( buffer == nullptr )
         shouldNotOccur(__LINE__, "buffer(%p)", buffer);

       if( verify(buffer) )
         swOnline= false;

       pool.free_put();
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Producer::run
//
// Purpose-
//       Drive the Push Thread
//
//----------------------------------------------------------------------------
void
   Producer::run( void )
{
   while( swOnline ) {
     char* buffer= pool.free_get();
     if( swOnline ) {
       if( buffer == nullptr )
         shouldNotOccur(__LINE__, "buffer(%p)", buffer);

       produce(buffer);
     }
     pool.init_put();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       RecvThread::run
//
// Purpose-
//       Drive Recv Thread
//
//----------------------------------------------------------------------------
void
   RecvThread::run( void )
{
   sockaddr_in         addr;        // Socket address information
   socklen_t           L;           // Generic int
   int                 list;        // Socket handle
   int                 talk;        // Socket handle

   int                 rc;

   // Create the listener socket
   list= socket(AF_INET, SOCK_STREAM, 0); // Create the listener socket
   if( list < 0)                    // Socket not created
     shouldNotOccur(__LINE__, "%d= socket()", list);

   memset(&addr,0,sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = INADDR_ANY;
   addr.sin_port= htons(port);      // We pick the port

   rc= bind(list, (sockaddr*)&addr, sizeof(addr));
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= bind()", rc);

   L=sizeof(addr);
   rc= getsockname(list, (sockaddr*)&addr, &L);
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= getsockname()", rc);

   // Start listening
   rc= listen(list, 1);
   if( rc < 0 )
     shouldNotOccur(__LINE__, "%d= listen()", rc);

   char hostName[512];              // The host name
   gethostname(hostName, sizeof(hostName));
   debugf("%s:%d Ready\n", hostName, port);

   // Accept the next connection
   L= sizeof(addr);
   talk= accept(list, (sockaddr*)&addr, &L);
   if( talk < 0 )
     shouldNotOccur(__LINE__, "%d= listen(%d)", talk, list);

   // Set large buffer size
   size_t size= BufferPool::get_SIZE(); // Get the buffer size
// setOption(talk, SO_RCVBUF, size);

   IFHCDM( logf("Connection(%d) opened\n", talk); )

   // Drive the socket
   while( swOnline ) {
     char* buffer= verifier.pool.free_get();
     if( swOnline ) {
       if( buffer == nullptr ) {
         swOnline= false;
         shouldNotOccur(__LINE__, "buffer(%p)");
       }

       int left= size;
////   logf("recv...\n");
       while( left > 0 ) {
         int L= recv(talk, buffer+size-left, left, 0); // Receive the buffer
         if( L <= 0 ) {
           swOnline= false;
           debugf("%4d ERROR: %d= recv(%d)\n", __LINE__, L, left);
           break;
         }

         left -= L;
////     if( left != 0 ) logf("...%3d...\n", L);
       }
////   logf("...recv\n");
       count++;
     }

     verifier.pool.init_put();
   }

   close(talk);
   IFHCDM( logf("Connection(%d) closed\n", talk); )
// close(list);
}

//----------------------------------------------------------------------------
//
// Method-
//       RecvThread::stop
//
// Purpose-
//       Stop the Recv Thread
//
//----------------------------------------------------------------------------
void
   RecvThread::stop( void )
{  IFHCDM( traceh("RecvThread(%p).stop\n", this); )
   verifier.pool.post(); }

//----------------------------------------------------------------------------
//
// Method-
//       XmitThread::run
//
// Purpose-
//       Drive Xmit Thread
//
//----------------------------------------------------------------------------
void
   XmitThread::run( void )
{
   sockaddr_in         addr;        // Socket address information
   const char*         server;      // The server name
   int                 talk;        // Socket handle

   int                 rc;

   // Create the talk socket
   talk= socket(AF_INET, SOCK_STREAM, 0); // Create the socket
   if( talk < 0)                    // Socket not created
     shouldNotOccur(__LINE__, "%d= socket()", talk);

   server= "localhost";             // Default, localhost
   if( command != nullptr )         // If not null, must be xmit:servername
     server= command+5;             // Get the servername
   struct hostent* hostent= gethostbyname(server); // Get host entry
   if( hostent == nullptr ) {       // If cannot locate
     debugf("Cannot locate host '%s'\n", server);
     return;
   }

   memset(&addr,0,sizeof(addr));
   addr.sin_family= AF_INET;
// addr.sin_addr.s_addr= htonl(0x7f010101);
   addr.sin_addr.s_addr= *(in_addr_t*)hostent->h_addr;
   addr.sin_port= htons(port);
int Q= (int)ntohl(*(in_addr_t*)hostent->h_addr);
debugf("h_addr: %d.%d.%d.%d\n", (Q>>24)&0x00ff, (Q>>16)&0x00ff, (Q>>8)&0x00ff, Q&0x00ff);

   rc= connect(talk, (sockaddr*)&addr, sizeof(addr));
   if( rc != 0 )
     shouldNotOccur(__LINE__, "%d= connect()", rc);
   IFHCDM( logf("Connection(%d) opened\n", talk); )

   // Drive the socket
   size_t size= BufferPool::get_SIZE(); // Get the buffer size
// setOption(talk, SO_SNDBUF, size);
   while( swOnline ) {
     char* buffer= producer.pool.init_get();
     if( swOnline ) {
       if( buffer == nullptr ) {
         swOnline= false;
         shouldNotOccur(__LINE__, "buffer(%p)");
       }

////   logf("send...\n");
       int L= send(talk, buffer, size, 0); // Transmit the buffer
////   logf("...send\n");
       if( L != size ) {
         swOnline= false;
         debugh("%4d shouldNotOccur: %d= send(%zd)\n", __LINE__, L, size);
         break;
       }

       count++;
     }

     producer.pool.free_put();
   }

   close(talk);
   IFHCDM( logf("Connection(%d) closed\n", talk); )
}

//----------------------------------------------------------------------------
//
// Method-
//       XmitThread::stop
//
// Purpose-
//       Stop the Xmit Thread
//
//----------------------------------------------------------------------------
void
   XmitThread::stop( void )
{  IFHCDM( traceh("XmitThread(%p).stop\n", this); )
   producer.pool.post(); }

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
   fprintf(stderr, "-port:number\tPort number (default 8080)\n");
   fprintf(stderr, "-recv | -xmit:server-name\n");
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
   error= false;                    // Default, no error found
   verify= false;

   port= 8080;
   debug= false;
   verbose= 0;

   //-------------------------------------------------------------------------
   // Examine parameters
   //-------------------------------------------------------------------------
   for(j=1; j<argc; j++)            // Examine the parameter list
   {
     if( argv[j][0] == '-' )        // If this is a switch list
     {
       if( strcmp(argv[j], "-help") == 0 )
         error= true;

       else if( memcmp(argv[j], "-port:", 6) == 0 )
         port= atol(argv[j]+6);

       else if( strcmp(argv[j], "-recv") == 0 )
         command= argv[j]+1;

       else if( memcmp(argv[j], "-xmit:", 5) == 0 )
         command= argv[j]+1;

       else if( memcmp(argv[j], "-verbose:", 9) == 0 )
         verbose= atol(argv[j]+9);

       else
       {
         for(i=1; argv[j][i] != '\0'; i++) // Examine the switch list
         {
           switch(argv[j][i])       // Examine the switch
           {
             case 'd':              // -d (debug)
               debug= true;
               break;

             case 'h':              // -h (help)
               error= true;
               break;

             case 'v':              // -v (verify)
               verify= true;
               break;

             default:               // If invalid switch
               error= true;
               fprintf(stderr, "Invalid switch '%c'\n", (int)argv[j][i]);
               break;
           }
         }
       }
     }
     else                           // Argument
     {
       error= true;
       fprintf(stderr, "Invalid parameter: '%s'\n", argv[j]);
     }
   }

   //-------------------------------------------------------------------------
   // Check and validate
   //-------------------------------------------------------------------------
   if( debug && verbose == 0 )
     verbose= 1;
   if( verbose )
     debug= true;

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
   // Debugging controls
   //-------------------------------------------------------------------------
   Debug debug;                     // Debugging object
   debug.set_head(Debug::Heading(Debug::HEAD_THREAD | Debug::HEAD_TIME));
// IFHCDM( debug.set_mode(Debug::MODE_INTENSIVE); )
   Debug::set(&debug);

   //-------------------------------------------------------------------------
   // Process the function
   //-------------------------------------------------------------------------
   swOnline= true;
   producer.start();
   verifier.start();

   if( command == nullptr ) {       // If internal(xmit/recv) test
     rThread.start();
     Thread::sleep(1);
   } else if( strcmp(command, "recv") == 0 ) { // If receive-only test
     rThread.start();
     rThread.join();
     exit(EXIT_SUCCESS);
   }

   xThread.start();
   int runtime= 30;
   debugf("SpeedTest running for %d seconds\n", runtime);
   Thread::sleep(runtime);

   size_t count= xThread.count;
   if( swOnline ) {
     count *= BufferPool::get_SIZE();
     double bps= (double)count / (double)runtime;
     debugf("%8.3f Bytes/second\n", bps);
   } else {
     debugf("TEST FAILED\n");
   }

   swOnline= false;
   Thread::sleep(1);
   xThread.stop();
   if( command == nullptr )
     rThread.stop();

   verifier.stop();
   producer.stop();

// IFHCDM( debugf("%4d HCDM\n", __LINE__); )
   verifier.join();
// IFHCDM( debugf("%4d HCDM\n", __LINE__); )
   producer.join();
// IFHCDM( debugf("%4d HCDM\n", __LINE__); )
   xThread.join();
// IFHCDM( debugf("%4d HCDM\n", __LINE__); )
   if( command == nullptr ) {
     rThread.join();
//   IFHCDM( debugf("%4d HCDM\n", __LINE__); )
   }

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   Debug::set(nullptr);
   term();
   return 0;
}
