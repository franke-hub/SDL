//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Thread.cpp
//
// Purpose-
//       Thread method implementations.
//
// Last change date-
//       2022/10/16
//
// Implementation note-
//       The global Thread synchronization mutex is used to insure:
//         1) When a Thread starts, Thread::thread represents the running
//            thread BEFORE thread->run invocation.
//         2) When a running Thread terminates, thread.detach() is only
//            called once. The thread.detach() method is idempotent.
//         3) Proper serialization of the thread id map.
//
//       We need to avoid a possible deadlock between the (internal) Thread
//       mutex and the exposed Debug mutex. Since Thread::static_debug uses
//       debugf we need to obtain the Debug mutex if it might be invoked.
//       When needed, we always get the Debug mutex first.
//       Debug methods can invoke Thread::current but this should not be a
//       problem. If this turns out to be wrong, Thread::current will need to
//       get the Debug mutex.
//
//       In some environments after detach the std::thread field can't be
//       relied on to obtain the thread id. We preserve the thread id
//       separately to work around this problem.
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic<> statistics
#include <chrono>                   // Used by Thread::sleep()
#include <map>                      // Used to map std::thread::id to Thread*
#include <mutex>                    // For lock_guard, mutex
#include <string>                   // For std::string
#include <system_error>             // Used by Thread::start()

#include <pub/Debug.h>              // For debugging
#include <pub/Exception.h>          // For debugging
#include <pub/Named.h>              // For pub::Named
#include "pub/Thread.h"             // For pub::Thread, implemened

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods
using std::atomic_size_t;

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#if false                           // TODO: REMOVE?
#  define MAP_DEBUG(x) {x}
#  define MAP_DEBUG_DECL(x) x
#else
#  define MAP_DEBUG(x) {}
#  define MAP_DEBUG_DECL(x)
#endif

enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbiosity, higher is more verbose
};

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
const Thread::id_t     Thread::null_id; // Null Thread id

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
typedef std::map<Thread::id_t, Thread*> Map; // Id to Thread map
typedef Map::iterator  iterator;    // Map iterator
typedef Map::const_iterator
                       const_iterator; // Map const_iterator

// Tasks may still exist after static destruction, but the Map destructor
// makes the Map unusable in some environments.
// In an attempt to carry on, we make the Map persistent.
static Map*            map_ptr= new Map(); // (Needs to persist)
static Map&            map= *map_ptr; // The active Thread map
static std::recursive_mutex
                       mutex;       // Global synchronization mutex

// Statistics
static size_t          max_run= 0;  // Maximum running Thread count
static size_t          running= 0;  // Number of running Threads
static size_t          started= 0;  // Number of started Threads

//----------------------------------------------------------------------------
//
// Subroutine-
//       thread_start
//
// Purpose-
//       Start the Thread, insuring Thread::thread initialized first.
//
//----------------------------------------------------------------------------
static void
   exceptional(                     // Exceptional thread debugging
     Thread*           thread)      // For this thread
{
   thread->debug("Exception");
   Thread::static_debug("Exception");
}

