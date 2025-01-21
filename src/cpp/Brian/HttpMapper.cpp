//----------------------------------------------------------------------------
//
//       Copyright (C) 2025 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HttpMapper.cpp
//
// Purpose-
//       Implement HttpMapper.h
//
// Last change date-
//       2025/01/16
//
//----------------------------------------------------------------------------
#include <forward_list>             // For std::forward_list
#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr, std::weak_ptr, ...
#include <mutex>                    // For std::mutex
#include <string>                   // For std::string

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/utility.h>            // For pub::utility::to_string, ...
#include <pub/utility.i>            // For conversion subroutines

#include "HttpListen.h"             // For HttpListen
#include "HttpMapper.h"             // For HttpMapper, implemented

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For convenience
using namespace PUB::debugging;     // For debugging subroutines
using PUB::utility::to_string;      // For convenience
using std::string;                  // For convenience

typedef pub::Ioda::Mesg             Mesg; // For convenience

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
HttpMapper*            HttpMapper::singleton= nullptr; // *THE* HttpMapper

//----------------------------------------------------------------------------
// Global constructor/destructor
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
static struct Global_init_term {
   Global_init_term( void )
{  if( HCDM ) debugh("HttpMapper::Global_init_term!\n"); }

   ~Global_init_term( void )
{  if( HCDM ) debugh("HttpMapper::Global_init_term~\n");

   HttpMapper::shutdown();
}
}  global_init_term;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Method-
//       HttpMapper::HttpMapper
//       HttpMapper::~HttpMapper
// Purpose-
//       Constructors
//       Destructor
//
//----------------------------------------------------------------------------
   HttpMapper::HttpMapper( void )   // (Default) constructor
{  if( HCDM ) debugh("HttpMapper(%p)!\n", this);
   INS_DEBUG_OBJ("HttpMapper");
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   HttpMapper::~HttpMapper( void )  // Destructor
{  if( HCDM ) debugh("HttpMapper(%p)~\n", this);

   REM_DEBUG_OBJ("HttpMapper");
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpMapper::make
//
// Purpose-
//       Make: (singleton) HttpMapper
//
//----------------------------------------------------------------------------
HttpMapper*                         // The (singleton) HttpMapper
   HttpMapper::make( void )         // Make: HttpMapper*
{  if( HCDM ) debugh("HttpMapper::make\n");

   std::mutex mutex;                // Single-threaded make
   std::lock_guard<decltype(mutex)> lock(mutex);

   // It's unlikely but possible that this is not the first make invocation
   if( singleton )                  // If already created
     return singleton;

   singleton= new HttpMapper();     // Create the singleton HttpMapper
   return singleton;                // (Returning it)
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpMapper::shutdown
//
// Purpose-
//       Termination shutdown
//
//----------------------------------------------------------------------------
void
   HttpMapper::shutdown( void )     // Termination shutdown
{  if( HCDM ) debugh("HttpMapper::shutdown\n");

   if( singleton ) {
     singleton->debug("shutdown");
     delete singleton;
     singleton= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpMapper::debug
//
// Purpose-
//       Debug the HttpMapper
//
//----------------------------------------------------------------------------
void
   HttpMapper::debug(               // Debugging display
     const char*       info) const  // Informational display
{  if( HCDM ) debugh("HttpMapper(%p)::debug(%s)\n", this, info);

   status();
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpMapper::insert
//       HttpMapper::locate
//       HttpMapper::remove
//       HttpMapper::status
//
// Purpose-
//       Insert HttpListen onto Map
//       Locate HttpListen with Map
//       Remove HttpListen from Map
//       Display HttpListen Map
//
//----------------------------------------------------------------------------
void
   HttpMapper::insert(              // Insert onto Map
     Listen_t          listen)      // This Listener
{  if( HCDM ) debugh("HttpMapper::insert(%s)\n", s2c(listen->get_host()));

   std::string name= listen->get_host();
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     MapIter_t mi= map.find(name);
     if( mi != map.end() )          // If it's already mapped
       throw std::out_of_range(
           to_string("HttpMapper::insert(%s) is a duplicate", s2c(name)) );

     // For exposition: Both versions operate correctly
     if( true )
       map.insert({name, listen});
     else
       map[name]= listen;

     mi= map.find(name);
     INS_DEBUG_MAP("Mapper.MAP", &mi->second);
   }}}}
}

std::shared_ptr<HttpListen>
   HttpMapper::locate(              // Locate with Map
     std::string       host) const  // This host address:port
{  if( HCDM ) debugh("HttpMapper::locate(%s)\n", s2c(host));

   Listen_t listen= nullptr;        // The HttpListen (default= none)
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     Map_t::const_iterator mi= map.find(host);
     if( mi != map.end() )          // If it's mapped
       listen= mi->second;
   }}}}

   return listen;
}

void
   HttpMapper::remove(              // Remove from Map
     Listen_t          listen)      // This HttpListen
{  if( HCDM ) debugh("HttpMapper::remove(%s)\n", s2c(listen->get_host()));

   std::string name= listen->get_host();
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter_t mi= map.find(name);
     if( mi != map.end() ) {        // If it's mapped
       REM_DEBUG_MAP("Mapper.MAP", &mi->second);
       map.erase(mi);               // (Remove it from the map)
     }
   }}}}
}

void
   HttpMapper::status( void ) const  // Display HttpListen Map
{
   std::forward_list<Listen_t> list; // HttpListen list

   // Copy the HttpListen's from the map into the list (under mutex protection)
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);
     for(auto& it: map) {
       list.push_front(it.second);
     }
   }}}}

   // Display all HttpListens
   for(auto& it: list) {
     it->status();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpMapper::stop(void)
//       HttpMapper::stop(const char*)
//
// Purpose-
//       Stop all Listeners
//       Stop one Listener
//
//----------------------------------------------------------------------------
void
   HttpMapper::stop( void )         // Stop *ALL* Listeners
{  if( HCDM ) debugh("HttpMapper::stop\n");

   std::forward_list<std::shared_ptr<HttpListen>> list; // Listener list

   // Copy the std::shared_ptr<HttpListen> from the map into the list
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);
     for(auto& it: map) {
       list.push_front(it.second);
     }
   }}}}

   // Stop all Listeners
   for(auto& it: list) {
     it->stop();
   }
}

void
   HttpMapper::stop(                // Stop the Listener
     const char*       _url)        // At this URL
{  if( HCDM ) debugh("HttpMapper::stop(%s)\n", _url ? _url : "");

   std::shared_ptr<HttpListen> listener= locate(_url);
   if( listener ) {
     listener->stop();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       HttpMapper::wait(void)
//       HttpMapper::wait(const char*)
//
// Purpose-
//       Wait for all Listeners
//       Wait for one Listener
//
//----------------------------------------------------------------------------
void
   HttpMapper::wait( void )         // Wait for *ALL* Listeners
{  if( HCDM ) debugh("HttpMapper::wait\n");

   std::forward_list<std::shared_ptr<HttpListen>> list; // Listener list

   // Copy the std::shared_ptr<HttpListen> from the map into the list
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);
     for(auto& it: map) {
       list.push_front(it.second);
     }
   }}}}

   // Wait for and remove each Listener
   for(auto& it: list) {
     it->wait();
     remove(it);
   }
}

void
   HttpMapper::wait(                // Wait for the Listener
     const char*       _url)        // At this URL
{  if( HCDM ) debugh("HttpMapper::wait(%s)\n", _url ? _url : "");

   std::shared_ptr<HttpListen> listener= HttpMapper::get()->locate(_url);
   if( listener )
     listener->wait();
}
