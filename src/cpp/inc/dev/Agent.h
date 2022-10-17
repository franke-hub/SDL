//----------------------------------------------------------------------------
//
//       Copyright (C) 2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       http/Agent.h
//
// Purpose-
//       HTTP Agent objects: ClientAgent and ServerAgent.
//
// Last change date-
//       2022/10/16
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_AGENT_H_INCLUDED
#define _LIBPUB_HTTP_AGENT_H_INCLUDED

#include <cstdlib>                  // For size_t
#include <cstring>                  // For memcmp
#include <functional>               // For std::function
#include <list>                     // For std::list
#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::mutex, std::lock_guard
#include <string>                   // For std::string
#include <netinet/in.h>             // For in_port_t
#include <sys/socket.h>             // For socket

#include <pub/Named.h>              // For pub::Named (base class)
#include <pub/Select.h>             // For pub::Select
#include <pub/Socket.h>             // For pub::Socket::sockaddr_u
#include <pub/Thread.h>             // For pub::Thread (base class)

#include "dev/bits/devconfig.h"     // For HTTP config controls

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class ClientAgent;
class Client;
class Listen;
class Options;
class ServerAgent;

//----------------------------------------------------------------------------
//
// Struct-
//       ClientConnectionPair
//
// Purpose-
//       The ClientAgent's map key
//
//----------------------------------------------------------------------------
struct ClientConnectionPair {       // The ClientAgent map key
typedef Socket::sockaddr_u          sockaddr_u;

sockaddr_u             peer;        // The Server's internet address
sockaddr_u             host;        // The Client's internet address

   ClientConnectionPair(            // Constructor
     const sockaddr_u& peer,        // The Server's internet address
     const sockaddr_u& host)        // The Client's internet address
:  peer(peer), host(host) {}

   ClientConnectionPair(            // Copy constructor
     const ClientConnectionPair& src)
:  peer(src.peer), host(src.host) {}

ClientConnectionPair&
   operator=(const ClientConnectionPair& src) // Assignment operator
{  peer= src.peer; host= src.host; return *this; }

bool operator<(const ClientConnectionPair& rhs) const
{
   int cc= memcmp(&peer, &rhs.peer, sizeof(peer));
   if( cc == 0 )
     cc= memcmp(&host, &rhs.host, sizeof(host));

   return (cc < 0);
}

   explicit operator std::string() const // Cast to std::string
{  return std::string("{") + peer.to_string() + ";" + host.to_string() + "}"; }
}; // struct ClientConnectionPair

//----------------------------------------------------------------------------
//
// Class-
//       Agent
//
// Purpose-
//       The Agent owns the ClientAgent and the ServerAgent
//
// Implementation notes-
//       Agent::shutdown is used for an orderly shutdown.
//
//----------------------------------------------------------------------------
class Agent {                       // AgentOwner class
//----------------------------------------------------------------------------
// ClientAgent::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::shared_ptr<ClientAgent>          client_ptr;
typedef std::shared_ptr<ServerAgent>          server_ptr;

//----------------------------------------------------------------------------
// Agent::Attributes
//----------------------------------------------------------------------------
protected:
client_ptr             client;      // The ClientAgent
server_ptr             server;      // The ServerAgent

//----------------------------------------------------------------------------
// Agent::Constructors, destructor
//----------------------------------------------------------------------------
public:
   Agent( void ) = default;
   ~Agent( void ) = default;

//----------------------------------------------------------------------------
// Agent::Methods
//----------------------------------------------------------------------------
client_ptr
   get_client( void ) { return nullptr; } // NOT IMPLEMENTED

server_ptr
   get_server( void ) { return nullptr; } // NOT IMPLEMENTED

void
   shutdown( void ) { } // NOT IMPLEMENTED
}; // class Agent

