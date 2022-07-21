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
//       2022/02/11
//
//----------------------------------------------------------------------------
#ifndef _PUB_HTTP_AGENT_H_INCLUDED
#define _PUB_HTTP_AGENT_H_INCLUDED

#include <cstdlib>                  // For size_t
#include <cstring>                  // For memcmp
#include <functional>               // For std::function
#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::mutex, std::lock_guard
#include <string>                   // For std::string
#include <netinet/in.h>             // For in_port_t
#include <sys/socket.h>             // For socket

#include <pub/Socket.h>             // For pub::Socket::sockaddr_u

namespace pub::http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Client;                       // pub::http::Client object
class Listen;                       // pub::http::Listen object
class Options;                      // pub::http::Options object

//----------------------------------------------------------------------------
//
// Class-
//       ClientAgent
//
// Purpose-
//       Define the ClientAgent class.
//
//----------------------------------------------------------------------------
class ClientAgent {                 // ClientAgent class
//----------------------------------------------------------------------------
// ClientAgent::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef Socket::sockaddr_u sockaddr_u; // Using pub::Socket::sockaddr_u

struct op_lt {                      // Less than compare operator
bool operator()(const sockaddr_u& lhs, const sockaddr_u& rhs) const
{  return memcmp(&lhs, &rhs, sizeof(sockaddr_u)) < 0; }
}; // struct op_lt

typedef std::map<sockaddr_u, std::shared_ptr<Client>, op_lt>
                       Map_t;       // The Client Map type
typedef Map_t::const_iterator
                       const_iterator; // The Client Map const iterator type
typedef Map_t::iterator
                       iterator;    // The Client Map const iterator type

//----------------------------------------------------------------------------
// ClientAgent::Attributes
//----------------------------------------------------------------------------
std::weak_ptr<ClientAgent> self;    // Self-reference

int                    connect_error; // Latest connect error

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

static std::shared_ptr<ClientAgent>
   make( void );                    // Creator

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
// ClientAgent::Map control methods (mutex protected)
//----------------------------------------------------------------------------
protected:
int                                 // Return code, 0 expected
   get_client(                      // Get Client connection specifier
     const char*       host,        // *INP* The host name
     const char*       port,        // *INP* The port number or service name
     sockaddr_u&       sockaddr,    // *OUT* The sockaddr_u
     socklen_t&        socklen);    // *OUT* The sockaddr length

std::shared_ptr<Client>             // The associated Client
   map_insert(                      // Associate
     const sockaddr_u&  id,         // This connectionID with
     std::shared_ptr<Client>
                       client);     // This Client

std::shared_ptr<Client>             // The associated Client
   map_locate(                      // Locate Client
     const sockaddr_u& id) const;   // For this connectionID

std::shared_ptr<Client>             // The removed Client
   map_remove(                      // Remove Client
     const sockaddr_u&  id);        // For this connectionID

//----------------------------------------------------------------------------
//
// Method-
//       ClientAgent::connect
//
// Purpose-
//       Create Client connection
//
// Options-
//       TBD
//
//----------------------------------------------------------------------------
public:
std::shared_ptr<Client>             // The associated Client
   connect(                         // Get Client connection
     std::string       host,        // The host name
     std::string       port,        // The port number or name (HTTP, ...)
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
   disconnect(                      // Remove Client connection
     Client*           client);     // For this Client

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
typedef Socket::sockaddr_u sockaddr_u; // Using pub::Socket::sockaddr_u

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
int                    connect_error; // Latest connect error

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

static std::shared_ptr<ServerAgent>
   make( void );                    // Creator

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
}; // class ServerAgent
} // namespace pub::http
#endif // _PUB_HTTP_AGENT_H_INCLUDED
