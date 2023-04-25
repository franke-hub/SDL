//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       http/Listen.h
//
// Purpose-
//       HTTP Listen object.
//
// Last change date-
//       2023/04/16
//
// Implementation notes-
//       The Listen object is the Server analog to a Client Agent.
//       It has a listener Socket, and creates a new Server for each new
//       Client connection.
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_HTTP_LISTEN_H_INCLUDED
#define _LIBPUB_HTTP_LISTEN_H_INCLUDED

#include <cstdlib>                  // For size_t
#include <cstring>                  // For memcmp
#include <functional>               // For std::function
#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string

#include <pub/config.h>             // For _ATTRIBUTE_PRINTF macro
#include <pub/Debug.h>              // For pub::Debug
#include <pub/Socket.h>             // For pub::Socket

#include "pub/http/Options.h"       // For pub::http::Options
#include "pub/http/Request.h"       // For pub::http::ServerRequest

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
namespace http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Request;
class Server;
class ListenAgent;

//----------------------------------------------------------------------------
//
// Class-
//       Listen
//
// Purpose-
//       Define the Listen class. (The ServerAgent)
//
//----------------------------------------------------------------------------
class Listen {                      // Listen class
//----------------------------------------------------------------------------
// Listen::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef Socket::sockaddr_u                    sockaddr_u;
typedef std::function<void(void)>             f_close; // Callback handlers
typedef std::function<void(ServerRequest&)>   f_request;

struct op_lt {                      // Compare operator
bool operator()(const sockaddr_u& lhs, const sockaddr_u& rhs) const
{  return memcmp(&lhs, &rhs, sizeof(sockaddr_u)) < 0; }
}; // struct op_lt

typedef std::map<sockaddr_u, std::shared_ptr<Server>, op_lt>
                       Map_t;       // The Server Map type
typedef Map_t::const_iterator
                       const_iterator; // The Server Map const iterator type
typedef Map_t::iterator
                       iterator;    // The Server Map iterator type

enum FSM                            // Finite State Machine states
{  FSM_RESET= 0                     // Reset - closed
,  FSM_READY= 1                     // Operational
,  FSM_CLOSE= 2                     // Close in progress
}; // enum FSM

//----------------------------------------------------------------------------
// Listen::Attributes
//----------------------------------------------------------------------------
protected:
std::weak_ptr<Listen>  self;        // Self-reference
ListenAgent*           agent;       // Our owning Agent

Socket                 listen;      // The listen Socket
Map_t                  map;         // The Server map
std::mutex             mutex;       // The Server map mutex

Debug                  log;         // The logger object
Options                opts;        // The Listen Options

int                    fsm= FSM_RESET; // Finite State Machine state

// Callback handlers
f_close                h_close;     // The close event handler
f_request              h_request;   // The request event handler

//----------------------------------------------------------------------------
// Listen::Protected methods
//----------------------------------------------------------------------------
_LIBPUB_PRINTF(3,4)
void
   trace(                           // Trace socket operation
     int               line,        // For this source code line
     const char*       fmt,         // Format string
                       ...) const;  // The PRINTF argument list

//----------------------------------------------------------------------------
// Listen::Constructor, destructor, creator, init_request (helper)
//----------------------------------------------------------------------------
public:
   Listen(                          // Constructor
     ListenAgent*      agent,       // The creating ListenAgent
     const sockaddr_u& addr,        // Target internet address
     socklen_t         size,        // sizeof(addr)
     const Options*    opts= nullptr); // Listen options
   ~Listen( void );                 // Destructor

static std::shared_ptr<Listen>      // The Listener
   make(                            // Create Listener
     ListenAgent*      agent,       // The creating ListenAgent
     const sockaddr_u& addr,        // Target internet address
     socklen_t         size,        // sizeof(addr)
     const Options*    opts= nullptr); // Listen options

//----------------------------------------------------------------------------
// Listen::debug
//----------------------------------------------------------------------------
void
   debug(                            // Debugging display
     const char*       info="") const; // Caller information

//----------------------------------------------------------------------------
// Listen::Accessor methods
//----------------------------------------------------------------------------
void
   do_request(                      // Drive the Request event handler
     ServerRequest*    request)     // For this ServerRequest
{  h_request(*request); }

ListenAgent*
   get_agent( void ) const          // Get ListenAgent
{  return agent; }

int                                 // The socket handle (<0 if not connected)
   get_handle( void ) const         // Get socket handle
{  return listen.get_handle(); }

std::shared_ptr<Listen>
   get_self( void ) const           // Get self-reference
{  return self.lock(); }

const sockaddr_u&                   // The connectionID
   get_host_addr( void ) const      // Get connectionID
{  return listen.get_host_addr(); }

const char*                         // The Option value
   get_option(                      // Get Option value
     const char*       name) const  // For this Option name
{  return opts.locate(name); }

void
   on_close(const f_close& f)       // Set close event handler
{  h_close= f; }

void
   on_request(const f_request& f)   // Set Request event handler
{  h_request= f; }

void
   opt_append(const Options&);      // Append listen options

void
   opt_reset(const Options&);       // Reset listen options

//----------------------------------------------------------------------------
// Listen::Methods
//----------------------------------------------------------------------------
void
   async(int);                      // Handle asynchronous polling event

void
   close( void );                   // Close the Listen

void
   disconnect(Server*);             // Disconnect Server

_LIBPUB_PRINTF(2, 3)
void
   logf(                            // Write to log file
     const char*       fmt,         // The PRINTF format string
                       ...);        // The PRINTF argument list

void
   reset( void );                   // Reset the Listen, closing all Servers

//----------------------------------------------------------------------------
// Listen::Map control methods (mutex protected)
//----------------------------------------------------------------------------
protected:
void
   map_insert(                      // Associate
     const sockaddr_u& id,          // This connectionID with
     std::shared_ptr<Server>
                       server);     // This Server

std::shared_ptr<Server>             // The associated Server
   map_locate(                      // Locate Server
     const sockaddr_u& id);         // For this connectionID

void
   map_remove(                      // Remove Server
     const sockaddr_u& id);         // For this connectionID
}; // class Listen
}  // namespace http
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_HTTP_LISTEN_H_INCLUDED
