//----------------------------------------------------------------------------
//
//       Copyright (c) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestSock.cpp
//
// Purpose-
//       Test Socket object.
//
// Last change date-
//       2022/05/27
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE
#define _GNU_SOURCE                 // For ppoll
#endif
#include <atomic>                   // For std::atomic
#include <new>                      // For std::bad_alloc
#include <string>                   // For std::string

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Event.h>              // For pub::Event
#include <pub/Interval.h>           // For pub::Interval
#include <pub/Semaphore.h>          // For pub::Semaphore
#include "pub/Socket.h"             // For pub::Socket, tested
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Worker.h>             // For pub::Worker, pub::WorkerPool, ...

#include "pub/TEST.H"               // For VERIFY, ...
#include "pub/Wrapper.h"            // For class Wrapper

using namespace _PUB_NAMESPACE;     // For pub:: classes
using namespace pub::debugging;     // For debugging functions
using namespace pub::utility;       // For utility`functions
using std::atomic;

#define debugf         pub::debugging::debugf
#define opt_hcdm       pub::Wrapper::opt_hcdm
#define opt_verbose    pub::Wrapper::opt_verbose

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  STD_PORT= 8080                   // Our STD port number

// Debugging options - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
,  USE_CONNECT_RETRY= 0             // Number of retries before error
,  USE_LINGER= true                 // Use SO_LINGER timeout for client
,  USE_STOP_HCDM= false             // Write StreamServer::stop messages

// For packets, the polling operation occurs before each packet op
// For streams, the polling operation occurs before each accept
,  USE_POLL_BLOCK= 0                // Use blocking, don't poll
,  USE_POLL_NONBLOCK= 1             // Use non-blocking, don't poll
,  USE_POLL_POLL= 2                 // Use poll before accept or packet op
,  USE_POLL_SELECT= 3               // Use SocketSelect polling

,  USE_APOLL= USE_POLL_POLL         // Use accept polling method
,  USE_RPOLL= USE_POLL_POLL         // Use recv polling method
,  USE_SPOLL= USE_POLL_POLL         // Use send polling method
}; // Generic enum

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define LOCK_GUARD(x) std::lock_guard<decltype(x)> lock(x)

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static char            obuff[256];  // Packet output buffer
static std::string     peer_addr;   // The server's name:port string
static Event           test_start;  // The test start Event

static int             error_count= 0; // Error counter
static int             retry_count= 0; // Connection retry counter
static int             running= 0;  // Test running indicator

// Packet statistics
static atomic<size_t>  rp_again;    // Number of read EAGAIN retries
static atomic<size_t>  rp_block;    // Number of read EWOULDBLOCK retries
static atomic<size_t>  rp_count;    // Number of read operations completed

static atomic<size_t>  wp_again;    // Number of write EAGAIN retries
static atomic<size_t>  wp_block;    // Number of write EWOULDBLOCK retries
static atomic<size_t>  wp_count;    // Number of write operations completed

// Stream statistics
static atomic<size_t>  rs_again;    // Number of read EAGAIN retries
static atomic<size_t>  rs_block;    // Number of read EWOULDBLOCK retries
static atomic<size_t>  rs_count;    // Number of read operations completed

static atomic<size_t>  ws_again;    // Number of write EAGAIN retries
static atomic<size_t>  ws_block;    // Number of write EWOULDBLOCK retries
static atomic<size_t>  ws_count;    // Number of write operations completed

//----------------------------------------------------------------------------
// Extended options
//----------------------------------------------------------------------------
static int             opt_client= false;
static int             opt_packet=  false;
static int             opt_runtime= 0;
static int             opt_server= false;
static int             opt_stream= false;
static const char*     opt_target= nullptr;
static int             opt_thread= true;  // TODO: DEFAULT FALSE
static int             opt_worker= true;  // TODO: DEFAULT FALSE
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"client",   no_argument,       &opt_client,    true}
,  {"datagram", no_argument,       &opt_packet,    true}
,  {"packet",   no_argument,       &opt_packet,    true}
,  {"runtime",  required_argument, nullptr,           0}
,  {"server",   optional_argument, &opt_server,    true}
,  {"stream",   no_argument,       &opt_stream,    true}
,  {"stress",   no_argument,       &opt_stream,    true}
,  {"thread",   no_argument,       &opt_thread,    true}
,  {"worker",   no_argument,       &opt_worker,    true}
,  {"nothread", no_argument,       &opt_thread,    false} // TODO: REMOVE
,  {"noworker", no_argument,       &opt_worker,    false} // TODO: REMOVE
,  {0, 0, 0, 0}                     // (End of option list)
};

