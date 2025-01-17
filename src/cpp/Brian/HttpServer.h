//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2025 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpServer.h
//
// Purpose-
//       The HTTP Server object.
//
// Last change date-
//       2025/01/16
//
//----------------------------------------------------------------------------
#ifndef HTTPSERVER_H_INCLUDED
#define HTTPSERVER_H_INCLUDED

#include <functional>               // For std::function
#include <memory>                   // For std::shared_ptr, std::weak_ptr, ...
#include <mutex>                    // For std::recursive_mutex
#include <string>                   // For std::string

#include <pub/Dispatch.h>           // For pub::dispatch::Item, ...
#include <pub/Ioda.h>               // For pub::Ioda
#include <pub/Signals.h>            // For pub::Signals
#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread, base class

#include "Common.h"                 // For StaticCommon
#include "HttpListen.h"             // For HttpListen

//----------------------------------------------------------------------------
//
// Class-
//       HttpServer
//
// Purpose-
//       Define the HttpServer class.
//
//----------------------------------------------------------------------------
class HttpServer {                  // HttpServer class
//----------------------------------------------------------------------------
// HttpServer::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef std::shared_ptr<HttpListen> Listen_t;
typedef std::shared_ptr<HttpServer> Server_t;
typedef   std::weak_ptr<HttpServer> Server_wt;

typedef pub::Ioda                   Ioda_t;
typedef pub::Socket                 Socket_t;
typedef pub::dispatch::LambdaTask   Task_t;
typedef pub::dispatch::Work_i       Work_i;

typedef StaticCommon::Event                   Event;
typedef pub::signals::Connector<Event>        Connector;

//----------------------------------------------------------------------------
// HttpServer::Item | Work Item
//----------------------------------------------------------------------------
class Item : public pub::dispatch::Item {
public:
Server_t               server;      // The associated Server
Ioda_t                 ioda;        // The I/O data area
}; // class HttpServer::Item

//----------------------------------------------------------------------------
// HttpServer::Attributes
//----------------------------------------------------------------------------
protected:
Server_wt              self;        // (Weak) self-reference

Connector              connector;   // Our run_diagnostics Connector
std::recursive_mutex   mutex;       // Mutex, protects close sequence
Listen_t               owner;       // Our HttpListen owner
Socket_t*              socket;      // The connection Socket

Task_t                 dh_task;     // The data handler task
Task_t                 rd_task;     // The Reader Task
Task_t                 wr_task;     // The Writer Task

bool                   operational= false; // TRUE while operational

//----------------------------------------------------------------------------
// HttpServer::Constructor, destructor, creator
//----------------------------------------------------------------------------
protected:
   HttpServer(                      // Constructor
     Listen_t          owner,       // Creator
     Socket_t*         socket);     // Server Socket

public:
virtual
   ~HttpServer( void );             // Destructor

static Server_t                     // The HttpServer
    make(Listen_t, Socket_t*);      // Make HttpServer

//----------------------------------------------------------------------------
// HttpServer::debug || Display debugging information
//----------------------------------------------------------------------------
void
   debug(const char* info= "") const; // Display debugging information

//----------------------------------------------------------------------------
// HttpServer::get_peer || Get peer address:port string
//----------------------------------------------------------------------------
std::string                         // The peer address:port string
   get_peer( void ) const;          // Get peer address:port string

//----------------------------------------------------------------------------
// HttpServer::get_self || Get std::shared_ptr<HttpServer>
//----------------------------------------------------------------------------
Server_t                            // The shared_ptr<Server>
   get_self( void ) const           // Get shared_ptr<Server>
{  return self.lock(); }

//----------------------------------------------------------------------------
// HttpServer::get_socket || Get pub::Socket*
//----------------------------------------------------------------------------
Socket_t*                           // The Socket*
   get_socket( void )               // Get Socket*
{  return socket; }

//----------------------------------------------------------------------------
// HttpServer::is_operational || Expose operational state
//----------------------------------------------------------------------------
bool                                // The operational state
   is_operational( void ) const     // Get operational state
{  return operational; }

//----------------------------------------------------------------------------
// HttpServer::on_work || Replace server data handler
//----------------------------------------------------------------------------
void
   on_work(                         // Replace data handler work method
     Work_i            f)
{  dh_task.on_work(f); }

//----------------------------------------------------------------------------
// HttpServer::Task enqueue methods
//----------------------------------------------------------------------------
void
   handle(                          // Add work item to dh_task
     HttpServer::Item* item)
{  dh_task.enqueue(item); }

void
   reader(                          // Add work item to rd_task
     HttpServer::Item* item)
{  rd_task.enqueue(item); }

void
   writer(                          // Add work item to wr_task
     HttpServer::Item* item)
{  wr_task.enqueue(item); }

//----------------------------------------------------------------------------
// HttpServer::Methods
//----------------------------------------------------------------------------
void
   close( void );                   // Close and delete the Socket

virtual void
   stop( void );                    // Stop the HttpServer
}; // class HttpServer
#endif // HTTPSERVER_H_INCLUDED
