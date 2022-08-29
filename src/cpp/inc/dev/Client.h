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
//       http/Client.h
//
// Purpose-
//       HTTP Client object.
//
// Last change date-
//       2022/08/29
//
//----------------------------------------------------------------------------
#ifndef _PUB_HTTP_CLIENT_H_INCLUDED
#define _PUB_HTTP_CLIENT_H_INCLUDED

#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::mutex, super class
#include <string>                   // For std::string
#include <stdint.h>                 // For integer type

#include <pub/Dispatch.h>           // For pub::Dispatch objects
#include <pub/Semaphore.h>          // For pub::Semaphore
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread

#include "pub/http/Data.h"          // For pub::http::Buffer
#include "pub/http/Stream.h"        // For pub::http::Stream

namespace pub::http {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class ClientAgent;                  // pub::http::ClientAgent
class ClientItem;                   // pub::http::ClientItem (internal)
class ClientRequest;                // pub::http::ClientRequest
class ClientResponse;               // pub::http::ClientResponse
class ClientStream;                 // pub::http::ClientStream
class Options;                      // pub::http::Options
class Request;                      // pub::http::Request
class Response;                     // pub::http::Response

//----------------------------------------------------------------------------
//
// Class-
//       Client
//
// Purpose-
//       Define the (lockable) Client class.
//
//----------------------------------------------------------------------------
class Client : public std::mutex {  // Client class (lockable)
//----------------------------------------------------------------------------
// Client::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef Socket::sockaddr_u   sockaddr_u; // Using Socket::sockaddr_u
typedef dispatch::LambdaDone LambdaDone; // Using dispatch::LambdaDone
typedef dispatch::LambdaTask LambdaTask; // Using dispatch::LambdaTask

//----------------------------------------------------------------------------
// Client::Attributes
//----------------------------------------------------------------------------
protected:
// Callback handlers ---------------------------------------------------------
std::function<void(void)>
                       h_close;     // The close event handler
std::function<void(ClientItem*)>
                       h_writer;    // The Client's write (protocol) handler
//----------------------------------------------------------------------------

std::weak_ptr<Client>  self;        // Self-reference
std::weak_ptr<ClientAgent>
                       agent;       // Our owning Agent

SSL_CTX*               context= nullptr; // SSL context
mutable Buffer         ibuffer;     // Our input buffer
mutable Buffer         obuffer;     // Our output buffer
const char*            proto_id;    // The Client's protocol/version
pub::Semaphore         semaphore;   // Used to limit HTTP/1 write operations
Socket*                socket= nullptr; // Connection Socket
StreamSet              stream_set;  // Our set of Streams
LambdaTask             task;        // Output task
Thread*                thread= nullptr; // Client read Thread

bool                   operational= false; // TRUE while operational

//----------------------------------------------------------------------------
// Client::Destructor, constructors
//----------------------------------------------------------------------------
public:
   ~Client( void );                 // Destructor
   Client(                          // Constructor
     ClientAgent*      owner,       // Our agent
     const sockaddr_u& addr,        // Target internet address
     socklen_t         size,        // Target internet address length
     const Options*    opts= nullptr); // Client Options

static std::shared_ptr<Client>      // The Client
   make(                            // Get Client
     ClientAgent*      owner,       // Our agent
     const sockaddr_u& addr,        // Target internet address
     socklen_t         size,        // Target internet address length
     const Options*    opts= nullptr); // Client Options

//----------------------------------------------------------------------------
// Client::debug
//----------------------------------------------------------------------------
void debug(const char*) const;      // Debugging display
void debug( void ) const            // Debugging display
{  debug(""); }

//----------------------------------------------------------------------------
// Client::Accessor methods
//----------------------------------------------------------------------------
Buffer&                             // The input Buffer
   get_buffer( void ) const         // Get input Buffer
{  ibuffer.reset(); return ibuffer; }

int                                 // The socket handle (<0 if not connected)
   get_handle( void ) const         // Get socket handle
{  return socket->get_handle(); }

const sockaddr_u&                   // The connected internet address
   get_peer_addr( void ) const      // Get connected internet address
{  return socket->get_peer_addr(); }

const char*
   get_proto_id( void ) const       // Get protocol/version
{  return proto_id; }

std::shared_ptr<Client>
   get_self( void ) const           // Get self-reference
{  return self.lock(); }

std::shared_ptr<Stream>             // The associated Stream
   get_stream(uint32_t id) const    // Locate the Stream given Stream::ident
{  return stream_set.get_stream(id); }

void
   on_close(                        // Set close event handler
     std::function<void(void)> f)
{  h_close= f; }

//----------------------------------------------------------------------------
// Client::Methods
//----------------------------------------------------------------------------
void
   close( void );                   // Close the client

void
   connection_error(const char*);   // Handle connection error

std::shared_ptr<ClientRequest>      // The ClientRequest
   request(                         // Create a ClientRequest
     const Options*    opts= nullptr); // The associated Options

void
   run( void );                     // Operate the ClientThread

void
   writeStream(ClientStream*);      // Write ClientStream Request/Response

int                                 // Return code, 0 expected
   wait( void );                    // Wait for current Requests to complete

//----------------------------------------------------------------------------
// Client::Protected methods
//----------------------------------------------------------------------------
protected:
void
   enqueue(                         // Enqueue a Request
     ClientItem*       item);       // The associated ClientItem

std::function<void(ClientItem*)>
   protocol1a( void );              // Define the HTTP/1 protocol handler

std::function<void(ClientItem*)>
   protocol1b( void );              // Define the HTTP/1 protocol handler

std::function<void(ClientItem*)>
   protocol2( void );               // Define the HTTP/2 protocol handler

size_t                              // Read length
   read(int, size_t);               // Line number, maximum length

size_t                              // Read length
   read(size_t);                    // Maximum length

ssize_t                             // Written length
   write(int, const void*, size_t); // Write to Socket

ssize_t                             // Written length
   write(const void*, size_t);      // Write to Socket
}; // class Client
} // namespace pub::http
#endif // _PUB_HTTP_CLIENT_H_INCLUDED
