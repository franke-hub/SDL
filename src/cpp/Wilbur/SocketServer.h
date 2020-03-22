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
//       SocketServer.h
//
// Purpose-
//       Serve a request.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef SOCKETSERVER_H_INCLUDED
#define SOCKETSERVER_H_INCLUDED

#ifndef INTERFACE_H_INCLUDED
#include "Interface.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Socket;

//----------------------------------------------------------------------------
//
// Class-
//       SocketServer
//
// Purpose-
//       Serve a request.
//
//----------------------------------------------------------------------------
class SocketServer : public Interface // Network request socket server
{
//----------------------------------------------------------------------------
// SocketServer::Attributes
//----------------------------------------------------------------------------
protected:
Socket&                socket;      // The connection Socket

//----------------------------------------------------------------------------
// SocketServer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~SocketServer( void );           // Destructor

protected:
   SocketServer(                    // Constructor
     Socket*           socket);     // The connection Socket

//----------------------------------------------------------------------------
// SocketServer::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // TRUE if no work available
   work( void ) = 0;                // Handle server request/response
}; // class SocketServer

#endif // SOCKETSERVER_H_INCLUDED
