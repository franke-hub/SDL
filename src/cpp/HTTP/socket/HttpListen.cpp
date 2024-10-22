//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpListen.cpp
//
// Purpose-
//       Http Listener
//
// Last change date-
//       2024/10/08
//
// Implementation notes-
//       Derived from ~/src/cpp/lib/dev/Listen.cpp 2023/12/13
//
//----------------------------------------------------------------------------
#include <memory>                   // For std::shared_ptr
#include <stdio.h>                  // For perror

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Socket.h>             // For pub::Socket

#include "HttpListen.h"             // For Listen, implemented
#include "HttpServer.h"             // For Server

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::Socket;
using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  DEFAULT_PORT= 8080               // Default port number
}; // enum

//----------------------------------------------------------------------------
//
// Method-
//       Listener::Listener
//       Listener::~Listener
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Listener::Listener( void )       // Constructor
{  }                                // (Placeholder)

   Listener::~Listener( void )      // Destructor
{  }                                // (Placeholder)

//----------------------------------------------------------------------------
//
// Method-
//       Listener::run
//
// Purpose-
//       Run the Listener Thread
//
//----------------------------------------------------------------------------
void
   Listener::run( void )            // Run the Listener Thread
{
   // Create and initialize the listener Socket
   Socket::sockaddr_u sockaddr; // Socket address information
   Socket listen;               // The listener Socket
   int rc= listen.open(AF_INET, SOCK_STREAM);
   if( rc ) {                       // If failure
     perror("open failed");
     return;
   }

   // (Needed before the bind)
   int optval= true;
   listen.set_option(SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

   rc= listen.bind(DEFAULT_PORT);   // Set listener port
   if( rc ) {                       // If failure
     perror("bind failed");
     return;
   }

   rc= listen.listen();             // Begin listening
   if( rc ) {                       // If failure
     perror("listen failed");
     return;
   }

   debugf("%s:%d listening\n", Socket::gethostname().c_str(), DEFAULT_PORT);
   operational= true;               // We are operational

   // Accept connections
   do {                             // While operational
     Socket* socket= listen.accept(); // Get next server
     if( socket == nullptr ) {      // If accept error
       perror("accept error (ignored)");
       continue;
     }

     // (The Server is a self-starting, self-deleting Thread)
     new Server(socket);            // Create new Server
  } while( operational );

   // Terminate the listener
   rc= listen.close();
   if( rc )                         // If failure
     perror("listen close failed");
}
