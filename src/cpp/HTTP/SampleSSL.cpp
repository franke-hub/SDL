//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       SampleSSL.cpp
//
// Purpose-
//       Sample HTTP/HTTPS Client/Server, using openssl socket layer.
//
// Last change date-
//       2022/06/15
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic<>
#include <mutex>                    // For std::lock_guard, ...

#include <errno.h>                  // For errno
#include <fcntl.h>                  // For open, O_*, ...
#include <getopt.h>                 // For getopt()
#include <string.h>                 // For memset, ...
#include <openssl/err.h>            // For ERR_error_string
#include <openssl/ssl.h>            // For openssl SSL methods
#include <unistd.h>                 // For close, ...
#include <sys/mman.h>               // For mmap, ...
#include <sys/time.h>               // For timeval

#include <pub/utility.h>            // For to_string, visify
#include <pub/Debug.h>              // For debugging
#include <pub/Event.h>              // For pub::Event
#include <pub/Interval.h>           // For pub::Interval
#include <pub/Semaphore.h>          // For pub::Semaphore
#include "pub/Socket.h"             // The pub::Socket Object
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Worker.h>             // For pub::Worker, pub::WorkerPool

using namespace _PUB_NAMESPACE;     // For Socket, ...
using namespace _PUB_NAMESPACE::debugging; // Debugging functions
using _PUB_NAMESPACE::utility::visify;
using std::atomic;
using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  SSL_PORT= 8443                   // Our SSL port number
,  STD_PORT= 8080                   // Our STD port number

// Default options - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
,  USE_RUNTIME= 10                  // Default test runtime
,  USE_CLIENT= true                 // Include client tests?
,  USE_THREAD= true                 // Use multiple clients?
,  USE_SERVER= true                 // Include Servers?
,  USE_STRESS= true                 // Run stress tests?
,  USE_WORKER= true                 // Use ServerWorker?
,  USE_VERBOSE= 0                   // Default verbosity
}; // Generic enum

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define LOCK_GUARD(x) std::lock_guard<decltype(x)> lock(x)

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::string     host_name;   // The host name
static Interval        interval;    // Stress interval timer
static string          STD_addr;    // STD "host:port" connection address
static string          SSL_addr;    // SSL "host:port" connection address
static int             is_server= false; // Run a server?

// SSL controls
static SSL_CTX*        client_CTX= nullptr; // Common client SSL_CTX
static SSL_CTX*        server_CTX= nullptr; // Common server SSL_CTX

// Test controls
static Event           test_start;  // The test start Event
static int             error_count= 0; // Error counter
static int             running= false; // Test running indicator

// Statistics
static atomic<size_t>  op_count;    // Operation counter

//----------------------------------------------------------------------------
// HTTP responses
//----------------------------------------------------------------------------
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
// Options
//----------------------------------------------------------------------------
static int             opt_help= false; // --help or error
static int             opt_index;   // Option index
static int             opt_runtime= USE_RUNTIME; // Server run time
static int             opt_client=  USE_CLIENT; // Use client?
static int             opt_server=  USE_SERVER; // Use server?
static int             opt_stress=  USE_STRESS; // Use stress test?
static int             opt_thread=  USE_THREAD; // Use client thread?
static int             opt_worker=  USE_WORKER; // Use worker?
static int             opt_verbose= USE_VERBOSE; // --verbose{=verbosity}

