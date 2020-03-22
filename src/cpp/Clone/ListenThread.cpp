//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       ListenThread.cpp
//
// Purpose-
//       Implement ListenThread object methods
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <stdlib.h>
#include <unistd.h>

#include <com/Atomic.h>
#include <com/Barrier.h>
#include <com/Debug.h>
#include <com/define.h>             // For NULL
#include <com/Socket.h>

#include "ListenThread.h"
#include "ServerThread.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       createServer
//
// Purpose-
//       Create a ServerThread
//
//----------------------------------------------------------------------------
static void
   createServer(                       // Create a ServerThread
     Socket*           socket,         // Using this Socket
     const char*       path)           // And this initial path
{
   try {
     new ServerThread(socket, path);   // Create the Server Thread
   } catch( const char* X ) {
     fprintf(stderr, "%4d ListenThread create server exception(%s)\n",
             __LINE__, X);
   } catch(...) {
     fprintf(stderr, "%4d ListenThread create server exception(%s)\n",
             __LINE__, "...");
   }
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
   IFSCDM( debugf("%4d ListenThread(%p)::~ListenThread()\n", __LINE__, this); )

   if( path != NULL )
   {
     free(path);
     path= NULL;
   }
}

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
:  CommonThread(NULL)
,  port(port)
{
   IFSCDM( debugf("%4d ListenThread(%p)::ListenThread(%p)\n", __LINE__, this,
                  socket); )

   path= getcwd(NULL, 0);           // Get current directory
   if( path == NULL )
     throwf("Listen:%d getcwd", __LINE__);

   if( buffer != NULL )             // We don't need the transfer buffer
   {
     mx_buffer->release(buffer);
     buffer= NULL;
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
long                                // Return code (always 0)
   ListenThread::run( void )        // Operate this ListenThread
{
   IFSCDM( debugf("%4d ListenThread(%p)::run()...\n", __LINE__, this); )

   // Initialize the Listener socket
   socket= new Socket(Socket::ST_STREAM); // The listener Socket
   if( socket == NULL )
     throwf("ListenThread:%d unable to create socket", __LINE__);

   msgout("Server: Host(%s:%d) Path(%s) %s\n",
          socket->getHostName(), port, path,
          Socket::addrToChar(Socket::getAddr()));

   // Operate the thread
   fsm= FSM_READY;                  // Indicate operational
   for(;;)                          // Wait for connections
   {
     Socket* server= socket->listen(port);
     if( server == NULL )
     {
       msgerr("%4d Listen: error(%s)", __LINE__, socket->getSocketEI());
       break;
     }

     createServer(server, path);
   }

   // Thread termination
   term();                          // Indicate terminated

   IFSCDM( debugf("%4d ...ListenThread(%p)::run()\n", __LINE__, this); )
   return 0;
}

