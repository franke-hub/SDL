//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       HttpServer.cpp
//
// Purpose-
//       Implement HttpServer.h
//
// Last change date-
//       2025/01/22
//
// Implementation note-
//       Derived from ~/src/cpp/HTTP/socket/HttpServer.cpp 2024/10/24
//
//----------------------------------------------------------------------------
#include <forward_list>             // For std::forward_list
#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr, std::weak_ptr, ...
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string
#include <cstdio>                   // For perror

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For namespace pub::dispatch objects
#include <pub/Ioda.h>               // For pub::Ioda
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread, HttpListen base class
#include <pub/utility.h>            // For pub::utility::to_string, ...
#include <pub/utility.i>            // For conversion subroutines

#include "HttpListen.h"             // For HttpListen
#include "HttpServer.h"             // For HttpServer, implemented

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For convenience
using namespace PUB::debugging;     // For debugging subroutines
using PUB::utility::to_string;      // For convenience
using std::string;                  // For convenience

typedef pub::Ioda::Mesg   Mesg;     // For convenience

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  INP_SIZE= 65536                  // Input buffer size
,  USE_ITEM_HCDM= true              // Use HttpServer::Item traces?
}; // enum

//----------------------------------------------------------------------------
// Global constructor/destructor, currently unused
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
static struct GlobalInitTerm {      // Global initialization/termination
inline
   GlobalInitTerm( void )
{  if( HCDM ) debugh("HttpServer::GlobalInitTerm!\n");
}

inline
   ~GlobalInitTerm( void )
{  if( HCDM ) debugh("HttpServer::GlobalInitTerm~\n");
}
}  global_initterm;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Struct-
//       DH_task
//
// Purpose-
//       The *DEFAULT* data handler task
//
//----------------------------------------------------------------------------
static const char*     page200=     // The 200 Dummy File message
         "<html><head><title>PAGE 200</title></head>\r\n"
         "<body><h1 align=\"center\">Default Response Page</h1>\r\n"
         "No Body's Home, Paige\r\n"
         "</body></html>\r\n"
         ;

struct DH_task {
typedef HttpServer::Item            Item_t;
typedef std::shared_ptr<HttpServer> Server_t;

void operator()(pub::dispatch::Item* _item)
{
   Item_t* item= dynamic_cast<Item_t*>(_item);
   if( item == nullptr ) {
     _item->post(_item->CC_ERROR_IT);
   } else {
     Server_t server= item->server;
     if( !server->is_operational() ) {
       server->close();
       pub::dispatch::Disp::post(item, Item_t::CC_PURGE);
       return;
     }

     pub::Ioda& ioda= item->ioda;
     ioda.reset();                  // (Discard the request data)

     // Write the canned response
     ioda.put( "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n" );
     ioda.put( to_string("Content-length: %zd\r\n\r\n", strlen(page200)) );
     ioda.put( page200 );
     server->writer(item);          // Send response
   }
}
}; // struct DH_task

//----------------------------------------------------------------------------
//
// Struct-
//       RD_task
//
// Purpose-
//       The Reader Task
//
//----------------------------------------------------------------------------
struct RD_task {
typedef HttpServer::Item            Item_t;
typedef std::shared_ptr<HttpServer> Server_t;
typedef pub::Socket*                Socket_t;

void operator()(pub::dispatch::Item* _item)
{
   Item_t* item= dynamic_cast<Item_t*>(_item);
   if( item == nullptr ) {
     _item->post(_item->CC_ERROR_IT);
   } else {
     Server_t server= item->server;
     if( !server->is_operational() ) {
       server->close();
       pub::dispatch::Disp::post(item, Item_t::CC_PURGE);
       return;
     }

     pub::Ioda& ioda= item->ioda;
     Socket_t socket= server->get_socket();

     Mesg mesg;
     ioda.set_rd_mesg(mesg, INP_SIZE);
     ssize_t L= socket->recvmsg(&mesg, 0);
     if( L <= 0 ) {
       if( HCDM && VERBOSE )
         errorh("HttpServer(%p): recvmsg error %d:%s\n", server.get()
               , errno, strerror(errno));

       server->close();
       pub::dispatch::Disp::post(item, Item_t::CC_ERROR);
       return;
     }

     // Handle the request
     ioda.set_used(L);
     server->handle(item);
   }
}
}; // struct RD_task

//----------------------------------------------------------------------------
//
// Struct-
//       WR_task
//
// Purpose-
//       The Writer Task
//
//----------------------------------------------------------------------------
struct WR_task {
typedef HttpServer::Item            Item_t;
typedef std::shared_ptr<HttpServer> Server_t;
typedef pub::Socket*                Socket_t;

void operator()(pub::dispatch::Item* _item)
{
   Item_t* item= dynamic_cast<Item_t*>(_item);
   if( item == nullptr ) {
     _item->post(_item->CC_ERROR_IT);
   } else {
     Server_t server= item->server;
     if( !server->is_operational() ) {
       server->close();
       pub::dispatch::Disp::post(item, Item_t::CC_PURGE);
       return;
     }

     pub::Ioda& ioda= item->ioda;
     Socket_t socket= server->get_socket();

     Mesg mesg;
     ioda.set_wr_mesg(mesg);
     ssize_t L= socket->sendmsg(&mesg, 0);
     if( L <= 0 ) {
       if( HCDM && VERBOSE )
         errorh("HttpServer(%p): sendmsg error %d:%s\n", server.get()
               , errno, strerror(errno));

       server->close();
       pub::dispatch::Disp::post(item, Item_t::CC_ERROR);
       return;
     }

     // Re-use Ioda for read
     ioda.reset(L);
     server->reader(item);
   }
}
}; // struct WR_task