static struct option   OPTS[]=      // Options
{  {"help",     no_argument,       &opt_help,     true}
,  {"runtime",  required_argument, nullptr,          0}
,  {"verbose",  optional_argument, nullptr,          0}

,  {"client",      no_argument,    &opt_client,   true} // Use client test?
,  {"server",      no_argument,    &opt_server,   true} // Run server?
,  {"stress",      no_argument,    &opt_stress,   true} // Use stress test?
,  {"thread",      no_argument,    &opt_thread,   true} // Use multi-client?
,  {"worker",      no_argument,    &opt_worker,   true} // Use server workers?
,  {"no-client",   no_argument,    &opt_client,  false} // Don't use client
,  {"no-server",   no_argument,    &opt_server,  false} // Don't run server
,  {"no-stress",   no_argument,    &opt_stress,  false} // Don't use stress
,  {"no-thread",   no_argument,    &opt_thread,  false} // Don't use multi-
,  {"no-worker",   no_argument,    &opt_worker,  false} // Don't use workers
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX
{  OPT_HELP
,  OPT_RUNTIME
,  OPT_VERBOSE
};

//----------------------------------------------------------------------------
//
// Subroutine-
//       CTX_error
//
// Purpose-
//       Handle CTX creation error
//
//----------------------------------------------------------------------------
static void
   CTX_error(                       // Handle CTX creation error
     const char*       fmt)         // Format string (with one %s)
{
   char buffer[256];                // Working buffer
   long E= ERR_get_error();         // Get last error code
   ERR_error_string(E, buffer);
   std::string S= pub::utility::to_string(fmt, buffer);
   throw SocketException(S);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       ctx_password_cb
//
// Purpose-
//       Our pem_password_cb
//
//----------------------------------------------------------------------------
static int                          // Actual password length
   ctx_password_cb(                 // Our pem password callback
    char*              buff,        // Return buffer address (for password)
    int                size,        // Return buffer length  (for password)
    int                encrypt,     // TRUE:encryption, FALSE:decryption
    void*              userdata)    // User data
{
   if( HCDM )
     debugf("%4d HCDM(%p,%d,%d,%p)\n", __LINE__, buff, size, encrypt, userdata);

   if( encrypt ) {                  // If encryption
     debugf("%4d HCDM SHOULD NOT OCCUR\n", __LINE__);
     return -1;                     // (NOT SUPPORTED)
   }

   const char* result= "xxyyz";     // Our (not so secret) password
   int L= strlen(result);           // Resultant length
   if( L > size ) L= size;          // (Cannot exceed maximum)

   memcpy(buff, result, L);         // Set the resultant
   return L;
}

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
//       initialize_SSL
//
// Purpose-
//       Initialize SSL             // (Not needed)
//
//----------------------------------------------------------------------------
static inline void
   initialize_SSL( void )           // Initialize SSL
{
#if 0
static std::mutex      mutex;       // Latch protecting initialized
static int             initialized= false; // TRUE when initialized

   LOCK_GUARD(mutex);
   if( !initialized ) {
     SSL_library_init();
     SSL_load_error_strings();
     ERR_load_BIO_strings();
     OpenSSL_add_all_algorithms();

     initialized= true;
   }
#endif
}

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
//       new_client_CTX
//
// Purpose-
//       Create a client SSL_CTX
//
//----------------------------------------------------------------------------
static SSL_CTX*
   new_client_CTX( void )           // Create a client SSL_CTX
{
   const SSL_METHOD* method= TLS_client_method();
   SSL_CTX* context= SSL_CTX_new(method);
   if( context == nullptr )
     CTX_error("SSL_CTX_new: %s");

   SSL_CTX_set_mode(context, SSL_MODE_AUTO_RETRY);
   SSL_CTX_set_default_passwd_cb(context, ctx_password_cb);

   return context;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       new_server_CTX
//
// Purpose-
//       Create a server SSL_CTX
//
//----------------------------------------------------------------------------
static SSL_CTX*
   new_server_CTX(                  // Create a client SSL_CTX
     const char*       pub_file,    // The public cert file name
     const char*       key_file)    // The private key file name
{
   const SSL_METHOD* method= TLS_server_method();
   SSL_CTX* context= SSL_CTX_new(method);
   if( context == nullptr )
     CTX_error("SSL_CTX_new: %s");
   SSL_CTX_set_default_passwd_cb(context, ctx_password_cb);

   if( SSL_CTX_use_certificate_file(context, pub_file, SSL_FILETYPE_PEM) <= 0 ) {
     debugf("new_serverCTX(%s,%s) invalid public file\n", pub_file, key_file);
     CTX_error("use_certificate file: %s");
   }

   if( SSL_CTX_use_PrivateKey_file(context, key_file, SSL_FILETYPE_PEM) <= 0 ) {
     debugf("new_serverCTX(%s,%s) invalid key file\n", pub_file, key_file);
     CTX_error("use_PrivateKey file: %s");
   }

   if( !SSL_CTX_check_private_key(context) )
   {
     debugf("new_server_CTX(%s,%s) key mismatch\n", pub_file, key_file);
     CTX_error("Public/private key mismatch: %s");
   }

   SSL_CTX_set_mode(context, SSL_MODE_AUTO_RETRY);

   return context;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       reconnect
//
// Purpose-
//       Attempt to create a connection, ignoring any result.
//
//----------------------------------------------------------------------------
static void
   reconnect(                       // Attempt a dummy connection
     int               port)        // To this port
{
   try {
     Socket socket;

     socket.open(AF_INET, SOCK_STREAM, 0);
     std::string name_port= host_name;
     name_port += ':';
     name_port += std::to_string(port);
     socket.connect(name_port);
     Thread::sleep(0.125);
   } catch(...) {
     debugf("%4d catch(...)\n", __LINE__);
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

   if( ERRNO ) {                    // If error
     tracef(" %d:%s\n", ERRNO, strerror(ERRNO)); // (errno information)
   } else {
     tracef("\n");
   }

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
   // Reset statistics
   error_count= 0;
   op_count.store(0);

   // Start the test
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
//       STD_client
//
// Purpose-
//       Standard client stress test thread
//
//----------------------------------------------------------------------------
class STD_client : public Thread {
public:
char                   buffer[32768]; // Input buffer
Event                  event;         // Thread started event

//----------------------------------------------------------------------------
// Constructors/Destructor
//----------------------------------------------------------------------------
   STD_client() = default;
   ~STD_client() = default;

//----------------------------------------------------------------------------
//
// Method-
//       STD_client::client
//
// Purpose-
//       STD client test: single HTTP open/write/read/close op
//
// Implementation note-
//       This Thread can still be running after the server is stopped, so we
//       ignore any errors received when not still running.
//
//----------------------------------------------------------------------------
void
   client( void )                   // Standard HTTP operation
{
static const char*     request=     // The request data
   "GET /std HTTP/1.1\r\n"
   "\r\n"
   ;

   Socket socket;
   try {
     int rc= socket.open(AF_INET, SOCK_STREAM, PF_UNSPEC);
     if( rc ) {
       if( !running )
         return;
       trace(__LINE__, "STD_client %d=open", rc);
       throw pub::Exception("STD_client open Failure");
     }

     // Connect to server
     rc= socket.connect(STD_addr);
     if( rc < 0 ) {
       if( !running )
         return;
       trace(__LINE__, "STD_client %d= connect", rc);
       throw pub::Exception("STD_client connect Failure");
     }

     // Write/read
     ssize_t L= socket.write(request, strlen(request));
     if( L <= 0 ) {
       if( !running )
         return;
       trace(__LINE__, "STD_client %zd= write(%zd)", L, strlen(request));
       throw pub::Exception("STD_client write Failure");
     }
     if( opt_verbose > 1 )
       debugh("STD_client %zd= write(%s)\n", L, visify(request).c_str());

     L= socket.read(buffer, sizeof(buffer)-1);
     if( L <= 0 ) {
       if( !running )
         return;
       trace(__LINE__, "STD_client %zd= read(%zd)", L, strlen(request));
       throw pub::Exception("STD_client read Failure");
     }
     buffer[L]= '\0';
     if( opt_verbose > 1 )
       debugh("STD_client %zd= read(%s)\n", L, visify(buffer).c_str());

     ++op_count;
   } catch(pub::Exception& X) {
     debugh("STD_client %s\n", X.to_string().c_str());
     ++error_count;
   } catch(std::exception& X) {
     debugh("STD_client what(%s)\n", X.what());
     ++error_count;
   } catch(...) {
     debugh("STD_client catch(...) socket(%4d)\n", socket.get_handle());
     ++error_count;
   }

   // Close the connection
   int rc= socket.close();
   if( rc ) {
     if( !running )
       return;
     trace(__LINE__, "STD_client %d= close", rc);
     throw pub::Exception("STD_client close Failure");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       STD_client::run
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
     debugh("%4d pub::Exception: %s\n", __LINE__, X.to_string().c_str());
   } catch(std::exception& X) {
     debugh("%4d std::exception what(%s)\n", __LINE__, X.what());
   }

   event.reset();
   if( opt_verbose > 1 )
     debugf("Stream client %s terminated\n", STD_addr.c_str());
}

//----------------------------------------------------------------------------
//
// Method-
//       STD_client::stress
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
     thread_count= 16; // OPT_STREAM_CLIENTS;

   STD_client* client[thread_count];
   for(int i= 0; i<thread_count; i++) {
     client[i]= new STD_client();
     client[i]->start();
     client[i]->event.wait();
   }

// if( opt_verbose ) {
     if( !opt_server )
       debugf("\n");
     debugf("--STD stress test: Started\n");
// }

   timer_thread.start();
   timer_thread.join();

   for(int i= 0; i<thread_count; i++)
     client[i]->join();

   // Statistics
// if( opt_verbose ) {
     debugf("--STD stress test: %s\n", error_count ? "FAILED" : "Complete");
     debugf("%'16ld Operations\n", op_count.load());
     debugf("%'18.1f Operations/second\n"
           , double(op_count.load()) / opt_runtime);
// }
}
}; // class STD_client

//----------------------------------------------------------------------------
//
// Class-
//       SSL_client
//
// Purpose-
//       SSL client stress test thread
//
//----------------------------------------------------------------------------
class SSL_client : public Thread {
public:
char                   buffer[32768]; // Input buffer
Event                  event;         // Thread started event

//----------------------------------------------------------------------------
// Constructors/Destructor
//----------------------------------------------------------------------------
   SSL_client() = default;
   ~SSL_client() = default;

//----------------------------------------------------------------------------
//
// Method-
//       SSL_client::client
//
// Purpose-
//       SSL client test: single HTTP open/write/read/close op
//
// Implementation note-
//       This Thread can still be running after the server is stopped, so we
//       ignore any errors received when not still running.
//
//----------------------------------------------------------------------------
void
   client( void )                   // Standard HTTP operation
{
static const char*     request=     // The request data
   "GET /ssl HTTP/1.1\r\n"
   "\r\n"
   ;

   SSL_socket socket(client_CTX);
   try {
     int rc= socket.open(AF_INET, SOCK_STREAM, PF_UNSPEC);
     if( rc ) {
       if( !running )
         return;
       trace(__LINE__, "SSL_client %d=open", rc);
       throw pub::Exception("SSL_client open Failure");
     }

     // Connect to server
     rc= socket.connect(SSL_addr);
     if( rc < 0 ) {
       if( !running )
         return;
       trace(__LINE__, "SSL_client %d= connect", rc);
       throw pub::Exception("SSL_client connect Failure");
     }

     // Write/read
     ssize_t L= socket.write(request, strlen(request));
     if( L <= 0 ) {
       if( !running )
         return;
       trace(__LINE__, "SSL_client %zd= write(%zd)", L, strlen(request));
       throw pub::Exception("SSL_client write Failure");
     }
     if( opt_verbose > 1 )
       debugh("SSL_client %zd= write(%s)\n", L, visify(request).c_str());

     L= socket.read(buffer, sizeof(buffer)-1);
     if( L <= 0 ) {
       if( !running )
         return;
       trace(__LINE__, "SSL_client %zd= read(%zd)", L, strlen(request));
       throw pub::Exception("SSL_client read Failure");
     }
     buffer[L]= '\0';
     if( opt_verbose > 1 )
       debugh("SSL_client %zd= read(%s)\n", L, visify(buffer).c_str());

     ++op_count;
   } catch(pub::Exception& X) {
     debugh("SSL_client %s\n", X.to_string().c_str());
     ++error_count;
   } catch(std::exception& X) {
     debugh("SSL_client what(%s)\n", X.what());
     ++error_count;
   } catch(...) {
     debugh("SSL_client catch(...) socket(%4d)\n", socket.get_handle());
     ++error_count;
   }

   // Close the connection
   int rc= socket.close();
   if( rc ) {
     if( !running )
       return;
     trace(__LINE__, "SSL_client %d= close", rc);
     throw pub::Exception("SSL_client close Failure");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_client::run
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
     debugf("Stream client %s terminated\n", SSL_addr.c_str());
}

//----------------------------------------------------------------------------
//
// Method-
//       SSL_client::stress
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
     thread_count= 32; // OPT_STREAM_CLIENTS;

   SSL_client* client[thread_count];
   for(int i= 0; i<thread_count; i++) {
     client[i]= new SSL_client();
     client[i]->start();
     client[i]->event.wait();
   }

// if( opt_verbose ) {
     if( !opt_server )
       debugf("\n");
     debugf("--SSL stress test: Started\n");
// }

   timer_thread.start();
   timer_thread.join();

   for(int i= 0; i<thread_count; i++)
     client[i]->join();

   // Statistics
// if( opt_verbose ) {
     debugf("--SSL stress test: %s\n", error_count ? "FAILED" : "Complete");
     debugf("%'16ld Operations\n", op_count.load());
     debugf("%'18.1f Operations/second\n"
           , double(op_count.load()) / opt_runtime);
// }
}
}; // class SSL_client

//----------------------------------------------------------------------------
//
// Class-
//       ServerWorker
//
// Purpose-
//       Serve one HTTP connection.
//
//----------------------------------------------------------------------------
class ServerWorker : public Worker { // The ServerWorker object
//----------------------------------------------------------------------------
// ServerWorker::Attributes
//----------------------------------------------------------------------------
public:
char                   buffer[4096]; // Input buffer;
Socket*                client;      // The client socket

//----------------------------------------------------------------------------
// ServerWorker::Constructors/Destructor
//----------------------------------------------------------------------------
public:
   ServerWorker(                    // Constructor
     Socket*           socket)      // Connection socket
:  Worker(), client(socket)
{  if( HCDM ) debugh("ServerWorker(%p)::ServerWorker(%p)\n", this, socket); }

virtual
   ~ServerWorker( void )            // Destructor
{  if( HCDM ) debugh("ServerWorker(%p)::~ServerWorker()...\n", this);

   delete client;
}

//----------------------------------------------------------------------------
// ServerWorker::work
//----------------------------------------------------------------------------
public:
virtual void
   work( void )                     // Process work
{
   if( opt_verbose > 1 )
     debugh("ServerWorker::work()\n");

   try {
     run();
   } catch(pub::Exception& X) {
     debugh("ServerWorker %s\n", X.to_string().c_str());
   } catch(std::exception& X) {
     debugh("ServerWorker what(%s)\n", X.what());
   } catch(...) {
     debugh("ServerWorker catch(...)\n");
   }

   delete this;                     // When done, delete this ServerWorker
}

//----------------------------------------------------------------------------
// ServerWorker::run
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
       trace(__LINE__, "ServerWorker %zd= read", L);
       break;
     } else if( L == 0 ) {
       if( count == 0 ) {
         debugh("%4d ServerWorker HCDM\n", __LINE__); // (SSL) bug workaround
         continue;
       }
       break;
     }

     buffer[L]= '\0';
     if( opt_verbose > 1 )
       debugh("ServerWorker %zd= read(%s)", L, visify(buffer).c_str());

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
       trace(__LINE__, "ServerWorker %zd= write(%zd)", L, strlen(mess));
       break;
     }

     if( opt_verbose > 1 )
       debugh("ServerWorker %zd= write(%s)\n", L, visify(mess).c_str());
   }

   // Client closed or in error state. Allow immediate port re-use
   struct linger optval;
   optval.l_onoff= 1;
   optval.l_linger= 0;
   client->set_option(SOL_SOCKET, SO_LINGER, &optval, sizeof(optval));
}
}; // class ServerWorker

//----------------------------------------------------------------------------
//
// Class-
//       STD_ServerThread
//
// Purpose-
//       The Server Thread
//
//----------------------------------------------------------------------------
class STD_ServerThread : public Thread { // The server Thread
//----------------------------------------------------------------------------
// STD_ServerThread::Attributes
//----------------------------------------------------------------------------
public:
std::mutex             mutex;       // Object lock
Semaphore              sem;         // Startup Semaphore
Socket                 listen;      // Our listener Socket

bool                   operational; // TRUE while operational
int                    port;        // Listener port

//----------------------------------------------------------------------------
// STD_ServerThread::Constructors
//----------------------------------------------------------------------------
public:
   ~STD_ServerThread( void )        // Destructor
{  }

