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
//       HttpServerPlugin.cpp
//
// Purpose-
//       HttpServerPlugin implementation methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <com/Debug.h>

#include "Common.h"

#include "HttpServer.h"
#include "HttpServerPlugin.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef BRINGUP
#undef  BRINGUP                     // If defined, BRINGUP Mode
#endif

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerPlugin::~HttpServerPlugin
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HttpServerPlugin::~HttpServerPlugin( void )
{
   IFHCDM( traceh("HttpServerPlugin(%p)::~HttpServerPlugin()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerPlugin::HttpServerPlugin
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HttpServerPlugin::HttpServerPlugin( // Constructor
     const char*       name)        // The name of the library
:  Plugin(name)
{
   IFHCDM( traceh("HttpServerPlugin(%p)::HttpServerPlugin(%s)\n", this, name); )

   if( dynamic_cast<HttpServer*>(interface) == NULL )
     throwf("Library(%s) not HttpServer object", name);
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerPlugin::serve
//
// Purpose-
//       Handle HTTP request/response
//
//----------------------------------------------------------------------------
void
   HttpServerPlugin::serve(         // Handle HTTP request/response
     HttpRequest&      Q,           // The HTTP request
     HttpResponse&     S)           // The HTTP response
{
   HttpServer* server= (HttpServer*)interface;
   server->serve(Q, S);
}

