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
//       HttpServer.cpp
//
// Purpose-
//       Implement HttpServer.h
//
// Last change date-
//       2024/10/08
//
// Implementation note-
//       Derived from ~/src/cpp/lib/dev/Server.cpp 2023/07/29
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Ioda.h>               // For pub::Ioda
#include <pub/Socket.h>             // For pub::Socket
#include <pub/utility.h>            // For pub::utility::to_string, ...

#include "HttpServer.h"             // For HttpServer, implemented

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;
using PUB::utility::to_string;
using PUB::utility::visify;
using std::string;

typedef pub::Ioda::Mesg   Mesg;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

,  INP_SIZE= 65536                  // Input buffer size
}; // enum

//----------------------------------------------------------------------------
// Constant data areas
//----------------------------------------------------------------------------
static const char*     page200=     // The 200 Dummy File message
         "<html><head><title>PAGE 200</title></head>\r\n"
         "<body><h1 align=\"center\">Default Response Page</h1>\r\n"
         "No Body's Home, Paige\r\n"
         "</body></html>\r\n"
         ;

//----------------------------------------------------------------------------
//
// Method-
//       Server::Server
//       Server::~Server
//
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   Server::Server(                  // Constructor
     pub::Socket*      _socket)     // The server Socket
:  Thread(), socket(_socket)
{  if( HCDM ) { debugf("Server started\n"); }

   // Allow immediate port re-use on close
   struct linger optval;
   optval.l_onoff= 1;
   optval.l_linger= 0;
   socket->set_option(SOL_SOCKET, SO_LINGER, &optval, sizeof(optval));

   // Set receive timeout
   struct timeval tv= {3,0};        // 3.0 second timout
   socket->set_option(SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

   // Self-start the Server Thread
   start();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Server::~Server( void )          // Destructor
{  if( HCDM ) { debugf("Server terminated\n"); }

   // Close and delete the socket
   close();                         // (Close deletes the Socket)
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::close
//
// Purpose-
//       Close the Socket
//
//----------------------------------------------------------------------------
void
   Server::close( void )            // Close the Socket
{  if( HCDM ) { debugf("Server::close\n"); }

   if( socket ) {
     socket->close();
     delete socket;
     socket= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::request
//
// Purpose-
//       Handle HTTP request
//
//----------------------------------------------------------------------------
void
   Server::request(                 // Handle HTTP request
     std::string       text)        // The request text
{  (void)text;                      // (Ignore the text)

    response(200, page200);
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::response
//
// Purpose-
//       Send HTTP response
//
//----------------------------------------------------------------------------
void
   Server::response(                // Send HTTP response
     int               status,      // The status code
     std::string       html)        // The response HTML
{
   std::string resp= to_string("HTTP/1.1 %d OK\r\n", status);
   resp += "Content-type: text/html\r\n";
   resp += to_string("Content-length: %zd\r\n", html.size());
   resp += "\r\n";
   resp += html;

   socket->write(resp.c_str(), resp.size());
}

//----------------------------------------------------------------------------
//
// Method-
//       Server::run
//
// Purpose-
//       Operate the Socket
//
//----------------------------------------------------------------------------
void
   Server::run( void )              // Operate the Socket
{  if( HCDM ) { debugf("Server::running\n"); }

   try {
     ssize_t L;
     for(;;) {
       Ioda ioda;
       Mesg mesg;
       ioda.set_rd_mesg(mesg, INP_SIZE);
       L= socket->recvmsg(&mesg, 0);
       if( L <= 0 )
         break;

       ioda.set_used(L);

       // Handle the request
       request((std::string)ioda);
     }
   } catch( std::exception& X ) {
     debugf("std::exception what(%s)\n", X.what());
   } catch(...) {
     debugf("catch(...)\n");
   }

   delete this;                     // (Self deletion)
}
