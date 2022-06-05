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
//       2022/06/02
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE
#define _GNU_SOURCE                 // For ppoll
#endif
#include <atomic>                   // For std::atomic<>
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
using namespace pub::utility;       // For utility functions
using std::atomic;

#define debugf         pub::debugging::debugf
#define opt_hcdm       pub::Wrapper::opt_hcdm
#define opt_verbose    pub::Wrapper::opt_verbose

//----------------------------------------------------------------------------
// Ignore undefined flags (These flags are/were undefined on Cygwin)
#ifndef MSG_CONFIRM
#define MSG_CONFIRM 0
#endif

#ifndef POLLRDHUP
#define POLLRDHUP 0
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  STD_PORT= 8080                   // Our port number
}; // Generic enum

enum                                // Debugging enum
// Debugging/experimental options  - - - - - - - - - - - - - - - - - - - - - -
{  OPT_PACKET_CLIENTS= 2            // Multi-thread packet client thread count
,  OPT_STREAM_CLIENTS= 16           // Multi-thread stream client thread count
,  USE_CONNECT_RETRY= 0             // Number of retries before error
,  USE_LINGER= true                 // Use SO_LINGER timeout for client
,  USE_STOP_HCDM= false             // Write StreamServer::stop messages
,  USE_PACKET_CONFIRM= false        // Use packet MSG_CONFIRM flag
,  USE_PACKET_CONNECT= false        // Use connect, not set_peer_addr
}; // Debugging  enum

enum                                // Polling experimental controls
// For packets, the polling operation occurs before each packet op
// For streams, the polling operation occurs before each accept
{  USE_POLL_BLOCK= 0                // Use blocking, don't poll
,  USE_POLL_NONBLOCK= 1             // Use non-blocking, don't poll
,  USE_POLL_POLL= 2                 // Use poll before accept or packet op
,  USE_POLL_SELECT= 3               // Use SocketSelect polling

,  USE_APOLL= USE_POLL_POLL         // Use accept polling method
,  USE_RPOLL= USE_POLL_POLL         // Use recv polling method
,  USE_SPOLL= USE_POLL_POLL         // Use send polling method
}; // Polling experimental controls

#if USE_PACKET_CONFIRM              // (Prefer false)
  static constexpr const int pkt_confirm= MSG_CONFIRM;
#else
  static constexpr const int pkt_confirm= 0;
#endif

//----------------------------------------------------------------------------
// Constant data
//----------------------------------------------------------------------------
static const char*     poll_method[]= // Polling experimental control names
{  "BLOCK"
,  "NONBLOCK"
,  "POLL"
,  "SELECT"
};

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define LOCK_GUARD(x) std::lock_guard<decltype(x)> lock(x)

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::string     peer_addr;   // The server's name:port string
static bool            is_server= false; // TRUE if running server w/o client

// Test controls
static int             error_count= 0; // Error counter
static int             retry_count= 0; // Connection retry counter
static int             running= 0;  // Test running indicator
static Event           test_start;  // The test start Event

// Packet client statistics
static atomic<size_t>  pcc_count;   // Number of operations completed
static atomic<size_t>  pcr_again;   // Number of read poll retries
static atomic<size_t>  pcr_count;   // Number of read operations completed
static atomic<size_t>  pcw_again;   // Number of write poll retries
static atomic<size_t>  pcw_count;   // Number of write operations completed

// Packet server statistics
static atomic<size_t>  psr_again;   // Number of read poll/select retries
static atomic<size_t>  psr_block;   // Number of read if_retry() retries
static atomic<size_t>  psr_count;   // Number of read operations completed
static atomic<size_t>  psw_count;   // Number of write operations completed

// Stream client statistics
static atomic<size_t>  scc_count;    // Number of operations completed
static atomic<size_t>  scr_count;    // Number of read operations completed
static atomic<size_t>  scw_count;    // Number of write operations completed

