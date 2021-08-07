//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2021 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Thing.cpp
//
// Purpose-
//       Define static Thing attributes.
//
// Last change date-
//       2021/08/06
//
// Implementation notes-
//       Fast array (de)allocation skips allocation counting/checking, but
//       preallocation/deallocate_all logic counts them.
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic
#include <memory>                   // For std::shared_ptr
#include <mutex>                    // For std::lock_guard
#include <new>                      // For in-place ::delete

#include <assert.h>                 // For assert()
#include <com/Debug.h>              // For com::Debug
#include <obj/Object.h>             // For obj::Object
#include <obj/Latch.h>              // For obj::Latch

#include "Thing.h"

using namespace _OBJ_NAMESPACE;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#undef  STATS                       // Ignore statistical counters
#define STATS                       // Use statistical counters

enum // Compile-time constants
{  HCDM= false                      // Hard Core Debug Mode?
,  THING_CACHE= 8                   // Number of fast-cache items
,  USE_ALLOCATION_PRELOAD= true     // Use allocation preloading?
}; // Compile-time constants

#if true
#define MAX_CACHED_THINGS 1000000   // Maximum number of cached Things
#define MIN_CACHED_THINGS  500000   // Minimum number of cached Things
#else
#define MAX_CACHED_THINGS 1048576   // Maximum number of cached Things
#define MIN_CACHED_THINGS  524288   // Minimum number of cached Things
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#ifdef STATS
  #define IFSTATS(x) {x}
#else
  #define IFSTATS(x) {}
#endif

//----------------------------------------------------------------------------
// External data area
//----------------------------------------------------------------------------
int                    Thing_base::errorCount= 0;


//----------------------------------------------------------------------------
//
// Struct-
//       NewThing
//
// Purpose-
//       Free pool Thing
//
//----------------------------------------------------------------------------
struct NewThing {
   NewThing*           next;        // Next free NewThing
};

