//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2023 Frank Eskesen.
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
//       2023/04/21
//
// Implementation note-
//       The global Thread synchronization mutex is used to insure that:
//         1) When a Thread starts, Thread::thread represents the running
//            thread BEFORE thread->run invocation.
//         2) The thread.detach() method is idempotent, and (the associated)
//            tl_current->joinable is maintained properly.
//
//       In some environments, after detach the std::thread field can't be
//       relied on to obtain the thread id. To work around this, we preserve
//       both the std::thread and the std::thread::id.
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic<> statistics
#include <chrono>                   // Used by Thread::sleep()
#include <mutex>                    // For lock_guard, mutex
#include <sstream>                  // For std::stringstream
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
struct tls {                        // Thread Local Storage
Thread*                thread= nullptr; // The current Thread
int                    joinable= true; // TRUE while thread is joinable

   tls(Thread* thread= nullptr) : thread(thread) { }
}; // struct tls

static std::recursive_mutex
                       mutex;       // Global synchronization mutex

static tls             tl_main;     // Main tls
static __thread tls*   tl_current;  // Our thread-local storage

// Statistics
static size_t          detached= 0; // Number of detached Threads
static atomic_size_t   max_run= 0;  // Maximum running Thread count
static atomic_size_t   running= 0;  // Number of running Threads
static atomic_size_t   started= 0;  // Number of started Threads

// Global initialization/termination
static struct Global {
inline
   Global( void )                   // Initialize main()'s tl_current
{  if( false ) debugf("Debug::Global!\n");
   tl_current= &tl_main;            // Main program's tl_current
   tl_current->joinable= false;     // Main programs aren't joinable
}
}  init_term;

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

   std::lock_guard<decltype(mutex)> lock(mutex);

   if( _tls ) {
     if( ((tls*)_tls)->joinable ) {
       ((tls*)_tls)->joinable= false;
       ++detached;
       thread.detach();
     }

     ((tls*)_tls)->thread= nullptr;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::get_id_string
//
// Purpose-
//       Convert std::thread::id to string
//
//----------------------------------------------------------------------------
std::string                         // Associated string
   Thread::get_id_string(           // Represent id as a string
     const id_t&         id)        // The thread id
{
   if( sizeof(id) == sizeof(size_t*) ) {
     char buff[24];
     sprintf(buff, "0x%.14zx", *(size_t*)(&id));
     return buff;
   }

   if( id == null_id )
     return "null_id";

   std::stringstream ss;
   ss << id;
   return ss.str();
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::current
//
// Purpose-
//       Get the current thread
//
// Implementation notes-
//       Behavior is undefined when using Thread::current in a pthread.
//
//----------------------------------------------------------------------------
Thread*                             // The current Thread
   Thread::current( void )          // Get current Thread
{  return tl_current ? tl_current->thread : nullptr; }

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
     debugf("%'16zd detached\n", detached);
     debugf("%'16zd max_run\n",  max_run.load());
     debugf("%'16zd running\n",  running.load());
     debugf("%'16zd started\n",  started.load());
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
   std::lock_guard<decltype(mutex)> lock(mutex);

   if( _tls ) {
     if( ((tls*)_tls)->joinable ) {
       ((tls*)_tls)->joinable= false;
       ++detached;
       thread.detach();
     }
   }
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
   _tls= new tls(this);             // (Deleted in drive() on thread return)

   for(;;) {                        // (Retry if resource unavailable)
     try {
       std::lock_guard<decltype(mutex)> lock(mutex);

       std::thread t(Thread::drive, this);
       thread= std::move(t);
       break;
     } catch(const std::system_error& X) {
       if( X.code() != std::errc::resource_unavailable_try_again ) {
         delete (tls*)_tls;
         _tls= nullptr;
         throw;
       }

       sleep(0.001);                // One millisecond delay before retry
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::drive
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

void
   Thread::drive(                   // Start
     Thread*           thread)      // This thread
{
   try {
     {{{{                         // (Insure thread->thread initialized)
       std::lock_guard<decltype(mutex)> lock(mutex);
     }}}}
     thread->id= std::this_thread::get_id();

     tl_current= (struct tls*)thread->_tls;
     tl_current->thread= thread;
     tl_current->joinable= true;

     ++started;
     std::size_t was_running= ++running;
     std::size_t old_running= max_run.load();
     while( was_running > old_running )
       max_run.compare_exchange_weak(old_running, was_running);

     thread->run();
     --running;

     {{{{
       std::lock_guard<decltype(mutex)> lock(mutex);

       if( tl_current->thread == thread ) { // (If the Thread still exists)
         thread->id= null_id;       // It's no longer running
         thread->_tls= nullptr;     // (Remove soon-to-be dangling reference)
       }

       if( !tl_current->joinable )  // (Maintain detached count)
         --detached;
     }}}}

     delete tl_current;
     tl_current= nullptr;
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
}
} // namespace _LIBPUB_NAMESPACE
