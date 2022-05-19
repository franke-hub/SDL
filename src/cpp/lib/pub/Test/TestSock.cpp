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
//       2022/05/18
//
// Implementation notes-
//       TODO: Implement --client, --server, --thread options
//
//----------------------------------------------------------------------------
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
#include <pub/Socket.h>             // For pub::Socket, tested
#include <pub/Thread.h>             // For pub::Thread

#include "pub/TEST.H"               // For VERIFY, ...
#include "pub/Wrapper.h"            // For class Wrapper

// For SocketSelect

using namespace _PUB_NAMESPACE;     // For pub:: classes
using namespace pub::debugging;     // For debugging functions
using namespace pub::utility;       // For utility`functions

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
,  USE_CLIENT_COUNT= 16             // Number of stress test clients
,  USE_LINGER= true                 // Use SO_LINGER timeout for client
,  USE_STOP_HCDM= false             // Write StreamServer::stop messages
,  USE_RECV_SELECT= true            // Use recv SocketSelect?
,  USE_SEND_SELECT= true            // Use send SocketSelect?

// TODO: REMOVE UNUSED OPTIONS
// Default options - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
,  USE_RUNTIME= 60                  // Delay before terminating
,  USE_CLIENT= true                 // Include Clients?
,  USE_THREAD= true                 // Use StreamClient (else direct call)
,  USE_TRACE=  true                 // Use memory trace?
,  USE_SERVER= true                 // Include Servers?
,  USE_WORKER= true                 // Use WorkerObject (else direct call)
}; // Generic enum

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define SIZEOF_BUFFER       (10000) // Maximum buffer size

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define LOCK_GUARD(x) std::lock_guard<decltype(x)> lock(x)

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
#if 0
static Socket::Port    hostPort= 7015; // Host Port (For linux firewall)
static Socket::Addr    peerAddr;    // Peer Addr
static Socket::Port    peerPort;    // Peer Port
static Semaphore       semaphore;   // Stress control semaphore

static char*           buffer= nullptr; // Transmission buffer
static char*           checker= nullptr; // Transmission checker buffer
static int             delay;       // TRUE iff using send delay
static int             sender;      // POSITIVE iff sender, NEGATIVE iff receiver
#endif

static std::string     host_name;   // The host name
static int             error_count= 0;  // Error counter

