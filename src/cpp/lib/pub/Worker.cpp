//----------------------------------------------------------------------------
//
//       Copyright (C) 2019-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Worker.cpp
//
// Purpose-
//       Worker object methods.
//
// Last change date-
//       2022/10/09
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic<>
#include <mutex>                    // For std::lock_guard

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include "pub/Latch.h"              // For pub::Latch objects
#include <pub/Semaphore.h>          // For pub::Semaphore
#include <pub/Thread.h>             // For pub::Thread
#include "pub/Worker.h"             // For pub:: Worker, implemented
#include <pub/utility.h>            // For pub::utility::on_exception

using namespace _LIBPUB_NAMESPACE::debugging; // For debugging methods
using std::atomic_uint;
using std::atomic_size_t;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <pub/ifmacro.h>

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class WorkerThread;                 // The Worker thread

//----------------------------------------------------------------------------
// Static attributes
//----------------------------------------------------------------------------
enum { MAX_THREADS= 128 };          // The built-in thread pool size

static Latch           mutex;       // Access mutex
static WorkerThread*   pool[MAX_THREADS]; // The built-in thread pool
// tic WorkerThread*   static_pool[MAX_THREADS]; // The built-in thread pool
// tic WorkerThread**  pool= static_pool; // The current thread pool

// Statistical counters
static atomic_size_t   max_running(0); // Maximum number of running threads
// tic unsigned        max_size= MAX_THREADS; // Maximum size of thread pool
static atomic_uint     max_used(0); // Maximum number of pool threads
static atomic_size_t   running(0);  // Current number of running threads
static const unsigned  size= MAX_THREADS; // Size of thread pool
static unsigned        used= 0;     // Current number of pool threads
static atomic_size_t   workers(0);  // Number of WorkerPool::work() invocations

//----------------------------------------------------------------------------
//
// Class-
//       WorkerThread
//
// Purpose-
//       The Thread used to drive a Worker thread pool.
//
//----------------------------------------------------------------------------
class WorkerThread : public Thread { // The WorkerThread Thread
//----------------------------------------------------------------------------
// WorkerThread::Attributes
//----------------------------------------------------------------------------
protected:
bool                   operational; // TRUE while operational
Semaphore              sem;         // State switch event Semaphore
Worker*                worker;      // The current Worker

//----------------------------------------------------------------------------
// WorkerThread::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~WorkerThread( void ) {}         // Destructor

   WorkerThread(                    // Constructor
     Worker*           worker= nullptr) // Associated Worker
:  Thread(), operational(true), sem(), worker(worker)
{  start(); }

//----------------------------------------------------------------------------
// WorkerThread::Accessors
//----------------------------------------------------------------------------
inline bool
   is_operational( void )
{  return operational; }

//----------------------------------------------------------------------------
// WorkerThread::done()
//
// Handle work completion.
//----------------------------------------------------------------------------
inline void
   done( void )                      // Work complete
{
   --running;
   WorkerThread* thread= this;
   unsigned now_used= 0;

   if( operational )
   {{{{ // PERFORMANCE CRITICAL ==============================================
     std::lock_guard<decltype(mutex)> lock(mutex);

     if( used < size ) {
       pool[used++]= thread;
       thread= nullptr;
       now_used= used;
     }
   }}}} // PERFORMANCE CRITICAL ==============================================

   if( thread ) {
     thread->detach();
     thread->stop();
   }

   unsigned was_maxi= 0;            // (Avoids load if pool size unchanged)
   while( now_used > was_maxi ) {
     if( max_used.compare_exchange_weak(was_maxi, now_used) )
       break;
   }
}

//----------------------------------------------------------------------------
// WorkerThread::drive()
//
// Call this function once for each unit of work to be processed.
//----------------------------------------------------------------------------
virtual void
   drive(                           // Drive
     Worker*           worker)      // This Worker
{
   this->worker= worker;
   sem.post();
}

//----------------------------------------------------------------------------
// WorkerThread::stop()
//
// Terminate Thread processing.
//----------------------------------------------------------------------------
virtual void
   stop( void )                     // Terminate processing
{
   operational= false;
   sem.post();
}

//----------------------------------------------------------------------------
// WorkerThread::run()
//
// Operate the WorkerThread.
//----------------------------------------------------------------------------
protected:
void
   run( void )                      // Operate the Thread
{
   while( operational ) {
     if( worker != nullptr ) {
       try {
         worker->work();
       } catch(Exception& X) {
         debugging::debugh("WorkerException: %s\n", X.to_string().c_str());
         utility::on_exception(X.to_string());
       } catch(std::exception& X) {
         debugging::debugh("WorkerException: what(%s)\n", X.what());
         utility::on_exception(X.what());
       } catch(...) {
         debugging::debugh("WorkerException: ...\n");
         utility::on_exception("...");
       }
     }
     worker= nullptr;

     done();
     sem.wait();
   }

   delete this;
}
}; // class WorkerThread

