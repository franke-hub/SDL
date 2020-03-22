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
//       HttpServerPlugin.h
//
// Purpose-
//       Define the HttpServerPlugin object.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef HTTPSERVERPLUGIN_H_INCLUDED
#define HTTPSERVERPLUGIN_H_INCLUDED

#ifndef PLUGIN_H_INCLUDED
#include "Plugin.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class HttpRequest;
class HttpResponse;

//----------------------------------------------------------------------------
//
// Class-
//       HttpServerPlugin
//
// Purpose-
//       HttpServer Plugin.
//
//----------------------------------------------------------------------------
class HttpServerPlugin : public Plugin { // HttpServer Plugin
//----------------------------------------------------------------------------
// HttpServerPlugin::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HttpServerPlugin( void );       // Destructor
   HttpServerPlugin(                // Constructor
     const char*       name);       // The name of the library

private:                            // Bitwise copy is prohibited
   HttpServerPlugin(const HttpServerPlugin&); // Disallowed copy constructor
HttpServerPlugin&
   operator=(const HttpServerPlugin&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// HttpServerPlugin::Methods
//----------------------------------------------------------------------------
public:
virtual void
   serve(                           // Handle HTTP request/response
     HttpRequest&      request,     // The HTTP request
     HttpResponse&     response);   // The HTTP response
}; // class HttpServerPlugin

#endif // HTTPSERVERPLUGIN_H_INCLUDED
