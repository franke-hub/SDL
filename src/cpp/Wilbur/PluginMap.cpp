//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       PluginMap.cpp
//
// Purpose-
//       PluginMap implementation methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include "PluginMap.h"

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
//
// Method-
//       PluginMap::~PluginMap
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   PluginMap::~PluginMap( void )    // Destructor
{
   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       PluginMap::PluginMap
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   PluginMap::PluginMap( void )     // Default constructor
:  pluginMap()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       PluginMap::begin/end
//
// Purpose-
//       Get pluginMap iterators
//
//----------------------------------------------------------------------------
PluginMap::Iterator                 // The PluginMap iterator
   PluginMap::begin( void ) const   // Get PluginMap iterator
{
   return pluginMap.begin();
}

PluginMap::Iterator                 // The PluginMap iterator
   PluginMap::end( void ) const     // Get PluginMap iterator
{
   return pluginMap.end();
}

//----------------------------------------------------------------------------
//
// Method-
//       PluginMap::delPlugin
//
// Purpose-
//       Delete Plugin.
//
//----------------------------------------------------------------------------
Plugin*                             // The associated Plugin
   PluginMap::delPlugin(            // Delete Plugin
     const std::string&name)        // With this Plugin name
{
   Plugin*             result= NULL;// Resultant
   Map::iterator       mi;          // Map iterator

   mi= pluginMap.find(name);
   if( mi != pluginMap.end() )
   {
     result= mi->second;
     pluginMap.erase(mi);
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PluginMap::getPlugin
//
// Purpose-
//       Get Plugin
//
//----------------------------------------------------------------------------
Plugin*                             // The Plugin
   PluginMap::getPlugin(            // Get Plugin
     const std::string&name) const  // For this name
{
   Iterator            mi;          // Map iterator

   mi= pluginMap.find(name);
   if( mi == pluginMap.end() )
     return NULL;

   return mi->second;
}

//----------------------------------------------------------------------------
//
// Method-
//       PluginMap::setPlugin
//
// Purpose-
//       Set Plugin
//
//----------------------------------------------------------------------------
void
   PluginMap::setPlugin(            // Set Plugin value
     const std::string&name,        // Associate this Plugin name
     Plugin*           value)       // With this Plugin value
{
   pluginMap[name]= value;
}

//----------------------------------------------------------------------------
//
// Method-
//       PluginMap::reset
//
// Purpose-
//       Reset PluginMap, deleting all map elements
//
//----------------------------------------------------------------------------
void
   PluginMap::reset( void )         // Reset the PluginMap
{
   Iterator mi;
   for(mi= pluginMap.begin(); mi != pluginMap.end(); mi++)
     delete mi->second;

   pluginMap.clear();
}

