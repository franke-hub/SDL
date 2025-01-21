//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpCommand.cpp
//
// Purpose-
//       Implement HttpCommand and HttpService
//
// Last change date-
//       2025/01/20
//
//----------------------------------------------------------------------------
#include <forward_list>             // For std::forward_list
#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr, std::weak_ptr, ...
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string
#include <cstdio>                   // For perror

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Ioda.h>               // For pub::Ioda
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread, Listen base class
#include <pub/utility.h>            // For pub::utility::to_string, ...
#include <pub/utility.i>            // For conversion subroutines

#include "HttpListen.h"             // For HttpListen
#include "HttpMapper.h"             // For HttpMapper
#include "HttpServer.h"             // For HttpServer

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
,  USE_CURL= true                   // Initialization: Read from listeners?
}; // enum

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
//-------------------------------------------------------------------------
// Command_listen::Constructors/destructor
//-------------------------------------------------------------------------
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

   std::shared_ptr<HttpListen> listener= HttpMapper::get()->locate(url);
   if( listener ) {
     debugh("There is already a Listener at '%s'\n", s2c(url));
     return;
   }

   try {
     listener= HttpListen::make(url);
     if( listener->is_operational() ) {
       // insert(listener);
       listener->start();
     }

     // delete listener;            // (Message already written)
   } catch(std::exception& X) {
     debugh("Unable to start Listener at '%s' %s\n", s2c(url), X.what());
     listener= nullptr;
   } catch(...) {
     debugh("Unable to start Listener at '%s', Exception\n", s2c(url));
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

   HttpMapper::get()->status();
}

//----------------------------------------------------------------------------
//
// Method-
//       Command_listen::stop(void)
//       Command_listen::stop(const char*)
//
// Purpose-
//       Stop all Listeners
//       Stop one Listener
//
//----------------------------------------------------------------------------
void
   stop( void )                     // Stop *ALL* Listeners
{  if( HCDM ) debugh("Command_listen::stop\n");

   HttpMapper::get()->stop();
}

void
   stop(                            // Stop the listener
     const char*       _url)        // At this URL
{  if( HCDM ) debugh("Command_listen::stop(%s)\n", _url ? _url : "");

   HttpMapper::get()->stop(_url);
}

//----------------------------------------------------------------------------
//
// Method-
//       Command_listen::wait(void)
//       Command_listen::wait(const char*)
//
// Purpose-
//       Wait for all Listeners
//       Wait for one Listener
//
//----------------------------------------------------------------------------
void
   wait( void )                     // Wait for *ALL* Listeners
{  if( HCDM ) debugh("Command_listen::wait\n");

   HttpMapper::get()->wait();
}

void
   wait(                            // Wait for listener
     const char*       _url)        // At this URL
{  if( HCDM ) debugh("Command_listen::wait(%s)\n", _url ? _url : "");

   HttpMapper::get()->wait(_url);
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
       stop(argv[2]);
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

     Command::command("status");

     if( false ) {                  // Auto-generate diagnostic signal?
       StaticCommon::DiagnosticEvent event;
       static_common->run_diagnostics.signal(event);
     }
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
//       Handle startup_complete event, running "http-init"
//
// Implementation note-
//       Constructs our startup_event_handler during static initialization,
//       initializing startup_event_handler.connector. We don't have any other
//       good place except for ~startup_event_handler to reset the connector,
//       but it's about to be reset there by ~connector anyway.
//
//----------------------------------------------------------------------------
struct startup_event_handler_t {    // Handle startup event
typedef pub::signals::Event         Event;
typedef pub::signals::Connector     Connector;
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
}; // class Service_listen
static Service_listen service_listen;
