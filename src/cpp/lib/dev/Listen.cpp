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
//       Listen.cpp
//
// Purpose-
//       Implement http/Listen.h
//
// Last change date-
//       2022/07/05
//
//----------------------------------------------------------------------------
#include <new>                      // For std::bad_alloc
#include <cstring>                  // For memset()
#include <mutex>                    // For std::mutex, std::lock_guard
#include <memory>                   // For std::shared_ptr
#include <ostream>                  // For std::ostream
#include <stdexcept>                // For std::out_of_range, ...
#include <string>                   // For std::string

#include <assert.h>                 // For assert macro
#include <errno.h>                  // For errno
#include <netdb.h>                  // For gethostbyname()
#include <stdio.h>                  // For fprintf()
#include <stdint.h>                 // For integer types
#include <unistd.h>                 // For gethostname
#include <arpa/inet.h>              // For inet_ntop()
#include <netinet/in.h>             // For struct sockaddr_
#include <sys/socket.h>             // For socket functions

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Socket.h>             // For pub::Socket
#include <pub/utility.h>            // For pub::utility::to_string()

#include "pub/http/Agent.h"         // For pub::http::ServerAgent, owner
#include "pub/http/Listen.h"        // The pub::http::Listen, implemented
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Request.h"       // For pub::http::Request
#include "pub/http/Server.h"        // For pub::http::Server
#include "pub/http/Stream.h"        // For pub::http::Stream
#include "pub/http/utility.h"       // For namespace pub::http::utility

using namespace _PUB_NAMESPACE;
using namespace _PUB_NAMESPACE::debugging;
// using pub::Socket;
// using pub::Socket::sockaddr_u;
using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?

,  DEFAULT_PORT= 8080               // Default port number
}; // enum

static constexpr const char* LOG_FILE= "log/HttpServer.log";

