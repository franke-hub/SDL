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
//       Agent.cpp
//
// Purpose-
//       Implement http/Agent.h
//
// Last change date-
//       2022/07/05
//
//----------------------------------------------------------------------------
#include <memory>                   // For std::shared_ptr
#include <new>                      // For std::bad_alloc
#include <cassert>                  // For assert
#include <cstring>                  // For memset
#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr
#include <ostream>                  // For std::ostream
#include <stdexcept>                // For std::out_of_range, ...
#include <string>                   // For std::string

#include <netdb.h>                  // For addrinfo, ...
#include <stdio.h>                  // For fprintf
#include <stdint.h>                 // For integer types
#include <string.h>                 // For strcmp
#include <arpa/inet.h>              // For inet_ntop()

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Socket.h>             // For pub::Socket::sockaddr_u

#include "pub/http/Agent.h"         // For pub::http::Agent objects, implemented
#include "pub/http/Client.h"        // For pub::http::Client
#include "pub/http/Listen.h"        // For pub::http::Listen
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Request.h"       // For pub::http::Request

using namespace _PUB_NAMESPACE;
using namespace _PUB_NAMESPACE::debugging;
//using _PUB_NAMESPACE::Socket::sockaddr_u;
//using _PUB_NAMESPACE::utility::to_string;
using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
// HCDM= false                      // Hard Core Debug Mode?
}; // enum

namespace pub::http {               // Implementation namespace
//----------------------------------------------------------------------------
//
// Subroutine-
//       port_name_to_number
//
// Purpose-
//       Convert a named service to the associated default port number string.
//
// Implementation notes-
//       If not found, the name is returned untranslated. (Assumed numeric)
//
//----------------------------------------------------------------------------
#if 0 // TODO: REMOVE (DEFERRED)
static const struct {               // Service name to number correlator
const char*            name;        // The service name
const char*            number;      // The service port number string
}                      name_to_number[]=
{   {"HTTP",  "80"}
,   {"HTTPS", "433"}
,   {nullptr, nullptr}
};

static in_port_t                    // The port numbername
   port_name_to_number(             // Convert named service to default port
     const char*       name)        // The port name
{
   for(int i= 0; name_to_number[i].name; ++i) {
     if( strcmp(name, name_to_number[i].name) == 0 ) {
       name= name_to_number[i].number;
       break;
     }
   }

   int port= atoi(name);            // Convert to number
   if( port < 0 || port > UINT16_MAX ) // If out of range
     port= 0;                       // (Error value)
   return (in_port_t)port;
}
#endif

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::~ClientAgent
//       ClientAgent::ClientAgent
//
// Purpose-
//       Destructor
//       Constructors
//
//----------------------------------------------------------------------------
   ClientAgent::~ClientAgent( void ) // Destructor
{  if( HCDM )
     debugh("http::~ClientAgent(%p)\n", this);

   reset();
}