   STD_ServerThread(                // Constructor
     int               port)        // Listener port
:  Thread()
,  sem(), listen(), operational(false), port(port)
{  }

//----------------------------------------------------------------------------
// STD_ServerThread::Methods
//----------------------------------------------------------------------------
public:
virtual void
   run( void )                      // Operate this Thread
{
   debugh("STD_ServerThread::run() port(%d)\n", port );

   // Initialize the socket
   int rc= listen.open(AF_INET, SOCK_STREAM, 0);
   if( rc ) {                       // If failure
     trace(__LINE__, "STD_server:open");
     return;
   }

   int optval= true;                // (Needed *before* the bind)
   listen.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   rc= listen.bind(port);           // Set port number
   if( rc ) {                       // If failure
     trace(__LINE__, "STD_server:bind");
     return;
   }
   rc= listen.listen();
   if( rc ) {                       // If failure
     trace(__LINE__, "STD_server:listen");
     return;
   }

   // Ready to go
   operational= true;
   sem.post();                      // Indicate started

   try {
     while( operational ) {
       Socket* client= listen.accept();

       {{{{
         LOCK_GUARD(mutex);
         if( operational && client ) {
           ServerWorker* worker= new ServerWorker(client);
           if( opt_worker )
             WorkerPool::work(worker);
           else
             worker->work();
           client= nullptr;
         }
       }}}}

       delete client;
     }
   } catch(pub::Exception& X) {
     debugh("Exception: %s\n", X.to_string().c_str());
   } catch(std::exception& X) {
     debugh("std::Exception what(%s)\n", X.what());
   }
}

void
   stop( void )                     // Terminate this Thread
{
   sem.reset();

   {{{{
     LOCK_GUARD(mutex);
     operational= false;            // Indicate terminated
     listen.close();                // Close the Socket
   }}}}

   //-------------------------------------------------------------------------
   // We may need to attempt a dummy connection complete the listen.
   // (We ignore any and all errors that might occur doing this.)
   reconnect(STD_PORT);
}
}; // class STD_ServerThread

//----------------------------------------------------------------------------
//
// Class-
//       SSL_ServerThread
//
// Purpose-
//       The Server Thread
//
//----------------------------------------------------------------------------
class SSL_ServerThread : public Thread { // The server Thread
//----------------------------------------------------------------------------
// SSL_ServerThread::Attributes
//----------------------------------------------------------------------------
public:
std::mutex             mutex;       // Object lock
Semaphore              sem;         // Startup Semaphore
SSL_socket             listen;      // Our listener SSL_socket

bool                   operational; // TRUE while operational
int                    port;        // Listener port

//----------------------------------------------------------------------------
// SSL_ServerThread::Constructors
//----------------------------------------------------------------------------
public:
   ~SSL_ServerThread( void )        // Destructor
{  }

