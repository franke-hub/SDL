//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       UrlConnection.cpp
//
// Purpose-
//       UrlConnection implementation methods.
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "Common.h"
#include "TextBuffer.h"

#include "UrlConnection.h"

using std::string;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, I/O Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Method-
//       UrlConnection::~UrlConnection
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   UrlConnection::~UrlConnection( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       UrlConnection::UrlConnection
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   UrlConnection::UrlConnection( void ) // Default constructor
:  socket(Socket::ST_STREAM)
,  url()
{
}

   UrlConnection::UrlConnection(    // Constructor
     const Url&        url)         // For this URL
:  socket(Socket::ST_STREAM)
,  url(url)
{
}

   UrlConnection::UrlConnection(    // Constructor
     const std::string&url)         // For this URL
:  socket(Socket::ST_STREAM)
,  url(url)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       UrlConnection::connect
//
// Purpose-
//       Connect to server.
//
//----------------------------------------------------------------------------
int                                   // Return code, 0 OK
   UrlConnection::connect( void )     // Connect to server
{
   Socket::Addr        addr;        // Socket address
   int                 port;        // The port number

   int                 rc;

   // Close any existing connection
   socket.close();
   socket.setSocketEC(0);

   // Validate URL
   if( url.getURI() == "" )
     return CC_INVALID_URL;

   // Get network address
   addr= Socket::nameToAddr(url.getHost().c_str());
   if( addr == 0 )
   {
     IFHCDM( printf("%4d host(%s) unknown\n", __LINE__, url.getHost().c_str()); )
     return CC_UNKNOWN_HOST;
   }

   // Get network port
   port= url.getPort();
   if( port < 0 )
     port= url.getDefaultPort();

   // Connect to server
   rc= socket.connect(addr, port);
   IFIODM( printf("%d= socket.connect(%s,%d)\n",
                  rc, url.getHost().c_str(), port); )
   if( rc != 0 )
   {
     IFHCDM( printf("%4d socket.connect() Error(%s)\n", __LINE__,
                    socket.getSocketEI()); )
     return CC_CANT_CONNECT;
   }

   return CC_OK;
}

//----------------------------------------------------------------------------
//
// Method-
//       UrlConnection::disconnect
//
// Purpose-
//       Reset this UrlConnection.
//
//----------------------------------------------------------------------------
void
   UrlConnection::disconnect( void )// Reset this UrlConnection
{
   socket.close();
}

