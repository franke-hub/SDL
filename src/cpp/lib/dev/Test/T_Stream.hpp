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
//       2022/07/31
//
//----------------------------------------------------------------------------
#ifndef T_STREAM_HPP_INCLUDED
#define T_STREAM_HPP_INCLUDED

#include <atomic>                   // For std::atomic
#include <memory>                   // For std::shared_ptr
#include <cstdint>                  // For UINT16_MAX
#include <fcntl.h>                  // For open, O_*, ...
#include <getopt.h>                 // For getopt_long()
#include <unistd.h>                 // For close, ...
#include <sys/mman.h>               // For mmap, ...
#include <sys/stat.h>               // For stat

#include <pub/TEST.H>               // For VERIFY macro (TODO: REMOVE)
#include <pub/Debug.h>              // For debugging classes and functions
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Event.h>              // For pub::Event
#include <pub/Statistic.h>          // For pub::Statistic (TODO: REMOVE)
#include <pub/Thread.h>             // For pub::Thread
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::utility::to_string, visify

#include "pub/http/Agent.h"         // For pub::http::ClientAgent, ServerAgent
#include "pub/http/Client.h"        // For pub::http::Client
#include "pub/http/Data.h"          // For pub::http::Data, pub::http::Hunk
#include "pub/http/Listen.h"        // For pub::http::Listen
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Request.h"       // For pub::http::Request
#include "pub/http/Response.h"      // For pub::http::Response
#include "pub/http/Server.h"        // For pub::http::Server
#include "pub/http/Stream.h"        // For pub::http::Stream

using namespace pub;                // For pub classes
using namespace pub::debugging;     // For pub debugging functions
using namespace pub::http;          // For pub::http classes
using namespace std;                // For std classes
using pub::utility::visify;         // (Import)

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  IODM= false                      // Input/Output Debug Mode?
,  VERBOSE= 1                       // Verbosity: Higher is more verbose

,  DIR_MODE= (S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH) // Directory mode
,  MAX_REQUEST_COUNT= 16            // Maximum running request count
,  MAX_RESPONSE_SIZE= 0x00100000    // Maximum response data length
,  PROT_RW= (PROT_READ | PROT_WRITE) // Read/write access mode
,  TRACE_SIZE= 0x00100000           // Default trace table size (1M)
,  USE_LOGGER= false                // Option: Use logger
};

// Imported Options
typedef const char CC;
static constexpr CC*   HTTP_GET=  Options::HTTP_METHOD_GET;
static constexpr CC*   HTTP_HEAD= Options::HTTP_METHOD_HEAD;
static constexpr CC*   HTTP_POST= Options::HTTP_METHOD_POST;
static constexpr CC*   HTTP_PUT=  Options::HTTP_METHOD_PUT;

static constexpr CC*   HTTP_SIZE= Options::HTTP_HEADER_LENGTH;
static constexpr CC*   HTTP_TYPE= Options::HTTP_HEADER_TYPE;

static const char*     cert_file= "public.pem";  // The public certificate file
static const char*     priv_file= "private.pem"; // The private key file

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Debug*          debug= nullptr; // Our Debug object
static string          host= "localhost"; // The connection host
static string          port= "8080"; // The connection port
static double          test_interval= 2.0; // Stress test interval, in seconds
static string          test_url= "/"; // The stress test URL
static void*           trace_table= nullptr; // The internal trace area

//----------------------------------------------------------------------------
// Options
//----------------------------------------------------------------------------
static int             opt_help= false; // --help (or error)
static int             opt_hcdm= false; // Hard Core Debug Mode
static int             opt_index;   // Option index

static const char*     opt_debug= nullptr; // --debug
static int             opt_verbose= -1; // --brief or --verbose
static int             opt_bringup= false; // Run bringup test?
static int             opt_client= false; // Run basic client test?
static int             opt_server= false; // Run server?
static int             opt_stress= false; // Run client stress test?
static int             opt_trace= false;  // Create trace file?
static int             opt_verify= false; // Verify file data?

