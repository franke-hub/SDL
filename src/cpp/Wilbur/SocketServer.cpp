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
//       SocketServer.cpp
//
// Purpose-
//       SocketServer implementation methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <com/Debug.h>

#include "SocketServer.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef logf
#define logf traceh                 // Alias for trace w/header
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Method-
//       SocketServer::~SocketServer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   SocketServer::~SocketServer( void )
{
   IFHCDM( logf("SocketServer(%p)::~SocketServer()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       SocketServer::SocketServer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   SocketServer::SocketServer(      // Constructor
     Socket*           socket)      // Associated Socket
:  socket(*socket)
{
   IFHCDM( logf("Server(%p)::Server(%p)\n", this, socket); )
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::work
//
// Purpose-
//       Handle request/response
//
//----------------------------------------------------------------------------
int                                 // TRUE iff no work available
   SocketServer::work( void )       // Handle request/response
{
   logf("SocketServer(%p)::work() ShouldNotOccur\n", this);
   return TRUE;
}