   SSL_ServerThread(                // Constructor
     SSL_CTX*          context,     // SSL_CTX
     int               port)        // Listener port
:  Thread()
,  sem(), listen(context), operational(true), port(port)
{  listen.open(AF_INET, SOCK_STREAM, 0); }

//----------------------------------------------------------------------------
// SSL_ServerThread::Methods
//----------------------------------------------------------------------------
public:
virtual void
   run( void )                      // Operate this Thread
{
   debugh("SSL_ServerThread::run() port(%d)\n", port );

   // Default SO_REUSEADDR=true
   int optval= true;
   listen.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   listen.bind(port);               // Set port number
   listen.listen();
   sem.post();                      // Indicate started

   try {
     while( operational ) {
       Socket* client= listen.accept();

       {{{{
         LOCK_GUARD(mutex);
         if( operational && client ) {
           ServerWorker* worker= new ServerWorker(client);
           if( opt_worker )
             WorkerPool::work(worker);
           else
             worker->work();
           client= nullptr;
         }
       }}}}

       delete client;
     }
   } catch(pub::Exception& X) {
     debugh("Exception: %s\n", X.to_string().c_str());
   } catch(std::exception& X) {
     debugh("std::Exception what(%s)\n", X.what());
   }
}

void
   stop( void )                     // Terminate this Thread
{
   sem.reset();

   {{{{
     LOCK_GUARD(mutex);
     operational= false;              // Indicate terminated
     listen.close();                  // Close the Socket
   }}}}

   //-------------------------------------------------------------------------
   // We may need to attempt a dummy connection complete the listen.
   // (We ignore any and all errors that might occur doing this.)
   reconnect(SSL_PORT);
}
}; // class SSL_ServerThread

//----------------------------------------------------------------------------
//
// Subroutine-
//       dirty
//
// Purpose-
//       Quick and dirty test.
//
//----------------------------------------------------------------------------
static inline void
   dirty( void )                    // Quick and dirty test
{
   // Not currently used
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       info
//
// Purpose-
//       Parameter description.
//
//----------------------------------------------------------------------------
static void
   info( void)                      // Parameter description
{
   fprintf(stderr, "SampleSSL [options]\n"
                   "Options:\n"
                   "  --{no-}client\n"
                   "  --{no-}server\n"
                   "  --{no-}thread\n"
                   "  --{no-}worker\n"
                   "  --runtime=value\n"
                   "  --verbose{=value}\n"
          );

   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static void
   init( void)                      // Initialize
{
   client_CTX= new_client_CTX();
   server_CTX= new_server_CTX("public.crt", "private.key");

   host_name= Socket::gethostname();
   STD_addr= host_name + ":" + std::to_string(STD_PORT);
   SSL_addr= host_name + ":" + std::to_string(SSL_PORT);

   is_server= opt_server && !opt_client && !opt_stress;

   setlocale(LC_NUMERIC, "");     // Allows printf("%'d\n", 123456789);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       parm
//
// Purpose-
//       Parameter analysis.
//
//----------------------------------------------------------------------------
static void
   parm(                            // Parameter analysis
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 C;

   //-------------------------------------------------------------------------
   // Argument analysis
   //-------------------------------------------------------------------------
   opterr= 0;                       // Do not write error messages

   while( (C= getopt_long(argc, (char**)argv, ":", OPTS, &opt_index)) != -1 )
     switch( C )
     {
       case 0:
         switch( opt_index )
         {
           case OPT_RUNTIME:
             opt_runtime= atoi(optarg);
             break;

           case OPT_VERBOSE:
             opt_verbose= 1;
             if( optarg )
               opt_verbose= atoi(optarg);
             options::pub_verbose= opt_verbose;
             break;

           default:
             break;
         }
         break;

       case ':':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Option requires an argument '%s'.\n",
                           argv[optind-1]);
         else
           fprintf(stderr, "Option requires an argument '-%c'.\n", optopt);
         break;

       case '?':
         opt_help= true;
         if( optopt == 0 )
           fprintf(stderr, "Unknown option '%s'.\n", argv[optind-1]);
         else if( isprint(optopt) )
           fprintf(stderr, "Unknown option '-%c'.\n", optopt);
         else
           fprintf(stderr, "Unknown option character '0x%x'.\n", optopt);
         break;

       default:
         fprintf(stderr, "%4d SNO ('%c',0x%x).\n", __LINE__, C, C);
         exit( EXIT_FAILURE );
     }

   if( opt_help )
     info();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       term
//
// Purpose-
//       Terminate
//
//----------------------------------------------------------------------------
static void
   term( void)                      // Terminate
{
   SSL_CTX_free(client_CTX);
   SSL_CTX_free(server_CTX);
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
extern int                          // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   unsigned int        errorCount= 0;

   // Initialize
   parm(argc, argv);
   init();

   // Set debugging object
   Debug debug;
   Debug::set(&debug);
   debug.set_head(Debug::HEAD_THREAD);
   if( HCDM || opt_verbose > 1 ) debug.set_mode(Debug::MODE_INTENSIVE);
debug.set_mode(Debug::MODE_INTENSIVE);
   debug.debugh("SampleSSL Started...\n");

   debugf("\n");
   debugf("Settings:\n");
   debugf("%5d: runtime\n",  opt_runtime);
   debugf("%5s: stress\n",   torf(opt_stress));
   debugf("%5s: client\n",   torf(opt_client));
   debugf("%5s: thread\n",   torf(opt_thread));
   debugf("%5s: server\n",   torf(opt_server));
   debugf("%5s: worker\n",   torf(opt_worker));
   debugf("%5d: verbose\n",  opt_verbose);
   debugf("\n");

   try {
     STD_ServerThread std_server(STD_PORT); // Our STD_ServerThread
     SSL_ServerThread ssl_server(server_CTX, SSL_PORT); // Our SSL_ServerThread

     if( opt_server ) {
       std_server.start();
       ssl_server.start();

       std_server.sem.wait();       // Wait until started
       ssl_server.sem.wait();       // Wait until started
     }

     if( opt_client ) {             // Run client test?
       STD_client std_client;       // Our STD_client thread
       SSL_client ssl_client;       // Our SSL_client thread
       std_client.client();
       ssl_client.client();
     }

     if( opt_stress ) {             // Run stress test?
       debugf("\n");
       WorkerPool::reset();
       STD_client::stress();        // Run STD stress test
       Thread::sleep(0.125);        // Completion delay
       WorkerPool::debug();

       debugf("\n");
       WorkerPool::reset();
       SSL_client::stress();        // Run SSL stress test
       Thread::sleep(0.125);        // Completion delay
       WorkerPool::debug();
     }

     if( is_server ) {              // Run server?
       Thread::sleep(opt_runtime);
     }

     if( opt_server ) {
       std_server.stop();
       ssl_server.stop();
       std_server.join();
       ssl_server.join();
     }

     // line(__LINE__);
     Thread::sleep(0.5);            // Completion delay

   } catch(pub::Exception& X) {
     errorCount++;
     debugf("pub::Exception: %s\n", X.to_string().c_str());
   } catch(std::exception& X) {
     errorCount++;
     debugf("std::Exception what(%s)\n", X.what());
   } catch(...) {
     debugf("catch(...)\n");
   }

   debug.debugf("...SampleSSL complete(%d)\n", errorCount);
   Debug::set(nullptr);
   term();

   return ( errorCount != 0 );
}