//----------------------------------------------------------------------------
//
// Method-
//       HttpServer::Item::Item
//       HttpServer::Item::~Item
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   HttpServer::Item::Item( void )   // Constructor
{  if( HCDM || USE_ITEM_HCDM )
     traceh("HttpServer::Item(%p)!\n", this);

   INS_DEBUG_OBJ("HttpServer::Item");
}

   HttpServer::Item::~Item( void )  // Destructor
{  if( HCDM || USE_ITEM_HCDM )
     traceh("HttpServer::Item(%p)~\n", this);

   REM_DEBUG_OBJ("HttpServer::Item");
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServer::HttpServer
//       HttpServer::~HttpServer
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   HttpServer::HttpServer(          // Constructor
     Listen_t          _owner,      // Creator
     Socket_t*         _socket)     // Server Socket
:  owner(_owner), socket(_socket)
{  if( HCDM )
     debugh("HttpServer(%p)!(%p) %s\n", this, _socket, s2c(get_peer()));
   INS_DEBUG_OBJ("HttpServer");

   // Set run_diagnostics handler
   typedef pub::signals::Event_t Event_t;
   typedef StaticCommon::DiagnosticEvent DiagnosticEvent;
   connector= static_common->run_diagnostics.connect([this](Event_t& _event) {
     DiagnosticEvent* event= dynamic_cast<DiagnosticEvent*>(&_event);
     if( event )
       debugf("DiagnosticEvent %d\n", event->id);

     debug("diagnostics");
   });

   // Set default work handlers
   dh_task.on_work(DH_task());
   rd_task.on_work(RD_task());
   wr_task.on_work(WR_task());

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
   HttpServer::~HttpServer( void )  // Destructor
{  if( HCDM ) debugh("HttpServer(%p)~\n", this);

   // Close and delete the socket
   if( socket )                     // If Socket exists
     close();                       // Close and delete the Socket

   REM_DEBUG_OBJ("HttpServer");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::shared_ptr<HttpServer>         // Creator
   HttpServer::make(                // Make: std::shared_ptr<Socket>
     Listen_t          _owner,      // Creator
     pub::Socket*      _socket)     // The HttpServer Socket
{  if( HCDM ) debugh("HttpServer::make(%p)\n", _socket );

   std::shared_ptr<HttpServer> server(new HttpServer(_owner, _socket));
   server->self= server;

   _owner->insert(server);
   return server;
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServer::debug
//
// Purpose-
//       Display diagnostic information
//
//----------------------------------------------------------------------------
void
   HttpServer::debug(               // Display debugging information
     const char*       info) const  // Caller information
{
   std::lock_guard<pub::Debug> lock(*Debug::get());

   debugf("\nHttpServer(%p)::debug(%s) operational(%d)\n", this
         , info, operational);

   if( socket )
     debugf("..socket(%p) %s<==>%s\n", socket
           , s2c(socket->get_host_addr().to_string())
           , s2c(socket->get_peer_addr().to_string()) );
   else
     debugf("..socket(nullptr)\n");

   dh_task.debug("DH_task");
   rd_task.debug("RD_task");
   wr_task.debug("WR_task");
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServer::get_peer
//
// Purpose-
//       Get peer address:port string
//
//----------------------------------------------------------------------------
std::string                         // The peer address:port string
   HttpServer::get_peer( void ) const // Get peer address:port string
{  return socket->get_peer_addr().to_string(); }

//----------------------------------------------------------------------------
//
// Method-
//       HttpServer::close
//
// Purpose-
//       Close the Socket
//
//----------------------------------------------------------------------------
void
   HttpServer::close( void )        // Close the Socket
{  if( HCDM ) debugh("HttpServer(%p)::close(%p)\n", this, socket);

   operational= false;              // (If not already,) go non-operational

   std::lock_guard<decltype(mutex)> lock(mutex);
   if( socket ) {                   // (Only close/delete socket once)
     // Remove this HttpServer from HttpListen's map
     owner->remove(get_self());
     if( HCDM ) debugh("HttpServer(%p)::close %s\n", this, s2c(get_peer()));

     socket->close();
     delete socket;
     socket= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServer::stop
//
// Purpose-
//       Stop running the HttpServer
//
//----------------------------------------------------------------------------
void
   HttpServer::stop( void )         // Terminate Socket reading
{  if( HCDM ) debugh("HttpServer(%p)::stop\n", this);

   close();                         // Close the Socket
}
