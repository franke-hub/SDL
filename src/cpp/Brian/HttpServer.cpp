//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpServer.cpp
//
// Purpose-
//       Implement HttpServer.h
//
// Last change date-
//       2024/12/20
//
// Implementation note-
//       Derived from ~/src/cpp/HTTP/socket/Server.cpp 2024/10/24
//
//----------------------------------------------------------------------------
#include <forward_list>             // For std::forward_list
#include <map>                      // For std::map
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string
#include <cstdio>                   // For perror

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Ioda.h>               // For pub::Ioda
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread, Listen base class
#include <pub/utility.h>            // For pub::utility::to_string, ...
#include <pub/utility.i>            // For conversion subroutines

#include "HttpServer.h"             // For HttpServer, implemented

#include "Common.h"                 // For Common::start Signal
#include "Command.h"                // For Command
#include "Service.h"                // For Service

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For convenience
using namespace PUB::debugging;     // For debugging subroutines
using PUB::utility::to_string;      // For convenience
using PUB::utility::visify;         // For convenience
using std::string;                  // For convenience

typedef pub::Ioda::Mesg             Mesg; // For convenience

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  DEFAULT_PORT= 8080               // Default port number
,  INP_SIZE= 65536                  // Input buffer size
,  USE_CURL= false                  // Initialization: Read from listeners?
}; // enum

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
static const char*     page200=     // The 200 Dummy File message
         "<html><head><title>PAGE 200</title></head>\r\n"
         "<body><h1 align=\"center\">Default Response Page</h1>\r\n"
         "No Body's Home, Paige\r\n"
         "</body></html>\r\n"
         ;