static const char*     OSTR= ":";   // The getopt_long optstring parameter
static struct option   OPTS[]=      // The getopt_long longopts parameter
{  {"help",    no_argument,       &opt_help,    true} // --help
,  {"hcdm",    no_argument,       &opt_hcdm,    true} // --hcdm

,  {"debug",   required_argument, nullptr,      0} // --debug <string>
,  {"verbose", optional_argument, &opt_verbose, 0} // --verbose {optional}
,  {"bringup", no_argument,       &opt_bringup,  true} // --bringup
,  {"client",  no_argument,       &opt_client,  true} // --client
,  {"host",    required_argument, nullptr,      0} // --host <string>
,  {"port",    required_argument, nullptr,      0} // --port <string>
,  {"server",  optional_argument, &opt_server,  true} // --server
,  {"stress",  no_argument,       &opt_stress,  true} // --stress
,  {"time",    required_argument, nullptr,      0} // --time <string>
,  {"trace",   no_argument,       &opt_trace,  true} // --trace
,  {"verify",  no_argument,       &opt_verify,  true} // --verify
,  {0, 0, 0, 0}                     // (End of option list)
};

enum OPT_INDEX                      // Must match OPTS[]
{  OPT_HELP
,  OPT_HCDM

,  OPT_DEBUG
,  OPT_VERBOSE
,  OPT_BRINGUP
,  OPT_CLIENT
,  OPT_HOST
,  OPT_PORT
,  OPT_SERVER
,  OPT_STRESS
,  OPT_TIME
,  OPT_TRACE
,  OPT_VERIFY
,  OPT_SIZE
};
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
//       Generate 200 (normal) response
//       Generate 405 method not supported message
//       Generate 500 server error message
//
//----------------------------------------------------------------------------
static string                       // The 200 Dummy File message
   page200(string body)
{  static const char* args[]=
               { "<html><head><title>PAGE 200</title></head>"
               , "<body><h1 align=\"center\">Default Response Page</h1>"
               , "File[{}]"
               , "</body></html>"
               , nullptr
               };
   return do_JOIN(args, body);
}

static string                       // The 403 forbidden
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

