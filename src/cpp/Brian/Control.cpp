//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Control.cpp
//
// Purpose-
//       Implement Control.h
//
// Last change date-
//       2025/01/09
//
//----------------------------------------------------------------------------
#include <forward_list>             // For std::forward_list
#include <map>                      // For std::map
#include <mutex>                    // For std::mutex
#include <stdexcept>                // For std::out_of_range

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Dispatch.h>           // For pub::dispatch objects
#include <pub/Exception.h>          // For pub::Exception
#include <pub/Latch.h>              // For pub::Latch
#include <pub/utility.h>            // For pub::utility::to_string

#include "Control.h"                // For Control, implemented

#define PUB _LIBPUB_NAMESPACE
using PUB::Debug;                   // For pub::Debug::get()
using PUB::Exception;               // For pub::Exception
using PUB::utility::to_string;      // For pub::utility::to_string
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
typedef std::map<std::string, Control*>       Map_t; // The Map type
typedef Map_t::iterator                       MapIter_t; // The Map iterator

static pub::Latch      mutex;      // Map synchronization Latch
static Map_t*          _default= nullptr; // The default Control map
static Map_t*          _map= nullptr; // The active Control map

//----------------------------------------------------------------------------
// Global destructor
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
static int             global_destructor_invoked= false;

static struct GlobalDestructor {    // Termination cleanup
inline
   ~GlobalDestructor( void )
{  if( HCDM ) debugh("Control::GlobalDestructor~\n");

   global_destructor_invoked= true;

   delete _default;
   _default= nullptr;
   delete _map;
   _map= nullptr;
}
}  globalDestructor;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_map
//
// Purpose-
//       Get the Control map, allocating it if necessary.
//
//----------------------------------------------------------------------------
static Control::Map_t*              // The Map_t*
   get_map( void )                  // Get Map_t*, possibly creating it
{
   if( global_destructor_invoked )  // (Should not occur)
     return nullptr;

   if( _map == nullptr ) {
     _map= new Map_t();
     _default= new Map_t();
   }

   return _map;
}

//----------------------------------------------------------------------------
//
// Method-
//       Control::Control
//
// Purpose-
//       Constructor
//
// Implementation notes-
//       NULL names (which default to "") are not even inserted into the map.
//
//----------------------------------------------------------------------------
   Control::Control(                // Constructor
     const char*       name)        // The Control name
{  if( name ) { this->name= name; } }

//----------------------------------------------------------------------------
//
// Method-
//       Control::~Control
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Control::~Control( void )        // Destructor
{  }

//----------------------------------------------------------------------------
//
// Subroutine-
//       Control::insert
//       Control::locate
//
// Purpose-
//       Insert|locate operations
//
//----------------------------------------------------------------------------
Control*                            // The inserted or current Control
   Control::insert(                 // Insert
     Control*          control)     // This Control
{  if( HCDM ) debugh("Control::insert(%s)\n", control->get_name().c_str());

   Map_t* map= get_map();           // Get/create the map
   if( map == nullptr )
     return nullptr;

   std::string name= control->get_name();
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map->find(name);
     if( mi != map->end() ) {       // If it's already mapped
       if( true )
         return mi->second;
       else
         throw std::out_of_range(to_string("Control::insert(%s) is a duplicate"
                                          , name.c_str()));
     }

     // For exposition: Both versions operate correctly
     if( false )
       (*map)[name]= control;
     else
       map->insert({name, control});
   }}}}

   return control;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
Control*                            // The associated Control, if present
   Control::locate(                 // Get the Control associated
     std::string       name)        // With this name
{
   Map_t* map= get_map();           // Get/create the map
   if( map == nullptr )
     return nullptr;

   Control* control= nullptr;       // The associated Control (default= none)
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map->find(name);
     if( mi != map->end() )          // If it's mapped
       control= mi->second;
   }}}}

   return control;
}