//----------------------------------------------------------------------------
// Extended options
//----------------------------------------------------------------------------
static const char*     opt_client= nullptr;
static int             opt_dgram=  false;
static int             opt_runtime= 0;
static int             opt_server= false;
static int             opt_stress= false;
static struct option   opts[]=      // The getopt_long parameter: longopts
{  {"client",   required_argument, nullptr,           0}
,  {"datagram", no_argument,       &opt_dgram,     true}
,  {"runtime",  required_argument, nullptr,           0}
,  {"server",   no_argument,       &opt_server,    true}
,  {"stress",   no_argument,       &opt_stress,    true}
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
//       reconnect
//
// Purpose-
//       Attempt to create a connection, ignoring all errors.
//
//----------------------------------------------------------------------------
static void
   reconnect(                       // Attempt a dummy connection
     int               port)        // To this port
{
   try {
     Socket socket;                 // (Socket closed by destructor)
     int rc= socket.open(AF_INET, SOCK_STREAM, PF_UNSPEC);
     if( rc == 0 ) {
       std::string name_port= host_name;
       name_port += ':';
       name_port += std::to_string(port);
       rc= socket.connect(name_port);
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
//       std_client
//
// Purpose-
//       Run client connection test.
//
//----------------------------------------------------------------------------
static void
   std_client( void )               // Standard client test
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
     int port= STD_PORT;
     std::string name_port= host_name;
     name_port += ':';
     name_port += std::to_string(port);
     rc= socket.connect(name_port);
     if( rc < 0 ) {
       debugh("%d= STD connect %d:%s\n", rc, errno, strerror(errno));
       if( ++error_count < 10 )
         return;
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
   } catch(pub::Exception& X) {
     debugh("std_client: %s\n", X.to_string().c_str());
     ++error_count;
   } catch(std::exception& X) {
     debugh("std_client: what(%s)\n", X.what());
     ++error_count;
   } catch(...) {
     debugh("std_client: catch(...) socket(%4d)\n", socket.get_handle());
     ++error_count;
   }

   // Close the connection
   int rc= socket.close();
   if( rc ) {
     debugh("%d= STD close %d:%s\n", rc, errno, strerror(errno));
     throw pub::Exception("Client open Failure");
   }

   if( error_count ) {
     debugf("STD Client: FAILED\n");
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       std_stress
//
// Purpose-
//       Run server stress test.
//
//----------------------------------------------------------------------------
static void
   std_stress( void )               // Standard stress test
{
   if( opt_verbose )
     debugf("--stress test: Started\n");

   error_count= 0;                  // No errors yet
   long op_count= 0;
   double runtime= 0.0;
   Interval interval;
   interval.start();
   for(;;) {
     std_client();
     runtime= interval.stop();
     if( error_count || runtime >= double(opt_runtime) )
       break;

     op_count++;
   }

   // Statistics
   if( opt_verbose ) {
     debugf("--stress test: %s\n", error_count ? "FAILED" : "Complete");
     debugf("%'10ld Operations\n", op_count);
     debugf("%'14.3f Seconds\n", runtime);
     debugf("%'14.3f Operations/second\n", double(op_count) / runtime);
   }
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
//       PacketClient
//
// Purpose-
//       Packet (datagram) client thread
//
//----------------------------------------------------------------------------
class PacketClient : public Thread {
protected:
public:
Event                  event;       // Thread ready event

   PacketClient() = default;
   ~PacketClient() = default;

virtual void
   run()                            // PacketClient::run
{
   event.post();
}

void
   stop()                           // PacketClient::stop
{
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
std::mutex             mutex;       // Protects 'operational'

public:
char                   ibuff[512];  // Input buffer
char                   obuff[256];  // Output buffer
Event                  event;       // Thread ready event
Socket                 packet;      // Packet handler Socket
SocketSelect           select;      // Socket selector
ssize_t                r_again= 0;  // Receive EAGAIN counter
ssize_t                r_block= 0;  // Receive EWOULDBLOCK counter
ssize_t                r_count= 0;  // Received packet counter
ssize_t                s_again= 0;  // Send EAGAIN counter
ssize_t                s_count= 0;  // Sent packet counter

int                    operational= false; // TRUE while operational
int                    port=STD_PORT; // Listener Port

   PacketServer()
:  Thread(), mutex(), event(), packet(), select()
{
   for(unsigned i= 0; i<sizeof(obuff); ++i)
     obuff[i]= i;
}

   ~PacketServer() = default;

virtual void
   run()                            // PacketServer::run
{
   int rc= packet.open(AF_INET, SOCK_DGRAM, PF_UNSPEC);
   if( rc ) {
     errorp("PacketServer.open");
     return;
   }

   int optval= true;                // (Needed before the bind)
   packet.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   rc= packet.bind(port);           // Set port number
   if( rc ) {
     errorp("PacketServer.bind");
     return;
   }

   try {
     select.insert(&packet, POLLIN);

     operational= true;
     if( opt_verbose )
       debugf("\nPacket %s:%d operational\n", host_name.c_str(), port);
     event.post();

     while( operational && error_count == 0 ) {
       if( USE_RECV_SELECT ) {
         Socket* socket= select.select(1000); // 1 second timeout

         if( socket == nullptr ) {
           error_count += VERIFY( errno == EAGAIN );
           if( error_count )
             break;
           ++r_again;
           continue;
         }

         error_count += VERIFY( socket == &packet );
         if( error_count )
           break;
       }

       ssize_t L;
       memset(ibuff, 0, sizeof(ibuff));
       if( USE_RECV_SELECT ) {
         L= packet.read(ibuff, sizeof(ibuff));
         if( opt_hcdm )
           traceh("%4zd= packet.read\n", L);
       } else {
         L= packet.recv(ibuff, sizeof(ibuff), MSG_DONTWAIT);
         if( opt_hcdm )
           traceh("%4zd= packet.recv(,,MSG_DONTWAIT)\n", L);
         if( L <= 0 ) {
           error_count += VERIFY( errno == EAGAIN || errno == EWOULDBLOCK );
           if( error_count )
             break;
           Thread::sleep(1.0/128.0);
           ++r_block;
           continue;
         }
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
       ++r_count;
     }
   } catch(pub::Exception& X) {
     debugh("Exception: %s\n", X.to_string().c_str());
   } catch(std::exception& X) {
     debugh("std::Exception what(%s)\n", X.what());
   }

   if( opt_verbose )
     debugf("Packet %s:%d terminated\n", host_name.c_str(), port);
}

void
   stop()                           // PacketServer::stop
{
   {{{{
     LOCK_GUARD(mutex);
     operational= false;            // Indicate terminated

//   int rc= packet.close();        // Close the Socket (NOT NEEDED)
//   if( opt_verbose )
//     debugh("%4d %s %d= packet.close()\n", __LINE__, __FILE__, rc);
   }}}}

   //-------------------------------------------------------------------------
   // We now create a dummy connection to complete any pending listen,
   // ignoring any errors that occur.
// reconnect(port);                 // NOT NEEDED or VALID
}

void
   test()                           // PacketServer::test
{
   Socket packet;                   // Datagram transmission Socket
   SocketSelect select;             // Socket selector

   int rc= packet.open(AF_INET, SOCK_DGRAM, PF_UNSPEC);
   if( rc ) {
     errorp("PacketServer.open");
     return;
   }

   int optval= true;                // (Needed before the bind)
   packet.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   std::string conn= std::to_string(port);
   conn= ":" + conn;
   rc= packet.connect(conn);        // Connect to port
   if( rc ) {
     errorp("PacketServer.bind");
     return;
   }

   select.insert(&packet, POLLOUT);
   if( opt_verbose )
     debugf("--datagram test: Started\n");

   error_count= 0;                  // No errors yet
   Interval interval;
   interval.start();
   while( error_count == 0 && interval.stop() < opt_runtime ) {
     if( USE_SEND_SELECT ) {
       Socket* socket= select.select(63); // Approx. 1/16 second timeout
       if( socket == nullptr ) {
         ++s_again;
         continue;
       }
       error_count += VERIFY( socket == &packet );
       if( error_count )
         break;
     }
     ssize_t L= packet.write(obuff, sizeof(obuff));
     error_count += VERIFY( L == sizeof(obuff) );
     if( error_count )
       break;
     ++s_count;
   }

   Thread::sleep(0.125);            // (Wait for in transit test packet)
   if( opt_verbose ) {
     debugf("--datagram test: %s\n", error_count ? "FAILED" : "Complete");
     debugf("%'10zd Send again\n", s_again);
     debugf("%'10zd Send count\n", s_count);
     debugf("%'10zd Recv again\n", r_again);
     debugf("%'10zd Recv block\n", r_block);
     debugf("%'10zd Recv count\n", r_count);
     debugf("%'10zd Lost count\n", s_count - r_count);
     debugf("%'10.0f Operations/second\n", double(r_count) / opt_runtime);
   }
}
}; // class PacketServer

//----------------------------------------------------------------------------
//
// Class-
//       StreamClient
//
// Purpose-
//       Stream client thread
//
//----------------------------------------------------------------------------
class StreamClient : public Thread {
protected:
public:
Event                  event;       // Thread ready event

   StreamClient() = default;
   ~StreamClient() = default;

virtual void
   run()                            // StreamClient::run
{
   event.post();
}

void
   stop()                           // StreamClient::stop
{
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
std::mutex             mutex;       // Protects 'operational'

public:
char                   buffer[32768]; // Input buffer
Socket*                client= nullptr; // Client Socket
Event                  event;       // Thread ready event
Socket                 listen;      // Listener Socket

int                    operational= false; // TRUE while operational
int                    port=STD_PORT; // Listener Port

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

   rc= listen.bind(port);           // Set port number
   if( rc ) {
     errorp("StreamServer.bind");
     listen.close();
     return;
   }

   operational= true;
   if( opt_verbose )
     debugf("\nServer %s:%d operational\n", host_name.c_str(), port);
   event.post();

   try {
     while( operational && error_count == 0 ) {
       client= listen.listen();
       if( client ) {
         serve(client);
         delete client;
         client= nullptr;
       }
     }
   } catch(pub::Exception& X) {
     debugh("Exception: %s\n", X.to_string().c_str());
   } catch(std::exception& X) {
     debugh("std::Exception what(%s)\n", X.what());
   }

   if( opt_verbose )
     debugf("Server %s:%d terminated\n", host_name.c_str(), port);
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
   {{{{
     LOCK_GUARD(mutex);
     operational= false;            // Indicate terminated

     if( USE_STOP_HCDM ) {
       if( false ) {                // TRUE: Tests "::accept would block" logic
         debugh("%4d %s stop DISABLED\n", __LINE__, __FILE__);
         return;
       }
       debugh("%4d %s stop\n", __LINE__, __FILE__);
     }

     if( client) {
       int rc= client->close();     // Close the client Socket
       if( USE_STOP_HCDM || opt_verbose > 1 )
         debugh("%4d %s %d= client->close()\n", __LINE__, __FILE__, rc);
     }
     int rc= listen.close();        // Close the listener Socket
     if( USE_STOP_HCDM || opt_verbose > 1 )
       debugh("%4d %s %d= listen.close()\n", __LINE__, __FILE__, rc);
   }}}}

   //-------------------------------------------------------------------------
   // We now create a dummy connection to complete any pending listen,
   // ignoring any errors that occur.
   reconnect(port);
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
//   fprintf(stderr, "  --client=host:port\tUse remote server\n");
     fprintf(stderr, "  --datagram\tRun datagram test\n");
     fprintf(stderr, "  --runtime=<seconds>\n");
//   fprintf(stderr, "  --server\tRun server\n");
     fprintf(stderr, "  --stress\tRun stress test\n");
//   fprintf(stderr, "  --thread\tRun multi-threaded client stress test\n");
   });

   tc.on_init([](int argc, char* argv[]) // (Unused in this sample)
   { (void)argc; (void)argv;        // (Unused arguments)

     if( HCDM )
       opt_hcdm= true;

     if( VERBOSE > opt_verbose )
       opt_verbose= VERBOSE;

     if( opt_hcdm ) {
//     debug_set_head(Debug::HEAD_THREAD);
       debug_set_mode(Debug::MODE_INTENSIVE);
     }

     if( !opt_client && !opt_server && !opt_dgram && !opt_stress )
       opt_stress= true;

     if( (opt_client || opt_server || opt_stress) && opt_runtime == 0 )
       opt_runtime= 20;

     setlocale(LC_NUMERIC, "");     // Allows printf("%'d\n", 123456789);

     host_name= Socket::get_host_name();

     return 0;
   });

   tc.on_term([]()                  // (Unused in this sample)
   {
   });

   tc.on_parm([tr](std::string name, const char* value)
   {
     // --client: NOT CODED YET

     if( name == "runtime" ) {
       if( value == nullptr )
         value= "60";
       opt_runtime= tr->ptoi(value, name.c_str());
     }

     return 0;
   });

   tc.on_main([tr](int, char*[])
   {
     if( opt_verbose ) {
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);
       debugf("\n");
       debugf("Settings:\n");
//     if( opt_client )
//       debugf("%5s: client %s\n",   "true", opt_client);
//     else
//       debugf("%5s: client\n",   "false");
       debugf("%5s: datagram\n", torf(opt_dgram));
       debugf("%5s: server\n",   torf(opt_server));
       debugf("%5d: runtime\n",  opt_runtime);
       debugf("%5s: stress\n",   torf(opt_stress));
//     debugf("%5s: thread\n",   torf(opt_thread));
//     debugf("%5s: trace\n",    torf(opt_trace));
       debugf("%5d: verbose\n",  opt_verbose);
       debugf("%5s: USE_LINGER\n", torf(USE_LINGER));
       debugf("%5s: USE_RECV_SELECT\n", torf(USE_RECV_SELECT));
       debugf("%5s: USE_SEND_SELECT\n", torf(USE_SEND_SELECT));
     }

//   StreamClient stream_client;
     StreamServer stream_server;

     if( opt_dgram ) {
       PacketServer packet_server;
       packet_server.start();
       packet_server.event.wait();
       packet_server.test();
       packet_server.stop();
       packet_server.join();
     }

     if( opt_stress ) {
       stream_server.start();
       stream_server.event.wait();
       std_stress();
       stream_server.stop();
       stream_server.join();
     } else if( false ) { // NOT READY YET
       if( opt_server ) {
         stream_server.start();
         stream_server.event.wait();
       }
       if( opt_client ) {
         std_client();
       } else if( opt_server ) {
         Thread::sleep(opt_runtime);
       }
     }

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}