   ClientAgent::ClientAgent( void ) // Default constructor
{  if( HCDM )
     debugh("http::ClientAgent(%p)\n", this);
}

std::shared_ptr<ClientAgent>
   ClientAgent::make( void )        // Default creator
{  if( HCDM )
     debugh("http::ClientAgent::make\n");

   std::shared_ptr<ClientAgent> A= std::make_shared<ClientAgent>();
   A->self= A;
   return A;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   ClientAgent::debug(const char* info) const // Debugging display
{  debugf("http::ClientAgent(%p)::debug(%s)\n", this, info);

   int index= 0;                    // (Artificial) index
   for(const_iterator it= map.begin(); it != map.end(); ++it) {
     std::shared_ptr<Client> client= it->second;
     string S= client->get_peer_addr().to_string();
     debugf("..[%2d] Client(%p): %s\n", index, client.get(), S.c_str());
     ++index;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::get_client
//       ClientAgent::map_insert
//       ClientAgent::map_locate
//       ClientAgent::map_remove
//
// Purpose-
//       Get Client sockaddr_u connectionID
//       Insert sockaddr_u::Client* map entry
//       Locate Client* from sockaddr_u
//       Remove sockaddr_u::Client* map entry
//
// Implementation notes-
//       Protected by mutex
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   ClientAgent::get_client(         // Get Client connectionID
     const char*       host,        // *INP* The host name
     const char*       port,        // *INP* The port number or service name
     sockaddr_u&       sock_addr,   // *OUT* The sockaddr_u
     socklen_t&        sock_size)   // *OUT* The sockaddr length
{
   sock_addr.reset();               // Initialize resultant
   sock_size= 0;

   string nps= host; nps += ':'; nps += port;
   sockaddr_storage hostaddr;
   socklen_t hostsize= sizeof(hostaddr);
   int rc= Socket::name_to_addr(nps, (sockaddr*)&hostaddr, &hostsize, AF_INET);
   if( rc ) {                       // If error
     debugh("ClientAgent::connect(%s:%s) invalid host:port\n", host, port);
     connect_error= rc;
     return -1;
   }

   sock_addr.copy((sockaddr*)&hostaddr, hostsize);
   sock_size= hostsize;
   return 0;
}

std::shared_ptr<Client>             // The associated Client
   ClientAgent::map_insert(         // Associate
     const sockaddr_u& id,          // This sockaddr_u with
     std::shared_ptr<Client>
                       client)      // This Client
{  if( HCDM )
     debugh("http::ClientAgent(%p)::insert(%s)\n", this
           , id.to_string().c_str());

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const_iterator it= map.find(id);
     if( it != map.end() )          // If found
       return it->second;           // (Duplicate entry)

     map[id]= client;               // Insert the entry
   }}}}

   if( HCDM )
     debugh("%p= ClientAgent(%p)::insert(%s)\n", client.get(), this
           , id.to_string().c_str());

   return client;
}

std::shared_ptr<Client>             // The associated Client
   ClientAgent::map_locate(         // Locate Client
     const sockaddr_u& id) const    // For this sockaddr_u
{
   std::shared_ptr<Client> client;  // Default, not found

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const_iterator it= map.find(id);
     if( it != map.end() )          // If found
       client= it->second;
   }}}}

   if( HCDM ) {
     string S= id.to_string();
     debugh("%p= ClientAgent(%p)::locate(%s)\n", client.get(), this, S.c_str());
   }

   return client;
}

std::shared_ptr<Client>             // The associated Client
   ClientAgent::map_remove(         // Remove Client
     const sockaddr_u&  id)         // For this sockaddr_u
{
   std::shared_ptr<Client> client;  // Default, not found

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     iterator it= map.find(id);
     if( it != map.end() ) {        // If found
       client= it->second;          // (Return removed Client)
       map.erase(it);               // Remove it from the map
     }
   }}}}

   if( HCDM ) {
     string S= id.to_string();
     debugh("%p= ClientAgent(%p)::remove(%s)\n", client.get(), this, S.c_str());
   }

   return client;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::connect
