//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       diag-shared_ptr.cpp
//
// Purpose-
//       Implement diag-shared_ptr.h.
//
// Last change date-
//       2023/12/05
//
// Implementation notes-
//       Depending on global initialization ordering, static shared_ptr
//       definitions can confuse std::map.
//       Symptoms include:
//         ((r_map->size() > 0 && r_map_begin() == r_map->end()) == true).
//       Your results may vary.
//
//----------------------------------------------------------------------------
#include <map>                      // For std::map
#include <memory>                   // For std::shared_ptr, ...
#include <mutex>                    // For std::lock_guard
#include <string>                   // For std::string

#include <pub/Debug.h>              // For namespace pub::debugging methods
#include "pub/diag-shared_ptr.h"    // For shared_ptr diagnostics, implemented

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;
using namespace PUB::debugging;

namespace std { // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
namespace pub_diag {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // enum

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
static void map_term( void );

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
typedef intptr_t       addr_t;

typedef std::map<addr_t, string>    container_t; // Container*, Container name
typedef std::map<addr_t, addr_t>    reference_t; // Get*, Ref*

std::mutex             i_mutex;     // Protects map_init, map_term
std::mutex             c_mutex;     // Protects c_map modification
std::mutex             r_mutex;     // Protects r_map modification
container_t*           c_map= nullptr; // The container map
reference_t*           r_map= nullptr; // The reference map

// Note that global destructors are invoked one by one.
// Our position on that list is indeterminate.
static struct GlobalDestructor {
   ~GlobalDestructor(void)
{
   if( c_map && r_map && (c_map->size() > 1 || r_map->size() > 0) ) {
     debugf("%4d %s GlobalDestructor\n", __LINE__, __FILE__);
     Debug_ptr::debug("GlobalDestructor");
   }

   map_term();
}  // GlobalDestructor~
}  globalDestructor;

//----------------------------------------------------------------------------
//
// Subroutine-
//       map_init
//
// Purpose-
//       Initialize the maps (If not already initialized)
//
//----------------------------------------------------------------------------
static void
   map_init( void )
{
   if( c_map )
     return;

   std::lock_guard<decltype(i_mutex)> lock(i_mutex);
   if( c_map )
     return;

   c_map= new container_t();
   r_map= new reference_t();

   (*c_map)[addr_t(nullptr)]= "Nullptr";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       map_term
//
// Purpose-
//       Terminate mapping
//
//----------------------------------------------------------------------------
static void
   map_term( void )
{  std::lock_guard<decltype(i_mutex)> lock(i_mutex);

   delete c_map;
   delete r_map;
   c_map= nullptr;
   r_map= nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       (std::pub_diag::)debug_ptr<void>::debug_ptr<void>
//       (std::pub_diag::)debug_ptr<void>::~debug_ptr<void>
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   debug_ptr<void>::debug_ptr(void) // We could create the r_map entry here
{                                   // (but we don't)
   if( HCDM && VERBOSE > 1 ) debugf("debug_ptr(%p)!\n", this);
}

   debug_ptr<void>::~debug_ptr(void) // We erase the r_map entry here
{  if( HCDM && VERBOSE > 1 ) debugf("debug_ptr(%p)~\n", this);

   std::lock_guard<decltype(r_mutex)> lock(r_mutex);
   if( r_map )
     r_map->erase(addr_t(this));
}

//----------------------------------------------------------------------------
//
// Method-
//       (std::pub_diag::)Debug_ptr::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Debug_ptr::debug(const char* info)
{
   debugf("debug_ptr::debug(%s)\n", info);

   {{{{
     std::lock_guard<decltype(i_mutex)> i_lock(i_mutex);
     if( c_map == nullptr || r_map == nullptr ) {
       debugf("..Nothing mapped..\n");
       return;
     }
   }}}}

   std::lock_guard<decltype(c_mutex)> c_lock(c_mutex);
   std::lock_guard<decltype(r_mutex)> r_lock(r_mutex);

   // Display by address, intermixing containers and references
   addr_t c_last= addr_t(-1);
   auto cx= c_map->begin();
   addr_t c_addr= addr_t(-1);
   if( cx != c_map->end() )
     c_addr= cx->first;

   auto rx= r_map->begin();
   addr_t r_addr= addr_t(-1);
   if( rx != r_map->end() )
     r_addr= rx->first;

   while( cx != c_map->end() || rx != r_map->end() ) {
     if( cx != c_map->end() && (r_addr >= c_addr || rx == r_map->end()) ) {
       debugf("\n%#14zx %s\n", c_addr, cx->second.c_str());
       c_last= c_addr;
       c_addr= -1;
       if( ++cx != c_map->end() )
         c_addr= cx->first;
       continue;
     }

     addr_t r_that= rx->second;
     string r_name= "Not mapped";
     auto r_cx= c_map->find(r_that);
     if( r_cx != c_map->end() )
       r_name= r_cx->second;

     addr_t r_offs= r_addr - c_last; // Calculate offset (with max)
     if( r_offs >= 0x00010000 )
       r_offs= 0x0000ffff;
     debugf("%.4zx %#14zx->%#14zx %s\n", r_offs, r_addr, r_that, r_name.c_str());
     r_addr= -1;
     if( ++rx != r_map->end() )
       r_addr= rx->first;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       (std::pub_diag::)Debug_ptr::insert
//
// Purpose-
//       Insert an object into the container map
//
//----------------------------------------------------------------------------
void
   Debug_ptr::insert(
     const void*       self,        // The object's address
     std::string       name)        // The object's name
{  std::lock_guard<decltype(c_mutex)> lock(c_mutex);

   map_init();
   (*c_map)[addr_t(self)]= name;
}

void
//----------------------------------------------------------------------------
//
// Method-
//       Debug_ptr::remove
//
// Purpose-
//       Remove an object from the container map
//
//----------------------------------------------------------------------------
   Debug_ptr::remove(               // Remove an object from the container map
     const void*       self)        // The object's address
{  std::lock_guard<decltype(c_mutex)> lock(c_mutex);

   if( c_map )                      // Remove the index
     c_map->erase(addr_t(self));
}

//----------------------------------------------------------------------------
//
// Method-
//       Debug_ptr::update
//
// Purpose-
//       Update the reference map
//
//       typedef std::map<void*, string>     container_t; // Container*, name
//       typedef std::map<void*, void*>      reference_t; // Get*, Ref*
//----------------------------------------------------------------------------
void
   Debug_ptr::update(               // Update the reference map
     const void*       self,        // The debug_ptr's address
     const void*       that)        // The referenced address
{  std::lock_guard<decltype(r_mutex)> lock(r_mutex);

   map_init();
   if( that )
     (*r_map)[addr_t(self)]= addr_t(that); // Insert/update the reference
   else
     r_map->erase(addr_t(self));    // Remove the reference (if present)
}
}  // namespace pub_diag
}  // namespace std