//----------------------------------------------------------------------------
// HTTP responses
//----------------------------------------------------------------------------
static const char*     http400=     // HTTP 400 (INVALID) response
   "HTTP/1.1 400 !INVALID!\r\n"
   "Content-type: text/html\r\n"
   "Content-length: 58\r\n"
   "\r\n"
   "<html>\r\n"                     // 8
   "<body>\r\n"                     // 8
   "<h1>400 !INVALID!</h1>\r\n"     // 24
   "</body>\r\n"                    // 8
   "</html>\r\n"                    // 8
   ;

static const char*     http404=     // HTTP 404 (NOT FOUND) response
   "HTTP/1.1 404 Not Found\r\n"
   "Content-type: text/html\r\n"
   "Content-length: 58\r\n"
   "\r\n"
   "<html>\r\n"                     // 8
   "<body>\r\n"                     // 8
   "<h1>404 NOT FOUND</h1>\r\n"     // 24
   "</body>\r\n"                    // 8
   "</html>\r\n"                    // 8
   ;

static const char*     http200=     // HTTP 200 (NORMAL) response
   "HTTP/1.1 200 OK\r\n"
   "Server: RYO\r\n"
   "Content-type: text/html\r\n"
   "Content-length: 58\r\n"
   "\r\n"
   "<html>\r\n"                     // 8
   "<body>\r\n"                     // 8
   "<h1>Hello, World!</h1>\r\n"     // 24
   "</body>\r\n"                    // 8
   "</html>\r\n"                    // 8
   ;