//
// Purpose-
//       Create Client connection
//
//----------------------------------------------------------------------------
std::shared_ptr<Client>             // The associated Client
   ClientAgent::connect(            // Create Client connection
     std::string       host_,       // The host name
     std::string       port_,       // The port number or name (HTTP, ...)
     const Options*    opts)        // The associated Options
{
   const char* host= host_.c_str();
   const char* port= port_.c_str();
   if( HCDM )
     debugh("http::ClientAgent(%p)::connect(%s:%s)\n", this, host, port);

   sockaddr_u id;                   // (Set by get_client)
   socklen_t  sz;                   // "
   int rc= get_client(host, port, id, sz);
   if( rc )                         // If unknown host/port
     return nullptr;                // Return, error reported by get_client

   std::shared_ptr<Client> client= map_locate(id);
   if( client.get() )               // If already in map, use it
     return client;

   // No existing Client, create one
   client= Client::make(this, id, sz, opts);
   if( client->get_handle() <= 0 )  // If connect failure
     return nullptr;                // Return, error reported by Client

   map_insert(id, client);          // Add client to map

   return client;
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::disconnect
//
// Purpose-
//       Remove Client connection
//
//----------------------------------------------------------------------------
void
   ClientAgent::disconnect(         // Remove Client connection
     Client*           client)      // For this Client
{  if( HCDM )
     debugh("ClientAgent(%p)::disconnect(%p)\n", this, client);

   map.erase(client->get_peer_addr()); // Remove Client from map
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::reset
//
// Purpose-
//       Reset the ClientAgent
//
//----------------------------------------------------------------------------
void
   ClientAgent::reset( void )       // Reset the ClientAgent
{  if( HCDM ) debugh("ClientAgent(%p)::reset\n", this);

   // Close all Clients
   // Note: client->close() closes the Client's socket. This immediately
   // stops the Client from processing requests.
   // The Client then calls disconnect, removing its entry from the map.
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

// debugh("%4d ClientAgent HCDM deleting Clients...\n", __LINE__);
     for(iterator it= map.begin(); it != map.end(); it= map.begin()) {
       std::shared_ptr<Client> client= it->second;
// debugf("...Client(%p)\n", client.get());
       client->close();
     }
// debugf("...All Clients deleted\n");
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerAgent::~ServerAgent
//       ServerAgent::ServerAgent
//       ServerAgent::make
//
// Purpose-
//       Destructor
//       Constructors
//       Creators
//
//----------------------------------------------------------------------------
   ServerAgent::~ServerAgent( void ) // Destructor
{  if( HCDM )
     debugh("http::~ServerAgent(%p)\n", this);

   reset();
}

   ServerAgent::ServerAgent( void ) // Default constructor
{  if( HCDM )
     debugh("http::ServerAgent(%p)\n", this);
}

std::shared_ptr<ServerAgent>
   ServerAgent::make( void ) // Default creator
{  if( HCDM )
     debugh("http::ServerAgent::make\n");

   std::shared_ptr<ServerAgent> A= std::make_shared<ServerAgent>();
   A->self= A;
   return A;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerAgent::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   ServerAgent::debug(const char* info) const // Debugging display
{  debugf("http::ServerAgent(%p)::debug(%s)\n", this, info);

   int index= 0;                    // (Artificial) index
   for(const_iterator it= map.begin(); it != map.end(); ++it) {
     std::shared_ptr<Listen> listen= it->second;
     string S= listen->get_host_addr().to_string();
     debugf("..[%2d] Listen(%p): %s\n", index, listen.get(), S.c_str());
     ++index;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerAgent::get_server
//       ServerAgent::map_insert
//       ServerAgent::map_locate
//       ServerAgent::map_remove
//
// Purpose-
//       Get Server connectionID
//       Insert sockaddr_u::Listen* map entry
//       Locate Listen* from sockaddr_u
//       Remove sockaddr_u::Listen* map entry
//
// Implementation notes-
//       Protected by mutex
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 expected
   ServerAgent::get_server(         // Get Server connectionID
     std::string       host_,       // *INP* The host name (for interface)
     std::string       port_,       // *INP* The port number or service name
     sockaddr_u&       sock_addr,   // *OUT* The sockaddr_u
     socklen_t&        sock_size,   // *OUT* The sockaddr length
     sa_family_t       family)      // The address family
{
   sock_addr.reset();               // Initialize resultant
   sock_size= 0;

   if( host_ == "" )                // If defaulted host
     host_= Socket::gethostname();
   string nps= host_; nps += ':'; nps += port_;
   sockaddr_storage peeraddr;
   socklen_t peersize= sizeof(peeraddr);
   int rc= Socket::name_to_addr(nps, (sockaddr*)&peeraddr, &peersize, family);
   if( rc ) {                       // If error
     debugh("ClientAgent::connect(%s) invalid host:port\n", nps.c_str());
     connect_error= rc;
     return -1;
   }

   sock_addr.copy((sockaddr*)&peeraddr, peersize);
   sock_size= peersize;
   return 0;
}

std::shared_ptr<Listen>             // The associated Listen
   ServerAgent::map_insert(         // Associate
     const sockaddr_u& id,          // This sockaddr_u with
     std::shared_ptr<Listen>
                       listen)      // This Listen
{  if( HCDM )
     debugh("http::ServerAgent(%p)::insert(%s)\n", this
           , id.to_string().c_str());

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const_iterator it= map.find(id);
     if( it != map.end() )          // If found
       return it->second;           // (Duplicate entry)

     map[id]= listen;               // Insert the entry
   }}}}

   if( HCDM )
     debugh("%p= ServerAgent(%p)::insert(%s)\n", listen.get(), this
           , id.to_string().c_str());

   return listen;
}

std::shared_ptr<Listen>             // The associated Listen
   ServerAgent::map_locate(         // Locate Listen
     const sockaddr_u& id) const    // For this sockaddr_u
{
   std::shared_ptr<Listen> listen;  // Default, not found

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const_iterator it= map.find(id);
     if( it != map.end() )          // If found
       listen= it->second;
   }}}}

   if( HCDM ) {
     string S= id.to_string();
     debugh("%p= ServerAgent(%p)::locate(%s)\n", listen.get(), this, S.c_str());
   }

   return listen;
}

