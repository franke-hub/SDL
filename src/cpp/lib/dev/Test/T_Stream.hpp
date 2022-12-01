//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       T_Stream.hpp
//
// Purpose-
//       T_Stream.cpp classes
//
// Last change date-
//       2022/11/27
//
//----------------------------------------------------------------------------
#ifndef T_STREAM_HPP_INCLUDED
#define T_STREAM_HPP_INCLUDED

#include <atomic>                   // For std::atomic
#include <memory>                   // For std::shared_ptr
#include <cstddef>                  // For offsetof
#include <cstdint>                  // For UINT16_MAX
#include <ctime>                    // For time, ...
#include <fcntl.h>                  // For open, O_*, ...
#include <getopt.h>                 // For getopt_long()
#include <unistd.h>                 // For close, ...
#include <sys/mman.h>               // For mmap, ...
#include <sys/signal.h>             // For signal, ...

#include <pub/TEST.H>               // For VERIFY macro
#include <pub/Debug.h>              // For debugging classes and functions
#include <pub/Dispatch.h>           // For namespace pub::dispatch
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Event.h>              // For pub::Event
#include <pub/Statistic.h>          // For pub::Statistic
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::utility::to_string, visify
#include <pub/Worker.h>             // For pub::WorkerPool::reset

#include "pub/http/Agent.h"         // For pub::http::ClientAgent, ListenAgent
#include "pub/http/Client.h"        // For pub::http::Client
#include "pub/http/Ioda.h"          // For pub::http::Ioda
#include "pub/http/Listen.h"        // For pub::http::Listen
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Recorder.h"      // For pub::Recorder
#include "pub/http/Request.h"       // For pub::http::Request
#include "pub/http/Response.h"      // For pub::http::Response
#include "pub/http/Server.h"        // For pub::http::Server
#include "pub/http/Stream.h"        // For pub::http::Stream

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For pub classes
using namespace PUB::debugging;     // For pub debugging functions
using namespace PUB::http;          // For pub::http classes
using PUB::utility::visify;         // (Import)
using namespace std;                // For std classes

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  IODM= false                      // Input/Output Debug Mode?
,  VERBOSE= 1                       // Verbosity: Higher is more verbose

,  DIR_MODE= (S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH) // Directory mode
,  MAX_REQUEST_COUNT= 4             // Maximum running request count
,  MAX_RESPONSE_SIZE= 0x00100000    // Maximum response data length
,  PROT_RW= (PROT_READ | PROT_WRITE) // Read/write access mode
,  TRACE_SIZE= 0x00100000           // Default trace table size (1M)
,  USE_INTENSIVE= true              // Option: Use intensive debug mode
,  USE_LOGGER= false                // Option: Use logger
,  USE_SIGNAL= false                // Option: Use signal handler
,  USE_TIMING_RECORD= false         // Option: Use timing record
}; // generic enum

enum                                // Default option values
{  DEFAULT_OPTIONS                  // (Dummy)
,  OPT_THREAD= 4                    // Stress test client thread count

,  USE_CLIENT= false                // --client
,  USE_SERVER= false                // --server
,  USE_STRESS= 0                    // --stress
,  USE_TRACE=  false                // --trace
,  USE_VERIFY= false                // --verify
,  USE_WORKER= true                 // --worker (Server threads)
}; // default options

static constexpr const double USE_RUNTIME= 2.0; // --runtime

// Imported Options
typedef const char CC;
static constexpr CC*   HTTP_GET=  Options::HTTP_METHOD_GET;
static constexpr CC*   HTTP_HEAD= Options::HTTP_METHOD_HEAD;
static constexpr CC*   HTTP_POST= Options::HTTP_METHOD_POST;
static constexpr CC*   HTTP_PUT=  Options::HTTP_METHOD_PUT;

static constexpr CC*   HTTP_SIZE= Options::HTTP_HEADER_LENGTH;
static constexpr CC*   HTTP_TYPE= Options::HTTP_HEADER_TYPE;

static constexpr CC*   cert_file= "public.pem";  // The public certificate file
static constexpr CC*   priv_file= "private.pem"; // The private key file

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Debug*          debug= nullptr; // Our Debug object
static string          host= "localhost"; // The connection host
static string          port= "8080"; // The connection port
static string          test_url= "/"; // The stress test URL
static void*           trace_table= nullptr; // The internal trace area

static ClientAgent*    client_agent= nullptr; // Our ClientAgent
static ListenAgent*    listen_agent= nullptr; // Our ListenAgent

