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
//       2022/06/12
//
// Implementation note-
//       The global Thread synchronization mutex is used to insure:
//         1) When a Thread starts, Thread::thread represents the running
//            thread BEFORE thread->run invocation.
//         2) When a running Thread terminates, thread.detach() is only
//            called once. The thread.detach() method is idempotent.
//         3) Proper serialization of the thread id map.
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

#include "pub/Thread.h"             // Method declarations

using namespace _PUB_NAMESPACE::debugging; // For debugging methods
using std::atomic_size_t;

namespace _PUB_NAMESPACE {
//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
const std::thread::id  Thread::null_id; // Null Thread id

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
typedef std::map<std::thread::id, Thread*> Map; // Id to Thread map
typedef Map::iterator  MapIter;     // Map iterator

static Map             map;         // The active Thread map
static std::mutex      mutex;       // Global synchronization mutex

// Statistics
static atomic_size_t   max_run(0);  // Maximum running Thread count
static atomic_size_t   running(0);  // Number of running Threads
static atomic_size_t   started(0);  // Number of started Threads

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
   {{{{                             // Synchronize thread move and map
     std::lock_guard<decltype(mutex)> lock(mutex);

     map[std::this_thread::get_id()]= thread; // Add thread to the map
   }}}}

   try {
     ++started;
     size_t was_running= ++running;
     size_t was_maximum= max_run.load();
     while( was_running > was_maximum ) {
       if( max_run.compare_exchange_weak(was_maximum, was_running) )
         break;
     }

     thread->run();

     --running;
   } catch(pub::Exception& X) {
     debugf("%4d Thread(%p)::run(), pub::Exception: %s\n", __LINE__
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

   {{{{                             // Synchronize map
     std::lock_guard<decltype(mutex)> lock(mutex);

     const MapIter mi= map.find(std::this_thread::get_id());
     if( mi != map.end() )          // If the thread still exists
       map.erase(mi);               // Remove it from the map
   }}}}
}

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
{// debugging::tracef("Thread(%p).~Thread\n", this); // Normally commented out
   std::lock_guard<decltype(mutex)> lock(mutex);

   const MapIter mi= map.find(thread.get_id());
   if( mi != map.end() )
     map.erase(mi);

   if( thread.joinable() )
     thread.detach();
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
:  Object(), thread()
{//debugging::tracef("Thread(%p).Thread\n", this); // Normally commented out
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

   std::lock_guard<decltype(mutex)> lock(mutex);

   const MapIter mi= map.find(std::this_thread::get_id());
   if( mi != map.end() )
     thread= mi->second;

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
{  debugf("Thread(%p)::debug(%s)\n", this, info);

   std::string name= "";
   const Named* named= dynamic_cast<const Named*>(this);
   if( named )
     name= " Named(" + named->get_name() + ")";

   debugf("..thread(%s)%s\n", get_id_string().c_str(), name.c_str());
   debugf("..joinable(%d)\n", joinable());
}

void
   Thread::static_debug(            // Debugging display
     const char*       info)        // Caller information
{  debugf("Thread::static_debug(%s)\n", info ? info : "");

   {{{{ std::lock_guard<decltype(mutex)> lock(mutex);
     debugf("%'16zd detached\n",    running.load() - map.size());
     debugf("%'16zd max_running\n", max_run.load());
     debugf("%'16zd running\n",     running.load());
     debugf("%'16zd started\n",     started.load());

     if( info && map.size() ) {
       debugf("..[-thread id-] [---Thread--]\n");
       for(auto mi : map ) {
         Thread* thread= mi.second;
         intptr_t ip= (intptr_t)thread;
         debugf("..[%s] (%#11lx) joinable(%d)\n"
               , get_id_string(mi.first).c_str(), ip, thread->joinable());
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
{
   std::lock_guard<decltype(mutex)> lock(mutex);

   if( thread.joinable() )
     thread.detach();
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
       std::lock_guard<decltype(mutex)> lock(mutex);

       std::thread t(thread_start, this);
       thread= std::move(t);
       break;
     } catch(const std::system_error& X) {
       if( X.code() != std::errc::resource_unavailable_try_again )
         throw;

       sleep(0.001);                // One millisecond delay before retry
     }
   }
}
} // namespace _PUB_NAMESPACE
