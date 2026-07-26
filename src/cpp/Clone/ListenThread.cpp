//----------------------------------------------------------------------------
//
//       Copyright (c) 2014-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       ListenThread.cpp
//
// Purpose-
//       Implement ListenThread object methods
//
// Last change date-
//       2026/07/23
//
//----------------------------------------------------------------------------
#include "IoCommon.h"               // For I/O common objects and subroutines
#include "ListenThread.h"           // For ListenThread, implemented
#include "ServerThread.h"           // For ServerThread

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // Generic enum

//----------------------------------------------------------------------------
//
// Method-
//       ListenThread::ListenThread
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   ListenThread::ListenThread(      // Constructor
     int               port)        // Connection port
:  Thread(), port(port)
{
   if( HCDM )
     debugf("ListenThread(%p)::ListenThread(%p)\n", this, socket);

   init_path= getcwd(nullptr, 0);   // Get current directory
   if( init_path == nullptr )
     throwf("Listen:%d getcwd", __LINE__);
}

//----------------------------------------------------------------------------
//
// Method-
//       ListenThread::~ListenThread
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   ListenThread::~ListenThread( void ) // Destructor
{
   if( HCDM )
     debugf("ListenThread(%p)::~ListenThread()\n", this);

   delete socket;
   socket= nullptr;

   if( init_path ) {
     free(init_path);
     init_path= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ListenThread::run
//
// Purpose-
//       Operate the ListenThread.
//
//----------------------------------------------------------------------------
void
   ListenThread::run( void )        // Operate this ListenThread
{
   if( HCDM ) debugf("ListenThread(%p)::run()...\n", this);

   // Initialize the Listener socket
   socket= new Socket();            // The listener Socket
   if( socket == nullptr )
     throwf("%4d ListenThread: unable to create listener", __LINE__);

   int rc= socket->open(AF_INET, SOCK_STREAM, PF_UNSPEC);
   if( env_iodm )
     msglog("%4d= open(%d,%d,%d)\n", rc, AF_INET, SOCK_STREAM, PF_UNSPEC);
   if( rc )
     throwf("%4d ListenThread: %d= open(%d,%d,%d)\n", __LINE__, rc
           , AF_INET, SOCK_STREAM, PF_UNSPEC);

   int optval= true;                // (Needed before the bind)
   rc= socket->set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
   if( env_iodm )
     msglog("%4d= set_option(%d,%d,...)\n", rc
           , SOL_SOCKET, SO_REUSEADDR);
   if( rc )
     throwf("%4d ListenThread: %d= set_option(%d,%d,...)\n", __LINE__
           , rc, SOL_SOCKET, SO_REUSEADDR);

   rc= socket->bind(port);          // Set port number
   if( env_iodm )
     msglog("%4d= bind(%d)\n", rc, port);
   if( rc )
     throwf("%4d ListenThread: %d= bind(%d)\n", __LINE__, rc, port);

   rc= socket->listen();            // Begin listening
   if( env_iodm )
     msglog("%4d= listen()\n", rc);
   if( rc )
     throwf("%4d ListenThread: %d= listen()\n", __LINE__, rc);

   msgout("Server: Host(%s:%d) %s %s\n"
         , s2c(socket->gethostname()), port
         , s2c(socket->get_host_addr().to_string()), init_path);

   // Operate the thread
   for(;;) {                        // Wait for connections
     Socket* server= socket->accept();
     if( server == nullptr ) {
       msgioerr("Listen: accept() error");
       break;
     }

     new ServerThread(server, init_path); // Create the Server Thread
   }

   if( HCDM ) debugf("...ListenThread(%p)::run()\n", this);
}
