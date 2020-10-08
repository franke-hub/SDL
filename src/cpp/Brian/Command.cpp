//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Command.cpp
//
// Purpose-
//       Command object methods
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <map>
#include <mutex>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pub/Debug.h>              // For debugging
#include <pub/utility.h>            // For to_string

#include "Command.h"
#include "Common.h"

using pub::Exception;
using pub::utility::to_string;
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <pub/ifmacro.h>

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
CommandMap::Map_t       CommandMap::map; // The actual CommandMap
class CommandMap        CommandMap; // The map accessor object

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::mutex       mutex;      // Synchronization control

//----------------------------------------------------------------------------
//
// Method-
//       CommandMap::locate()
//       CommandMap::remove()
//       CommandMap::operator[]
//
// Purpose-
//       Locate|remove|iinsert operations
//
//----------------------------------------------------------------------------
Command*                            // The associated Command, if present
   CommandMap::locate(              // Get associated Command
     std::string       name) const  // With this name
{
   Command* command= nullptr;       // The associated Command

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map.find(name);
     if( mi != map.end() )          // If it's mapped
       command= mi->second;
   }}}}

   return command;
}

void
   CommandMap::remove(              // Remove
     Command*          command)     // This Command
{
   std::string name= command->get_name();

   std::lock_guard<decltype(mutex)> lock(mutex);

   const MapIter_t mi= map.find(name);
   if( mi != map.end() && mi->second == command )
     map.erase(mi);
}

Command&                            // The associated Command
   CommandMap::operator[](          // Locate a Command
     std::string       name) const  // With this name
{
   Command* command= locate(name);  // Get associated Command

   if( command == nullptr )         // If the Command isn't mapped
       throw Exception(to_string("CommandMap::[%s] not found",
                       name.c_str()));

   return *command;
}

Command&                            // The associated Command
   CommandMap::operator[](          // Insert
     Command*          command)     // This Command
{
   std::string name= command->get_name();

   std::lock_guard<decltype(mutex)> lock(mutex);

   const MapIter_t mi= map.find(name);
   if( mi != map.end() )            // If it's already mapped
     throw Exception(to_string("CommandMap::insert(%s) duplicated",
                     name.c_str()));

   map[name]= command;
   return *command;
}

//----------------------------------------------------------------------------
//
// Method-
//       Command::work
//
// Purpose-
//       Process Command
//
//----------------------------------------------------------------------------
void
   Command::work(int, char**)       // Handle Command
//   int               argc,        // Argument count (UNUSED parameter)
//   char*             argv[])      // Argument array (UNUSED parameter)
{  }

//----------------------------------------------------------------------------
//
// Static class-
//       Command_list
//
// Purpose-
//       List the commands
//
//----------------------------------------------------------------------------
static class Command_list : public Command {
public:
virtual
   ~Command_list() {}
   Command_list() : Command("list")
{  CommandMap[this]; }

virtual void
   work(int, char**)                // Handle Command
//   int               argc,        // Argument count (UNUSED parameter)
//   char*             argv[])      // Argument array (UNUSED parameter)
{
   debugf("Command list: ");
   typedef CommandMap::MapIter_t MapIter_t;
   for(MapIter_t mi= CommandMap.begin(); mi != CommandMap.end(); ++mi) {
     if( mi != CommandMap.begin() )
       debugf(", ");
     std::string s= mi->first;

     debugf("%s", s.c_str());
   }
   debugf("\n");
}
} command_list; // static class Command_list

//----------------------------------------------------------------------------
//
// Static class-
//       Command_quit
//
// Purpose-
//       Terminate processing
//
//----------------------------------------------------------------------------
static class Command_quit : public Command {
public:
virtual
   ~Command_quit() {}
   Command_quit() : Command("quit")
{  CommandMap[this]; }

virtual void
   work(                            // Handle Command
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{  Command::work(argc, argv);
   Common::get()->shutdown();
}
} command_quit; // static class Command_quit