//============================================================================
//
// Class-
//       Command_listen
//
// Purpose-
//       Start and stop Listeners
//
//----------------------------------------------------------------------------
class Command_listen : public Command {
//----------------------------------------------------------------------------
// Command_listen::Attributes
protected:
typedef std::map<string, Listen*>   Map_t; // Our map type
typedef Map_t::iterator             Map_iter_t; // Our map iterator type

Map_t                   map;        // Our Listener Map
std::recursive_mutex    mutex;      // Protects map

//-------------------------------------------------------------------------
// Command_listen::Constructors/destructor
public:
   Command_listen( void )           // Constructor
:  Command("listen")
{  if( HCDM ) debugh("Command_listen!\n"); }

// Service_listen::stop invokes stop, so we don't need to stop listeners again
// in the destructor.
   ~Command_listen( void )          // Destructor
{  if( HCDM ) debugh("Command_listen~\n"); }

//----------------------------------------------------------------------------
//
// Method-
//       Command_listen::done
//
// Purpose-
//       Handle Listener termination
//
//----------------------------------------------------------------------------
void
   done(                            // Handle termination of
     Listen*           listener)    // This Listener
{  string url= listener->get_url();
   if( HCDM ) debugh("Command_listen::done(%s)\n", s2c(url));

{{{{
   std::lock_guard<decltype(mutex)> lock(mutex);
   remove(listener);
}}}}
// debugh("Listener(%s) removed\n", s2c(url)); // (Don't debug with lock)
}

//----------------------------------------------------------------------------
//
// Method-
//       Command_listen::insert
//       Command_listen::locate
//       Command_listen::remove
//
// Purpose-
//       Insert a Listener into the Map
//       Locate a Listener
//       Remove a Listener
//
// Implementation notes:
//       Caller must hold mutex
//
//----------------------------------------------------------------------------
Listen*                             // The Listener, if inserted
   insert(                          // Insert
     Listen*           listener)    // This Listener
{  string url= listener->get_url();

   if( locate(url) ) {              // If already in map
     // TODO: How was a duplicate Listen allowed to be created?
     debugh("Command_listen::insert(%s) failed, duplicate\n", s2c(url));
     delete listener;
     return nullptr;
   }

   map[url]= listener;
   if( HCDM )
     debugh("Command_listen::insert(%s)\n", s2c(url));
   return listener;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Listen*                             // The Listener, if available
   locate(                          // Locate Listener
     string            url)         // With this URL
{
   Listen* listener= nullptr;       // Default, not found
   const Map_iter_t mi= map.find(url);
   if( mi != map.end() )            // If found
     listener= mi->second;

   if( HCDM )
     debugh("%p= locate(%s)\n", listener, s2c(url));
   return listener;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   remove(                          // Remove
     Listen*           listener)    // This Listener
{  string url= listener->get_url();

   const Map_iter_t mi= map.find(url);
   if( mi != map.end() && mi->second == listener ) {
     map.erase(mi);
     if( HCDM )
       debugh("Command_listen::remove(%s)\n", s2c(url));
   } else {
     if( HCDM )
       debugh("Command_listen::remove(%s) failed, not found\n", s2c(url));
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Command_listen::start
//
// Purpose-
//       Start a Listener
//
//----------------------------------------------------------------------------
void
   start(                           // Start a listener
     const char*       _url)        // At this URL
{  if( HCDM ) debugh("Command_listen::start(%s)\n", _url ? _url : "");

   string url= Socket::gethostname();
   if( _url && _url[0] != '\0' ) {
     if( _url[0] == ':' )           // If only port number specified
       url += _url;
     else
       url= _url;
   } else {
     url += to_string(":%d", DEFAULT_PORT);
   }

   std::lock_guard<decltype(mutex)> lock(mutex);

   Listen* listener= locate(url);
   if( listener ) {
     debugh("There is already a Listener at '%s'\n", s2c(url));
     return;
   }

   try {
     listener= new Listen(url);
     if( listener->is_operational() ) {
       insert(listener);
       listener->start();
       return;
     }

      delete listener;              // (Message already written)
   } catch(std::exception& X) {
     debugh("Unable to start Listener at '%s' %s\n", s2c(url), X.what());
     delete(listener);
     listener= nullptr;
   } catch(...) {
     debugh("Unable to start Listener at '%s', Exception\n", s2c(url));
     delete(listener);
     listener= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Command_listen::status
//
// Purpose-
//       Display active listeners
//
//----------------------------------------------------------------------------
void
   status( void )                   // Display active listeners
{  if( HCDM ) debugh("Command_listen::status\n");

   std::forward_list<Listen*> list; // Listener list

   // Copy the Listen* from the map into the list
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);
     for(auto& it: map) {
       list.push_front(it.second);
     }
   }}}}

   // Display all Listeners
   debugf("Listeners:\n");
   for(auto& it: list) {
     debugf("%s\n", s2c(it->get_url()));
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Command_listen::stop(void)
//       Command_listen::stop(Listen*)
//       Command_listen::stop(const char*)
//
// Purpose-
//       Stop all Listeners
//       Stop one Listener
//       Stop one Listener
//
//----------------------------------------------------------------------------
void
   stop( void )                     // Stop *ALL* Listeners
{  if( HCDM ) debugh("Command_listen::stop\n");

   std::forward_list<Listen*> list; // Listener list

   // Copy the Listen* from the map into the list
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);
     for(auto& it: map) {
       list.push_front(it.second);
     }
   }}}}

   // Stop all Listeners
   for(auto& it: list) {
     it->stop();
   }
}

void
   stop(                            // Stop
     Listen*           listener)    // This Listener
{  string url= listener->get_url();
   if( HCDM ) debugh("Command_listen::stop(%s)\n", s2c(url));

   listener->stop();
}