// Stream server statistics
static atomic<size_t>  ssr_again;    // Number of read if_retry() retries
static atomic<size_t>  ssr_count;    // Number of read operations completed
static atomic<size_t>  ssw_count;    // Number of write operations completed

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
// HTTP request and responses
//----------------------------------------------------------------------------
static const char* httpREQ=         // HTTP request data
   "GET / HTTP/1.1\r\n"
   "\r\n"
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

   if( pfd.revents & (POLLHUP | POLLRDHUP) )
     return true;                   // (Peer socket was closed)

   return false;
}

static bool
   if_closed(const SocketSelect& select, const Socket* socket)
{
   const struct pollfd* pfd= select.get_pollfd(socket);
   if( pfd == nullptr )
     return true;                   // (Our socket was closed )

   if( pfd->revents & (POLLHUP | POLLRDHUP) )
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
{  return errno == EAGAIN || errno == EWOULDBLOCK; }
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       line
//
// Purpose-
//       Debugging: Easy to insert/remove code tracker.
//
//----------------------------------------------------------------------------
static inline void
   line(int line)
{  debugf("%4d %s HCDM\n", line, __FILE__); }
// line(__LINE__);

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

   pcc_count.store(0);              // Packet client operation counter
   pcr_again.store(0);              // Packet client read counters
   pcr_count.store(0);
   pcw_again.store(0);              // Packet client write counters
   pcw_count.store(0);
   psr_again.store(0);              // Packet server read counters
   psr_block.store(0);
   psr_count.store(0);
   psw_count.store(0);              // Packet server write counters

   scc_count.store(0);              // Stream client operation counter
   scr_count.store(0);              // Stream client read counters
   scw_count.store(0);              // Stream client write counters
   ssr_again.store(0);              // Stream server read counters
   ssr_count.store(0);
   ssw_count.store(0);              // Stream server write counters

   WorkerPool::reset();
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
{  return cc ? "true" : "false"; }

//----------------------------------------------------------------------------
//
// Subroutine-
//       trace
//
// Purpose-
//       Socket operation informational message
//
// Implementation notes-
//       Preserves errno
//
//----------------------------------------------------------------------------
static void
   trace(                           // Write trace message
     int               line,        // For this source code line
     const char*       fmt,         // Format string
                       ...)         // The PRINTF argument list
_LIBPUB_PRINTF(2,3);

static void
   trace(                           // Write trace message
     int               line,        // For this source code line
     const char*       fmt,         // Format string
                       ...)         // The PRINTF argument list
{
   int ERRNO= errno;                // (Preserve errno)
   va_list             argptr;      // Argument list pointer

   LOCK_GUARD(*Debug::get());

   if( line != 0 )
     traceh("%4d ", line);          // (Heading)

   va_start(argptr, fmt);           // Initialize va_ functions
   vtracef(fmt, argptr);            // (User error message)
   va_end(argptr);                  // Close va_ functions

   if( ERRNO )                      // If error
     tracef(" %d:%s\n", ERRNO, strerror(ERRNO)); // (errno information)
   else
     tracef("\n");

   errno= ERRNO;                    // (Restore errno)
}

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
static TimerThread timer_thread;    // *THE* TimerThread

//----------------------------------------------------------------------------
//
// Class-
//       PacketClient
//
// Purpose-
//       Packet client stress test thread
//
//----------------------------------------------------------------------------
class PacketClient : public Thread {
public:
char                   buffer[32768]; // Input/output buffer

size_t                 last_recv= 0; // Last receive packet identifier
size_t                 last_send= 0; // Last sent packet identifier

Socket                 packet;      // Datagram Socket
struct pollfd          pfd= {};     // Poll file descriptor

//----------------------------------------------------------------------------
// Constructors/Destructor
//----------------------------------------------------------------------------
   PacketClient()
:  Thread()
{
   int rc= packet.open(AF_INET, SOCK_DGRAM, PF_UNSPEC);
   if( rc ) {
     trace(__LINE__, "PacketClient %d= open", rc);
     return;
   }

   int optval= true;                // (Needed before the bind)
   packet.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
   packet.set_flags( packet.get_flags() | O_NONBLOCK );

   pfd.events= POLLIN | POLLOUT;

   if( USE_PACKET_CONNECT ) {       // (Prefer false)
     rc= packet.connect(peer_addr); // Connect to peer
     if( rc )
       trace(__LINE__, "PacketClient %d=connect", rc);
   } else {
     rc= packet.set_peer_addr(peer_addr); // Set peer address
     if( rc )
       trace(__LINE__, "PacketClient %d=set_peer_addr(%s)", rc
            , peer_addr.c_str());
   }
}

   ~PacketClient() = default;

//----------------------------------------------------------------------------
//
// Method-
//       PacketClient::client
//
// Purpose-
//       Read all responses, send one packet
//
// Implementation notes-
//       PacketClient always uses polling and non-blocking sockets.
//
//----------------------------------------------------------------------------
virtual void
   client()
{
   int rc= packet.poll(&pfd, 63);    // Approx. 1/16 second timeout
   if( rc < 0 ) {
     ++error_count;
     trace(__LINE__, "PacketClient %d= poll", rc);
     return;
   }

   if( rc == 0 ) {
     ++pcr_again;
     return;
   }

   // Read all responses
   if( pfd.revents & POLLIN ) {
     for(int count= 0;;++count) {
       struct sockaddr sockaddr;
       socklen_t socklen= sizeof(sockaddr);

       ssize_t
       L= packet.recvfrom(buffer, sizeof(buffer), 0 , &sockaddr, &socklen);
       if( L <= 0 ) {
         if( if_retry() )
           break;
         if( count == 0 )
           trace(__LINE__, "PacketClient %zd= recvfrom", L);

         ++error_count;
         trace(__LINE__, "PacketClient %zd= recvfrom", L);
       }

       if( running ) {              // (Avoids pcc_count/pcr_count mismatch)
         ++pcc_count;
         ++pcr_count;
       }

       // TODO: VERIFY RESPONSE (optional)
       ++last_recv;                 // (TODO placeholder)
     }
   }

   // Send next request
   if( (pfd.revents & POLLOUT) == 0 ) {
     ++pcw_again;
     return;
   }

   std::string S= "GET ";
   S += std::to_string(++last_send);
   S += "/\r\n";
   // Notes: packet.peer_addr/peer_size were set during construction, so we
   // don't have to specify it here. (See Socket.h sendto inline)
   ssize_t L;
   if( USE_PACKET_CONNECT )         // (Prefer false)
     L= packet.send(S.c_str(), S.size(), pkt_confirm);
   else
     L= packet.sendto(S.c_str(), S.size(), pkt_confirm);
   // trace(__LINE__, "PacketClient %zd= sendto", L);
   if( size_t(L) == S.size() ) {
     ++pcw_count;
     return;
   }

   ++error_count;
   trace(__LINE__, "PacketClient %zd= sendto (%zd expected)", L, S.size());
}

//----------------------------------------------------------------------------
//
// Method-
//       PacketClient::run
//
// Purpose-
//       Run single client packet stress test (while TimerThread active.)
//
//----------------------------------------------------------------------------
virtual void
   run()                            // PacketClient::run
{
   last_recv= 0;                    // Reset the sequence identifiers
   last_send= 0;

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
     debugf("Packet client %s terminated\n", peer_addr.c_str());
}

//----------------------------------------------------------------------------
//
// Method-
//       PacketClient::stress
//
// Purpose-
//       Run packet client/server stress test.
//
//----------------------------------------------------------------------------
static void
   stress( void )                   // Packet stress test
{
   int thread_count= 1;
   if( opt_thread )
     thread_count= OPT_PACKET_CLIENTS;

   PacketClient* client[thread_count];
   for(int i= 0; i<thread_count; i++) {
     client[i]= new PacketClient();
     client[i]->start();
   }
   Thread::sleep(0.125);          // (Delay allows all Threads to start

   if( opt_verbose ) {
     if( !opt_server )
       debugf("\n");
     debugf("--packet test: Started\n");
   }

   timer_thread.start();
   timer_thread.join();

   for(int i= 0; i<thread_count; i++)
     client[i]->join();

   // Statistics
   // Note: pcc_count counts completed operations:
   //   client.send=>server.recv/send=>client.recv
   //   Some packets could be lost in server.send=>client.recv
   if( opt_verbose ) {
     debugf("--packet test: %s\n", error_count ? "FAILED" : "Complete");
     debugf("%'16zd Recv again\n", pcr_again.load());
     debugf("%'16zd Recv count\n", pcr_count.load());
     debugf("%'16zd Send again\n", pcw_again.load());
     debugf("%'16zd Send count\n", pcw_count.load());
     debugf("%'16zd Lost count\n", pcw_count.load() - pcr_count.load());
     debugf("%'16zd Operations\n", pcc_count.load());
     debugf("%'18.1f Operations/second\n"
           , double(pcc_count.load()) / opt_runtime);
   }
}
}; // class PacketClient

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
char                   buffer[32768]; // Input/output buffer
Event                  event;       // Thread ready event
Socket                 packet;      // Packet Socket
struct pollfd          pfd= {};     // Poll file descriptor
SocketSelect           select;      // Socket selector

int                    operational= false; // TRUE while operational

//----------------------------------------------------------------------------
// Constructors/Destructor
//----------------------------------------------------------------------------
   PacketServer()
:  Thread(), event()
{
   int rc= packet.open(AF_INET, SOCK_DGRAM, PF_UNSPEC);
   if( rc ) {
     trace(__LINE__, "PacketServer %d= open", rc);
     return;
   }

   int optval= true;                // (Needed before the bind)
   packet.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   if( USE_RPOLL == USE_POLL_NONBLOCK )
     packet.set_flags( packet.get_flags() | O_NONBLOCK );

   rc= packet.bind(STD_PORT);       // Set port number
   if( rc ) {
     trace(__LINE__, "PacketServer %d= bind", rc);
     return;
   }

   // Set default timeout
   struct timeval tv= { 3, 0 };     // 3.0 second timeout
   packet.set_option(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
   packet.set_option(SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

   if( USE_RPOLL == USE_POLL_SELECT )
     select.insert(&packet, POLLIN);
   else if( USE_RPOLL == USE_POLL_NONBLOCK )
     packet.set_flags( packet.get_flags() | O_NONBLOCK );

   pfd.events= POLLIN;
}

   ~PacketServer() = default;

//----------------------------------------------------------------------------
//
// Method-
//       PacketServer::run
//
// Purpose-
//       Operate the PacketServer
//
//----------------------------------------------------------------------------
virtual void
   run()                            // PacketServer::run
{
   operational= true;
   if( opt_verbose )
     debugf("Packet server %s operational\n", peer_addr.c_str());
   event.post();

   try {
     while( operational && error_count == 0 ) {
       switch( USE_RPOLL ) {
         case USE_POLL_POLL: {{{{
           int rc= packet.poll(&pfd, 1000); // 1 second timeout
           if( false )
             trace(__LINE__, "%d= poll %d {%.4x,%.4x}", rc
                  , pfd.fd, pfd.events, pfd.revents);
           if( rc < 0 ) {
             if( if_closed(pfd) )
               return;

             ++error_count;
             errorf("%4d ERROR: %d= packet.poll() %d:%s\n", __LINE__, rc
                   , errno, strerror(errno));
           }
           if( rc == 0 ) {
             ++psr_again;
             continue;
           }
           break;
         }}}}
         case USE_POLL_SELECT: {{{{
           Socket* socket= nullptr;
           socket= select.select(1000); // 1 second timeout
           if( socket == nullptr ) {
             if( if_closed(select, &packet) )
               break;

             error_count += VERIFY( if_retry() );
             ++psr_again;
             continue;
           }
           error_count += VERIFY( socket == &packet );
           break;
         }}}}
         default:
           // USE_POLL_BLOCK || USE_POLL_NONBLOCK
           break;
       }

       if( operational && !error_count )
         serve();
     }
   } catch(pub::Exception& X) {
     debugh("Exception: %s\n", X.to_string().c_str());
   } catch(std::exception& X) {
     debugh("std::Exception what(%s)\n", X.what());
   }

   // Statistics
   if( opt_verbose ) {
     debugf("--packet server info:\n");
     debugf("%'16zd Recv again\n", psr_again.load());
     debugf("%'16zd Recv block\n", psr_block.load());
     debugf("%'16zd Recv count\n", psr_count.load());
     debugf("%'16zd Send count\n", psw_count.load());

     debugf("Packet server %s terminated\n", peer_addr.c_str());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PacketServer::serve
//
// Purpose-
//       Serve one packet
//
//----------------------------------------------------------------------------
void
   serve( void )                   // Read and process one input packet
{
   struct sockaddr   from;
   socklen_t         size= sizeof(from);
   ssize_t R= packet.recvfrom(buffer, sizeof(buffer), 0, &from, &size);
   if( opt_hcdm ) {
     int ERRNO= errno;
     traceh("PacketServer %zd= packet.recvfrom\n", R);
     errno= ERRNO;
   }

   if( R > 0 ) {
     ++psr_count;

     ssize_t S= packet.sendto(buffer, R, pkt_confirm, &from, size);
     // trace(__LINE__, "PacketServer %zd= sendto", S);
     if( R != S ) {
       trace(__LINE__, "PacketServer %zd= sendto (%zd expected)", S, R);
       return;
     }

     ++psw_count;
     return;
   }

   // Error recovery for packet.recvfrom
   if( packet.is_open() ) {
     if( if_retry() ) {
       ++psr_block;
       return;
     }

     trace(__LINE__, "PacketServer %zd= packet.recvfrom", R);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PacketServer::stop
//
// Purpose-
//       Stop the PacketServer
//
//----------------------------------------------------------------------------
void
   stop()
{
   operational= false;              // Indicate terminated
   event.reset();
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
Event                  event;         // Thread started event

//----------------------------------------------------------------------------
// Constructors/Destructor
//----------------------------------------------------------------------------
   StreamClient() = default;
   ~StreamClient() = default;

//----------------------------------------------------------------------------
//
// Method-
//       StreamClient::client
//
// Purpose-
//       Standard StreamClient test: single HTTP open/write/read/close op
//
// Implementation note-
//       This Thread can still be running after the server is stopped, so we
//       ignore any errors received when not still running.
//
//----------------------------------------------------------------------------
void
   client( void )                   // Standard HTTP operation
{
   Socket socket;
   try {
     int rc= socket.open(AF_INET, SOCK_STREAM, PF_UNSPEC);
     if( rc ) {
       if( !running )
         return;
       trace(__LINE__, "StreamClient %d=open", rc);
       throw pub::Exception("StreamClient open Failure");
     }

     // Connect to server
     rc= socket.connect(peer_addr);
     if( rc < 0 ) {
       if( !running )
         return;
       trace(__LINE__, "StreamClient %d= connect", rc);
       if( ++retry_count <= USE_CONNECT_RETRY ) {
         Thread::sleep(5.0);
         return;
       }
       throw pub::Exception("StreamClient connect Failure");
     }

     // Write/read
     ssize_t L= socket.write(httpREQ, strlen(httpREQ));
     if( L <= 0 ) {
       if( !running )
         return;
       trace(__LINE__, "StreamClient %zd= write(%zd)", L, strlen(httpREQ));
       throw pub::Exception("StreamClient write Failure");
     }
     if( opt_verbose > 1 )
       debugh("StreamClient %zd= write(%s)\n", L, visify(httpREQ).c_str());
     ++scw_count;

     L= socket.read(buffer, sizeof(buffer)-1);
     if( L <= 0 ) {
       if( !running )
         return;
       trace(__LINE__, "StreamClient %zd= read(%zd)", L, strlen(httpREQ));
       throw pub::Exception("StreamClient read Failure");
     }
     buffer[L]= '\0';
     if( opt_verbose > 1 )
       debugh("StreamClient %zd= read(%s)\n", L, visify(buffer).c_str());

     if( running ) {                // (Avoids scc_count/scr_count mismatch)
       ++scr_count;
       ++scc_count;
     }
   } catch(pub::Exception& X) {
     debugh("StreamClient %s\n", X.to_string().c_str());
     ++error_count;
   } catch(std::exception& X) {
     debugh("StreamClient what(%s)\n", X.what());
     ++error_count;
   } catch(...) {
     debugh("StreamClient catch(...) socket(%4d)\n", socket.get_handle());
     ++error_count;
   }

   // Close the connection
   int rc= socket.close();
   if( rc ) {
     if( !running )
       return;
     trace(__LINE__, "StreamClient %d= close", rc);
     throw pub::Exception("StreamClient close Failure");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamClient::run
//
// Purpose-
//       Run single client stream stress test (while TimerThread active.)
//
//----------------------------------------------------------------------------
virtual void
   run()
{
   event.post();                    // Indicate ready
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

   event.reset();
   if( opt_verbose > 1 )
     debugf("Stream client %s terminated\n", peer_addr.c_str());
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamClient::stress
//
// Purpose-
//       Run stream client/server stress test.
//
//----------------------------------------------------------------------------
static void
   stress( void )                   // Stream stress test
{
   int thread_count= 1;
   if( opt_thread )
     thread_count= OPT_STREAM_CLIENTS;

   StreamClient* client[thread_count];
   for(int i= 0; i<thread_count; i++) {
     client[i]= new StreamClient();
     client[i]->start();
     client[i]->event.wait();
   }

   if( opt_verbose ) {
     if( !opt_server )
       debugf("\n");
     debugf("--stream test: Started\n");
   }

   timer_thread.start();
   timer_thread.join();

   for(int i= 0; i<thread_count; i++)
     client[i]->join();

   // Statistics marker
   if( opt_verbose ) {
     debugf("--stream test: %s\n", error_count ? "FAILED" : "Complete");
     debugf("%'16ld Recv count\n", scr_count.load());
     debugf("%'16ld Send count\n", scw_count.load());
     debugf("%'16ld Operations\n", scc_count.load());
     debugf("%'18.1f Operations/second\n"
           , double(scc_count.load()) / opt_runtime);
   }
}
}; // class StreamClient

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
// Constructors/Destructor
//----------------------------------------------------------------------------
public:
   StreamWorker(                    // Constructor
     Socket*           client)      // Connection socket
:  client(client)
{  if( HCDM ) debugh("StreamWorker(%p)::StreamWorker(%p)\n", this, client); }

virtual
   ~StreamWorker( void )            // Destructor
{  if( HCDM ) debugh("StreamWorker(%p)::~StreamWorker()...\n", this);

   delete client;
}

//----------------------------------------------------------------------------
// StreamWorker::work
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

//----------------------------------------------------------------------------
// StreamWorker::run
//----------------------------------------------------------------------------
void
   run( void )                      // Process work
{
   // Set default timeout
   struct timeval tv= { 3, 0 };     // 3.0 second timeout
   client->set_option(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
   client->set_option(SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

   for(size_t count= 0;;++count) {
     buffer[0]= '\0';
     ssize_t L= client->read(buffer, sizeof(buffer)-1);
     if( L < 0 ) {
       trace(__LINE__, "StreamWorker %zd= read", L);
       break;
     } else if( L == 0 ) {
       if( count == 0 ) {
         debugh("%4d StreamWorker HCDM\n", __LINE__); // (SSL) bug workaround
         continue;
       }
       break;
     }
     ++ssr_count;

     buffer[L]= '\0';
     if( opt_verbose > 1 )
       debugh("StreamWorker %zd= read(%s)\n", L, visify(buffer).c_str());

     const char* mess= nullptr;
     const char* C= buffer;
     std::string meth= get_token(C);
     std::string what= get_token(C);
     std::string http= get_token(C);
     if( meth == "GET" && http == "HTTP/1.1" ) {
       if( what == "/" || what == "/index.html"
           || what == "/std" || what == "/ssl" )
         mess= http200;
       else
         mess= http404;
     } else {
       mess= http400;
     }

     L= client->write(mess, strlen(mess));
     if( L <= 0 ) {
       trace(__LINE__, "StreamWorker %zd= write(%zd)", L, strlen(mess));
       break;
     }
     ++ssw_count;

     if( opt_verbose > 1 )
       debugh("StreamWorker %zd= write(%s)\n", L, visify(mess).c_str());
   }

   // Client closed or in error state. Allow immediate port re-use
   // When used, this code avoids a client "Connection refused" problem.
   if( USE_LINGER ) {               // Optional only for debugging
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
//       StreamServer
//
// Purpose-
//       Stream server thread
//
//----------------------------------------------------------------------------
class StreamServer : public Thread {
public:
char                   buffer[32768]; // Input buffer
Event                  event;       // Thread ready event
Socket                 listen;      // Listener Socket
Socket*                socket= nullptr; // Client Socket

int                    operational= false; // TRUE while operational

//----------------------------------------------------------------------------
// Constructors/Destructor
//----------------------------------------------------------------------------
   StreamServer() = default;
   ~StreamServer() = default;

//----------------------------------------------------------------------------
//
// Method-
//       StreamServer::run
//
// Purpose-
//       Operate the StreamServer
//
//----------------------------------------------------------------------------
virtual void
   run()
{
   int rc= listen.open(AF_INET, SOCK_STREAM, PF_UNSPEC);
   if( rc ) {
     trace(__LINE__, "StreamServer %d= open", rc);
     return;
   }

   int optval= true;                // (Needed before the bind)
   listen.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   if( USE_APOLL == USE_POLL_NONBLOCK )
     listen.set_flags( listen.get_flags() | O_NONBLOCK );

   rc= listen.bind(STD_PORT);       // Set port number
   if( rc ) {
     trace(__LINE__, "StreamServer %d= bind", rc);
     listen.close();
     return;
   }
   rc= listen.listen();             // Set listener socket
   if( rc ) {
     trace(__LINE__, "StreamServer %d= listen", rc);
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
             ++ssr_again;
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
             ++ssr_again;
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
   if( opt_verbose ) {
     debugf("--stream server info:\n");
     debugf("%'16ld Recv again\n", ssr_again.load());
     debugf("%'16ld Recv count\n", ssr_count.load());
     debugf("%'16ld Send count\n", ssw_count.load());

     if( opt_worker )
       WorkerPool::debug();

     debugf("Stream server %s terminated\n", peer_addr.c_str());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       StreamServer::stop
//
// Purpose-
//       Stop the StreamServer
//
//----------------------------------------------------------------------------
void
   stop()
{
   operational= false;              // Indicate terminated
   event.reset();

   if( USE_STOP_HCDM ) {
     if( false ) {                  // TRUE: Tests accept block logic
       debugh("%4d %s stop DISABLED\n", __LINE__, __FILE__);
       return;
     }
     debugh("%4d %s stop\n", __LINE__, __FILE__);
   }

   int rc= listen.close();        // Close the listener Socket
   if( USE_STOP_HCDM || opt_verbose > 1 )
     debugh("%4d %s %d= listen.close()\n", __LINE__, __FILE__, rc);

   //-------------------------------------------------------------------------
   // Create a dummy connection to complete any pending listen, ignoring any
   // errors that occur.
   reconnect();
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

     fprintf(stderr, "  --server=host:port\tUse remote server\n");
     fprintf(stderr, "  --thread\tRun multi-threaded stream client\n");
     fprintf(stderr, "  --worker\tRun multi-threaded stream server\n");
   });

   tc.on_init([](int, char**)
   {
     if( HCDM )
       opt_hcdm= true;

     if( VERBOSE > opt_verbose )
       opt_verbose= VERBOSE;

     if( opt_hcdm || true ) {
       debug_set_head(Debug::HEAD_THREAD);
       debug_set_mode(Debug::MODE_INTENSIVE);
     }

     setlocale(LC_NUMERIC, "");     // Allows printf("%'d\n", 123456789);

     if( opt_target ) {
       opt_server= false;
       peer_addr= opt_target;
     } else {
       is_server= !opt_client && !opt_packet && !opt_stream;
       opt_server= true;
       peer_addr= Socket::get_host_name() + ":" + std::to_string(STD_PORT);
     }

     if( !opt_client && !opt_server && !opt_packet && !opt_stream )
       opt_client= true;

     if( opt_runtime == 0 && (opt_packet || opt_stream) )
       opt_runtime= 20;

     return 0;
   });

   tc.on_main([tr](int, char*[])
   {
     if( opt_verbose ) {
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);
       debugf("\n");
       debugf("Settings:\n");
       debugf("%5d: runtime\n",  opt_runtime);
       if( opt_target )
         debugf("%5s: server: %s\n", torf(is_server), opt_target);
       else
         debugf("%5s: server: %s\n", torf(is_server), peer_addr.c_str());
       debugf("%5d: verbose\n",opt_verbose);

       debugf("%5s: client\n", torf(opt_client));
       debugf("%5s: packet\n", torf(opt_packet));
       debugf("%5s: stream\n", torf(opt_stream));
       debugf("%5s: thread\n", torf(opt_thread));
       debugf("%5s: worker\n", torf(opt_worker));

       // Debugging, experimentation
       debugf("\n");
       debugf("%5s: USE_LINGER\n", torf(USE_LINGER));
       debugf("%5s: USE_PACKET_CONFIRM\n", torf(USE_PACKET_CONFIRM));
       debugf("%5s: USE_PACKET_CONNECT\n", torf(USE_PACKET_CONNECT));

       debugf("%5d: USE_APOLL: %s\n", USE_APOLL, poll_method[USE_APOLL]);
       debugf("%5d: USE_RPOLL: %s\n", USE_RPOLL, poll_method[USE_RPOLL]);
       debugf("%5d: USE_SPOLL: %s\n", USE_SPOLL, poll_method[USE_SPOLL]);

       debugf("%5d: OPT_PACKET_CLIENTS\n", OPT_PACKET_CLIENTS);
       debugf("%5d: OPT_STREAM_CLIENTS\n", OPT_STREAM_CLIENTS);
     }

     StreamClient stream_client;
     StreamServer stream_server;
     PacketServer packet_server;

     if( opt_client ) {
       // Client statistics are not provided for this test.
       PacketClient packet_client;
       reset_statistics();
       if( opt_verbose )
         debugf("\n");
       if( opt_server ) {
         stream_server.start();
         packet_server.start();
         packet_server.event.wait();
         stream_server.event.wait();
       }

       if( opt_verbose )
         debugf("--client test: Started\n");
       running= true;
       packet_client.client();
       stream_client.client();
       running= false;
       if( opt_verbose )
         debugf("--client test: Complete\n");

       if( opt_server ) {
         packet_server.stop();
         stream_server.stop();
         packet_server.join();
         stream_server.join();
       }
     }

     if( opt_packet ) {
       reset_statistics();
       PacketClient packet_client;
       if( opt_server ) {
         if( opt_verbose )
           debugf("\n");
         packet_server.start();
         packet_server.event.wait();
       }

       packet_client.stress();

       if( opt_server ) {
         packet_server.stop();
         packet_server.join();
       }
     }

     if( opt_stream ) {
       reset_statistics();
       if( opt_server ) {
         if( opt_verbose )
           debugf("\n");
         stream_server.start();
         stream_server.event.wait();
       }

       stream_client.stress();

       if( opt_server ) {
         stream_server.stop();
         stream_server.join();
       }
     }

     if( is_server ) {
       reset_statistics();
       if( opt_verbose )
         debugf("\n");
       packet_server.start();
       stream_server.start();
       packet_server.event.wait();
       stream_server.event.wait();

       Thread::sleep(opt_runtime);

       packet_server.stop();
       stream_server.stop();
       packet_server.join();
       stream_server.join();
     }

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
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
       if( value )
         opt_target= value;
     }

     return 0;
   });

   tc.on_term([]() {});             // (Unused so far)

   //-------------------------------------------------------------------------
   // Run the test
   int rc= tc.run(argc, argv);      // (Allow debugging statment before exit)
   return rc;
}
