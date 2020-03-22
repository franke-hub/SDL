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
//       PluginMap.h
//
// Purpose-
//       Define the PluginMap object.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#ifndef PLUGINMAP_H_INCLUDED
#define PLUGINMAP_H_INCLUDED

#include <map>
#include <string>

#ifndef PLUGIN_H_INCLUDED
#include "Plugin.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       PluginMap
//
// Purpose-
//       Map of Plugin objects.
//
//----------------------------------------------------------------------------
class PluginMap {                   // Plugin map
//----------------------------------------------------------------------------
// PluginMap::Attributes
//----------------------------------------------------------------------------
public:
typedef std::map<std::string, Plugin*> Map;
typedef Map::const_iterator            Iterator;

protected:
   Map                 pluginMap;   // Plugin map

//----------------------------------------------------------------------------
// PluginMap::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~PluginMap( void );              // Destructor
   PluginMap( void );               // Constructor

private:                            // Bitwise copy is prohibited
   PluginMap(const PluginMap&);     // Disallowed copy constructor
PluginMap&
   operator=(const PluginMap&);     // Disallowed assignment operator

//----------------------------------------------------------------------------
// PluginMap::Iterators
//----------------------------------------------------------------------------
public:
virtual Iterator                    // The begin iterator
   begin( void ) const;             // Get begin iterator

virtual Iterator                    // The end iterator
   end( void ) const;               // Get end iterator

//----------------------------------------------------------------------------
// PluginMap::Methods
//----------------------------------------------------------------------------
public:
virtual Plugin*                     // The Plugin
   delPlugin(                       // Delete Plugin
     const std::string&name);       // With this Plugin name

virtual Plugin*                     // The Plugin
   getPlugin(                       // Get Plugin
     const std::string&name) const; // For this Plugin name

virtual void
   setPlugin(                       // Set Plugin
     const std::string&name,        // Plugin name
     Plugin*           plugin);     // Plugin

//----------------------------------------------------------------------------
//
// Method-
//       reset
//
// Purpose-
//       Reset the Plugin map.
//       This method is called from ~PluginMap.
//       Its default action is to delete all associated Plugin objects.
//       Either override this method or use iteration to remove all
//       PluginMap items before deleting the PluginMap. (See below.)
//
// Removal iteration code-
//       // Given: PluginMap map;
//       Plugin::Iterator it;
//       for(it= map.begin(); it != map.end(); it= map.begin() )
//         it.delPlugin(it->first);
//
//----------------------------------------------------------------------------
virtual void
   reset( void );                   // Reset the Plugin map
}; // class PluginMap

#endif // PLUGINMAP_H_INCLUDED