//----------------------------------------------------------------------------
//
// Method-
//       WorkerPool::getMaxThreads
//       WorkerPool::setMaxThreads
//
// Purpose-
//       Accessors: size
//
//----------------------------------------------------------------------------
#if 0 // DEPRECATED ==========================================================
unsigned                            // The maximum number of pooled threads
   WorkerPool::getMaxThreads( void ) // Get maximum number of pooled threads
{  return size; }

void
   WorkerPool::setMaxThreads(        // Get maximum number of pooled threads
     unsigned          new_size)     // The maximum number of pooled threads
{  std::lock_guard<decltype(mutex)> lock(mutex);

   if( new_size < used ) {
     for(int i= new_size; i<used; i++) {
       WorkerThread* thread= pool[i];
       thread->detach();
       thread->stop();
     }

     used= new_size;
   }

   if( new_size > MAX_THREADS ) {
     WorkerThread** thread= new WorkerThread*[new_size];
     if( new_size > max_size )
       max_size= new_size;

     for(int i= 0; i<used; i++)
       thread[i]= pool[i];

     if( size > MAX_THREADS )
       delete[] pool;

     pool= thread;                   // Replace the pool
   } else {
     if( size > MAX_THREADS ) {
       for(int i= 0; i<used; i++)
         static_pool[i]= pool[i];

       delete[] pool;
     }

     pool= static_pool;
   }

   size= new_size;
}
#endif // DEPRECATED =========================================================

//----------------------------------------------------------------------------
//
// Method-
//       WorkerPool::get_running
//
// Purpose-
//       Accessor: running
//
//----------------------------------------------------------------------------
unsigned                            // The number of running threads
   WorkerPool::get_running( void )  // Get number of running threads
{  return running; }

//----------------------------------------------------------------------------
//
// Method-
//       WorkerPool::debug
//
// Purpose-
//       Display statistics
//
//----------------------------------------------------------------------------
void
   WorkerPool::debug(             // Debugging display
     const char*       info)      // Caller information (adds thread list)
{
   debugf("WorkerPool::debug(%s)\n", info ? info : "");

   debugf("%'16zd max_running\n", max_running.load());
// debugf("%'16d max_size\n",     max_size);
   debugf("%'16d max_pooled\n",   max_used.load());
   debugf("%'16zd running\n",     running.load());
// debugf("%'16d size\n",         size);
   debugf("%'16d pooled\n",       used);
   debugf("%'16zd workers\n",     workers.load());

   if( info ) {
     std::lock_guard<decltype(mutex)> lock(mutex);
     for(unsigned i= 0; i<used; i++) {
       WorkerThread* thread= pool[i];
       debugf("[%4d] %#.14zx\n", i, intptr_t(thread));
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WorkerPool::reset
//
// Purpose-
//       Reset (Empty) the WorkerThread pool
//
//----------------------------------------------------------------------------
void
   WorkerPool::reset( void )        // Reset (Empty) the WorkerThread pool
{  std::lock_guard<decltype(mutex)> lock(mutex);

   for(unsigned i= 0; i<used; i++) {
     WorkerThread* thread= pool[i];
     thread->detach();
     thread->stop();
   }

   // Reset the statistics
   max_running.store(0);
   max_used.store(0);
   running.store(0);
   used= 0;
   workers.store(0);
}

//----------------------------------------------------------------------------
//
// Method-
//       WorkerPool::work
//
// Purpose-
//       Drive the Worker
//
//----------------------------------------------------------------------------
void
   WorkerPool::work(                 // Process work
     Worker*           worker)       // Using this Worker
{
   ++workers;
   size_t was_running= ++running;
   size_t was_maximum= max_running.load();
   while( was_running > was_maximum ) {
     if( max_running.compare_exchange_weak(was_maximum, was_running) )
       break;
   }

   WorkerThread* thread= nullptr;

   {{{{ // PERFORMANCE CRITICAL ==============================================
     std::lock_guard<decltype(mutex)> lock(mutex);

     if( used > 0 )
       thread= pool[--used];
   }}}} // PERFORMANCE CRITICAL ==============================================

   if( thread )
     thread->drive(worker);
   else
     new WorkerThread(worker);
}
}  // namespace _LIBPUB_NAMESPACE
