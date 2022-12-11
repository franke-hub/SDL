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
//       2022/12/10
//
// Implementation notes-
//       TODO?: Create intermediate Connector object rather than a full Client.
//       TODO?: Create ClientListen, ServerListen for management.
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
#include <pub/Dispatch.h>           // For pub::dispatch::Wait
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Socket.h>             // For pub::Socket::sockaddr_u
#include <pub/Trace.h>              // For pub::Trace

#include "pub/http/Agent.h"         // For pub::http::Agent, implemented
#include "pub/http/Client.h"        // For pub::http::Client
#include "pub/http/Listen.h"        // For pub::http::Listen
#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Request.h"       // For pub::http::Request

using namespace _LIBPUB_NAMESPACE;
using namespace _LIBPUB_NAMESPACE::debugging;
using std::string;
typedef Socket::sockaddr_u          sockaddr_u;

namespace _LIBPUB_NAMESPACE::http { // Implementation namespace
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  POLL_TIMEOUT= 5000               // Select timeout, in milliseconds
,  USE_VERIFY= true                 // Use verification checking
}; // enum

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
//       ClientAgent::ClientAgent
//       ClientAgent::~ClientAgent
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   ClientAgent::ClientAgent( void ) // Default constructor
:  Named("pub::http::CAgent"), Thread()
{  if( HCDM )
     debugh("http::ClientAgent(%p)!\n", this);

   start();                         // Start polling
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   ClientAgent::~ClientAgent( void ) // Destructor
{  if( HCDM )
     debugh("http::ClientAgent(%p)~...\n", this);

   operational= false;
   reset();                         // Disconnect all Clients
   stop();                          // Terminate polling
   join();                          // Wait for polling completion

   if( HCDM )
     debugh("...http::ClientAgent(%p)~\n", this);
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

   std::lock_guard<decltype(mutex)> mlock(mutex);

   int index= 0;                    // (Artificial) index
   for(const_iterator it= map.begin(); it != map.end(); ++it) {
     std::shared_ptr<Client> client= it->second;
     string H= client->get_host_addr().to_string();
     string P= client->get_peer_addr().to_string();
     debugf("..[%2d] Client(%p): %s :: %s\n", index, client.get()
           , H.c_str(), P.c_str());
     ++index;
   }

   const Select* select= &this->select;
   std::lock_guard<Select> slock(*const_cast<Select*>(select));
   this->select.debug("ClientAgent");
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
     string            peer,        // The peer:port name
     const Options*    opts)        // The associated Options
{   if( HCDM )
     debugh("http::ClientAgent(%p)::connect(%s)\n", this, peer.c_str());

   // Create a new Client
   std::shared_ptr<Client> client= Client::make(this);

   for(int index= 0; index < 2; ++index ) { // Try AF_INET, then AF_INET6
     sockaddr_storage peer_addr;
     socklen_t peer_sz= sizeof(peer_addr);
     int AF= index ? AF_INET6 : AF_INET;
     int rc= Socket::name_to_addr(peer, (sockaddr*)&peer_addr, &peer_sz, AF);
     if( rc ) {                     // If error
       if( VERBOSE > 1 )
         debugf("ClientAgent::connect(%s) failure %s\n", peer.c_str()
               , index ? "ipv6" : "ipv4");
       continue;
     }

     // Connect to Server
     if( client->connect((sockaddr*)&peer_addr, peer_sz, opts) ) {
       map_insert(client->get_peer_addr(), client->get_host_addr(), client);
       return client;
     }
   }

   // Unable to connect
   errno= EINVAL;
   return nullptr;
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

   key_t key(client->get_peer_addr(), client->get_host_addr());
   map_remove(key);
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::reset
//
// Purpose-
//       Reset the ClientAgent
//
// Implementation notes-
//       Note: client->close closes the Client's socket, immediately stopping
//       request or response processing. The Client then calls disconnect,
//       removing its entry from the map.
//
//----------------------------------------------------------------------------
void
   ClientAgent::reset( void )       // Reset the ClientAgent
{  if( HCDM ) debugh("ClientAgent(%p)::reset\n", this);

   std::list<std::weak_ptr<Client>> list;

   if( HCDM )
     debugh("%4d ClientAgent HCDM copying the Client list...\n", __LINE__);
   {{{{                             // Copy the Client list
     std::lock_guard<decltype(mutex)> lock(mutex);

     for(auto it : map ) {
       std::shared_ptr<Client> client= it.second;
       list.emplace_back(client);
     }
   }}}}

   if( HCDM )
     debugh("%4d ClientAgent HCDM closing Clients...\n", __LINE__);
   for(auto it : list) {
     std::shared_ptr<Client> client= it.lock();
     if( client && client->is_operational() )
       client->close();
   }

   if( HCDM )
     debugf("...All Clients closed\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::run
//
// Purpose-
//       Run the ClientAgent socket selector
//
//----------------------------------------------------------------------------
void
   ClientAgent::run( void )        // Run the ClientAgent socket selector
{  if( HCDM ) debugh("%4d ClientAgent(%p)::run...\n", __LINE__, this);

   while( operational ) {
     try {
       Socket* socket= select.select(POLL_TIMEOUT);
       if( socket ) {
         const struct pollfd* poll= select.get_pollfd(socket);
         socket->do_select(poll->revents);
       } else if( HCDM ) {
         debugh("ClientAgent idle poll\n");
       }
     } catch(std::exception& X) {
       errorh("%4d %s exception: %s\n", __LINE__, __FILE__, X.what());
       debug("Exception (handled)");
     } catch(...) {
       errorh("%4d %s catch(...)\n", __LINE__, __FILE__);
       debug("Exception (handled)");
     }
   }

   if( HCDM )
     debugh("%4d ...ClientAgent(%p)::run\n", __LINE__, this);
}

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::stop
//
// Purpose-
//       Terminate ClientAgent run() loop
//
//----------------------------------------------------------------------------
void
   ClientAgent::stop( void )        // Terminate ClientAgent socket selector
{  if( HCDM ) debugh("%4d ClientAgent(%p)::stop...\n", __LINE__, this);

   operational= false;
   select.flush();

   if( HCDM ) debugh("%4d ...ClientAgent(%p)::stop\n", __LINE__, this);
}

//----------------------------------------------------------------------------
//
// Method-
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
void
   ClientAgent::map_insert(         // Associate
     const key_t&       key,        // This Client/Server pair with
     std::shared_ptr<Client>
                       client)      // This Client
{
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const_iterator it= map.find(key); // Locate the Client
     if( it != map.end() ) {        // If found
       debugh("ClientAgent::map_insert(%s) duplicate\n", string(key).c_str());
       throw std::runtime_error("usage error");
     }

     map[key]= client;               // Insert the entry
   }}}}

   if( HCDM )
     debugh("%p= ClientAgent(%p)::map_insert(%s)\n", client.get(), this
           , string(key).c_str());
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::shared_ptr<Client>             // The associated Client
   ClientAgent::map_locate(         // Locate Client
     const key_t&      key) const   // For this Client/Server pair
{
   std::shared_ptr<Client> client;  // Default, not found

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const_iterator it= map.find(key);
     if( it != map.end() )          // If found
       client= it->second;
   }}}}

   if( HCDM )
     debugh("%p= ClientAgent(%p)::map_locate(%s)\n", client.get(), this
           , string(key).c_str());

   return client;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   ClientAgent::map_remove(         // Remove Client
     const key_t&       key)        // For this Client/Server pair
{
   std::shared_ptr<Client> client;  // Default, not found

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     size_t n= map.erase(key);      // Remove Client/Server pair from the map
     if( n == 0 )
       debugh("ClientAgent(%p)::map_remove(%s) not found\n", this
             , string(key).c_str());
   }}}}

   if( HCDM )
     debugh("ClientAgent(%p)::map_remove(%s)\n", this, string(key).c_str());
}

