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
//       HttpServerPluginMap.cpp
//
// Purpose-
//       HttpServerPluginMap object methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <string>

#include <com/Debug.h>
#include <com/Reader.h>
#include <com/XmlNode.h>
#include <com/XmlParser.h>

#include "HttpServerPlugin.h"
#include "HttpServerPluginMap.h"
using namespace std;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef BRINGUP
#undef  BRINGUP                     // If defined, BRINGUP diagnostics
#endif

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef logf
#define logf traceh                 // Alias for trace w/header
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       invalidFile
//
// Purpose-
//       ABORT, invalid file.
//
//----------------------------------------------------------------------------
void
   invalidFile(                     // Handle invalid file
     const char*       name,        // With this name
     const string&     text= "")    // Addendum
{
   throwf("HttpServerPluginMap: file(%s) invalid, %s", name, text.c_str());
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerPluginMap::~HttpServerPluginMap
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HttpServerPluginMap::~HttpServerPluginMap( void ) // Destructor
{
   IFHCDM( logf("HttpServerPluginMap(%p)::~HttpServerPluginMap()\n", this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpServerPluginMap::HttpServerPluginMap
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HttpServerPluginMap::HttpServerPluginMap( // Constructor
     const char*       xml)         // The HttpServer.xml control file name
:  PluginMap()
{
   IFHCDM( logf("HttpServerPluginMap(%p)::HttpServerPluginMap(%s)\n", this, xml); )

   FileReader reader(xml);
   XmlParser parser;
   XmlNode* root= parser.parse(reader);

   if( root == NULL )
     invalidFile(xml, "no root node");
   if( root->getName() != "web-app" )
     invalidFile(xml, "root not web-app");

   XmlNode* servlet= root->getChild("servlet");
   while( servlet != NULL )
   {
     XmlNode* node= servlet->getAttrib("name");
     if( node == NULL )
       invalidFile(xml, "servlet missing name attribute");
     string name= node->getValue();

     XmlNode* mapping= servlet->getChild("mapping");
     if( mapping == NULL )
       invalidFile(xml, (name + ": missing mapping"));

     node= mapping->getAttrib("url");
     if( node == NULL )
       invalidFile(xml, (name + ":mapping: missing url"));
     string url= parser.getValue(node);

     node= mapping->getAttrib("lib");
     if( node == NULL )
       invalidFile(xml, (name + ":mapping: missing lib"));
     string lib= parser.getValue(node);

     HttpServerPlugin* plugin= new HttpServerPlugin(lib.c_str());
     setPlugin(url, plugin);

     servlet= servlet->getNext();
     while( servlet != NULL && servlet->getName() != "servlet" )
       servlet= servlet->getNext();
   }

   #ifdef BRINGUP
     logf("%4d HCDM HttpServerPlugin library\n", __LINE__);
     Iterator it;
     for(it= begin(); it != end(); it++)
     {
       logf("%p = '%s'\n", it->second, it->first.c_str());
     }
   #endif
}