std::shared_ptr<Listen>             // The associated Listen
   ServerAgent::map_remove(         // Remove Listen
     const sockaddr_u& id)          // For this sockaddr_u
{
   std::shared_ptr<Listen> listen;  // Default, not found

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     iterator it= map.find(id);
     if( it != map.end() ) {        // If found
       listen= it->second;          // (Return removed Listen)
       map.erase(it);               // Remove it from the map
     }
   }}}}

   if( HCDM ) {
     string S= id.to_string();
     debugh("%p= ServerAgent(%p)::remove(%s)\n", listen.get(), this, S.c_str());
   }

   return listen;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerAgent::connect
//
// Purpose-
//       Create Listen connection
//
//----------------------------------------------------------------------------
std::shared_ptr<Listen>             // The associated server Listener
   ServerAgent::connect(            // Create server Listener
     std::string       host_,       // The host name (for interface)
     std::string       port_,       // The port number or name (HTTP, ...)
     sa_family_t       family,      // The address family
     const Options*    opts)        // The associated Options
{
   const char* port= port_.c_str();
   if( HCDM )
     debugh("http::ServerAgent(%p)::connect(%s:%d)\n", this, port, family);

   sockaddr_u id;                   // (Set by get_server)
   socklen_t  sz;                   // (Set by get_server)
   int rc= get_server(host_, port_, id, sz, family);
   if( rc )                         // If unknown host/port
     return nullptr;                // Return, error reported by get_server

   std::shared_ptr<Listen> listen= map_locate(id);
   if( listen.get() )               // If already in map, use it
     return listen;

   // No existing Listen, create one
   listen= Listen::make(this, id, sz, opts);
   if( listen->get_handle() <= 0 )  // If connect failure
     return nullptr;                // Return, error reported by Listen

   map_insert(id, listen);          // Add Listen to map

   return listen;
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerAgent::disconnect
//
// Purpose-
//       Remove Server Listener
//
//----------------------------------------------------------------------------
void
   ServerAgent::disconnect(         // Remove Server Listener
     Listen*           listen)      // For this Listener
{  if( HCDM )
     debugh("ServerAgent(%p)::disconnect(%p)\n", this, listen);

   map.erase(listen->get_host_addr()); // Remove Listener from map
}

//----------------------------------------------------------------------------
//
// Method-
//       ServerAgent::reset
//
// Purpose-
//       Reset the ServerAgent
//
//----------------------------------------------------------------------------
void
   ServerAgent::reset( void )       // Reset the ServerAgent
{  if( HCDM ) debugh("ServerAgent(%p)::reset\n", this);

   // Close all Listeners. Each Listener then closes all its Servers.
   // Note: Listen::close() clears Listen::operational and closes the Listen's
   // socket. This immediately causes the Listen to stop processing requests.
   // Listen::close() invokes Listen::reset(), which closes all of its active
   // Servers using logic similar to this method's.
   // The Listen then calls disconnect, removing its entry from the map.

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

//// debugh("ServerAgent HCDM deleting Listens...\n");
     for(iterator it= map.begin(); it != map.end(); it= map.begin()) {
       std::shared_ptr<Listen> listen= it->second;
////   debugf("...Listen(%p)\n", listen.get());
       listen->close();
     }
//// debugf("...All Listens deleted\n");
   }}}}
}
}  // namespace pub::http
