//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
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
//       2020/07/08
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
#include <chrono>                   // Used by Thread::sleep()
#include <map>                      // Used to map std::thread::id to Thread*
#include <mutex>                    // Used for lock_guard, mutex
#include <system_error>             // Used by Thread::start()

#include <pub/Debug.h>              // For debugging
#include <pub/Exception.h>          // For debugging

#include "pub/Thread.h"             // Method declarations

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
   thread_start(                    // Start
     Thread*           thread)      // This thread
{
   {{{{                             // Synchronize thread move and map
     std::lock_guard<decltype(mutex)> lock(mutex);

     map[std::this_thread::get_id()]= thread; // Add thread to the map
   }}}}

   try {
     thread->run();
   } catch(pub::Exception& X) {
     fprintf(stderr, "Thread(%p)::run(), pub::Exception: %s\n", thread,
                     X.to_string().c_str());
   } catch(std::exception& X) {
     fprintf(stderr, "Thread(%p)::run(), std::Exception what(%s)\n", thread,
                     X.what());
   } catch(...) {
     fprintf(stderr, "Thread(%p)::run(), catch(...)\n", thread);
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
{//debugging::tracef("Thread(%p).~Thread\n", this); // Normally commented out
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
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
#if 0
void
   Thread::debug(                   // Debugging display
     const Thread*     thread)      // (OPTIONAL) thread
{
   Map copy;                        // (Copy of) The active Thread map

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     for(auto mi : map)
       copy[mi.first]= mi.second;
   }}}}

   using namespace debugging;
   if( thread )
     debugf("Thread(%p).debug() id(%s)\n", thread,
            get_id_string(thread->thread.get_id()).c_str());
   else
     debugf("Thread(*).debug()\n");
   debugf("..Map:\n");
   for(auto mi : copy )
     debugf("....[%s] (%p)\n", get_id_string(mi.first).c_str(), mi.second);
}
#endif

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