//----------------------------------------------------------------------------
//
// Method-
//       ListenAgent::ListenAgent
//       ListenAgent::~ListenAgent
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   ListenAgent::ListenAgent( void ) // Default constructor
:  Named("pub::http::LAgent"), Thread()
{  if( HCDM )
     debugh("http::ListenAgent(%p)!\n", this);

   start();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   ListenAgent::~ListenAgent( void ) // Destructor
{  if( HCDM )
     debugh("http::ListenAgent(%p)~...\n", this);

   operational= false;
   reset();                         // Disconnect all Listeners
   stop();                          // Terminate polling
   join();                          // Wait for polling completion

   if( HCDM )
     debugh("...http::ListenAgent(%p)~\n", this);
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       ListenAgent::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   ListenAgent::debug(const char* info) const // Debugging display
{  debugf("http::ListenAgent(%p)::debug(%s)\n", this, info);

   int index= 0;                    // (Artificial) index
   for(const_iterator it= map.begin(); it != map.end(); ++it) {
     std::shared_ptr<Listen> listen= it->second;
     string S= listen->get_host_addr().to_string();
     debugf("..[%2d] Listen(%p): %s\n", index, listen.get(), S.c_str());
     if( index )
       debugf("\n");
     listen->debug(info);
     ++index;
   }

   const Select* select= &this->select;
   std::lock_guard<Select> slock(*const_cast<Select*>(select));
   this->select.debug("ListenAgent");
}

//----------------------------------------------------------------------------
//
// Method-
//       ListenAgent::connect
//
// Purpose-
//       Create Listen connection
//
//----------------------------------------------------------------------------
std::shared_ptr<Listen>             // The associated server Listener
   ListenAgent::connect(            // Create server Listener
     std::string       host,        // The host:port name
     sa_family_t       family,      // The address family
     const Options*    opts)        // The associated Options
{
   if( HCDM )
     debugh("http::ListenAgent(%p)::connect(%s)\n", this, host.c_str());

   sockaddr_storage host_addr;
   socklen_t host_sz= sizeof(host_addr);
   int rc= Socket::name_to_addr(host, (sockaddr*)&host_addr, &host_sz, family);
   if( rc ) {                       // If error
     if( VERBOSE > 1 )
       debugh("ListenAgent::connect(%s) connect failure\n", host.c_str());
     errno= EINVAL;
     return nullptr;
   }

   sockaddr_u host_id;
   host_id.copy(&host_addr, host_sz);

   // Reuse existing Listen
   std::shared_ptr<Listen> listen= map_locate(host_id);
   if( listen.get() )               // If already in map, use it
     return listen;

   // No existing Listen, create one
   listen= Listen::make(this, host_id, host_sz, opts);
   if( listen->get_handle() <= 0 ) { // If connect failure
     errno= EINVAL;
     return nullptr;
   }

   map_insert(host_id, listen);      // Add Listen to map
   return listen;
}

//----------------------------------------------------------------------------
//
// Method-
//       ListenAgent::disconnect
//
// Purpose-
//       Remove Server Listener
//
//----------------------------------------------------------------------------
void
   ListenAgent::disconnect(         // Remove Server Listener
     Listen*           listen)      // For this Listener
{  if( HCDM )
     debugh("ListenAgent(%p)::disconnect(%p)\n", this, listen);

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     map.erase(listen->get_host_addr()); // Remove Listener from map
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       ListenAgent::reset
//
// Purpose-
//       Reset the ListenAgent
//
//----------------------------------------------------------------------------
void
   ListenAgent::reset( void )       // Reset the ListenAgent
{  if( HCDM ) debugh("ListenAgent(%p)::reset\n", this);

   std::list<std::weak_ptr<Listen>> list;

   if( HCDM )
     debugh("%4d ListenAgent HCDM copying the Listen list...\n", __LINE__);
   {{{{                             // Copy the Listen list
     std::lock_guard<decltype(mutex)> lock(mutex);

     for(auto it : map ) {
       std::shared_ptr<Listen> listen= it.second;
       list.emplace_back(listen);
     }
   }}}}

   if( HCDM )
     debugh("%4d ListenAgent HCDM resetting Listens...\n", __LINE__);
   for(auto it : list) {
     std::shared_ptr<Listen> listen= it.lock();
     if( listen )
       listen->reset();
   }

   if( HCDM )
     debugf("...All Listens reset\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       ListenAgent::run
//
// Purpose-
//       Run the ListenAgent socket selector
//
//----------------------------------------------------------------------------
void
   ListenAgent::run( void )        // Run the ListenAgent socket selector
{  if( HCDM ) debugh("%4d ListenAgent(%p)::run...\n", __LINE__, this);

   while( operational ) {
     try {
       Socket* socket= select.select(POLL_TIMEOUT);
       if( socket ) {
         const struct pollfd* poll= select.get_pollfd(socket);
         socket->do_select(poll->revents);
       } else if( HCDM ) {
         debugh("ListenAgent idle poll\n");
       }
     } catch(std::exception& X) {
       errorh("%4d %s exception: %s\n", __LINE__, __FILE__, X.what());
       debug("Exception (handled)");
     } catch(...) {
       errorh("%4d %s catch(...)\n", __LINE__, __FILE__);
       debug("Exception (handled)");
     }
   }

   if( HCDM )
     debugh("%4d ...ListenAgent(%p)::run\n", __LINE__, this);
}

//----------------------------------------------------------------------------
//
// Method-
//       ListenAgent::stop
//
// Purpose-
//       Terminate ListenAgent run() loop
//
//----------------------------------------------------------------------------
void
   ListenAgent::stop( void )        // Terminate ListenAgent socket selector
{  if( HCDM ) debugh("%4d ListenAgent(%p)::stop...\n", __LINE__, this);

   operational= false;
   select.flush();

   if( HCDM ) debugh("%4d ...ListenAgent(%p)::stop\n", __LINE__, this);
}

//----------------------------------------------------------------------------
//
// Method-
//       ListenAgent::map_insert
//       ListenAgent::map_locate
//       ListenAgent::map_remove
//
// Purpose-
//       Insert sockaddr_u::Listen* map entry
//       Locate Listen* from sockaddr_u
//       Remove sockaddr_u::Listen* map entry
//
// Implementation notes-
//       Protected by mutex
//
//----------------------------------------------------------------------------
std::shared_ptr<Listen>             // The associated Listen
   ListenAgent::map_insert(         // Associate
     const sockaddr_u& id,          // This sockaddr_u with
     std::shared_ptr<Listen>
                       listen)      // This Listen
{  if( HCDM )
     debugh("http::ListenAgent(%p)::insert(%s)\n", this
           , id.to_string().c_str());

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const_iterator it= map.find(id);
     if( it != map.end() )          // If found
       return it->second;           // (Duplicate entry)

     map[id]= listen;               // Insert the entry
   }}}}

   if( HCDM )
     debugh("%p= ListenAgent(%p)::insert(%s)\n", listen.get(), this
           , id.to_string().c_str());

   return listen;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::shared_ptr<Listen>             // The associated Listen
   ListenAgent::map_locate(         // Locate Listen
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
     debugh("%p= ListenAgent(%p)::locate(%s)\n", listen.get(), this, S.c_str());
   }

   return listen;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
std::shared_ptr<Listen>             // The associated Listen
   ListenAgent::map_remove(         // Remove Listen
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
     debugh("%p= ListenAgent(%p)::remove(%s)\n", listen.get(), this, S.c_str());
   }

   return listen;
}
}  // namespace _LIBPUB_NAMESPACE::::http
