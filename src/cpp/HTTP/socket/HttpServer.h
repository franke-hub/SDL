//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       http/Server.h
//
// Purpose-
//       HTTP Server object.
//
// Last change date-
//       2024/10/08
//
//----------------------------------------------------------------------------
#ifndef _SERVER_H_INCLUDED
#define _SERVER_H_INCLUDED

#include <string>                   // For std::string

#include <pub/Socket.h>             // For pub::Socket
#include <pub/Thread.h>             // For pub::Thread

//----------------------------------------------------------------------------
//
// Class-
//       Server
//
// Purpose-
//       Define the Server class.
//
// Implementation note-
//       Server is a self-deleting class.
//
//----------------------------------------------------------------------------
class Server : public pub::Thread {  // Server class
//----------------------------------------------------------------------------
// Server::Attributes
//----------------------------------------------------------------------------
protected:
pub::Socket*           socket= nullptr; // The connection Socket

//----------------------------------------------------------------------------
// Server::Constructor/destructor
//----------------------------------------------------------------------------
public:
   Server(pub::Socket*);            // Constructor

virtual
   ~Server( void );                 // Destructor

//----------------------------------------------------------------------------
// Server::Methods
//----------------------------------------------------------------------------
void
   close( void );                   // Close and delete the Socket

virtual void
   request(                         // Handle request
     std::string       text);       // The request text

virtual void
   response(                        // Send a response
     int               status,      // The status code
     std::string       html);       // The response HTML

virtual void
   run( void );                     // Operate the Server
}; // class Server
#endif // _SERVER_H_INCLUDED