bool                                // If listener extant
   stop(                            // Stop the listener
     const char*       _url)        // At this URL
{  if( HCDM ) debugh("Command_listen::stop(%s)\n", _url ? _url : "");

   std::lock_guard<decltype(mutex)> lock(mutex);

   Listen* listener= locate(_url);
   if( listener ) {
     listener->stop();
     return true;
   }

   debugh("Listener::stop(%s) URL not active\n", _url);
   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       Command_listen::wait(void)
//       Command_listen::wait(Listen*)
//       Command_listen::wait(const char*)
//
// Purpose-
//       Wait for all Listeners
//       Wait for one Listener
//       Wait for one Listener
//
//----------------------------------------------------------------------------
void
   wait( void )                     // Wait for *ALL* Listeners
{  if( HCDM ) debugh("Command_listen::wait\n");

   std::forward_list<Listen*> list; // Listener list

   // Copy the Listen* from the map into the list
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);
     for(auto& it: map) {
       list.push_front(it.second);
     }
   }}}}

   // Wait for all Listeners
   for(auto& it: list) {
     it->wait();
   }
}

void
   wait(                            // Wait for
     Listen*           listener)    // This Listener
{  if( HCDM ) debugh("Command_listen::wait(%s)\n", s2c(listener->get_url()));

   listener->wait();
}

void
   wait(                            // Wait for listener
     const char*       _url)        // At this URL
{  if( HCDM ) debugh("Command_listen::wait(%s)\n", _url ? _url : "");

   std::lock_guard<decltype(mutex)> lock(mutex);

   Listen* listener= locate(_url);
   if( listener ) {
     listener->wait();
     return;
   }

   debugh("Listener::wait(%s) URL not active\n", _url);
}

//----------------------------------------------------------------------------
//
// Method-
//       Command_listen::main
//
// Purpose-
//       Run the listen Command, invoking the listen method
//
//----------------------------------------------------------------------------
virtual Command::resultant          // Resultant
   main(                            // Handle Command
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   if( argc < 2 ) {                 // If no URL specified
     start(nullptr);
   } else if( strcasecmp(argv[1], "status") == 0 ) {
     status();
   } else if( strcasecmp(argv[1], "start") == 0 ) {
     if( argc >= 2 )
       start(argv[2]);
     else
       debugf("listen %s, URL missing\n", argv[1]);
   } else if( strcasecmp(argv[1], "stop") == 0 ) {
     if( argc >= 2 ) {
       if( stop(argv[2]) )
         wait(argv[2]);
     } else {
       debugf("listen %s, URL missing\n", argv[1]);
     }
   } else {                        // Not a command, [start] URL assumed
     start(argv[1]);
   }

   return nullptr;
}
}; // class Command_listen
static Command_listen command_listen;

//----------------------------------------------------------------------------
//
// Class-
//       Command_init
//
// Purpose-
//       Create listeners
//
//----------------------------------------------------------------------------
class Command_init : public Command {
//----------------------------------------------------------------------------
// Command_init::Constructors/destructor
public:
   Command_init( void )             // Constructor
:  Command("http-init") {}

virtual
   ~Command_init( void ) = default; // Destructor

//----------------------------------------------------------------------------
// Command_init::Methods
//----------------------------------------------------------------------------
virtual resultant                   // Resultant, command dependent
   main(int, char*[])               // Process the Command
//   int               argc,        // Argument count (UNUSED)
//   char*             argv[])      // Argument array (UNUSED)
{
   std::string host= getenv("HOSTNAME");
   host += ":8080";

   std::string local="localhost:8081";

   Command::command("listen start " + host);
   Command::command("listen start " + local);

   if( USE_CURL ) {
     Command::command("curl " + host);
     Command::command("curl " + local);
   }

   return nullptr;
}
}; // class Command_init
Command_init command_init;

//----------------------------------------------------------------------------
//
// Struct-
//       startup_event_handler_t
//
// Purpose-
//       Handle startup_complete event, running "init"
//
// Implementation note-
//       Constructs our startup_event_handler during static initialization,
//       initializing startup_event_handler.connector. We don't have any other
//       good place except for ~startup_event_handler to reset the connector,
//       but it's about to be reset there by ~connector anyway.
//
//----------------------------------------------------------------------------
struct startup_event_handler_t {    // Handle startup event
typedef StaticCommon::Event                   Event;
typedef pub::signals::Connector<Event>        Connector;
Connector              connector;   // Our connector

