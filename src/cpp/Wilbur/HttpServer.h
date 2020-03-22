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
//       HttpServer.h
//
// Purpose-
//       Serve an HTTP request.
//
// Last change date-
//       2012/01/01
//
// Implementation notes-
//       The base method only handle files.
//
//----------------------------------------------------------------------------
#ifndef HTTPSERVER_H_INCLUDED
#define HTTPSERVER_H_INCLUDED

#include "Interface.h"

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class HttpRequest;
class HttpResponse;

//----------------------------------------------------------------------------
//
// Class-
//       HttpServer
//
// Purpose-
//       Serve an HTTP request.
//
//----------------------------------------------------------------------------
class HttpServer : public Interface {  // Http request processor
//----------------------------------------------------------------------------
// HttpServer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HttpServer( void );             // Destructor
   HttpServer( void );              // Constructor

//----------------------------------------------------------------------------
// HttpServer::Methods
//----------------------------------------------------------------------------
public:
virtual void
   serve(                           // Handle HTTP request/response
     HttpRequest&      request,     // The HTTP request
     HttpResponse&     response);   // The HTTP response
}; // class HttpServer

#endif // HTTPSERVER_H_INCLUDED