namespace pub::http {               // Implementation namespace
//----------------------------------------------------------------------------
//
// Subroutine-
//       report_error
//
// Purpose-
//       Display listener socket error message
//
//----------------------------------------------------------------------------
static void
   report_error(                    // Display generic system error message
     int               line,        // For this source code line
     const char*       op)          // For this operation name
{  utility::report_error(line, "Listen", op); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       should_not_occur
//
// Purpose-
//       Display "SHOULD NOT OCCUR" message
//
//----------------------------------------------------------------------------
#if 1  // (UNUSED)
static void
   should_not_occur(                // Display "SHOULD NOT OCCUR"  message
     int               line)        // For this source code line
{  debugh("\n%4d %s SHOULD NOT OCCUR\n\n", line, __FILE__); }
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Listen::~Listen
//       Listen::Listen
//
// Purpose-
//       Destructor
//       Constructors
//
//----------------------------------------------------------------------------
   Listen::~Listen( void )          // Destructor
{  if( HCDM )
     debugh("Listen(%p)::~Listen\n", this);

   if( map.begin() != map.end() ) { // TODO: ?REMOVE? (Do we need this logic?)
     should_not_occur(__LINE__);
     reset();
   }
}

   Listen::Listen(                  // Default constructor
     ServerAgent*      agent,       // The creating ServerAgent
     const sockaddr_u& addr,        // Target internet address
     socklen_t         size,        // sizeof(addr)
     const Options*    opts_)       // Listener Options
:  Named("pub::http::Listen"), Thread()
,  agent(agent ? agent->get_self() : nullptr)
,  listen(), map(), mutex(), log(LOG_FILE)
,  operational(false)
,  h_close(utility::f_void())
,  h_request(init_request())
{  if( HCDM )
     debugh("Listen(%p)::Listen\n", this);

   if( opts_ )
     opts.append(*opts_);

   // Initialize the Socket, allowing port re-use
   int
   rc= listen.open(addr.su_af, SOCK_STREAM, 0);
   if( rc ) {                       // If failure
     report_error(__LINE__, "open");
     return;
   }

   int optval= true;                // (Needed *before* the bind)
   listen.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   rc= listen.bind(&addr.sa, size); // Initialize the server
   if( rc ) {                       // If failure
     report_error(__LINE__, "bind");
     return;
   }

   // We are operational
   log.set_file_mode("ab");
   debugf("Server: http://%s\n", addr.to_string().c_str());
     logf("Server: http://%s\n", addr.to_string().c_str());

   operational= true;
   start();
}

std::function<void(Request&)>
   Listen::init_request( void )     // Initialize h_request
{  return [](Request& Q) {
     std::shared_ptr<Stream> stream= Q.get_stream();
     stream->reject(501);           // (No request handler available)
   };
}

std::shared_ptr<Listen>             // The Listener
   Listen::make(                    // Create Listener
     ServerAgent*      agent,       // The creating ServerAgent
     const sockaddr_u& addr,        // Target internet address
     socklen_t         size,        // sizeof(addr)
     const Options*    opts)        // Listener Options
{  if( HCDM )
     debugh("Listen::make(%p,%p) %s\n", agent, opts, addr.to_string().c_str());

   std::shared_ptr<Listen> listen=
      std::make_shared<Listen>(agent, addr, size, opts);
   listen->self= listen;

   return listen;
}

//----------------------------------------------------------------------------
//
// Method-
//       Listen::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Listen::debug(const char* info) const // Debugging display
{  debugf("Listen(%p)::debug(%s) operational(%d)\n", this, info, operational);

   int index= 0;                    // (Artificial) index
   for(const_iterator it= map.begin(); it != map.end(); ++it) {
     std::shared_ptr<Server> server= it->second;
     string S= server->get_peer_addr().to_string();
     debugf("..[%2d] Server(%p): %s\n", index, server.get(), S.c_str());
     ++index;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Listen::trace
//
// Purpose-
//       I/O operation trace (preserving errno)
//
//----------------------------------------------------------------------------
void
   Listen::trace(                   // Trace socket operation
     int               line,        // For this source code line
     const char*       fmt,         // Format string
                       ...) const   // The PRINTF argument list
{
   va_list             argptr;      // Argument list pointer

   int ERRNO= errno;                // (Preserve errno)
   std::lock_guard<decltype(*Debug::get())> lock(*Debug::get());

   traceh("%4d Listen(%p): ", line, this); // (Heading)

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
// Method-
//       Listen::close
//
// Purpose-
//       Terminate the Listener
//
//----------------------------------------------------------------------------
void
   Listen::close( void )            // Terminate the Listener
{  if( HCDM )
     debugh("Listen(%p)::close\n", this);

   operational= false;

   // Disconnect from the Agent (Must be before close)
   std::shared_ptr<ServerAgent> agent= this->agent.lock();
   if( agent )
     agent->disconnect(this);

   // Make a dummy Listen connection
   Socket socket;
   int
   rc= socket.open(AF_INET, SOCK_STREAM, 0);
   if( rc ) report_error(__LINE__, "close: socket.open");
   rc= socket.connect((sockaddr*)&listen.get_host_addr(), sizeof(sockaddr_u));
   if( rc ) report_error(__LINE__, "close: socket.connect");

   // Close the Listen Socket
   rc= listen.close();              // Close the Listener Socket
   if( rc ) report_error(__LINE__, "close");
   h_close();                       // Drive the close handler

   // Close all Servers
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       Listen::disconnect
//
// Purpose-
//       Disconnect Server callback
//
//----------------------------------------------------------------------------
void
   Listen::disconnect(              // Server completion callback
     Server*           server)      // (For this server)
{  if( HCDM )
     debugh("Listen(%p)::disconnect(%p)\n", this, server );

   map.erase(server->get_peer_addr()); // Remove element from map
}

//----------------------------------------------------------------------------
//
// Method-
//       Listen::map_insert
//       Listen::map_locate
//       Listen::map_remove
//
// Purpose-
//       Insert internet_address::Server* map entry
//       Locate Server* from internet_address
//       Remove internet_address::Server* map entry
//
// Implementation notes-
//       Protected by mutex
//       TODO: Make consistent: insert/locate/remove not always used.
//
//----------------------------------------------------------------------------
std::shared_ptr<Server>             // The associated Server
   Listen::map_insert(              // Associate
     const sockaddr_u& id,          // This connectionID
     std::shared_ptr<Server>
                       server)      // This Server
{  if( HCDM ) {
     debugh("Listen(%p)::insert(...)\n", this);
   }

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const_iterator it= map.find(id);
     if( it != map.end() )          // If found
       return it->second;           // (Duplicate entry)

     map[id]= server;               // Insert the entry
   }}}}

   if( HCDM ) {
     debugh("%p= Listen(%p)::insert(%s)\n", server.get(), this
           , id.to_string().c_str());
   }

   return server;
}

std::shared_ptr<Server>             // The associated Server
   Listen::map_locate(              // Locate Server
     const sockaddr_u& id)          // For this connectionID
{
   std::shared_ptr<Server> server;  // Default, not found

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const_iterator it= map.find(id);
     if( it != map.end() )          // If found
       server= it->second;
   }}}}

   if( HCDM ) {
     debugh("%p= Listen(%p)::locate(%s)\n", server.get(), this
           , id.to_string().c_str());
   }

   return server;
}