   startup_event_handler_t( void )  // Constructor
{  if( HCDM ) debugh("startup_event_handler_t!\n");

   StaticCommon::make();            // (Initialize static_common)

   //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Our startup_complete signal handler.
   connector= static_common->startup_complete.connect([](Event&) {
     if( HCDM ) debugh("%4d HttpServer startup_complete.handler\n", __LINE__);

     Command::command("http-init"); // Process the "http-init" command
   });
}

   ~startup_event_handler_t( void )
{  if( HCDM ) debugh("startup_event_handler_t~\n"); }
}  startup_event_handler; // struct startup_event_handler_t

//============================================================================
//
// Class-
//       Service_listen
//
// Purpose-
//       On close, stop Listeners
//
//----------------------------------------------------------------------------
class Service_listen
:  public Service
,  public Service::has_stop         // We *DO NOT* support start
,  public Service::has_wait
{
//----------------------------------------------------------------------------
// Service_listen::Attributes
// (The Listener list is maintained by Command_listen)

//-------------------------------------------------------------------------
// Service_listen::Constructors/destructor
public:
   Service_listen( void )           // Constructor
:  Service("listen")
{  if( HCDM ) debugh("Service_listen!\n"); }

   ~Service_listen( void )          // Destructor
{  if( HCDM ) debugh("Service_listen~\n"); }

//----------------------------------------------------------------------------
//
// Method-
//       Service_listen::stop
//
// Purpose-
//       Stop Listeners
//
//----------------------------------------------------------------------------
virtual void
   stop(Service*)                   // Stop *ALL* Listeners
{  if( HCDM ) debugh("Service_listen::stop(Service*)\n");

   command_listen.stop();
}

void
   stop(                            // Stop
     Listen*           listen)      // This Listener
{  if( HCDM )
     debugh("Service_listen::stop(Listen(%s))\n", s2c(listen->get_url()));

   command_listen.stop(listen);
}

//----------------------------------------------------------------------------
//
// Method-
//       Service_listen::wait
//
// Purpose-
//       Wait for Listener completions
//
//----------------------------------------------------------------------------
virtual void
   wait(Service*)                   // Wait for *ALL* Listeners
{  if( HCDM ) debugh("Service_listen::wait(Service*)\n");

   command_listen.wait();
}

void
   wait(                            // Wait for
     Listen*           listen)      // This Listener
{  if( HCDM )
     debugh("Service_listen::wait(Listen(%s))\n", s2c(listen->get_url()));

   command_listen.wait(listen);
}
}; // class Service_listen
static Service_listen service_listen;

