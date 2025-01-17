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
//       HttpListen.cpp
//
// Purpose-
//       Implement HttpListen.h
//
// Last change date-
//       2025/01/16
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
#include <pub/Thread.h>             // For pub::Thread, HttpListen base class
#include <pub/utility.h>            // For pub::utility::to_string, ...
#include <pub/utility.i>            // For conversion subroutines

#include "HttpListen.h"             // For HttpListen, implemented
#include "HttpServer.h"             // For HttpServer
#include "HttpMapper.h"             // For HttpMapper::get

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
}; // enum

//----------------------------------------------------------------------------
//
// Subroutine-
//       io_error
//
// Purpose-
//       Handle I/O error
//
//----------------------------------------------------------------------------
static inline void // UNUSED??
   io_error(                        // Handle I/O error
     HttpListen*       listen)      // For this Listener
{
   errorh("HttpListen(%p) ERROR: %d:%s\n", listen, errno, strerror(errno));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       op_error
//
// Purpose-
//       Handle operation error
//
//----------------------------------------------------------------------------
static void
   op_error(                        // Handle operation error
     HttpListen*       listen,      // For this Listener
     const char*       op)          // And this operation
{
   errorh("HttpListen(%p) %s ERROR: %d:%s\n", listen, op
         , errno, strerror(errno));
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpListen::HttpListen
//       HttpListen::~HttpListen
//       HttpListen::_build
//
// Purpose-
//       Constructors
//       Destructor
//       Common construction
//
//----------------------------------------------------------------------------
   HttpListen::HttpListen( void )   // (Default) constructor
:  Thread(), host(Socket::gethostname() + to_string(":%d", DEFAULT_PORT))
{  if( HCDM ) debugh("HttpListen(%p)!\n", this);

   _build();                        // Common construction
}

   HttpListen::HttpListen(          // Constructor
     std::string       _host)       // "hostname:port"
:  Thread(), host(_host)
{  if( HCDM ) debugh("HttpListen(%p)!(%s)\n", this, s2c(_host));

   _build();                        // Common construction
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   HttpListen::~HttpListen( void )  // Destructor
{  if( HCDM ) debugh("HttpListen(%p)~\n", this);

   HttpMapper::get()->remove(this);
   close();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   HttpListen::_build( void )       // Common construction,
{  if( HCDM ) debugh("HttpListen(%p)::build\n", this);

   // Create and initialize the Listener Socket
   socket= new pub::Socket();       // Create the Socket
   int rc= socket->open(AF_INET, SOCK_STREAM);
   if( rc ) {                       // If failure
     errorh("HttpListen ERROR: ");
     perror("open failed");
     return;
   }
   if( HCDM )
     debugh("HttpListen(%p) socket(%p) host(%s)\n", this, socket, s2c(host));

   // (Needed before the bind)
   int optval= true;
   socket->set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   rc= socket->bind(s2c(host));     // Set Listener host:port
   if( rc ) {                       // If failure
     op_error(this, "bind");
     return;
   }

   rc= socket->listen();            // Begin listening
   if( rc ) {                       // If failure
     op_error(this, "listen");
     return;
   }

   operational= true;               // We are operational
   debugh(" online: %s\n", s2c(host));
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpListen::make
//
// Purpose-
//       Make: std::shared_ptr<HttpListen>
//
//----------------------------------------------------------------------------
std::shared_ptr<HttpListen>
   HttpListen::make( void )         // Make: std::shared_ptr<HttpListen>
{  if( HCDM ) debugh("HttpListen::make()\n");

   std::shared_ptr<HttpListen> listen(new HttpListen());
   listen->self= listen;

   HttpMapper::get()->insert(listen.get());
   return listen;
}

std::shared_ptr<HttpListen>
   HttpListen::make(                // Make: std::shared_ptr<HttpListen>
     std::string       _host)       // With this host name:port
{  if( HCDM ) debugh("HttpListen::make(%s)\n", s2c(_host));

#if 0 // DOES NOT COMPILE -- DOCUMENTATION ONLY
   std::shared_ptr<HttpListen> listen= std::make_shared<HttpListen>(_host);
#endif
   std::shared_ptr<HttpListen> listen(new HttpListen(_host));
   listen->self= listen;

   HttpMapper::get()->insert(listen.get());
   return listen;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpListen::debug
//
// Purpose-
//       Debug the HttpListener
//
//----------------------------------------------------------------------------
void
   HttpListen::debug(               // Debugging display
     const char*       info) const  // Informational display
{  if( HCDM ) debugh("HttpListen(%p)::debug(%s)\n", this, info);
   // NOT CODED YET
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpListen::close
//
// Purpose-
//       Close the HttpListen Socket
//
//----------------------------------------------------------------------------
void
   HttpListen::close( void )        // Close the HttpListen Socket
{  if( HCDM ) debugh("Listen(%p)::close\n", this);

   std::lock_guard<decltype(mutex)> lock(mutex);

   operational= false;              // Go non-operational
   if( socket ) {
     int rc= socket->close();
     if( rc )                       // If failure
       op_error(this, "close");     // (Message only, ignoring the error)

     delete socket;
     socket= nullptr;
     debugh("offline: %s\n", s2c(host)); // (Only one offline message)
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpListen::insert
//       HttpListen::locate
//       HttpListen::remove
//       HttpListen::status
//
// Purpose-
//       Insert Listener onto Map
//       Locate Listener with Map
//       Remove Listener from Map
//       Display Listener Map
//
//----------------------------------------------------------------------------
void
   HttpListen::insert(              // Insert onto Map
     Server_t          server)      // This HttpServer
{  if( HCDM ) debugh("Listen::insert(%s)\n", s2c(server->get_peer()));

   std::string name= server->get_peer();
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const Map_iterator_t mi= map.find(name);
     if( mi != map.end() )          // If it's already mapped
       throw std::out_of_range(
           to_string("HttpMapper::insert(%s) is a duplicate", s2c(name)) );

     // For exposition: Both versions operate correctly
     if( true )
       map.insert({name, server});
     else
       map[name]= server;
   }}}}
}

HttpListen::Server_t
   HttpListen::locate(              // Locate with Map
     std::string       peer) const  // With this peer address:port
{  if( HCDM ) debugh("Listen::locate(%s)\n", s2c(peer));

   Server_t server= nullptr;        // The HttpServer (default= none)
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     Map_t::const_iterator mi= map.find(host);
     if( mi != map.end() )          // If it's mapped
       server= mi->second;
   }}}}

   return server;
}

void
   HttpListen::remove(              // Remove from Map
     Server_t          server)      // This HttpServer
{  if( HCDM ) debugh("Listen::remove(%s)\n", s2c(server->get_peer()));

   std::string name= server->get_peer();
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const Map_iterator_t mi= map.find(name);
     if( mi != map.end() )          // If it's mapped
       map.erase(mi);               // (Remove it from the map)
   }}}}
}