std::shared_ptr<Server>             // The associated Server
   Listen::map_remove(              // Remove Server
     const sockaddr_u& id)          // For this connectionID
{
   std::shared_ptr<Server> server;  // Default, not found

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     iterator it= map.find(id);
     if( it != map.end() ) {        // If found
       server= it->second;          // (Return removed Server)
       map.erase(it);               // Remove it
     }
   }}}}

   if( HCDM ) {
     debugh("%p= Listen(%p)::remove(%s)\n", server.get(), this
           , id.to_string().c_str());
   }

   return server;
}

//----------------------------------------------------------------------------
//
// Method-
//       Listen::logf
//
// Purpose-
//       Write to log file
//
//----------------------------------------------------------------------------
void
   Listen::logf(                    // Write to log file
     const char*       fmt,         // The PRINTF format string
                       ...)         // The PRINTF argument list
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   log.vtracef(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

//----------------------------------------------------------------------------
//
// Method-
//       Listen::opt_append
//       Listen::opt_reset
//
// Purpose-
//       Append Options
//       Reset Options
//
//----------------------------------------------------------------------------
void
   Listen::opt_append(              // Append options
     const Options&    opts)
{  if( HCDM ) {
     debugh("Listen(%p)::set_options(%p)\n", this, &opts);
     opts.debug("Listen");
   }

   this->opts.append(opts);
}

void
   Listen::opt_reset(               // Reset options
     const Options&    opts)
{  if( HCDM ) {
     debugh("Listen(%p)::opt_reset(%p)\n", this, &opts);
     opts.debug("Listen");
   }

   this->opts= opts;
}

//----------------------------------------------------------------------------
//
// Method-
//       Listen::reset
//
// Purpose-
//       Reset the Listener, closing all Servers
//
//----------------------------------------------------------------------------
void
   Listen::reset( void )            // Reset the Listener, closing all Servers
{  if( HCDM )
     debugh("Listen(%p)::reset\n", this);

   // Close all servers
   // Note: server->close() closes the Server's socket, also causing
   // the Server to exit the read loop. The Server then calls disconnect
   // which removes the server entry from the map, removing its penultimate
   // shared_ptr<Server>. Exiting loop iteration removes its last known
   // shared_ptr, driving delete.
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

// debugh("Listen HCDM deleting Servers...\n");
     for(iterator it= map.begin(); it != map.end(); it= map.begin()) {
       std::shared_ptr<Server> server= it->second;
// debugf("...Server(%p) %s\n", server.get(), it->first->to_string().c_str());
       server->close();
       server->join();
       server= nullptr;
     }
// debugf("...All Servers deleted\n");
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       Listen::run
//
// Purpose-
//       Operate the Listener
//
//----------------------------------------------------------------------------
void
   Listen::run( void )              // Operate the Listener
{  if( HCDM )
     debugh("Listen(%p)::run\n", this);

   int rc= listen.listen();
   if( rc != 0 ) {
     trace(__LINE__, "%d= listen, Listen terminated", rc);
     return;
   }

   // Listen for new connections
   while(operational) {
     Socket* socket= listen.accept();
     if( socket ) {
       if( !operational ) {
         delete socket;
         break;
       }

       // Create Server
       const sockaddr_u& id= socket->get_peer_addr(); // ConnectionID
       if( !Socket::is_valid(id.su_af) ) { // If invalid socket family
debugf("%4d host(%s) peer(%s)\n", __LINE__, socket->get_host_addr().to_string().c_str(), socket->get_peer_addr().to_string().c_str());

         trace(__LINE__, "sa_family(%d)", id.su_af);
         delete socket;
         continue;
       }

       // Add server to map
       // Implementation note: additional locking is not required because
       // new Server objects are only created here, in the Listen thread.
       std::shared_ptr<Server> server= Server::make(this, socket);
       std::shared_ptr<Server> insert= map_insert(id, server);
       if( server.get() != insert.get() ) { // If duplicate entry
         string S= id.to_string();
         debugh("%4d %s DUPLICATED %s\n", __LINE__, __FILE__, S.c_str());
         map_remove(id);            // Disconnect the existing server
         insert->close();           // Terminate the existing server
         server= nullptr;           // Disallow the new connection
       } else {
         server->start();
       }
     } else if( operational) {      // If spurious failure
       debugh("%4d %s listen(%d:%s) ignored\n", __LINE__, __FILE__
             , errno, strerror(errno));
     }
   }

   // Non-operational
   reset();
}
}  // namespace pub::http