// Test controls
typedef std::atomic_size_t          atomic_count_t;
static atomic_count_t  error_count= 0; // Error counter
static atomic_count_t  send_op_count= 0; // The total send complete count
static Event           test_ended;  // The test ended Event
static Event           test_start;  // The start test Event
static int             running= false; // Test running indicator

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
static int             opt_hcdm= HCDM;  // --hcdm (Hard Core Debug Mode)
static int             opt_iodm= IODM;  // --iodm (I/O Debug Mode)
static int             opt_index;   // Option index

static const char*     opt_debug= nullptr; // --debug
static int             opt_verbose= VERBOSE; // --verbose
static int             opt_bringup= false; // Run bringup test?
static int             opt_client= USE_CLIENT; // Run basic client test?
static int             opt_major= -1; // Major test id TODO: REMOVE
static int             opt_minor= -1; // Minor test id TODO: REMOVE
static double          opt_runtime= USE_RUNTIME; // Stress test run time, in seconds
static int             opt_server= USE_SERVER; // Run server?
static int             opt_ssl= false;  // Run SSL client/server?
static int             opt_stress= USE_STRESS; // Run client stress test?

static int             opt_trace= USE_TRACE; // Create trace file?
static int             opt_verify= USE_VERIFY; // Verify file data?
static int             opt_worker= USE_WORKER; // Create server threads?

static const char*     OSTR= ":";   // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",    no_argument,       &opt_help,    true} // --help
,  {"hcdm",    no_argument,       &opt_hcdm,    true} // --hcdm
,  {"iodm",    no_argument,       &opt_iodm,    true} // --iodm

,  {"debug",   required_argument, nullptr,      0} // --debug <string>
,  {"verbose", optional_argument, &opt_verbose, 1} // --verbose {optional}
,  {"bringup", no_argument,       &opt_bringup,  true} // --bringup
,  {"client",  no_argument,       &opt_client,  true} // --client
,  {"host",    required_argument, nullptr,      0}    // --host <string>
,  {"port",    required_argument, nullptr,      0}    // --port <string>
,  {"major",   optional_argument, &opt_major,   0}    // --major
,  {"minor",   optional_argument, &opt_minor,   0}    // --minor
,  {"runtime", required_argument, nullptr,      0}    // --runtime <string>
,  {"server",  optional_argument, &opt_server,  true} // --server
,  {"ssl",     no_argument,       &opt_ssl,  true}    // --stress
,  {"stream",  optional_argument, &opt_stress,  OPT_THREAD} // --stream (alias)
,  {"stress",  optional_argument, &opt_stress,  OPT_THREAD} // --stress
,  {"trace",   no_argument,       &opt_trace,   true} // --trace
,  {"verify",  no_argument,       &opt_verify,  true} // --verify
,  {"worker",  no_argument,       &opt_worker,  true} // --worker

// This option can be used if USE_WORKER defaulted true
,  {"no-worker", no_argument,     &opt_worker,  false} // --no-worker
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_HELP
,  OPT_HCDM
,  OPT_IODM

,  OPT_DEBUG
,  OPT_VERBOSE
,  OPT_BRINGUP
,  OPT_CLIENT
,  OPT_HOST
,  OPT_PORT
,  OPT_MAJOR
,  OPT_MINOR
,  OPT_RUNTIME
,  OPT_SERVER
,  OPT_SSL
,  OPT_STREAM
,  OPT_STRESS
,  OPT_TRACE
,  OPT_VERIFY
,  OPT_WORKER

,  OPT_NO_WORKER
,  OPT_SIZE
};

//----------------------------------------------------------------------------
// Global constructor/destructor (For hard core debugging)
//----------------------------------------------------------------------------
static struct Global {
   Global( void )
{
   if( HCDM )
     printf("%4d %s Global!\n", __LINE__, __FILE__);
}

   ~Global( void )
{
   if( HCDM )
     printf("%4d %s Global~\n", __LINE__, __FILE__);
}
} global_constructor_destructor;

//----------------------------------------------------------------------------
//
// Subroutine-
//       i2v
//
// Purpose-
//       Convert intptr_t  to void*
//
//----------------------------------------------------------------------------
static inline void* i2v(intptr_t i) { return (void*)i; }

