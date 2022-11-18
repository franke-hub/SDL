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
//       2022/11/16
//
// Implementation notes-
//       TODO: Create ClientListen and ServerListen, used by ClientAgent.
//         ClientListen tracks server connections, using HTTP1 HTTP2, and
//         encryption to determine whether or not to share a connection.
//         ServerListen is basically this implementation.
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
#include <pub/Dispatch.h>           // For pub::namespace pub::dispatch
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Trace.h>              // For pub::Trace
#include <pub/utility.h>            // For pub::utility::to_string(), ...

#include "pub/http/Agent.h"         // For pub::http::ListenAgent, owner
#include "pub/http/Listen.h"        // For pub::http::Listen, implemented
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Request.h"       // For pub::http::Request
#include "pub/http/Server.h"        // For pub::http::Server
#include "pub/http/Stream.h"        // For pub::http::Stream

using namespace _LIBPUB_NAMESPACE;
using namespace _LIBPUB_NAMESPACE::debugging;
using _LIBPUB_NAMESPACE::utility::should_not_occur;
using std::string;

namespace _LIBPUB_NAMESPACE::http { // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  DEFAULT_PORT= 8080               // Default port number
}; // enum

static constexpr const char* LOG_FILE= "log/HttpServer.log";

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
{  utility::report_error(line, __FILE__, op); }

//----------------------------------------------------------------------------
//
// Method-
//       Listen::Listen
//       Listen::~Listen
//       Listen::make
//
// Purpose-
//       Constructors
//       Destructor
//       Creator
//
//----------------------------------------------------------------------------
   Listen::Listen(                  // Default constructor
     ListenAgent*      owner,       // The creating ListenAgent
     const sockaddr_u& addr,        // Target internet address
     socklen_t         size,        // sizeof(addr)
     const Options*    opts_)       // Listener Options
:  agent(owner)
,  listen(), map(), mutex(), log(LOG_FILE)
,  operational(false)
,  h_close([](void) { })
,  h_request([](ServerRequest& Q) {
     std::shared_ptr<ServerStream> stream= Q.get_stream();
     stream->reject(501);           // (No request handler available)
   })
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

   rc= listen.bind(&addr.sa, size); // Initialize the Listen socket
   if( rc ) {                       // If failure
     report_error(__LINE__, "bind");
     return;
   }

   rc= listen.listen();
   if( rc != 0 ) {
     report_error(__LINE__, "listen");
     return;
   }

   // Initialize asynchronous operation
   listen.set_flags( listen.get_flags() | O_NONBLOCK );
   listen.on_select([this](int revent) { async(revent); });
   owner->select.insert(&listen, POLLIN);

   // We are operational
   log.set_file_mode("ab");
   debugf("Server: http://%s\n", addr.to_string().c_str());
     logf("Server: http://%s\n", addr.to_string().c_str());

   operational= true;
   INS_DEBUG_OBJ("*Listen*");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Listen::~Listen( void )          // Destructor
{  if( HCDM )
     debugh("Listen(%p)::~Listen\n", this);

   if( map.begin() != map.end() ) { // TODO: ?REMOVE? (Do we need this logic?)
     debugf("\n\n%d %s >>>>>>>> UNEXPECTED <<<<<<<<\n\n", __LINE__, __FILE__);
     reset();
   }
   REM_DEBUG_OBJ("*Listen*");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::shared_ptr<Listen>
   Listen::make(                    // Create Listener
     ListenAgent*      agent,       // The creating ListenAgent
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
     if( index )
       debugf("\n");
     server->debug(info);
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
//       Listen::async
//
// Purpose-
//       Handle asynchronous polling event
//
//----------------------------------------------------------------------------
void
   Listen::async(                   // Handle Asynchronous Polling Event
     int               revent)      // Polling revent
{  if( HCDM )
     debugh("Listen(%p)::async(%.4x)\n", this, revent);

   if( !operational )
     return;

   if( revent & (POLLERR | POLLNVAL) ) {
     debugf("%4d HCDM Listen revent(%.4x)\n", __LINE__, revent);
     return;
   }

   Socket* socket= listen.accept();
   if( socket == nullptr ) {
     debugh("%4d %s accept error ignored: %d:%s\n", __LINE__, __FILE__
           , errno, strerror(errno));
     return;
   }

   // Validate the socket family
   const sockaddr_u& id= socket->get_peer_addr(); // ConnectionID
   if( !Socket::is_valid(id.su_af) ) { // If invalid socket family
debugf("%4d host(%s) peer(%s)\n", __LINE__, socket->get_host_addr().to_string().c_str(), socket->get_peer_addr().to_string().c_str());
     trace(__LINE__, "sa_family(%d)", id.su_af);
     delete socket;
     return;
   }

   // Add server to map
   // Implementation note: additional locking is not required because
   // new Server objects are only created here.
   std::shared_ptr<Server> server= Server::make(this, socket);
   std::shared_ptr<Server> insert= map_insert(id, server);
   if( server.get() != insert.get() ) { // If duplicate entry
     string S= id.to_string();
     debugh("%4d %s DUPLICATED %s\n", __LINE__, __FILE__, S.c_str());
     insert->close();           // Terminate the existing server
     map_remove(id);            // Remove the inserted server from the map
     server= nullptr;           // Disallow the new connection
   }
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

   // Terminate the Listen
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex); // Borrow our map mutex

     if( operational ) {
       operational= false;

       agent->disconnect(this);     // Remove our map entry
     }
   }}}}

   reset();                         // Close all servers (asynchronously)

   // Close the Listen Socket
   int rc= listen.close();
   if( rc && VERBOSE > 1 )
     report_error(__LINE__, "close");

   h_close();                       // Drive the close handler
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
//       Reset the Listen, asynchronously closing all Servers
//
//----------------------------------------------------------------------------
void
   Listen::reset( void )            // Reset the Listen, closing all Servers
{  if( HCDM )
     debugh("Listen(%p)::reset\n", this);

   std::list<std::weak_ptr<Server>> list;

   if( HCDM )
     debugh("%4d Listen HCDM copying the Server list...\n", __LINE__);
   {{{{                             // Copy the Server list
     std::lock_guard<decltype(mutex)> lock(mutex);

     for(auto it : map ) {
       std::shared_ptr<Server> server= it.second;
       list.emplace_back(server);
     }
   }}}}

   if( HCDM )
     debugh("%4d Listen HCDM closing Servers...\n", __LINE__);
   for(auto it : list) {
     std::shared_ptr<Server> server= it.lock();
     if( server )
       server->close();             // (Asynchronously) close the server
   }
   if( HCDM )
     debugf("...All Servers closed\n");
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
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
}  // namespace _LIBPUB_NAMESPACE::http