//----------------------------------------------------------------------------
//
// Subroutine-
//       line
//
// Purpose-
//       Debugging: Easy to remove code tracker.
//
//----------------------------------------------------------------------------
static inline void
   line(int line)
{  debugf("%4d %s HCDM\n", line, __FILE__); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_token
//
// Purpose-
//       Extract next token, "" if at end of line
//
//----------------------------------------------------------------------------
static std::string                  // The next token, "" if at end
   get_token(                       // Get next token
     const char*&       text)
{
   while( *text == ' ' )
     text++;

   const char* origin= text;
   while( *text != ' ' && *text != '\t'
       && *text != '\r' && *text != '\n' && *text != '\0' )
     text++;

   std::string result(origin, text - origin);
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       if_closed
//
// Purpose-
//       Returns TRUE if error due to host or peer closing a socket
//
// Implementation notes-
//       We can be in the middle of polling when a test terminates.
//       This is a somewhat normal condition, not an error.
//
//----------------------------------------------------------------------------
static bool
   if_closed(const struct pollfd& pfd)
{
   if( errno == EBADF )
     return true;                   // (Our socket was closed )

   int mask= POLLHUP;
   #ifdef POLLRDHUP
     mask= (POLLHUP | POLLRDHUP);
   #endif
   if( pfd.revents & mask )
     return true;                   // (Peer socket was closed)

   return false;
}

static bool
   if_closed(const SocketSelect& select, const Socket* socket)
{
   const struct pollfd* pfd= select.get_pollfd(socket);
   if( pfd == nullptr )
     return true;                   // (Our socket was closed )

   int mask= POLLHUP;
   #ifdef POLLRDHUP
     mask= (POLLHUP | POLLRDHUP);
   #endif
   if( pfd->revents & mask )
     return true;                   // (Peer socket was closed)

   return false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       if_retry
//
// Purpose-
//       Returns errno == EAGAIN || errno == EWOULDBLOCK
//
//----------------------------------------------------------------------------
static bool if_retry( void )
#if EAGAIN == EWOULDBLOCK
{  return errno == EAGAIN; }
#else
{  return errno == EAGAIN; || errno == EWOULDBLOCK }
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       reconnect
//
// Purpose-
//       Attempt to create a connection, ignoring all errors.
//
//----------------------------------------------------------------------------
static void
   reconnect( void )                // Attempt a dummy connection
{
   try {
     Socket socket;                 // (Socket closed by destructor)
     int rc= socket.open(AF_INET, SOCK_STREAM, PF_UNSPEC);
     if( rc == 0 ) {
       rc= socket.connect(peer_addr);
       if( USE_STOP_HCDM || opt_verbose > 1 )
         debugh("%4d %s %d= socket.connect\n", __LINE__, __FILE__, rc);
     }
     Thread::sleep(0.125);
   } catch(...) {
     debugf("%4d %s catch(...)\n", __LINE__, __FILE__);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       reset_statistics
//
// Purpose-
//       Reset statistic counters
//
//----------------------------------------------------------------------------
static void
   reset_statistics( void )
{
   error_count= 0;
   retry_count= 0;

   rp_again.store(0);
   rp_block.store(0);
   rp_count.store(0);
   wp_again.store(0);
   wp_block.store(0);
   wp_count.store(0);

   rs_again.store(0);
   rs_block.store(0);
   rs_count.store(0);
   ws_again.store(0);
   ws_block.store(0);
   ws_count.store(0);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       torf
//
// Purpose-
//       Return "true" or "false"
//
//----------------------------------------------------------------------------
static const char*                  // "true" or "false"
   torf(                            // True or False?
     int               cc)          // For this condition
{
   if( cc )
     return "true";
   return "false";
}

//----------------------------------------------------------------------------
//
// Class-
//       PacketWorker
//
// Purpose-
//       The PacketWorker object
//
//----------------------------------------------------------------------------
class PacketWorker : public pub::Worker { // The PacketWorker object
//----------------------------------------------------------------------------
// PacketWorker::Attributes
//----------------------------------------------------------------------------
public:
char                   ibuff[512];  // Input buffer
Socket*                client;      // The client socket

//----------------------------------------------------------------------------
// PacketWorker::Constructors/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~PacketWorker( void )            // Destructor
{  if( HCDM ) debugh("PacketWorker(%p)::~PacketWorker()...\n", this);

//   if( client )
//     delete client;
}

   PacketWorker(                    // Constructor
     Socket*           client)      // Connection socket
:  client(client)
{  if( HCDM ) debugh("PacketWorker(%p)::PacketWorker(%p)\n", this, client); }

//----------------------------------------------------------------------------
// PacketWorker::Methods
//----------------------------------------------------------------------------
public:
virtual void
   work( void )                     // Process work
{
   if( opt_verbose > 1 )
     debugh("PacketWorker::work()\n");

   try {
     run();
   } catch(pub::Exception& X) {
     debugh("PacketWorker: %s\n", X.to_string().c_str());
   } catch(std::exception& X) {
     debugh("PacketWorker: what(%s)\n", X.what());
   } catch(...) {
     debugh("PacketWorker: catch(...)\n");
   }
}

void
   run( void )                      // Process work
{
}
}; // class PacketWorker

//----------------------------------------------------------------------------
//
// Class-
//       StreamWorker
//
// Purpose-
//       The StreamWorker object
//
//----------------------------------------------------------------------------
class StreamWorker : public pub::Worker { // The StreamWorker object
//----------------------------------------------------------------------------
// StreamWorker::Attributes
//----------------------------------------------------------------------------
public:
char                   buffer[32768]; // Input buffer;
Socket*                client;      // The client socket

//----------------------------------------------------------------------------
// StreamWorker::Constructors/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~StreamWorker( void )            // Destructor
{  if( HCDM ) debugh("StreamWorker(%p)::~StreamWorker()...\n", this);

   if( client )
     delete client;
}

   StreamWorker(                    // Constructor
     Socket*           client)      // Connection socket
:  client(client)
{  if( HCDM ) debugh("StreamWorker(%p)::StreamWorker(%p)\n", this, client); }

//----------------------------------------------------------------------------
// StreamWorker::Methods
//----------------------------------------------------------------------------
public:
virtual void
   work( void )                     // Process work
{
   if( opt_verbose > 1 )
     debugh("StreamWorker::work()\n");

   try {
     run();
   } catch(pub::Exception& X) {
     debugh("StreamWorker: %s\n", X.to_string().c_str());
   } catch(std::exception& X) {
     debugh("StreamWorker: what(%s)\n", X.what());
   } catch(...) {
     debugh("StreamWorker: catch(...)\n");
   }

   delete this;                     // When done, delete this StreamWorker
}

void
   run( void )                      // Process work
{
   // Set default timeout
   struct timeval tv= { 3, 0 };     // 3.0 second timeout
   client->set_option(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
   client->set_option(SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

   for(size_t count= 0;;++count) {
     buffer[0]= '\0';
     int L= client->read(buffer, sizeof(buffer)-1);
     if( L <= 0 )
       break;
     buffer[L]= '\0';

     if( opt_verbose > 1 )
       debugh("Server: Read %d '%s'\n", L, visify(buffer).c_str());
     if( L <= 0 ) {
       if( L == 0 && count == 0 )
         debugh("Server: %4d HCDM\n", __LINE__); // Bug workaround
       break;
     }

     const char* C= buffer;
     std::string meth= get_token(C);
     std::string what= get_token(C);
     std::string http= get_token(C);
     if( meth != "GET" || http != "HTTP/1.1" ) {
       L= client->write(http400, strlen(http400));
       if( L <= 0 ) {
         debugh("Server write(http400) failure\n");
         break;
       }
       if( opt_verbose > 1 )
         debugh("Worker: Sent %d '%s'\n", L, visify(http400).c_str());
     } else {

     if( what != "/" && what != "/index.html"
         && what != "/std" && what != "/ssl" ) {
       L= client->write(http404, strlen(http404));
       if( L <= 0 ) {
         debugh("Server write(http404) failure\n");
         break;
       }
       if( opt_verbose > 1 )
         debugh("Worker: Sent %d '%s'\n", L, visify(http404).c_str());
     } else {

       L= client->write(http200, strlen(http200));
       if( L <= 0 ) {
         debugh("Server write(http200) failure\n");
         break;
       }
       if( opt_verbose > 1 )
         debugh("Worker: Sent %d '%s'\n", L, visify(http200).c_str());
     } }

     ++rs_count;
   }

   // This fixes an accept hang problem. Optional only to test alternatives.
   if( USE_LINGER ) {
     // Client closed or in error state. Allow immediate port re-use
     struct linger optval;
     optval.l_onoff= 1;
     optval.l_linger= 0;
     client->set_option(SOL_SOCKET, SO_LINGER, &optval, sizeof(optval));
   }
}
}; // class StreamWorker

//----------------------------------------------------------------------------
//
// Class-
//       TimerThread
//
// Purpose-
//       Background Thread that sets and clears `running`
//
//----------------------------------------------------------------------------
class TimerThread : public pub::Thread {
public:
   ~TimerThread( void ) = default;
   TimerThread( void ) = default;

virtual void
   run( void )
{
   running= true;
   test_start.post();

   Thread::sleep(opt_runtime);

   running= false;
   test_start.reset();
}
}; // class TimerThread
static TimerThread timer_thread;

//----------------------------------------------------------------------------
//
// Class-
//       PacketServer
//
// Purpose-
//       Packet (datagram) server thread
//
//----------------------------------------------------------------------------
class PacketServer : public Thread {
public:
char                   ibuff[512];  // Input buffer
Event                  event;       // Thread ready event
Socket                 listen;      // Packet listener Socket

int                    operational= false; // TRUE while operational

   PacketServer()
:  Thread(), event()
{
}

   ~PacketServer() = default;

virtual void
   run()                            // PacketServer::run
{
   int rc= listen.open(AF_INET, SOCK_DGRAM, PF_UNSPEC);
   if( rc ) {
     errorp("PacketServer.open");
     return;
   }

   int optval= true;                // (Needed before the bind)
   listen.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   if( USE_RPOLL == USE_POLL_NONBLOCK )
     listen.set_flags( listen.get_flags() | O_NONBLOCK );

   rc= listen.bind(STD_PORT);       // Set port number
   if( rc ) {
     errorp("PacketServer.bind");
     return;
   }

   operational= true;
   if( opt_verbose )
     debugf("Packet server %s operational\n", peer_addr.c_str());
   event.post();

   try {
     worker(&listen);
   } catch(pub::Exception& X) {
     debugh("Exception: %s\n", X.to_string().c_str());
   } catch(std::exception& X) {
     debugh("std::Exception what(%s)\n", X.what());
   }

   // Statistics
   if( opt_verbose && !opt_client && !opt_packet && !opt_stream ) {
     debugf("--packet info:\n");
     debugf("%'16zd Recv again\n", rp_again.load());
     debugf("%'16zd Recv block\n", rp_block.load());
     debugf("%'16zd Recv count\n", rp_count.load());
   }

   if( opt_verbose )
     debugf("Packet server %s terminated\n", peer_addr.c_str());
}

void
   stop()                           // PacketServer::stop
{
   operational= false;              // Indicate terminated
   event.reset();

   int rc= listen.close();          // Close the Socket
   if( opt_verbose > 1 )
     debugh("%4d %s %d= listen.close()\n", __LINE__, __FILE__, rc);
}

void
   stress()                         // PacketServer::stress
{
   Socket packet;                   // Datagram transmission Socket

   int rc= packet.open(AF_INET, SOCK_DGRAM, PF_UNSPEC);
   if( rc ) {
     errorp("PacketServer.open");
     return;
   }

   int optval= true;                // (Needed before the bind)
   packet.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   if( USE_SPOLL == USE_POLL_NONBLOCK )
     packet.set_flags( packet.get_flags() | O_NONBLOCK );

   rc= packet.connect(peer_addr);   // Connect to peer
   if( rc ) {
     errorp("PacketServer.bind");
     return;
   }

   SocketSelect select;             // Socket selector
   if( USE_SPOLL == USE_POLL_SELECT )
     select.insert(&packet, POLLOUT);

   struct pollfd pfd= {};
   pfd.events= POLLOUT;

   if( opt_verbose ) {
     if( !opt_server )
       debugf("\n");
     debugf("--packet test: Started\n");
   }

   Interval interval;
   interval.start();
   while( error_count == 0 && interval.stop() < opt_runtime ) {
     Socket* socket= &packet;
     switch( USE_SPOLL ) {
       case USE_POLL_POLL: {{{{
         rc= packet.poll(&pfd, 63); // Approx. 1/16 second timeout
         error_count += VERIFY( rc >= 0 );
         if( rc == 0 ) {
           ++wp_again;
           continue;
         }
       break;
       }}}}
       case USE_POLL_SELECT:
         socket= select.select(63); // Approx. 1/16 second timeout
         if( socket == nullptr ) {
           error_count += VERIFY( if_retry() );
           ++wp_again;
           continue;
         }
         error_count += VERIFY( socket == &packet );
       break;

       default:
         // USE_POLL_BLOCK || USE_POLL_NONBLOCK
         break;
     }

     if( error_count )
       break;

     ssize_t L= packet.write(obuff, sizeof(obuff));
     if( L <= 0 ) {
       if( USE_SPOLL == USE_POLL_NONBLOCK && if_retry() ) {
         ++wp_again;
         continue;
       }
     }

     error_count += VERIFY( L == sizeof(obuff) );
     if( error_count == 0 )
       ++wp_count;
   }

   // Statistics
   if( opt_verbose ) {
     debugf("--packet test: %s\n", error_count ? "FAILED" : "Complete");
     debugf("%'16zd Send again\n", wp_again.load());
     debugf("%'16zd Send count\n", wp_count.load());
     if( opt_server ) {
       debugf("%'16zd Recv again\n", rp_again.load());
       debugf("%'16zd Recv block\n", rp_block.load());
       debugf("%'16zd Recv count\n", rp_count.load());
       debugf("%'16zd Lost count\n", wp_count.load() - rp_count.load());
       debugf("%'18.1f Operations/second\n"
             , double(rp_count.load()) / opt_runtime);
     }
   }
}

void
   worker(                         // Process work
     Socket*           client)     // The packet client
{
   // Set default timeout
   struct timeval tv= { 3, 0 };     // 3.0 second timeout
   client->set_option(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
   client->set_option(SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

   SocketSelect select;
   if( USE_RPOLL == USE_POLL_SELECT )
     select.insert(client, POLLIN);
   else if( USE_RPOLL == USE_POLL_NONBLOCK )
     client->set_flags( client->get_flags() | O_NONBLOCK );

   struct pollfd pfd= {};
   pfd.events= POLLIN;

   while( operational && error_count == 0 ) {
     memset(ibuff, 0, sizeof(ibuff));

     Socket* socket= client;
     switch( USE_RPOLL ) {
       case USE_POLL_POLL: {{{{
         int rc= client->poll(&pfd, 1000); // 1 second timeout
         if( rc < 0 ) {
           if( if_closed(pfd) )
             return;

           ++error_count;
           errorf("%4d ERROR: %d= client->poll() %d:%s\n", __LINE__, rc
                 , errno, strerror(errno));
         }
         if( rc == 0 ) {
           ++rp_again;
           continue;
         }
         break;
       }}}}
       case USE_POLL_SELECT:
         socket= select.select(1000); // 1 second timeout
         if( socket == nullptr ) {
           if( if_closed(select, client) )
             return;

           error_count += VERIFY( if_retry() );
           ++rp_again;
           continue;
         }
         error_count += VERIFY( socket == client );
         break;

       default:
         // USE_POLL_BLOCK || USE_POLL_NONBLOCK
         break;
     }

     if( error_count )
       break;

     ssize_t L= client->read(ibuff, sizeof(ibuff));
     if( opt_hcdm )
       traceh("%4zd= client->read(\n", L);
     if( L <= 0 ) {
       if( !client->is_open() )
         break;
       error_count += VERIFY( if_retry() );
       if( error_count )
         break;

       Thread::yield();
       ++rp_block;
       continue;
     }

     error_count += VERIFY( L == sizeof(obuff) );
     for(unsigned i= 0; i<sizeof(obuff); ++i) {
       if( ibuff[i] != obuff[i] ) {
         ++error_count;
         debugf("ibuff[%d] %.2x != %.2x\n", i, ibuff[i], obuff[i]);
         break;
       }
     }
     if( error_count )
       break;
     ++rp_count;
   }
}
}; // class PacketServer

//----------------------------------------------------------------------------
//
// Class-
//       StreamClient
//
// Purpose-
//       Stream client stress test thread
//
//----------------------------------------------------------------------------
class StreamClient : public Thread {
public:
char                   buffer[32768]; // Input buffer

   StreamClient() = default;
   ~StreamClient() = default;

virtual void
   run()                            // StreamClient::run
{
   test_start.wait();               // Wait for start signal

   try {
     while( running && error_count == 0 ) {
       client();
     }
   } catch(pub::Exception& X) {
     debugh("Exception: %s\n", X.to_string().c_str());
   } catch(std::exception& X) {
     debugh("std::Exception what(%s)\n", X.what());
   }

   if( opt_verbose > 1 )
     debugf("Stream client %s terminated\n", peer_addr.c_str());
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamClient::client
//
// Purpose-
//       Client single HTTP operation
//
//----------------------------------------------------------------------------
static void
   client( void )                   // Standard client test
{
static const char* request=         // The request data
   "GET / HTTP/1.1\r\n"
   "\r\n"
   ;
static char buffer[8192];           // The response data buffer

   Socket socket;
   try {
     int rc= socket.open(AF_INET, SOCK_STREAM, PF_UNSPEC);
     if( rc ) {
       debugh("%d= STD open %d:%s\n", rc, errno, strerror(errno));
       throw pub::Exception("Client open Failure");
     }

     // Connect to server
     rc= socket.connect(peer_addr);
     if( rc < 0 ) {
       debugh("%d= STD connect %d:%s\n", rc, errno, strerror(errno));
       if( ++retry_count <= USE_CONNECT_RETRY ) {
         Thread::sleep(5.0);
         return;
       }
       throw pub::Exception("Client connect Failure");
     }

     // Write/read
     int L= socket.write(request, strlen(request));
     if( opt_verbose > 1 )
       debugh("STD Client: Wrote %d '%s'\n", L, visify(request).c_str());
     if( L <= 0 ) {
       int ERRNO= errno;
       debugh("%d= STD Write(%zd) %d,%s\n", L, strlen(request), ERRNO,
              strerror(ERRNO));
       throw pub::Exception("Client write Failure");
     }

     L= socket.read(buffer, sizeof(buffer)-1);
     if( L > 0 ) {
       buffer[L]= '\0';
       if( opt_verbose > 1 )
         debugh("STD Client: Read %d '%s'\n", L, visify(buffer).c_str());
     }

     ++ws_count;
   } catch(pub::Exception& X) {
     debugh("client: %s\n", X.to_string().c_str());
     ++error_count;
   } catch(std::exception& X) {
     debugh("client: what(%s)\n", X.what());
     ++error_count;
   } catch(...) {
     debugh("client: catch(...) socket(%4d)\n", socket.get_handle());
     ++error_count;
   }

   // Close the connection
   int rc= socket.close();
   if( rc ) {
     debugh("%d= STD close %d:%s\n", rc, errno, strerror(errno));
     throw pub::Exception("Client open Failure");
   }
}
}; // class StreamClient

//----------------------------------------------------------------------------
//
// Class-
//       StreamServer
//
// Purpose-
//       Stream server thread
//
//----------------------------------------------------------------------------
class StreamServer : public Thread {
public:
char                   buffer[32768]; // Input buffer
Socket*                socket= nullptr; // Client Socket
Event                  event;       // Thread ready event
Socket                 listen;      // Listener Socket

int                    operational= false; // TRUE while operational

   StreamServer() = default;
   ~StreamServer() = default;

virtual void
   run()                            // StreamServer::run
{
   int rc= listen.open(AF_INET, SOCK_STREAM, PF_UNSPEC);
   if( rc ) {
     errorp("StreamServer.open");
     return;
   }

   int optval= true;                // (Needed before the bind)
   listen.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   if( USE_APOLL == USE_POLL_NONBLOCK )
     listen.set_flags( listen.get_flags() | O_NONBLOCK );

   rc= listen.bind(STD_PORT);       // Set port number
   if( rc ) {
     errorp("StreamServer.bind");
     listen.close();
     return;
   }
   rc= listen.listen();             // Set listener socket
   if( rc ) {
     errorp("StreamServer.listen");
     listen.close();
     return;
   }

   SocketSelect select;
   if( USE_APOLL == USE_POLL_SELECT )
     select.insert(&listen, POLLIN);

   struct pollfd pfd= {};
   pfd.events= POLLIN;

   operational= true;
   if( opt_verbose )
     debugf("Stream server %s operational\n", peer_addr.c_str());
   event.post();

   try {
     while( operational && error_count == 0 ) {
       switch( USE_APOLL ) {
         case USE_POLL_POLL: {{{{
           rc= listen.poll(&pfd, 1000); // 1 second timeout
           if( rc < 0 ) {
             if( if_closed(pfd) )
               continue;

             ++error_count;
             errorf("%4d ERROR: %d= listen.poll() %d:%s\n", __LINE__, rc
                   , errno, strerror(errno));
           }
           if( rc == 0 ) {
             ++rs_again;
             continue;
           }
         break;
         }}}}
         case USE_POLL_SELECT:
           socket= select.select(1000); // 1 second timeout
           if( socket == nullptr ) {
             if( if_closed(select, &listen) )
               return;

             error_count += VERIFY( if_retry() );
             ++rs_again;
             continue;
           }
         break;

         default:
           // USE_POLL_BLOCK || USE_POLL_NONBLOCK
           break;
       }

       Socket* socket= listen.accept();
       if( socket ) {
         StreamWorker* worker= new StreamWorker(socket);
         if( opt_worker )
           WorkerPool::work(worker);
         else
           worker->work();
       }
     }
   } catch(pub::Exception& X) {
     debugh("Exception: %s\n", X.to_string().c_str());
   } catch(std::exception& X) {
     debugh("std::Exception what(%s)\n", X.what());
   }

   // Statistics
   if( opt_verbose && !opt_client && !opt_packet && !opt_stream ) {
     debugf("--stream info:\n");
     debugf("%'16ld Accept retr%s\n"
           , rs_again.load(), rs_again.load() == 1 ? "y" : "ies");
     debugf("%'16ld Operations\n", rs_count.load());
   }

   if( opt_verbose )
     debugf("Stream server %s terminated\n", peer_addr.c_str());
}

void
   serve(Socket* client)            // StreamServer::serve
{
   // Set default timeout
   struct timeval tv= { 3, 0 };     // 3.0 second timeout
   client->set_option(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
   client->set_option(SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

   for(int count= 0;;count++) {
     buffer[0]= '\0';
     int L= client->read(buffer, sizeof(buffer)-1);
     if( L <= 0 )
       break;
     buffer[L]= '\0';

     if( opt_verbose > 1 )
       debugh("Server: Read %d '%s'\n", L, visify(buffer).c_str());
     if( L <= 0 ) {
       if( L == 0 && count == 0 )
         debugh("Server: %4d HCDM\n", __LINE__); // Bug workaround
       break;
     }

     const char* C= buffer;
     std::string meth= get_token(C);
     std::string what= get_token(C);
     std::string http= get_token(C);
     if( meth != "GET" || http != "HTTP/1.1" ) {
       L= client->write(http400, strlen(http400));
       if( L <= 0 ) {
         debugh("Server write(http400) failure\n");
         break;
       }
       if( opt_verbose > 1 )
         debugh("Worker: Sent %d '%s'\n", L, visify(http400).c_str());
     } else {

     if( what != "/" && what != "/index.html"
         && what != "/std" && what != "/ssl" ) {
       L= client->write(http404, strlen(http404));
       if( L <= 0 ) {
         debugh("Server write(http404) failure\n");
         break;
       }
       if( opt_verbose > 1 )
         debugh("Worker: Sent %d '%s'\n", L, visify(http404).c_str());
     } else {

       L= client->write(http200, strlen(http200));
       if( L <= 0 ) {
         debugh("Server write(http200) failure\n");
         break;
       }
       if( opt_verbose > 1 )
         debugh("Worker: Sent %d '%s'\n", L, visify(http200).c_str());
     } }

     ++rs_count;
   }

   // This fixes an accept hang problem. Optional only to test alternatives.
   if( USE_LINGER ) {
     // Client closed or in error state. Allow immediate port re-use
     struct linger optval;
     optval.l_onoff= 1;
     optval.l_linger= 0;
     client->set_option(SOL_SOCKET, SO_LINGER, &optval, sizeof(optval));
   }
}

void
   stop()                           // StreamServer::stop
{
   operational= false;            // Indicate terminated
   event.reset();

   if( USE_STOP_HCDM ) {
     if( false ) {                // TRUE: Tests "::accept would block" logic
       debugh("%4d %s stop DISABLED\n", __LINE__, __FILE__);
       return;
     }
     debugh("%4d %s stop\n", __LINE__, __FILE__);
   }

   int rc= listen.close();        // Close the listener Socket
   if( USE_STOP_HCDM || opt_verbose > 1 )
     debugh("%4d %s %d= listen.close()\n", __LINE__, __FILE__, rc);

   //-------------------------------------------------------------------------
   // We now create a dummy connection to complete any pending listen,
   // ignoring any errors that occur.
   reconnect();
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamServer::stress
//
// Purpose-
//       Run stream server stress test.
//
//----------------------------------------------------------------------------
void
   stress( void )                   // Stream stress test
{
static int THREAD_COUNT= 16;        // Maximum thread count

   if( !opt_thread )
     THREAD_COUNT= 1;

   StreamClient* client[THREAD_COUNT];
   for(int i= 0; i<THREAD_COUNT; i++) {
     client[i]= new StreamClient();
     client[i]->start();
   }
   Thread::sleep(0.125);          // (Delay allows all Threads to start

   if( opt_verbose ) {
     if( !opt_server )
       debugf("\n");
     debugf("--stream test: Started\n");
   }

   timer_thread.start();
   timer_thread.join();

   // Statistics
   if( opt_verbose ) {
     debugf("--stream test: %s\n", error_count ? "FAILED" : "Complete");
     if( opt_server )
       debugf("%'16ld Accept retr%s\n", rs_again.load()
             , rs_again.load() == 1 ? "y" : "ies");
     debugf("%'16ld Operations\n", ws_count.load());
     debugf("%'16d Seconds\n", opt_runtime);
     debugf("%'18.1f Operations/second\n"
           , double(ws_count.load()) / opt_runtime);
   }
}
}; // class StreamServer

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int
   main(
     int               argc,
     char*             argv[])
{
   //-------------------------------------------------------------------------
   // Initialize
   Wrapper  tc= opts;               // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_info([]()
   {
     fprintf(stderr, "  --runtime=<seconds>\n");
     fprintf(stderr, "  --packet\tRun datagram test\n");
     fprintf(stderr, "  --stream\tRun stream test\n");

     fprintf(stderr, "  --server\tRun local server\n");
     fprintf(stderr, "  --server=host:port\tUse remote server\n");
     fprintf(stderr, "  --thread\tRun multi-threaded stream client\n");
     fprintf(stderr, "  --worker\tRun multi-threaded stream server\n");
   });

   tc.on_init([](int argc, char* argv[]) // (Unused in this sample)
   { (void)argc; (void)argv;        // (Unused arguments)

     if( HCDM )
       opt_hcdm= true;

     if( VERBOSE > opt_verbose )
       opt_verbose= VERBOSE;

     if( opt_hcdm || true ) {
       debug_set_head(Debug::HEAD_THREAD);
       debug_set_mode(Debug::MODE_INTENSIVE);
     }

     setlocale(LC_NUMERIC, "");     // Allows printf("%'d\n", 123456789);

     for(unsigned i= 0; i<sizeof(obuff); ++i)
       obuff[i]= i;

     if( !opt_client && !opt_server && !opt_packet && !opt_stream ) {
       opt_server= true;
       opt_client= true;
     }

     if( opt_target ) {
       opt_server= false;
       peer_addr= opt_target;
     } else {
       opt_server= true;
       peer_addr= Socket::get_host_name() + ":" + std::to_string(STD_PORT);
     }

     if( opt_runtime == 0 && (opt_packet || opt_stream) )
       opt_runtime= 20;

     return 0;
   });

   tc.on_main([tr](int, char*[])
   {
static const char* poll_method[]=
{  "BLOCK"
,  "NONBLOCK"
,  "POLL"
,  "SELECT"
};

     if( opt_verbose ) {
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);
       debugf("\n");
       debugf("Settings:\n");
       debugf("%5d: runtime\n",  opt_runtime);
       if( opt_target )
         debugf("%5s: server: %s\n",   torf(opt_server), opt_target);
       else
         debugf("%5s: server: %s\n",   torf(opt_server), peer_addr.c_str());
       debugf("%5d: verbose\n",opt_verbose);

       debugf("%5s: client\n", torf(opt_client));
       debugf("%5s: packet\n", torf(opt_packet));
       debugf("%5s: stream\n", torf(opt_stream));
       debugf("%5s: thread\n", torf(opt_thread));
       debugf("%5s: worker\n", torf(opt_worker));

       // Debugging, experimentation
       debugf("\n");
       debugf("%5s: USE_LINGER\n", torf(USE_LINGER));
       debugf("%5d: USE_APOLL: %s\n", USE_APOLL, poll_method[USE_APOLL]);
       debugf("%5d: USE_RPOLL: %s\n", USE_RPOLL, poll_method[USE_RPOLL]);
       debugf("%5d: USE_SPOLL: %s\n", USE_SPOLL, poll_method[USE_SPOLL]);
     }

{{{{
     PacketServer packet_server;
     StreamServer stream_server;

     if( opt_packet ) {
       reset_statistics();
       if( opt_server ) {
         if( opt_verbose )
           debugf("\n");
         packet_server.start();
         packet_server.event.wait();
         packet_server.stress();
         Thread::sleep(0.125);
         packet_server.stop();
         packet_server.join();
       } else {
         packet_server.stress();
       }
     }

     if( opt_stream ) {
       reset_statistics();
       if( opt_server ) {
         if( opt_verbose )
           debugf("\n");
         stream_server.start();
         stream_server.event.wait();
         stream_server.stress();
         Thread::sleep(0.125);
         stream_server.stop();
         stream_server.join();
       } else {
         stream_server.stress();
       }
     }

     if( opt_server && (opt_client || (!opt_packet && !opt_stream)) ) {
       reset_statistics();
       if( opt_verbose )
         debugf("\n");
       packet_server.start();
       packet_server.event.wait();
       stream_server.start();
       stream_server.event.wait();

       if( opt_client )
         StreamClient::client();

       Thread::sleep(opt_runtime);
       packet_server.stop();
       stream_server.stop();
       packet_server.join();
       stream_server.join();
     } else if( opt_client ) {
       StreamClient::client();
     }

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
}}}}
     return error_count != 0;
   });

   tc.on_parm([tr](std::string name, const char* value)
   {
     if( name == "runtime" ) {
       if( value == nullptr )
         value= "60";
       opt_runtime= tr->ptoi(value, name.c_str());
     }

     if( name == "server" ) {
       if( value ) {
         opt_server= false;
         opt_target= value;
       }
     }

     return 0;
   });

   tc.on_term([]() {});             // (Unused so far)

   //-------------------------------------------------------------------------
   // Run the test
   int rc= tc.run(argc, argv);      // (Allow debugging statment before exit)
   return rc;
}