//----------------------------------------------------------------------------
//
// Method-
//       Listen::build
//       Listen::Listen
//       Listen::~Listen
//
// Purpose-
//       Common construction
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
void
   Listen::build( void )            // Common construction
{  if( HCDM )  debugh("Listen(%p)::build\n", this);

   // Create and initialize the Listener Socket
   socket= new pub::Socket();       // Create the Socket
   int rc= socket->open(AF_INET, SOCK_STREAM);
   if( rc ) {                       // If failure
     errorh("Listen ERROR: ");
     perror("open failed");
     return;
   }
   if( HCDM )
     debugh("Listen(%p) socket(%p) url(%s)\n", this, socket, s2c(url));

   // (Needed before the bind)
   int optval= true;
   socket->set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   rc= socket->bind(s2c(url));      // Set Listener host:port
   if( rc ) {                       // If failure
     errorh("Listen ERROR: ");
     perror("bind failed");
     return;
   }

   rc= socket->listen();            // Begin listening
   if( rc ) {                       // If failure
     errorh("Listen ERROR: ");
     perror("listen failed");
     return;
   }

   operational= true;               // We are operational
   debugh(" online: %s\n", s2c(url));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Listen::Listen( void )           // (Default) constructor
:  Thread(),  url(Socket::gethostname() + to_string(":%d", DEFAULT_PORT))
{  if( HCDM ) debugh("Listen(%p)!()\n", this);

   build();                         // Common construction
}

   Listen::Listen(                  // Constructor
     std::string       _url)        // "hostname:port" URL
:  Thread(), url(_url)
{  if( HCDM ) debugh("Listen(%p)!(%s)\n", this, s2c(_url));

   build();                         // Common construction
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Listen::~Listen( void )          // Destructor
{  if( HCDM ) debugh("Listen(%p)~\n", this);

   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Listen::close
//
// Purpose-
//       Close the Listener Socket
//
//----------------------------------------------------------------------------
void
   Listen::close( void )            // Close the Listener Socket
{  if( HCDM ) debugh("Listen(%p)::close\n", this);

   std::lock_guard<decltype(mutex)> lock(mutex);

   operational= false;              // Go non-operational
   if( socket ) {
     int rc= socket->close();
     if( rc ) {                     // If failure
       errorh("Listen ERROR: ");
       perror("close failed");
     }

     delete socket;
     socket= nullptr;
     debugh("offline: %s\n", s2c(url)); // (Only one offline message)
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Listen::run
//
// Purpose-
//       Run the Listener Thread
//
//----------------------------------------------------------------------------
void
   Listen::run( void )              // Run the Listener Thread
{  if( HCDM ) debugh("Listen(%p).run\n", this);

   // Accept connections
   try {
     // Listeners can be stopped at any time
     while( operational ) {
       Socket* server= socket->accept(); // Get next server
       if( server == nullptr ) {    // If accept error
         if( operational ) {        // (No message if non-operational)
           errorh("Listen(%p) ", this);
           perror("accept error (ignored)");
         }
         continue;
       }

       Server* thread= new Server(server); // Create new Server thread
       if( !thread->is_operational() ) { // If initialization failed
         delete thread;             // (Constructor error message written)
         continue;
       }

       // (The Server is a self-deleting Thread)
       // If it was also self-starting it could complete before the message
       // indicating it started was displayed.
       if( HCDM )
         debugh("Listen(%s) %p= new Server(%p)\n", s2c(url), thread, server);
       thread->start();             // Start the Thread
     }
   } catch( std::exception& X ) {
     debugh("Listen::run std::exception what(%s)\n", X.what());
   } catch(...) {
     debugh("Listen::run catch(...)\n");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Listen::stop
//
// Purpose-
//       Stop the Listener
//
//----------------------------------------------------------------------------
void
   Listen::stop( void )             // Terminate Socket listening
{  if( HCDM ) debugh("Listen(%p)::stop '%s'\n", this, s2c(url));

   operational= false;
   close();                         // Close the Socket

   //-------------------------------------------------------------------------
   // Create a dummy connection to complete any pending accept, ignoring any
   // errors that occur.
   try {
     Socket dummy_connector;
     int rc= dummy_connector.open(AF_INET, SOCK_STREAM, PF_UNSPEC);
     if( rc == 0 ) {
       rc= dummy_connector.connect(url);
       if( HCDM ) {
         debugf("%4d HCDM %d= socket.connect(%s)\n", __LINE__
               , rc, url.c_str());
       }
     } else if( HCDM ) {
       debugf("%4d HCDM %d= socket.open\n", __LINE__, rc);
     }
   } catch(...) {
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Listen::wait
//
// Purpose-
//       Wait for Listener completion
//
//----------------------------------------------------------------------------
void
   Listen::wait( void )             // Wait for Listener completion
{  if( HCDM ) debugh("Listen(%p)::wait '%s'\n", this, s2c(url));

   join();                          // Wait for Listener completion
   command_listen.done(this);       // Remove us from the map (once)
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::Server
//       Server::~Server
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Server::Server(                  // Constructor
     pub::Socket*      _socket)     // The server Socket
:  pub::dispatch::Item(), Thread(), socket(_socket)
{  if( HCDM ) debugh("Server(%p)!(%p)\n", this, _socket);

   // Get Host and Port names
   // TODO: NOT CODED YET

   // Allow immediate port re-use on close
   struct linger optval;
   optval.l_onoff= 1;
   optval.l_linger= 0;
   socket->set_option(SOL_SOCKET, SO_LINGER, &optval, sizeof(optval));

   // Set receive timeout
   struct timeval tv= {3,0};        // 3.0 second timout
   socket->set_option(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

   operational= true;               // We are operational
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Server::~Server( void )          // Destructor
{  if( HCDM ) debugh("Server(%p)~\n", this);

   // Close and delete the socket
   if( socket )                     // If Socket exists
     close();                       // Close and delete the Socket
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::close
//
// Purpose-
//       Close the Socket
//
//----------------------------------------------------------------------------
void
   Server::close( void )            // Close the Socket
{  if( HCDM ) debugh("Server(%p)::close(%p)\n", this, socket);

   operational= false;              // (If not already,) go non-operational

   std::lock_guard<decltype(mutex)> lock(mutex);
   if( socket ) {                   // (Only close/delete socket once)
     socket->close();
     delete socket;
     socket= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::request
//
// Purpose-
//       Handle HTTP request
//
//----------------------------------------------------------------------------
void
   Server::request(                 // Handle HTTP request
     std::string       text)        // The request text
{  if( HCDM )
      debugh("Server(%p)::request({{{{\n%s\n}}}})\n", this, s2c(text));

    // NOT CODED YET, DUMMY RESPONSE
    response(200, page200);
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::response
//
// Purpose-
//       Send HTTP response
//
//----------------------------------------------------------------------------
void
   Server::response(                // Send HTTP response
     int               status,      // The status code
     std::string       html)        // The response HTML
{  if( HCDM ) {
     debugh("Server(%p)::response(%d,{{{{\n%s\n}}}})\n", this, status
           , s2c(html));
   }

   std::string resp= to_string("HTTP/1.1 %d OK\r\n", status);
   resp += "Content-type: text/html\r\n";
   resp += to_string("Content-length: %zd\r\n", html.size());
   resp += "\r\n";
   resp += html;
   write(resp);
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::run
//
// Purpose-
//       Operate the Socket
//
//----------------------------------------------------------------------------
void
   Server::run( void )              // Operate the Socket
{  if( HCDM ) debugh("Server(%p)::run\n", this);

   // Run detached
   detach();
   try {
     ssize_t L;
     while( operational ) {
       Ioda ioda;
       Mesg mesg;
       ioda.set_rd_mesg(mesg, INP_SIZE);
       L= socket->recvmsg(&mesg, 0);
       if( L <= 0 ) {
         if( HCDM || VERBOSE )
           errorh("Server(%p): recvmsg error %d:%s\n", this
                 , errno, strerror(errno));
         break;
       }

       // Handle the request
       ioda.set_used(L);
       request((std::string)ioda);
     }
   } catch( std::exception& X ) {
     debugh("Server::run std::exception what(%s)\n", X.what());
   } catch(...) {
     debugh("Server::run catch(...)\n");
   }

   delete this;                     // (Self deletion)
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::stop
//
// Purpose-
//       Stop running the Server
//
//----------------------------------------------------------------------------
void
   Server::stop( void )             // Terminate Socket reading
{  if( HCDM ) debugh("Server(%p)::stop\n", this);

   close();                         // Close the Socket
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::write
//
// Purpose-
//       Send (response) data
//
//----------------------------------------------------------------------------
void
   Server::write(                   // Send response data
     std::string       text)        // The response text
{  if( HCDM )
     debugh("Server(%p)::write({{{{\n%s\n}}}})\n", this, s2c(text));

   // TODO: Add error checking
   socket->write(s2c(text), text.size());
}