static string                       // The 404 file not found
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
// Class-
//       ClientThread
//
// Purpose-
//       The T_Stream client pseudo-thread.
//
//----------------------------------------------------------------------------
class ClientThread {                // The client pseudo-thread
std::shared_ptr<ClientAgent>
                       agent;       // Our ClientAgent
std::shared_ptr<Client>client;      // Our Client

bool                   operational= false; // TRUE while operational
int                    error_count= 0;  // The number of detected errors
std::atomic_size_t     cur_op_count= 0; // The number of running requests
std::atomic_size_t     tot_op_count= 0; // The total request count

// Callback handlers
std::function<void(void)>
                       do_NEXT;     // The next completion handler

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::~ClientThread
//       ClientThread::ClientThread
//
// Function-
//       Destructor
//       Constructor
//
//----------------------------------------------------------------------------
public:
   ~ClientThread( void ) = default;
   ClientThread( void )
{  agent= ClientAgent::make(); }

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
{
   if( HCDM && VERBOSE > 1 )
     debugh("\n\ndo_POST(%s,%s)\n\n", path.c_str(), data.c_str());

   std::shared_ptr<Request> Q= client->request();
   Q->method= HTTP_POST;
   Q->path= path;

   std::shared_ptr<Response> response= Q->get_response();
   do_RESP(response);

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
{
   // Google: "c++ shared_ptr lambda function" to see why a weak_ptr MUST be
   // captured instead of a shared_ptr.
   std::weak_ptr<Response> weak= S; // (Passed to lambda functions)

   S->on_error([weak](const std::string& mess) {
     std::shared_ptr<Response> S= weak.lock();
     if( HCDM && VERBOSE > 1 )
       debugh("Response(%p)::on_error(%s)\n", S.get(), mess.c_str());
     if( S ) {
       std::shared_ptr<Request> Q= S->get_request();
       debugh("Request(%p) %s %s error %s\n", Q.get(), Q->method.c_str()
             , Q->path.c_str(), mess.c_str());
     }
   });

   S->on_data([weak](const Hunk& hunk) {
     std::shared_ptr<Response> S= weak.lock();
     if( HCDM && VERBOSE > 1 )
       debugh("Response(%p)::on_data\n", S.get());
     if( S ) {
       Data& data= S->get_data();
       if( data.get_size() <= MAX_RESPONSE_SIZE )
         data.append(hunk);
     }
   });

   S->on_end([this, weak]( void ) { // (this captured for error_count)
     std::shared_ptr<Response> S= weak.lock();
     if( HCDM && VERBOSE > 1 )
       debugh("Response(%p)::on_end\n", S.get());
     if( S ) {
       std::shared_ptr<Request> Q= S->get_request();
       if( IODM && VERBOSE > 0 ) {
         debugf("Response code %d\n", S->get_code());
         Options& opts= (Options&)*S.get();
         for(Options::const_iterator it= opts.begin(); it != opts.end(); ++it)
            debugf("%s: %s\n", it->first.c_str(), it->second.c_str());
       }
       if( S->get_code() == 200 ) {
         if( opt_verify && Q->method == HTTP_GET ) {
           std::string path= Q->path;
           if( path == "/" )
             path= "/index.html";
           std::string have_string= S->get_data().get_string();
           std::string want_string= page200(path);
           if( want_string != have_string ) {
             error_count++;
             have_string= visify(have_string);
             want_string= visify(want_string);
             debugh("%4d %s Data verify error:\n", __LINE__, __FILE__);
             debugh("Have '%s'\n", have_string.c_str());
             debugh("Want '%s'\n", want_string.c_str());
           }
         }

         if( IODM && VERBOSE > 0 ) {
           Data& data= S->get_data();
           std::string data_string= data.get_string();
           if( data.get_size() > MAX_RESPONSE_SIZE )
             data_string=
             utility::to_string("<<Response data error: length(%ld) > %d>>"
                             , data.get_size(), MAX_RESPONSE_SIZE);
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
     std::string       path,        // The URL
     std::string       meth)        // The Request method
{
   if( HCDM && VERBOSE > 1 )
     debugh("\n\ndo_SEND(%s,%s)\n\n", meth.c_str(), path.c_str());

   ++cur_op_count;
   ++tot_op_count;

   std::shared_ptr<Request> Q= client->request();
   if( Q.get() == nullptr )
     return;                        // TODO: ? ADD ERROR RECOVERY HERE ?
   Q->method= meth;
   Q->path= path;

   std::shared_ptr<Response> S= Q->get_response();
   do_RESP(S);

   Q->on_end([this]() {
     --cur_op_count;
     do_NEXT();
   });

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
   client= agent->connect(host, port); // Create the client

   if( client ) {
     client->on_close([this]( void ) {
       if( HCDM )
         debugh("Client(%p)::on_close\n", client.get());
     });
   } else {
     debugf("Unable to connect %s:%s\n", host.c_str(), port.c_str());
     exit(EXIT_FAILURE);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::join
//
// Purpose-
//       Complete the client Thread
//
//----------------------------------------------------------------------------
void
   join( void )                     // Complete the client pseudo-thread
{  }                                // (Nothing needed)

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
int                                 // Error counter
   statistics( void )               // Display statistics
{
   int error_count= 0;              // Error counter

   // Verify object counters (TODO: REMOVE) vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
   pub::Statistic* stat= &Stream::obj_count;
   debugf("%'16ld {%2ld,%2ld,%2ld} Stream counts\n", stat->counter.load()
         , stat->minimum.load(), stat->current.load(), stat->maximum.load());

   stat= &Request::obj_count;
   debugf("%'16ld {%2ld,%2ld,%2ld} Request counts\n", stat->counter.load()
         , stat->minimum.load(), stat->current.load(), stat->maximum.load());

   stat= &Response::obj_count;
   debugf("%'16ld {%2ld,%2ld,%2ld} Response counts\n", stat->counter.load()
         , stat->minimum.load(), stat->current.load(), stat->maximum.load());

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

   return error_count;
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
int                                 // Error count
   test_client( void )              // Client functional test
{
   debugf("\nDriver.test_client...\n");
   error_count= 0;

   // Initialize
   get_client();
   do_NEXT= [](void) {if( HCDM && VERBOSE > 1 ) debugf("client.next NOP\n");};

   // Bringup tests
   do_SEND("/", HTTP_GET);
   do_SEND("/index.htm", HTTP_HEAD);

   do_POST("/post-test", "This is the post data, all of it.");

   do_SEND("/403-test", HTTP_GET);
   do_SEND("/404-test", HTTP_GET);
   do_SEND("/405-test", "MOVE");

   do_SEND("/tiny.html", HTTP_GET); // Used in stress test
   do_SEND("/utf8.html", HTTP_GET); // Regression test

   // Error tests
#if 0  // TODO: CLIENT RECOVERY
   if( HCDM || VERBOSE ) {
     client->wait();
     debugf("Malformed method...\n");
   }
   do_SEND("/", " GET");            // Malformed method
   if( HCDM || VERBOSE ) {
     client->wait();
     debugf("Malformed path...\n");
   }
   do_SEND(" /", "GET");            // Malformed path
// do_SEND( TODO: CODE );           // Malformed protocol
#endif

   do_SEND("/last.html", HTTP_GET); // The last request
   client->wait();                  // Wait for Client to complete
   Thread::sleep(0.125);            // Delay to allow Server completion

   Trace::trace(".TXT", __LINE__, "TS.agent reset");
   agent->reset();                  // Reset the Agent
   debugf("...Driver.test_client\n");
   Trace::trace(".TXT", __LINE__, "TS.client exit");

   return error_count;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientThread::test_stress
//
// Purpose-
//       Stress test
//
// Implementation note-
//       NOP iterations/second: BB(16.7M), RB(33.7M) <Local machines>
//
//----------------------------------------------------------------------------
int                                 // Error count
   test_stress( void )              // Stress test
{
   debugf("\nDriver.test_stress... (%.1f seconds)\n", test_interval);
   error_count= 0;

   if( false )
     printf("%s", page405("TEST").c_str());

   // Initialize
   get_client();
   do_NEXT= [this](void) {
     // debugh("stress.next... %s\n", operational ? "running" : "quiesced");
     pub::Statistic* stat= &Request::obj_count;
     while( operational && cur_op_count.load() < MAX_REQUEST_COUNT ) {
       if( HCDM && VERBOSE > 1 )    // Use detailed tracking?
         debugh("%4ld {%2ld,%2ld,%2ld} cur_op_count %zd\n"
                , stat->counter.load(), stat->minimum.load()
                , stat->current.load(), stat->maximum.load()
                , cur_op_count.load());
       do_SEND(test_url, HTTP_GET);
     }

     if( HCDM && VERBOSE > 1 )      // Use detailed tracking?
       debugh("%4ld {%2ld,%2ld,%2ld} %soperational, cur_op_count %zd\n"
              , stat->counter.load(), stat->minimum.load()
              , stat->current.load(), stat->maximum.load()
              , operational ? "" : "non-"
              , cur_op_count.load());
   };

   // Run the stress test
   operational= true;               // Indicate operational
   do_NEXT();                       // Prime the pump
   Thread::sleep(test_interval);    // Run the test
   operational= false;              // Indicate non-operational
   tot_op_count -= cur_op_count;    // (Current tests don't count)

   debugf("%'16.3f operations\n", (double)tot_op_count);
   debugf("%'16.3f operations/second\n", (double)tot_op_count/test_interval);

   // Cleanup
   client->wait();                  // Wait for operations to complete
   Thread::sleep(0.125);            // Delay to allow Server completion

   agent->reset();                  // Reset the Agent
   debugf("...Driver.test_stress\n");
   Trace::trace(".TXT", __LINE__, "TS.stress exit");

   return error_count;
}
}; // class ClientThread

//----------------------------------------------------------------------------
//
// Class-
//       ServerThread
//
// Purpose-
//       The T_Stream listener pseudo-thread
//
//----------------------------------------------------------------------------
class ServerThread {                // The listener pseudo-thread
std::shared_ptr<ServerAgent>
                       agent;       // Our ClientAgent
std::shared_ptr<Listen>
                       listen;      // Our Listener

Event                  completed;   // Our completion Event
bool                   operational= false; // TRUE while operational
size_t                 cur_op_count= 0; // The number of running requests
size_t                 tot_op_count= 0; // The total request count

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::~ServerThread
//       ServerThread::ServerThread
//
// Function-
//       Destructor
//       Constructor
//
//----------------------------------------------------------------------------
public:
   ~ServerThread( void )
{  }

   ServerThread( void )
:  agent(ServerAgent::make())
{
   Options opts;                    // Server Options
   opts.insert("cert", cert_file);  // The public certificate file
   opts.insert("key", priv_file);   // The private key file
   opts.insert("http1", "true");    // HTTP1 allowed

   listen= agent->connect("", port, AF_INET, &opts); // Create the Listener

   // Initialize the Listen handlers
   listen->on_close([this]( void ) {
     if( HCDM && VERBOSE > 1 )
       debugf("ServerThread(%p)::on_close\n", this);
     completed.post(0);
   });

   listen->on_request([this](Request& Q) {
     try {
       if( HCDM && VERBOSE > 1 )
         debugf("ServerThread(%p)::on_request(%s)\n", this
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
   do_FILE(Request& Q)
{
   if( HCDM ) debugf("ServerThread(%p)::do_FILE(%s)\n", this, Q.path.c_str());

   string path= Q.path;
   if( path[0] != '/' || path.find("/../") != string::npos ) {
     do_HTML(Q, 500, page500("parser fault"));
     return;
   }

   if( path == "/" )
     path= "/index.html";
// path= "html" + path;             // TODO: SCAFFOLDED

   if( path == "/403-test" )        // TODO: SCAFFOLDED
     do_HTML(Q, 403, page403(path));
   else if( path == "/404-test" )
     do_HTML(Q, 404, page404(path));
   else
     do_HTML(Q, 200, page200(path));
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
   do_HGET(Request& Q)
{  if( HCDM )
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
   do_HTML(Request& Q, int code, string html)
{  if( HCDM ) debugf("ServerThread(%p)::do_HTML(%d)\n", this, code);

   Response& S= *Q.get_response();
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
   do_POST(Request& Q)
{  if( HCDM ) debugf("ServerThread(%p)::do_POST\n", this);

   string body=
   utility::to_string("POST[%s]", Q.get_data().get_string().c_str());
   do_HTML(Q, 200, page200(body));
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::join
//
// Purpose-
//       Complete the listen Thread
//
//----------------------------------------------------------------------------
void
   join( void )                     // Complete the listen pseudo-thread
{  listen->join(); }                // Wait for Listener termination

//----------------------------------------------------------------------------
//
// Method-
//       ServerThread::run
//
// Function-
//       Operate the server Thread
//
//----------------------------------------------------------------------------
void
   run( void )
{
   debugf("\n\n%4d %s * * * SHOULD NOT OCCUR * * *\n\n", __LINE__, __FILE__);

   // Operate the server Thread
   operational= true;
   completed.wait();
   operational= false;
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
{  if( HCDM ) debugf("ServerThread(%p)::stop\n", this);

   listen->close();
}
}; // class ServerThread
#endif // T_STREAM_HPP_INCLUDED
