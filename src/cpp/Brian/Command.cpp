//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2024 Frank Eskesen.
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
//       2024/09/30
//
//----------------------------------------------------------------------------
#include <stdexcept>                // For std::out_of_range

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Latch.h>              // For pub::Latch
#include <pub/utility.h>            // For pub::utility::to_string

#include "Command.h"                // For Command, implemented
#include "Common.h"                 // For Common::shutdown

using pub::utility::to_string;
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Command::Map_t* _map= nullptr; // The actual Command Map*
static pub::Latch      mutex;      // Map insert/remove synchronization Latch

//----------------------------------------------------------------------------
// Global destructor
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
static int             global_destructor_invoked= false;
static struct GlobalDestructor {    // On unload, remove Debug::global
inline
   ~GlobalDestructor( void )
{  if( HCDM ) debugf("Command::GlobalDestructor~\n");

   delete _map;
   _map= nullptr;

   global_destructor_invoked= true;
}
}  globalDestructor;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Subroutine-
//       insert
//       remove
//
// Purpose-
//       Locate|remove|iinsert operations
//
//----------------------------------------------------------------------------
static void
   insert(                          // Insert
     Command*          command)     // This Command
{
   typedef Command::Map_t           Map_t;
   typedef Command::MapIter_t       MapIter_t;

   if( global_destructor_invoked )  // Do nothing if in unloading state
     return;

   std::string name= command->get_name();
   Map_t* map= Command::get_map();

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map->find(name);
     if( mi != map->end() )         // If it's already mapped
       throw std::out_of_range(to_string("Command::insert(%s) is a duplicate"
                                        , name.c_str()));

     (*map)[name]= command;
//   map->insert({name, command});
   }}}}
}

static void
   remove(                          // Remove
     Command*          command)     // This Command
{
   typedef Command::Map_t           Map_t;
   typedef Command::MapIter_t       MapIter_t;

   if( global_destructor_invoked )  // Do nothing if in unloading state
     return;

   std::string name= command->get_name();
   Map_t* map= Command::get_map();

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map->find(name);
     if( mi != map->end() && mi->second == command )
       map->erase(mi);
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       Command::Command
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Command::Command(                // Constructor
     const char*       name)        // The Command name
:  pub::Named(name)
{  insert(this); }

//----------------------------------------------------------------------------
//
// Method-
//       Command::~Command
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Command::~Command( void )        // Destructor
{  remove(this); }

//----------------------------------------------------------------------------
//
// Method-
//       Command::get_map
//
// Purpose-
//       Return the Map_t*
//
// Implementation notes-
//       The map is required during static initialization, and can be
//       erroneously requested during static destruction.
//
//----------------------------------------------------------------------------
Command::Map_t*                     // The Map_t*
   Command::get_map( void )         // Get Map_t*
{
   if( global_destructor_invoked )  // (Should not occur)
     return nullptr;                // (SEGFAULT expected)

   if( _map == nullptr )
     _map= new Map_t();

   return _map;
}

//----------------------------------------------------------------------------
//
// Method-
//       Command::locate
//
// Purpose-
//       Locate associated Command
//
//----------------------------------------------------------------------------
Command*                            // The associated Command, if present
   Command::locate(                 // Get the Command associated
     std::string       name)        // With this name
{
   Command* command= nullptr;       // The associated Command
   Map_t* map= get_map();

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map->find(name);
     if( mi != map->end() )          // If it's mapped
       command= mi->second;
   }}}}

   return command;
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
Command::resultant                  // Resultant, Command dependent
   Command::work(int, char**)       // Handle Command
//   int               argc,        // Argument count (UNUSED parameter)
//   char*             argv[])      // Argument array (UNUSED parameter)
{  return nullptr; }