void
   HttpListen::status( void ) const // Display the HttpServer Map
{
   std::forward_list<Server_t> list; // HttpServer list

   // Copy the HttpServer's from the map into the list (under mutex protection)
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);
     for(auto& it: map) {
       list.push_front(it.second);
     }
   }}}}

   // Display the HttpServer list
   debugf("%s\n", s2c(host));
   for(auto& it: list) {
     debugf("..%s\n", s2c(it->get_peer()));
   }
   debugf("\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpListen::run
//
// Purpose-
//       Run the HttpListen Thread
//
//----------------------------------------------------------------------------
void
   HttpListen::run( void )          // Run the HttpListen Thread
{  if( HCDM ) debugh("HttpListen(%p).run\n", this);

   // Accept connections
   try {
     // HttpListen can be stopped at any time
     while( operational ) {
       Socket* server_socket= socket->accept(); // Get next HttpServer Socket
       if( server_socket == nullptr ) { // If accept error
         if( operational )          // (No message if non-operational)
           op_error(this, "accept"); // (Message only, error ignored)

         continue;
       }

       Server_t server= HttpServer::make(get_self(), server_socket);
       HttpServer::Item* item= new HttpServer::Item();
       item->server= server;
       server->reader(item);        // Begin reading
     }
   } catch( std::exception& X ) {
     debugh("HttpListen::run std::exception what(%s)\n", X.what());
   } catch(...) {
     debugh("HttpListen::run catch(...)\n");
   }

   close();                         // Close and delete the Socket
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpListen::stop
//
// Purpose-
//       Stop the HttpListen
//
//----------------------------------------------------------------------------
void
   HttpListen::stop( void )         // Terminate HttpListen
{  if( HCDM ) debugh("HttpListen(%p)::stop '%s'\n", this, s2c(host));

   operational= false;
   close();                         // Close the Socket

   //-------------------------------------------------------------------------
   // Create a dummy connection to complete any pending accept, ignoring any
   // errors that occur.
   try {
     Socket dummy_connector;
     int rc= dummy_connector.open(AF_INET, SOCK_STREAM, PF_UNSPEC);
     if( rc == 0 ) {
       rc= dummy_connector.connect(host);
       if( HCDM ) {
         debugf("%4d HCDM %d= socket.connect(%s)\n", __LINE__, rc, s2c(host));
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
//       HttpListen::wait
//
// Purpose-
//       Wait for HttpListen completion
//
//----------------------------------------------------------------------------
void
   HttpListen::wait( void )         // Wait for HttpListen completion
{  if( HCDM ) debugh("HttpListen(%p)::wait '%s'\n", this, s2c(host));

   join();                          // Wait for HttpListen completion
// TODO FIX FIX FIX
// command_listen.done(self.lock()); // Remove us from the map (once)
}