//----------------------------------------------------------------------------
// Internal data area
//----------------------------------------------------------------------------
static std::atomic<Thing*> freeCache[THING_CACHE]=
    { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
static Latch           freeLatch;   // Single threaded allocation Latch
static std::atomic<NewThing*>
                       freePool(nullptr); // Free allocation pool

static std::atomic<size_t>
                       allocated(0); // Number of currently allocated Things
static std::atomic<size_t>
                       available(0); // Number of currently available Things

static std::atomic<size_t>
                       del_things(0); // Number of calls to Thing::op delete()
static std::atomic<size_t>
                       new_things(0); // Number of calls to Thing::op new()

static std::atomic<size_t>
                       op_dels(0); // Number of calls to ::delete()
static std::atomic<size_t>
                       op_news(0); // Number of calls to ::new()

static std::atomic<size_t>
                       max_allocated(0); // Maximum number of allocated Things
static std::atomic<size_t>
                       max_available(0); // Maximum number of available Things

//----------------------------------------------------------------------------
//
// Subroutine-
//       free_thing
//
// Purpose-
//       Free a Thing
//
//----------------------------------------------------------------------------
static void
   free_thing(void* addr, void* next= nullptr)
{
   if( HCDM && true  )
     debugf("0p%.10zx= free_thing->0p%.10zx\n", intptr_t(addr)
           , intptr_t(next));

   ::free(addr);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       malloc_thing
//
// Purpose-
//       Allocate a thing, throwing std::bad_alloc if failure
//
//----------------------------------------------------------------------------
static void*
   malloc_thing( void )
{
   void* addr= malloc(sizeof(std::shared_ptr<Thing>) + sizeof(Thing));
   if( addr == nullptr )
     throw std::bad_alloc();

   if( HCDM && true  )
     debugf("0p%.10zx= malloc_thing\n", intptr_t(addr));
   return addr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Thing_base::~Thing_base
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Thing_base::~Thing_base( void )  // Destructor
{
   if( HCDM )
     debugf("%4d: Thing(%p)::~Thing(%zd)\n", __LINE__, this, checkword);

   check(__LINE__, checkword);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thing_base::Thing_base
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Thing_base::Thing_base(          // Constructor
     size_t            checkword)   // Check word
:  prefix(PrefixValidator)
,  checkword(checkword)
,  suffix(SuffixValidator)
{
   if( HCDM )
     printf("%4d: Thing(%p)::Thing(%zd)\n", __LINE__, this, checkword);

   posAddr= (intptr_t)this;
   negAddr= ~posAddr;

   word[0]= word[1]= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Thing_base::string()
//
// Purpose-
//       Override obj::Object::string()
//
//----------------------------------------------------------------------------
const std::string                   // A String representation of this Object
   Thing_base::string( void ) const // Represent this Object as a String
{  return built_in::to_string("Thing(%p)::string %zd", this, checkword); }

//----------------------------------------------------------------------------
//
// Method-
//       Thing_base::check
//
// Purpose-
//       Internal consistency check
//
//----------------------------------------------------------------------------
int                                 // Error count
   Thing_base::check(               // Count errors
     int               lineno,      // Caller's line number
     size_t            checkword) const // Check word
{
   if( HCDM )
     debugf("%4d: Thing(%p)::check(%zd)\n", lineno, this, checkword);

   if( prefix != PrefixValidator ) {
     errorCount++;
     errorf("%4d: Thing(%p).check() prefix(%.8lx)\n", lineno, this, prefix);
   }

   if( (intptr_t)this != posAddr ) {
     errorCount++;
     errorf("%4d: Thing(%p).check() posAddr(%.8zx)\n", lineno, this, (size_t)posAddr);
   }

   if( ~((intptr_t)this) != negAddr ) {
     errorCount++;
     errorf("%4d: Thing(%p).check() negAddr(%.8zx)\n", lineno, this, (size_t)negAddr);
   }

   if( suffix != SuffixValidator ) {
     errorCount++;
     errorf("%4d: Thing(%p).check() suffix(%.8lx)\n", lineno, this, suffix);
   }

   if( checkword != this->checkword && checkword != 0 ) {
     errorCount++;
     errorf("%4d: Thing(%p).check(%.8zx) checkword(%.8zx)\n", lineno, this,
             checkword, this->checkword);
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Method-
//       Thing_base::debug_static
//
// Purpose-
//       Static debugging.
//
//----------------------------------------------------------------------------
void
   Thing_base::debug_static( void ) // Static debug display
{
   debugf("\n\n--------------------------------\n");
   #ifdef USE_THING_OBJ
     debugf("Thing::debug_static (USE_THING_OBJ)\n");
   #else
     debugf("Thing::debug_static (USE_SHARED_PTR)\n");
   #endif
   debugf("..freePool(%p) available(%'zd) max_available(%'zd)\n"
          , freePool.load(), available.load(), max_available.load());
   IFSTATS(
     debugf("..allocated(%'zd) max_allocated(%'zd)\n"
            "..new_things(%'zd) del_things(%'zd)\n"
            "..op_news(%'zd) op_dels(%'zd)\n"
            , allocated.load(), max_allocated.load()
            , new_things.load(), del_things.load()
            , op_news.load(), op_dels.load());
   )
}

//----------------------------------------------------------------------------
//
// Method-
//       get_allocated
//
// Purpose-
//       Return the allocated counter
//
//----------------------------------------------------------------------------
size_t                              // The allocated counter
   Thing_base::get_allocated( void ) // Get allocated counter
{  return allocated.load(); }

//----------------------------------------------------------------------------
//
// Method-
//       Thing_base::allocate
//
// Purpose-
//       Allocate a Thing
//
//----------------------------------------------------------------------------
void*                               // The Thing*
   Thing_base::allocate(std::size_t size) // The allocation length
{  if( HCDM ) debugf("Thing::allocate(%zd)\n", size);

   IFSTATS(new_things++;)

   {{{{ // Attempt lock-free allocation from the atomic list
     for(int i= 0; i<(int)THING_CACHE; i++) {
       Thing* thing= freeCache[i].load();
       if( thing ) {
         if( freeCache[i].compare_exchange_strong(thing, nullptr) ) {
           if( HCDM )
             debugf("0p%.10zx= fast_alloc[%d]\n", intptr_t(thing), i);
           return thing;
         }
       }
     }
   }}}}

   {{{{ // Attempt allocation from the free list
     std::lock_guard<decltype(freeLatch)> lock(freeLatch);

     NewThing* thing= freePool.load();
     while( thing ) {
       NewThing* next= thing->next;
       if( freePool.compare_exchange_strong(thing, next) ) {
         if( HCDM )
           debugf("0p%.10zx= list_alloc\n", intptr_t(thing));
         available--;
         return thing;
       }
     }
   }}}}

   static std::mutex one_shot;      // First time allocation Latch
   if( USE_ALLOCATION_PRELOAD && one_shot.try_lock() ) {
     // Pre-load the freePool list
     for(int i= 0; i<MIN_CACHED_THINGS; i++) {
       IFSTATS(
         allocated++;
         op_news++;
       )
       void* addr= malloc_thing();
       NewThing* thing= (NewThing*)addr;
       NewThing* old= freePool.load();
       thing->next= old;
       while( !freePool.compare_exchange_weak(old, thing) )
         thing->next= old;
       available++;
     }

     // Pre-load the freeCache array
     IFSTATS(
       allocated++;
       op_news++;
     )
     void* addr= malloc_thing();
//   available++;                   // The returned addr is NOT available

     for(int i= 0; i<(int)THING_CACHE; i++) {
       Thing* thing= (Thing*)addr;
       Thing* empty= nullptr;

       if( freeCache[i].compare_exchange_strong(empty, thing) ) {
         IFSTATS(
           allocated++;
           op_news++;
         )
         addr= malloc_thing();
         available++;
       }
     }

     IFSTATS(
       if( allocated.load() > max_allocated.load() )
         max_allocated.store(allocated.load()); // Close enough for gov. work
     )
     return addr;
   }

   IFSTATS(
     size_t current_used= ++allocated;
     size_t current_max=  max_allocated.load();
     while( current_used > current_max ) {
       if( max_allocated.compare_exchange_weak(current_max, current_used) )
         break;
     }

     op_news++;
   )

   return malloc_thing();
}

//----------------------------------------------------------------------------
//
// Method-
//       Thing_base::deallocate
//
// Purpose-
//       Deallocate a Thing
//
//----------------------------------------------------------------------------
void
   Thing_base::deallocate(void* addr, std::size_t size) // Deallocate
{  if( HCDM ) debugf("Thing::deallocate(%p,%zd)\n", addr, size);

   IFSTATS(del_things++;)

   // Atomically add to the atomic cache
   for(int i= 0; i<(int)THING_CACHE; i++) {
     Thing* oldThing= freeCache[i].load();
     if( oldThing == nullptr ) {
       if( freeCache[i].compare_exchange_strong(oldThing, (Thing*)addr) ) {
         if( HCDM )
           debugf("0p%.10zx= fast_dealloc[%d]\n", intptr_t(addr), i);
         return;
       }
     }
   }

   if( available.load() >= MAX_CACHED_THINGS ) {
     IFSTATS(
       allocated--;
       op_dels++;
     )
     free_thing(addr);
     return;
   }

   NewThing* thing= (NewThing*)addr;
   NewThing* old= freePool.load();
   thing->next= old;
   while( !freePool.compare_exchange_weak(old, thing) )
     thing->next= old;
   if( HCDM ) debugf("0p%.10zx= list_dealloc->0p%.10zx\n", intptr_t(addr)
                    , intptr_t(old));
   size_t current_used= ++available;

   size_t current_max=  max_available.load();
   while( current_used > current_max ) {
     if( max_available.compare_exchange_weak(current_max, current_used) )
       break;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Thing_base::deallocate_all
//
// Purpose-
//       Deallocate all internal available Things
//
//----------------------------------------------------------------------------
void
   Thing_base::deallocate_all( void ) // Delete all available Things
{  if( HCDM )
     debugf("Thing::deallocate_all\n");

   NewThing* thing= nullptr;        // (Declared here to keep in scope)
   {{{{ // Empty the atomic list
     if( HCDM )
       debugf("Atomic list\n");
     for(int i= 0; i<(int)THING_CACHE; i++) {
       Thing* thing= freeCache[i].load();
       if( thing ) {
         if( freeCache[i].compare_exchange_strong(thing, nullptr) ) {
           IFSTATS(
             allocated--;
             op_dels++;
           )
           free_thing(thing);
           if( USE_ALLOCATION_PRELOAD )
             available--;
         }
       }
     }
   }}}}

   if( HCDM )
     debugf("Pool list:\n");
   {{{{ // Empty the free list
     std::lock_guard<decltype(freeLatch)> lock(freeLatch);

     thing= freePool.load();
     while( !freePool.compare_exchange_strong(thing, nullptr) )
       ;
   }}}}

   while( thing ) {
     NewThing* next= thing->next;

     IFSTATS(
       allocated--;
       op_dels++;
     )
     free_thing(thing, next);
     available--;

     thing= next;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       ::operator delete
//       ::operator new
//
// Purpose-
//       Replacements for global operator new/delete
//
//----------------------------------------------------------------------------
void
   operator delete(void* addr)      // Global operator delete (replacement)
{
   if( HCDM && true  ) {            // (Can't use debugf here)
     printf("0p%.10zx= operator delete(addr)\n", intptr_t(addr));
     printf("0p%.10zx= ::free()\n", intptr_t(addr));
   }
   ::free(addr);
}
void
   operator delete(void* addr, size_t size)
{
   if( HCDM )                       // (Can't use debugf here)
     printf("0p%.10zx= operator delete(%zd)\n", intptr_t(addr), size);

   #ifdef USE_THING_OBJ
     if( size == sizeof(Thing) ) {
       Thing::deallocate(addr, size);
       return;
     }
   #else
     if( size == (sizeof(std::shared_ptr<Thing>) + sizeof(Thing)) ) {
       Thing::deallocate(addr, size);
       return;
   }
   #endif

   if( HCDM )                       // (Can't use debugf here)
     printf("0p%.10zx= ::free()\n", intptr_t(addr));
   ::free(addr);
}

void*
   operator new(size_t size)        // Global operator new (replacement)
{
   void* addr= nullptr;

   #ifdef USE_THING_OBJ
     if( size == sizeof(Thing) )
       addr= Thing::allocate(size);
   #else
     if( size == (sizeof(std::shared_ptr<Thing>) + sizeof(Thing)) )
       addr= Thing::allocate(size);
   #endif

   if( addr == nullptr )
     addr= malloc(size);
   if( HCDM && true  )               // (Can't use debugf here)
     printf("0p%.10zx= operator new(%zd)\n", intptr_t(addr), size);
   if( addr )
     return addr;

   throw std::bad_alloc();
}
