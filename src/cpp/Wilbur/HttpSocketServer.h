//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpSocketServer.h
//
// Purpose-
//       Serve an HTTP request.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef HTTPSOCKETSERVER_H_INCLUDED
#define HTTPSOCKETSERVER_H_INCLUDED

#include "SocketServer.h"

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class HttpRequest;
class HttpResponse;

//----------------------------------------------------------------------------
//
// Class-
//       HttpSocketServer
//
// Purpose-
//       Serve an HTTP request.
//
//----------------------------------------------------------------------------
class HttpSocketServer : public SocketServer { // Http socket server
//----------------------------------------------------------------------------
// HttpSocketServer::Attributes
//----------------------------------------------------------------------------
public:
int                    keepAlive;   // The current KeepAlive property

//----------------------------------------------------------------------------
// HttpSocketServer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HttpSocketServer( void );       // Destructor
   HttpSocketServer(                // Constructor
     Socket*           socket);     // The connection Socket

//----------------------------------------------------------------------------
// HttpSocketServer::Methods
//----------------------------------------------------------------------------
public:
virtual void
   serve(                           // Handle HTTP request/response
     HttpRequest&      request,     // The HTTP request
     HttpResponse&     response);   // The HTTP response

virtual int                         // TRUE to exit
   work( void );                    // Handle HTTP request/response
}; // class HttpSocketServer

#endif // HTTPSOCKETSERVER_H_INCLUDED