//----------------------------------------------------------------------------
//
// Subroutine-
//       do_JOIN
//
// Purpose-
//       Join strings
//
//----------------------------------------------------------------------------
static string                       // The JOINed string
   do_JOIN(                         // Join a string list
     const char**      args,        // The string array
     string            ins= "")     // The insertion string
{
   string out;
   for(size_t i= 0; args[i]; ++i) {
     string line= args[i];
     size_t x= line.find("{}");
     while( x != string::npos ) {
       line= line.substr(0, x) + ins + line.substr(x+2);
       x= line.find("{}");
     }
     out += line;
     out += "\r\n";
   }

   return out;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       page200
//       page403
//       page404
//       page405
//       page500
//
// Purpose-
//       Generate response message
//
//----------------------------------------------------------------------------
static string                       // The 200 Dummy File message
   page200(string body)
{  static const char* args[]=
               { "<html><head><title>PAGE 200</title></head>"
               , "<body><h1 align=\"center\">Default Response Page</h1>"
               , "Body[{}]"
               , "</body></html>"
               , nullptr
               };
   return do_JOIN(args, body);
}

static string                       // The 403 forbidden message
   page403(string file)
{  static const char* args[]=
               { "<html><head><title>FORBIDDEN</title></head>"
               , "<body><h1 align=\"center\">FORBIDDEN</h1>"
               , "File[{}] access forbidden."
               , "</body></html>"
               , nullptr
               };
   return do_JOIN(args, file);
}

static string                       // The 404 file not found message
   page404(string file)
{  static const char* args[]=
               { "<html><head><title>FILE NOT FOUND</title></head>"
               , "<body><h1 align=\"center\">FILE NOT FOUND</h1>"
               , "File[{}] not found."
               , "</body></html>"
               , nullptr
               };
   return do_JOIN(args, file);
}

static string                       // The 405 method not supported message
   page405(string meth)
{  static const char* args[]=
               { "<html><head><title>METHOD NOT ALLOWED</title></head>"
               , "<body><h1 align=\"center\">METHOD NOT ALLOWED</h1>"
               , "Method[{}] is not supported."
               , "</body></html>"
               , nullptr
               };
   return do_JOIN(args, meth);
}

static string                       // The 500 server error message
   page500(string info)
{  static const char* args[]=
               { "<html><head><title>SERVER ERROR</title></head>"
               , "<body><h1 align=\"center\">SERVER ERROR</h1>"
               , "[{}]"
               , "</body></html>"
               , nullptr
               };
   return do_JOIN(args, info);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       logger
//       log_request
//
// Purpose-
//       Write message to log
//       Log a request/response
//
//----------------------------------------------------------------------------
static void
   logger(string mess)              // Write message to log
{
   debugh("\n%s\n", mess.c_str());  // (For now)
}

static void
   log_request(Request& Q, Response& S) // Log a request/response
{
   if( USE_LOGGER ) {
     string mess=
     utility::to_string("{peer} [{time}] {http} %3d %s %s {}"
                       , S.get_code(), Q.method.c_str(), Q.path.c_str());
     logger(mess);
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
{  return cc ? "true" : "false"; }

//----------------------------------------------------------------------------
//
// Class-
//       TimerThread
//
// Purpose-
//       Background Thread that sets and clears `running`
//
//----------------------------------------------------------------------------
class TimerThread : public Thread {
public:
   ~TimerThread( void ) = default;
   TimerThread( void ) = default;

virtual void
   run( void )
{
   running= true;
   test_start.post();

   Thread::sleep(opt_runtime);      // (Run the test)

   running= false;
   test_start.reset();
}
}; // class TimerThread

//----------------------------------------------------------------------------
//
// Class-
//       ClientThread
//
// Purpose-
//       The T_Stream client Thread.
//
// Implementation notes-
//       Although each client thread operates asynchronously and independently,
//       the ClientThread simplifies initialization.
//
//----------------------------------------------------------------------------
class ClientThread : public Thread { // The client Thread
public:
std::shared_ptr<Client>client;      // The Client

std::atomic_size_t     cur_op_count= 0; // The number of running requests
Event                  ready;       // Thread ready event
Event                  ended;       // Thread ended event
Event                  send_end;    // Send completion event (for run_one)

static std::atomic_int client_serial; // Global serial number
int                    serial= -1;  // Serial number

// Callback handlers
std::function<void(void)>
                       do_NEXT;     // The next completion handler

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::ClientThread
//       ClientThread::~ClientThread
//
// Function-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   ClientThread( void ) : serial(client_serial++) {}
   ~ClientThread( void ) = default;

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   debug(const char* info= "")      // Debugging display
{  debugf("ClientThread(%p)::debug(%s)\n", this, info);

   debugf("..[%d] cur_op_count(%'zd)\n", serial, cur_op_count.load());
   debugf("..ready(%d) ended(%d) send_end(%d)\n"
         , ready.is_post(), ended.is_post(), send_end.is_post());
   if( client ) client->debug("ClientThread");
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::close
//
// Purpose-
//       Close the Client
//
//----------------------------------------------------------------------------
void
   close( void )                    // Close the Client
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] ClientThread::close\n", serial);

   if( client ) {
     client->close();
     client->wait();
     client= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::do_POST
//
// Function-
//       Create/write a POST request
//
//----------------------------------------------------------------------------
void
   do_POST(                         // Create a POST request
     std::string       path,        // The URL
     std::string       data)        // The POST data
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] do_POST(%s,%s)\n", serial, path.c_str(), data.c_str());

   std::shared_ptr<ClientStream> stream= client->make_stream();
   if( stream.get() == nullptr )
     return;

   std::shared_ptr<ClientRequest> Q= stream->get_request();
   Q->method= HTTP_POST;
   Q->path= path;

   std::shared_ptr<ClientResponse> S= stream->get_response();
   do_RESP(S);

   if( opt_iodm ) {
     debugf("do_POST(%s,%s)\n", path.c_str(), data.c_str());
//   (Options not set)
//   Options& opts= (Options&)*Q.get();
//   for(Options::const_iterator it= opts.begin(); it != opts.end(); ++it)
//      debugf("%s: %s\n", it->first.c_str(), it->second.c_str());
   }

   Q->write(data.c_str(), data.size()); // Write the POST data
   Q->write();
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::do_RESP
//
// Function-
//       Handle a response (Define response handlers)
//
//----------------------------------------------------------------------------
void
   do_RESP(                         // Handle
     std::shared_ptr<Response> S)   // This response
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] do_RESP(%p)\n", serial, S.get());

   // Google: "c++ shared_ptr lambda function" to see why a weak_ptr MUST be
   // captured instead of a shared_ptr.
   std::weak_ptr<Response> weak= S; // (Passed to lambda functions)

   S->on_error([this, weak](const std::string& mess) {
     std::shared_ptr<Response> S= weak.lock();
     if( opt_hcdm && opt_verbose )
       debugh("[%2d] on_error(%s) Response(%p)\n", serial
             , mess.c_str(), S.get());
     if( S ) {
       std::shared_ptr<Request> Q= S->get_request();
       debugh("Request(%p) %s %s error %s\n", Q.get(), Q->method.c_str()
             , Q->path.c_str(), mess.c_str());
     }
   });

   S->on_ioda([this, weak](Ioda& data) {
     std::shared_ptr<Response> S= weak.lock();
     if( opt_hcdm && opt_verbose )
       debugh("[%2d] on_ioda(%p) Response(%p)\n", serial, &data, S.get());
     if( S ) {
       Ioda& ioda= S->get_ioda();
       if( ioda.get_used() <= MAX_RESPONSE_SIZE )
         ioda += std::move(data);
     }
   });

   S->on_end([this, weak]( void ) {
     std::shared_ptr<Response> S= weak.lock();
     if( opt_hcdm && opt_verbose )
       debugh("[%2d] S.on_end Response(%p)\n", serial, S.get());
     if( S ) {
       std::shared_ptr<Request> Q= S->get_request();
       if( opt_iodm ) {
         debugh("Response code %d\n", S->get_code());
         Options& opts= (Options&)*S.get();
         for(Options::const_iterator it= opts.begin(); it != opts.end(); ++it)
            debugf("%s: %s\n", it->first.c_str(), it->second.c_str());
       }
       if( S->get_code() == 200 ) {
         if( opt_verify && Q->method == HTTP_GET ) {
           std::string path= Q->path;
           if( path == "/" )
             path= "/index.html";
           std::string have_string= (std::string)S->get_ioda();
           std::string want_string= page200(path);
           if( want_string != have_string ) {
             ++error_count;
             have_string= visify(have_string);
             want_string= visify(want_string);
             debugh("%4d %s Data verify error:\n", __LINE__, __FILE__);
             debugh("Have '%s'\n", have_string.c_str());
             debugh("Want '%s'\n", want_string.c_str());
           }
         }

         if( opt_iodm ) {
           Ioda& ioda= S->get_ioda();
           std::string data_string= (std::string)ioda;
           if( ioda.get_used() > MAX_RESPONSE_SIZE )
             data_string=
             utility::to_string("<<Response data error: length(%ld) > %d>>"
                             , ioda.get_used(), MAX_RESPONSE_SIZE);
           data_string= visify(data_string);
           debugh("Data: \n%s\n", data_string.c_str());
         }
       }
     }
   });
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::do_SEND
//
// Function-
//       Create/write a request (method "GET" or "HEAD")
//
//----------------------------------------------------------------------------
void
   do_SEND(                         // Create a request
     std::string       meth,        // The Request method
     std::string       path)        // The URL
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] do_SEND(%s,%s)\n", serial, meth.c_str(), path.c_str());

   ++cur_op_count;

   std::shared_ptr<ClientStream> stream= client->make_stream();
   if( stream.get() == nullptr ) {
     debugf("%4d %s Should Not Occur\n", __LINE__, __FILE__);
     return;
   }

   std::shared_ptr<ClientRequest> Q= stream->get_request();
   Q->method= meth;
   Q->path= path;

   std::shared_ptr<ClientResponse> S= stream->get_response();
   do_RESP(S);

   Q->on_end([this]() {
     if( opt_hcdm && opt_verbose )
       traceh("Q.on_end current(%zd) total(%zd) running(%d)\n"
             , cur_op_count.load(), send_op_count.load(), running);
     if( running )                  // (Only count running send completions)
       ++send_op_count;

     --cur_op_count;
     do_NEXT();
   });

   if( opt_iodm ) {
     debugh("do_SEND(%s,%s)\n", meth.c_str(), path.c_str());
//   (Options not set)
//   Options& opts= (Options&)*Q.get();
//   for(Options::const_iterator it= opts.begin(); it != opts.end(); ++it)
//      debugf("%s: %s\n", it->first.c_str(), it->second.c_str());
   }

   Q->write();
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::get_client
//
// Purpose-
//       Activate the Client
//
//----------------------------------------------------------------------------
void
   get_client( void )               // Activate the client
{
// Options opts;                    // TODO: Options TBD
   client= client_agent->connect(host + ":" + port); // Create the client

   if( !client ) {
     debugf("Unable to connect %s:%s\n", host.c_str(), port.c_str());
     exit(EXIT_FAILURE);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::run_one
//
// Purpose-
//       Process a complete client connection.
//
//----------------------------------------------------------------------------
virtual void
   run_one( void )                  // Process a complete operation
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] ClientThread::run_one...\n", serial);

   // Initialize
   send_end.reset();
   do_NEXT= [this](void) {
     if( !send_end.is_post() )
       send_end.post();
   };

   try {                            // Run the stress test
     get_client();
     do_SEND(HTTP_GET, test_url);
     send_end.wait();
     if( opt_minor == 0 ) {
       client->close();
       client= nullptr;
     } else {
       client->close();
       client->wait();
       client= nullptr;
     }
   } catch(Exception& X) {
     ++error_count;
     debugf("%4d Exception: %s\n", __LINE__, X.to_string().c_str());
   } catch(std::exception& X) {
     ++error_count;
     debugf("%4d std::Exception what(%s)\n", __LINE__, X.what());
   }

   if( opt_hcdm && opt_verbose )
     debugf("...[%2d] ClientThread.run_one\n", serial);
   Trace::trace(".TXT", serial, "TS.run_one exit");
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::run
//
// Purpose-
//       Operate the client Thread stress test
//
//----------------------------------------------------------------------------
virtual void
   run( void )                      // Operate the client Thread
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] ClientThread::run...\n", serial);

// debugf("%4d %s HCDM\n", __LINE__, __FILE__);
   //-------------------------------------------------------------------------
   // Client per connection version - in test -- -- -- -- -- -- -- -- -- -- --
   if( opt_major >= 0 ) {           // Use run_one()
     ended.reset();                 // Indicate not complete
     ready.post();                  // Indicate ready
     test_start.wait();             // Wait for start signal

     try {
       while( running && error_count == 0 )
         run_one();
     } catch(Exception& X) {
       ++error_count;
       debugf("%4d Exception: %s\n", __LINE__, X.to_string().c_str());
     } catch(std::exception& X) {
       ++error_count;
       debugf("%4d std::Exception what(%s)\n", __LINE__, X.what());
     } catch(...) {
       ++error_count;
       debugf("%4d catch(...)\n", __LINE__);
     }
     return;
   }

   //-------------------------------------------------------------------------
   // Single client version (DEFAULT)
   get_client();
   do_NEXT= [this](void) {
     if( opt_hcdm && opt_verbose )
       debugh("[%2d] do_NEXT current(%zd) total(%zd)\n", serial
             , cur_op_count.load(), send_op_count.load());

     // debugh("stress.next... %s\n", running ? "running" : "quiesced");
     statistic::Active* stat= &Request::obj_count;
     while( running && cur_op_count.load() < MAX_REQUEST_COUNT ) {
       if( opt_hcdm && opt_verbose ) // Use detailed tracking?
         debugh("%4ld {%2ld,%2ld,%2ld} cur_op_count %zd\n"
                , stat->counter.load(), stat->minimum.load()
                , stat->current.load(), stat->maximum.load()
                , cur_op_count.load());
       do_SEND(HTTP_GET, test_url);
     }

     if( opt_hcdm && opt_verbose ) // Use detailed tracking?
       debugh("%4ld {%2ld,%2ld,%2ld} %srunning, cur_op_count %zd\n"
              , stat->counter.load(), stat->minimum.load()
              , stat->current.load(), stat->maximum.load()
              , running ? "" : "NOT "
              , cur_op_count.load());
   };

   ended.reset();                   // Indicate not complete
   ready.post();                    // Indicate ready
   test_start.wait();               // Wait for start signal

   try {                            // Run the stress test
     do_NEXT();                     // Prime the pump
     test_ended.wait();             // Wait for the test to complete
   } catch(Exception& X) {
     ++error_count;
     debugf("%4d Exception: %s\n", __LINE__, X.to_string().c_str());
   } catch(std::exception& X) {
     ++error_count;
     debugf("%4d std::Exception what(%s)\n", __LINE__, X.what());
   } catch(...) {
     ++error_count;
     debugf("%4d catch(...)\n", __LINE__);
   }

   client->wait();                  // Wait for operations to complete
   ready.reset();                   // Not ready
   ended.post();                    // Indicate complete

   if( opt_hcdm && opt_verbose )
     debugf("...[%2d] ClientThread.run\n", serial);

   Trace::trace(".TXT", __LINE__, "CT.run exit");
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::start
//
// Function-
//       Start the ClientThread.
//
//----------------------------------------------------------------------------
virtual void
   start( void )                    // Start the ClientThread
{
   ended.reset();
   ready.reset();
   Thread::start();
   ready.wait();
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::statistics
//
// Purpose-
//       Statistics display
//
// Implementation notes-
//       Currently intended for bringup
//
//----------------------------------------------------------------------------
static void
   statistics( void )               // Display statistics
{
   // Verify object counters (TODO: REMOVE) vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
   statistic::Active* stat= &Stream::obj_count;
   debugf("%'16ld {%2ld,%2ld,%2ld} Stream counts\n", stat->counter.load()
         , stat->minimum.load(), stat->current.load(), stat->maximum.load());

   stat= &Request::obj_count;
   debugf("%'16ld {%2ld,%2ld,%2ld} Request counts\n", stat->counter.load()
         , stat->minimum.load(), stat->current.load(), stat->maximum.load());

   stat= &Response::obj_count;
   debugf("%'16ld {%2ld,%2ld,%2ld} Response counts\n", stat->counter.load()
         , stat->minimum.load(), stat->current.load(), stat->maximum.load());

   if( USE_TIMING_RECORD ) {
     debugf("\n\n");
     Recorder::get()->report([](Recorder::Record& record) {
       debugf("%s\n", record.h_report().c_str());
     }); // reporter.report
   }

   if( opt_verbose > 1 ) {
     debugf("\n");
     WorkerPool::debug();
     WorkerPool::reset();
   }

   error_count += VERIFY( Stream::obj_count.current.load() == 0 );
   error_count += VERIFY( Request::obj_count.current.load() == 0 );
   error_count += VERIFY( Response::obj_count.current.load() == 0 );
   // Verify object counters (TODO: REMOVE) ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

   // Reset the statistics
   stat= &Stream::obj_count;
   stat->counter.store(0);
   stat->minimum.store(0);
   stat->current.store(0);
   stat->maximum.store(0);

   stat= &Request::obj_count;
   stat->counter.store(0);
   stat->minimum.store(0);
   stat->current.store(0);
   stat->maximum.store(0);

   stat= &Response::obj_count;
   stat->counter.store(0);
   stat->minimum.store(0);
   stat->current.store(0);
   stat->maximum.store(0);

   Recorder::get()->reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::test_client
//
// Purpose-
//       Client functional test
//
//----------------------------------------------------------------------------
static void
   test_client( void )              // Client functional test
{  debugf("\nClientThread.test_client...\n");

// debugf("%4d %s HCDM\n", __LINE__, __FILE__);
   error_count= 0;

   ClientThread client;
   client.do_NEXT= [](void) {
     if( opt_hcdm && opt_verbose ) debugf("test_client.do_NEXT NOP\n");
   };
   client.get_client();

   // Bringup tests
   client.do_SEND(HTTP_GET, "/");
   client.do_SEND(HTTP_HEAD, "/index.htm");

   client.do_POST("/post-test", "This is the post data, all of it.");

   client.do_SEND(HTTP_GET, "/403-test");
   client.do_SEND(HTTP_GET, "/404-test");
   client.do_SEND("MOVE", "/405-test");

   client.do_SEND(HTTP_GET, "/tiny.html"); // Used in stress test
   client.do_SEND(HTTP_GET, "/utf8.html"); // Regression test

   // Error tests
#if 0  // TODO: CLIENT RECOVERY NEEDED FOR ERROR TESTS
   if( opt_verbose ) {
     client.wait();
     debugf("Malformed method...\n");
   }
   do_SEND(" GET", "/");            // Malformed method
   if( opt_verbose ) {
     client.wait();
     debugf("Malformed path...\n");
   }
   client.do_SEND("GET", " /");     // Malformed path
// client.do_SEND( TODO: CODE );    // Malformed protocol
#endif

   client.do_SEND(HTTP_GET, "/last.html"); // The last request
   client.wait();                   // Wait for Client to complete
   Thread::sleep(0.125);            // Delay to allow Server completion

   Trace::trace(".TXT", __LINE__, "TC.client close");
   client.close();                  // Close the Client
   WorkerPool::reset();
   debugf("...Driver.test_client\n");
   Trace::trace(".TXT", __LINE__, "TC.client exit");
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::test_stress
//
// Purpose-
//       Stress test
//
//----------------------------------------------------------------------------
static void
   test_stress( void )              // Stress test
{  debugf("\nClientThread.test_stress... (%.1f seconds)\n", opt_runtime);

// debugf("%4d %s HCDM\n", __LINE__, __FILE__);
   error_count= 0;

   //-------------------------------------------------------------------------
   // Client per connection version - in test -- -- -- -- -- -- -- -- -- -- --
   if( opt_major == 0 ) {           // run_one() bringup
     ClientThread ct;

     running= true;
     for(int i= 0; i<opt_stress; ++i) {
       ct.run_one();
     }
     running= false;

     double op_count= double(send_op_count.load());
     debugf("%'16.3f operations\n", op_count);
     debugf("%'16.3f operations/second\n", op_count/opt_runtime);

//debugf("\n"); client_agent->debug("ended (client)");
     client_agent->reset();
//debugf("%4d %s HCDM\n", __LINE__, __FILE__);
     client_agent->stop();
//debugf("\n"); client_agent->debug("reset (client)");

//debugf("\n"); listen_agent->debug("ended (server)");
     listen_agent->reset();
//debugf("%4d %s HCDM\n", __LINE__, __FILE__);
     listen_agent->stop();
//debugf("\n"); listen_agent->debug("reset (server)");

     return;
   }

   //-------------------------------------------------------------------------
   // Single client version (DEFAULT)
   ClientThread* client[opt_stress];
   for(int i= 0; i<opt_stress; i++) {
     client[i]= new ClientThread();
     client[i]->start();
   }

   if( opt_verbose )
     debugh("--%s_stress test: Started\n", opt_ssl ? "ssl" : "std");
   test_ended.reset();
   TimerThread timer_thread;
   timer_thread.start();
   timer_thread.join();
   test_ended.post();

   debugh("--%s_stream test: %s\n", opt_ssl ? "ssl" : "std"
         , error_count ? "FAILED" : "Complete");

   double op_count= double(send_op_count.load());
   debugf("%'16.3f operations\n", op_count);
   debugf("%'16.3f operations/second\n", op_count/opt_runtime);

   Trace::Record* R= Trace::trace(128-32);
   if( R ) {
     for(int i= 0; i<128; ++i) {
       ((char*)R)[i]= (char)(i % 32);
     }
     strcpy(R->value, "Stress test completed");
     R->trace("....");
   }

   for(int32_t i= 0; i<opt_stress; i++) {
     Trace::trace(".TST", "WAIT", client[i], i2v(i));
     client[i]->wait();
   }

//debugf("\n"); client_agent->debug("ended (client)");
   client_agent->reset();
//debugf("%4d %s HCDM\n", __LINE__, __FILE__);
   client_agent->stop();
//debugf("\n"); client_agent->debug("reset (client)");

//debugf("\n"); listen_agent->debug("ended (server)");
   listen_agent->reset();
//debugf("%4d %s HCDM\n", __LINE__, __FILE__);
   listen_agent->stop();
//debugf("\n"); listen_agent->debug("reset (server)");

   for(int i= 0; i<opt_stress; i++) {
     client[i]->close();
//client[i]->debug("test_stress");
     client[i]->join();
     delete client[i];
   }
//debugf("%4d %s All clients closed\n", __LINE__, __FILE__);
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::wait
//
// Purpose-
//       Wait for all outstanding requests to complete
//
//----------------------------------------------------------------------------
void
   wait( void )                     // Wait for outstanding request completion
{  if( opt_hcdm && opt_verbose )
     debugh("[%2d] wait ClientThread\n", serial);

   if( client )
     client->wait();
}
}; // class ClientThread

std::atomic_int        ClientThread::client_serial= 0; // Global serial number

//----------------------------------------------------------------------------
//
// Class-
//       ServerThread
//
// Purpose-
//       The T_Stream listener Thread
//
// Implementation notes-
//       The ServerThread is not a Thread. It's driven asynchronously.
//
//----------------------------------------------------------------------------
class ServerThread {                // The listener Thread
public:
std::shared_ptr<Listen>
                       listen;      // Our Listener

Event                  ready;       // Thread ready Event
Event                  ended;       // Thread ended Event

bool                   operational= false; // TRUE while operational

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::ServerThread
//       ServerThread::~ServerThread
//
// Function-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
public:
   ServerThread( void )
{
   Options opts;                    // Server Options
   opts.insert("cert", cert_file);  // The public certificate file
   opts.insert("key",  priv_file);  // The private key file
   opts.insert("http1", "true");    // HTTP1 allowed

   std::string host= ":"; host += port;
   listen= listen_agent->connect(host, AF_INET, &opts); // Create Listener

   // Initialize the Listen handlers
   listen->on_close([this]( void ) {
     if( opt_hcdm && opt_verbose )
       debugf("ServerThread(%p)::on_close\n", this);
   });

   listen->on_request([this](ServerRequest& Q) {
     try {
       if( opt_iodm || (opt_hcdm && opt_verbose) )
         debugh("ServerThread(%p)::on_request(%s)\n", this
               , Q.method.c_str());
       if( Q.method == HTTP_GET || Q.method == HTTP_HEAD )
         do_HGET(Q);
       else if( Q.method == HTTP_POST )
         do_POST(Q);
       else
         do_HTML(Q, 405, page405(Q.method));
     } catch(Exception& X) {
       do_HTML(Q, 500, page500((std::string)X));
     } catch(std::exception& X) {
       string info("std::exception(");
       info += X.what();
       info += ")";
       do_HTML(Q, 500, page500(info));
     } catch(...) {
       do_HTML(Q, 500, "catch(...)");
     }
   });

   ended.reset();
   ready.post();
   operational= true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   ~ServerThread( void )
{
   listen->on_close([]( void ) {});
   listen->on_request([](ServerRequest&) {});
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::do_FILE
//
// Function-
//       Read data file
//
//----------------------------------------------------------------------------
void
   do_FILE(ServerRequest& Q)
{
   if( opt_hcdm && opt_verbose )
     debugf("ServerThread(%p)::do_FILE(%s)\n", this, Q.path.c_str());

   string path= Q.path;
   if( path[0] != '/' || path.find("/../") != string::npos ) {
     do_HTML(Q, 500, page500("parser fault"));
     return;
   }

   // TODO: SCAFFOLDED <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
   if( path == "/403-test" )
     do_HTML(Q, 403, page403(path));
   else if( path == "/404-test" )
     do_HTML(Q, 404, page404(path));
   else if( path == "/405-test" )
     do_HTML(Q, 405, page405(path));
   else if( path == "/500-test" )
     do_HTML(Q, 500, page500(path));
   else {
     if( path == "/" )
       path= "/index.html";
     path= "html" + path;

     do_HTML(Q, 200, page200(path));
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::do_HGET
//
// Function-
//       Handle "GET" or "HEAD" request
//
//----------------------------------------------------------------------------
void
   do_HGET(ServerRequest& Q)
{  if( opt_hcdm && opt_verbose )
     debugf("ServerThread(%p)::do_%s\n", this, Q.method.c_str());

   do_FILE(Q);
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::do_HTML
//
// Function-
//       Generate HTML response
//
//----------------------------------------------------------------------------
void
   do_HTML(ServerRequest& Q, int code, string html)
{  if( opt_hcdm && opt_verbose )
     debugf("ServerThread(%p)::do_HTML(%d)\n", this, code);

   ServerResponse& S= *Q.get_response();
   S.set_code(code);                // Set response code
   log_request(Q, S);

   S.insert(HTTP_TYPE, "text/html; charset=utf-8");
   S.insert(HTTP_SIZE, std::to_string(html.size()));
   if( Q.method != HTTP_HEAD )
     S.write(html);

   S.write();
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::do_POST
//
// Function-
//       Handle "POST" request
//
//----------------------------------------------------------------------------
void
   do_POST(ServerRequest& Q)
{  if( opt_hcdm && opt_verbose )
     debugf("ServerThread(%p)::do_POST\n", this);

   string body=
   utility::to_string("POST[%s]", ((string)Q.get_ioda()).c_str());
   do_HTML(Q, 200, page200(body));
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::run
//
// Function-
//       Operate the server Thread
//
// Implementation notes-
//       The ServerThread actually runs completely asynchronously.
//       This implementation allows an alternate mechanism to be added.
//
//----------------------------------------------------------------------------
void
   run( void )
{  // NOT CODED YET
debugf("%4d %s HCDM\n", __LINE__, __FILE__);
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::stop
//
// Function-
//       Terminate the server Thread
//
//----------------------------------------------------------------------------
void
   stop( void )                     // Shut down the ServerThread
{  if( opt_hcdm ) debugf("ServerThread(%p)::stop\n", this);

   operational= false;
   listen->close();
   ended.post();
}
}; // class ServerThread
#endif // T_STREAM_HPP_INCLUDED
