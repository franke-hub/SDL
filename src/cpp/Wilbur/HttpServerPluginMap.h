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
//       HttpServerPluginMap.h
//
// Purpose-
//       Define the HttpServerPluginMap object.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef HTTPSERVERPLUGINMAP_H_INCLUDED
#define HTTPSERVERPLUGINMAP_H_INCLUDED

#ifndef PLUGINMAP_H_INCLUDED
#include "PluginMap.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       HttpServerPluginMap
//
// Purpose-
//       Map of HttpServerPlugin objects
//
// Implementation notes-
//       The constructor parses the HttpServer.xml file, building the
//       associated map.
//
//----------------------------------------------------------------------------
class HttpServerPluginMap : public PluginMap { // HttpServerPlugin map
//----------------------------------------------------------------------------
// HttpServerPluginMap::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HttpServerPluginMap( void );    // Destructor
   HttpServerPluginMap(             // Constructor
     const char*          name);    // The HttpServer.xml control file name

private:                            // Bitwise copy is prohibited
   HttpServerPluginMap(const HttpServerPluginMap&);  // Disallowed copy constructor
HttpServerPluginMap&
   operator=(const HttpServerPluginMap&); // Disallowed assignment operator
}; // class HttpServerPluginMap

#endif // HTTPSERVERPLUGINMAP_H_INCLUDED