//----------------------------------------------------------------------------
//
// Class-
//       ClientAgent
//
// Purpose-
//       Define the ClientAgent class.
//
//----------------------------------------------------------------------------
class ClientAgent : public Named, public Thread { // The ClientAgent class
//----------------------------------------------------------------------------
// ClientAgent::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef ClientConnectionPair        key_t;
typedef std::shared_ptr<Client>     client_ptr;
typedef Socket::sockaddr_u          sockaddr_u;

struct op_lt {                      // Less than compare operator
bool operator()(const key_t& lhs, const key_t& rhs) const
{  return lhs.operator<(rhs); }
}; // struct op_lt

typedef std::map<key_t, client_ptr, op_lt>
                       Map_t;       // The Client Map type
typedef Map_t::const_iterator
                       const_iterator; // The Client Map const iterator type
typedef Map_t::iterator
                       iterator;    // The Client Map const iterator type

//----------------------------------------------------------------------------
// ClientAgent::Attributes
//----------------------------------------------------------------------------
std::weak_ptr<ClientAgent> self;    // Self-reference

Select                 select;      // The Client Socket selector
int                    connect_error= 0; // Latest connect error
bool                   operational= true; // TRUE while operational

protected:
Map_t                  map;         // The Client map
mutable std::recursive_mutex
                       mutex;       // The Client map mutex

//----------------------------------------------------------------------------
// ClientAgent::Static attributes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ClientAgent::Destructor, constructors, creators
//----------------------------------------------------------------------------
public:
   ~ClientAgent( void );            // Destructor
   ClientAgent( void );             // Default constructor

//----------------------------------------------------------------------------
// ClientAgent::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }

//----------------------------------------------------------------------------
// ClientAgent::Accessor methods
//----------------------------------------------------------------------------
std::shared_ptr<ClientAgent>
   get_self( void ) const
{  return self.lock(); }

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::async
//
// Purpose-
//       Poll for work
//
//----------------------------------------------------------------------------
public:
void
   async( void );                   // Poll for work

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::connect
//
// Purpose-
//       Create Client connection
//
// Implementation note-
//       Options to be determined.
//
//----------------------------------------------------------------------------
std::shared_ptr<Client>             // The associated Client
   connect(                         // Get Client connection
     std::string       host,        // The host:port name
     const Options*    opts= nullptr); // The associated Options

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
   disconnect(                      // Disconnect
     Client*           client);     // This Client

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::reset
//
// Purpose-
//       Reset the ClientAgent, closing all Clients.
//
//----------------------------------------------------------------------------
void
   reset( void );                   // Reset the ClientAgent

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::run
//
// Purpose-
//       Run the ClientAgent socket selector (while operational)
//
//----------------------------------------------------------------------------
void
   run( void );                     // Run the ClientAgent socket selector

//----------------------------------------------------------------------------
// ClientAgent::Map control methods (mutex protected)
//----------------------------------------------------------------------------
protected:
void
   map_insert(                      // Associate
     const key_t&       key,        // This Server/Client internet address pair
     std::shared_ptr<Client>
                       client);     // With this Client

void
   map_insert(                      // Associate
     const sockaddr_u&  peer,       // This Server internet address and
     const sockaddr_u&  host,       // This Client internet address with
     std::shared_ptr<Client>
                       client)      // This Client
{  key_t key(peer, host); map_insert(key, client); }

std::shared_ptr<Client>             // The associated Client
   map_locate(                      // Locate Client with
     const key_t&       key) const; // This Server/Client internet address pair

void
   map_remove(                      // Remove
     const key_t&       key);       // This Client mapping
}; // class ClientAgent