static void
   thread_start(                    // Start
     Thread*           thread)      // This thread
{
   try {
     {{{{
       MAP_DEBUG_DECL(
         std::lock_guard<decltype(*Debug::get())> debug(*Debug::get());
       )
       std::lock_guard<decltype(mutex)> lock(mutex);

       map[thread->get_id()]= thread; // Add thread to the map

       ++started;
       ++running;
       if( running > max_run )
         max_run= running;

       MAP_DEBUG(
         debugh("%4d thread_start\n[%s]=%p inserted\n", __LINE__
               , thread->get_id_string().c_str(), thread);
         Thread::static_debug("thread insert");
       )
     }}}}

     thread->run();
   } catch(Exception& X) {
     debugf("%4d Thread(%p)::run(), Exception: %s\n", __LINE__
           , thread, X.to_string().c_str());
     exceptional(thread);
   } catch(std::exception& X) {
     debugf("%4d Thread(%p)::run(), std::exception what(%s)\n", __LINE__
           , thread, X.what());
     exceptional(thread);
   } catch(...) {
     debugf("%4d Thread(%p)::run(), catch(...)\n", __LINE__, thread);
     exceptional(thread);
   }

   // Note: *thread might already be deleted, so thread* cannot be used here.
   {{{{
     MAP_DEBUG_DECL(
       bool erased= false;
       std::lock_guard<decltype(*Debug::get())> debug(*Debug::get());
     )
     std::lock_guard<decltype(mutex)> lock(mutex);

     --running;
     iterator mi= map.find(std::this_thread::get_id());
     if( mi != map.end() ) {        // If the Thread's still mapped
       map.erase(mi);
       MAP_DEBUG( erased= true; )
     }

     MAP_DEBUG(
       debugh("%4d thread_start\n[%s]=%p removed(%d)\n", __LINE__
             , utility::to_string(std::this_thread::get_id()).c_str(), thread
             , erased);
       Thread::static_debug("thread remove");
     )
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::Thread
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Thread::Thread( void )           // Constructor
:  thread()
{  if( HCDM ) debugf("Thread(%p)::Thread\n", this); }

//----------------------------------------------------------------------------
//
// Method-
//       Thread::~Thread
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Thread::~Thread( void )          // Destructor
{  if( HCDM ) debugf("Thread(%p)::~Thread\n", this);

   MAP_DEBUG_DECL( bool erased= false; )
   {{{{
     MAP_DEBUG_DECL(
       std::lock_guard<decltype(*Debug::get())> debug(*Debug::get());
     )
     std::lock_guard<decltype(mutex)> lock(mutex);

     iterator mi= map.find(id);
     if( mi != map.end() ) {
       map.erase(mi);
       MAP_DEBUG( erased= true; )
     }

     MAP_DEBUG(
       debugh("%4d Thread::~Thread delete\n[%s]=%p erased(%d)\n", __LINE__
             , get_id_string().c_str(), this, erased);
       Thread::static_debug("thread_delete");
     )
   }}}}

   if( thread.joinable() )
     thread.detach();
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::current
//
// Purpose-
//       Get the current thread
//
//----------------------------------------------------------------------------
Thread*                             // The current Thread
   Thread::current( void )          // Get current Thread
{
   Thread* thread= nullptr;         // Default, not found
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     const_iterator mi= map.find(std::this_thread::get_id());
     if( mi != map.end() )
       thread= mi->second;
   }}}}

   return thread;
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::debug
//       Thread::static_debug
//
// Purpose-
//       Debugging displays
//
// Implementation notes-
//       It's not a good idea to call static_debug in production mode
//
//----------------------------------------------------------------------------
void
   Thread::debug(                   // Debugging display
     const char*       info) const  // Caller information
{
   const Named* named= dynamic_cast<const Named*>(this);
   std::string name= "";
   if( named )
     name= " Named(" + named->get_name() + ")";

   {{{{
     std::lock_guard<decltype(*Debug::get())> debug(*Debug::get());
     std::lock_guard<decltype(mutex)> lock(mutex);

     debugf("Thread(%p)::debug(%s)\n", this, info);
     debugf("..id(%s) joinable(%d)%s\n"
           , get_id_string().c_str(), joinable(), name.c_str());
   }}}}
}

void
   Thread::static_debug(            // Debugging display
     const char*       info)        // Caller information
{
   {{{{
     std::lock_guard<decltype(*Debug::get())> debug(*Debug::get());
     std::lock_guard<decltype(mutex)> lock(mutex);

     debugf("Thread::static_debug(%s)\n", info ? info : "");
     debugf("%'16zd detached\n", running - map.size());
     debugf("%'16zd max_run\n",  max_run);
     debugf("%'16zd running\n",  running);
     debugf("%'16zd started\n",  started);

     if( info && map.size() ) {
       debugf("..[----thread id---] (-----Thread-----)\n");
       for(auto mi : map ) {
         Thread* thread= mi.second;
         const Named* named= dynamic_cast<const Named*>(thread);
         std::string name= "";
         if( named )
           name= " " + named->get_name();

         debugf("..[%s] (%#.14zx) joinable(%d)%s\n"
               , get_id_string(mi.first).c_str(), intptr_t(thread)
               , thread->joinable(), name.c_str());
       }
     }
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::detach
//
// Purpose-
//       Detach execution thread from this object
//
//----------------------------------------------------------------------------
void
   Thread::detach( void )           // Detach excution thread from this object
{  if( HCDM ) debugf("Thread(%#.14zx)::detach\n", intptr_t(this));

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     if( thread.joinable() )
       thread.detach();
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::sleep
//
// Purpose-
//       Delay the current thread
//
//----------------------------------------------------------------------------
void
   Thread::sleep(                   // Delay the current Thread
     double            seconds)     // For this many seconds
{
   int64_t us= int64_t(seconds * 1000000); // Microsecond resolution
   if( us > 0 && double(us + 1000000) > seconds ) // If positive valid delay
     std::this_thread::sleep_for(std::chrono::microseconds(us));
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::start
//
// Purpose-
//       Start the Thread
//
//----------------------------------------------------------------------------
void
   Thread::start( void )            // Start this Thread
{
   for(;;)                          // (Retry if resource unavailable)
   {
     try {
       {{{{
         std::lock_guard<decltype(mutex)> lock(mutex);

         std::thread t(thread_start, this);
         id= t.get_id();
         thread= std::move(t);
       }}}}
       break;
     } catch(const std::system_error& X) {
       if( X.code() != std::errc::resource_unavailable_try_again )
         throw;

       sleep(0.001);                // One millisecond delay before retry
     }
   }
}
} // namespace _LIBPUB_NAMESPACE