//----------------------------------------------------------------------------
//
// Class-
//       ServerAgent
//
// Purpose-
//       Define the ServerAgent class.
//
//----------------------------------------------------------------------------
class ServerAgent {                 // ServerAgent class
//----------------------------------------------------------------------------
// ServerAgent::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef Socket::sockaddr_u sockaddr_u; // Using Socket::sockaddr_u

struct op_lt {                      // Less than compare operator
bool operator()(const sockaddr_u& lhs, const sockaddr_u& rhs) const
{  return memcmp(&lhs, &rhs, sizeof(sockaddr_u)) < 0; }
}; // struct op_lt

typedef std::map<sockaddr_u, std::shared_ptr<Listen>, op_lt>
                       Map_t;       // The Listen Map type
typedef Map_t::const_iterator
                       const_iterator; // The Listen Map const iterator type
typedef Map_t::iterator
                       iterator;    // The Listen Map iterator type

//----------------------------------------------------------------------------
// ServerAgent::Attributes
//----------------------------------------------------------------------------
Select                 select;      // The Server Socket selector
int                    connect_error= 0; // Latest connect error
bool                   operational= true; // TRUE while operational

protected:
std::weak_ptr<ServerAgent> self;    // Self-reference

Map_t                  map;         // The Server map
mutable std::recursive_mutex
                       mutex;       // The Server map mutex

//----------------------------------------------------------------------------
// ServerAgent::Static attributes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// ServerAgent::Destructor, constructors, creators
//----------------------------------------------------------------------------
public:
   ~ServerAgent( void );            // Destructor
   ServerAgent( void );             // Default constructor

//----------------------------------------------------------------------------
// ServerAgent::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }

//----------------------------------------------------------------------------
// ServerAgent::accessors
//----------------------------------------------------------------------------
std::shared_ptr<ServerAgent>
   get_self( void ) const
{  return self.lock(); }

//----------------------------------------------------------------------------
//
// Method-
//       ServerAgent::connect
//
// Purpose-
//       Create Listen connection
//
//----------------------------------------------------------------------------
public:
std::shared_ptr<Listen>             // The associated Listen
   connect(                         // Get Listen connection
     std::string       host,        // The host name (for interface)
     std::string       port,        // The port number or name (HTTP, ...)
     sa_family_t       family= AF_UNSPEC, // The address family
     const Options*    opts= nullptr); // The associated Options

//----------------------------------------------------------------------------
//
// Method-
//       ServerAgent::disconnect
//
// Purpose-
//       Remove Listener
//
//----------------------------------------------------------------------------
void
   disconnect(                      // Remove Listener
     Listen*           listen);     // For this Listener

//----------------------------------------------------------------------------
//
// Method-
//       ServerAgent::reset
//
// Purpose-
//       Reset the ServerAgent, closing all Listeners.
//
//----------------------------------------------------------------------------
void
   reset( void );                   // Reset the ServerAgent

//----------------------------------------------------------------------------
//
// Method-
//       ServerAgent::run
//
// Purpose-
//       Run the ServerAgent socket selector (while operational)
//
//----------------------------------------------------------------------------
void
   run( void );                     // Run the ServerAgent socket selector

//----------------------------------------------------------------------------
// ServerAgent::Map control methods (mutex protected)
//----------------------------------------------------------------------------
protected:
int                                 // Return code, 0 expected
   get_server(                      // Get Server connection specifier
     std::string       host,        // *INP* The host name (for interface)
     std::string       port,        // *INP* The port number or service name
     sockaddr_u&       sockaddr,    // *OUT* The sockaddr_u
     socklen_t&        socklen,     // *OUT* The sockaddr length
     sa_family_t       family= AF_UNSPEC); // The address family

std::shared_ptr<Listen>             // The associated Listen
   map_insert(                      // Associate
     const sockaddr_u& id,          // This connectionID with
     std::shared_ptr<Listen>
                       Listen);     // This Listen

std::shared_ptr<Listen>             // The associated Listen
   map_locate(                      // Locate Listen
     const sockaddr_u& id) const;   // For this connectionID

std::shared_ptr<Listen>             // The removed Listen
   map_remove(                      // Remove Listen
     const sockaddr_u& id);         // For this connectionID
}; // class ServerAgent
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_AGENT_H_INCLUDED
