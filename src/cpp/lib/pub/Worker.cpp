//----------------------------------------------------------------------------
//
//       Copyright (C) 2019-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Worker.cpp
//
// Purpose-
//       Worker object methods.
//
// Last change date-
//       2026/03/09
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic<>
#include <mutex>                    // For std::lock_guard, std::mutex
#include <cstdlib>                  // For malloc, free

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include "pub/Latch.h"              // For pub::Latch objects
#include "pub/Thread.h"             // For pub::Thread
#include "pub/Trace.h"              // For pub::Trace
#include "pub/Worker.h"             // For pub:: Worker, implemented
#include <pub/utility.h>            // For pub::utility::report_exception, ...
#include <pub/utility.i>            // For pub::utility conversion routines

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods
using pub::Trace;                   // For tracing methods
using pub::utility::to_void;        // For if( to_void(this) == nullptr )
using std::atomic_size_t;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

// Production mode settings: USE_ITRACE= false;
,  USE_IDEBUG= true                 // Use internal debugging?
,  USE_ITRACE= true                 // Use internal trace?
}; // (Generic) enum

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Static attributes
//----------------------------------------------------------------------------
enum { MAX_THREADS= 16 };           // The default pool_size

static Latch           pool_mutex;  // Pool static attribute access mutex
static WorkerThread*   static_pool[MAX_THREADS]; // The built-in thread pool
static WorkerThread**  pool= (WorkerThread**)&static_pool; // The current pool
static size_t          pool_size= MAX_THREADS; // Size of thread pool
static size_t          pool_used= 0; // Current number of pooled WorkerThreads

// Statistical counters
atomic_size_t          WorkerPool::del_workers(0); // WorkerThread delete count
atomic_size_t          WorkerPool::new_workers(0); // WorkerThread new count
atomic_size_t          WorkerPool::max_running(0); // Maximum running
atomic_size_t          WorkerPool::max_threads(0); // Maximum threads
atomic_size_t          WorkerPool::max_used(0); // Maximum pool Thread count

atomic_size_t          WorkerPool::running(0); // Running Thread count
atomic_size_t          WorkerPool::threads(0); // Current Thread count
atomic_size_t          WorkerPool::workers(0); // WorkerPool::work() calls

//----------------------------------------------------------------------------
// Global constructor/destructor
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
static struct Global_init_term {
   Global_init_term( void )
{  if( HCDM ) traceh("Worker::Global_init_term!\n"); }

   ~Global_init_term( void )
{  if( HCDM ) traceh("Worker::Global_init_term~\n");

   WorkerPool::reset();
   Thread::sleep(0.25);
}
}  global_init_term;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Class-
//       WorkerThread
//
// Purpose-
//       A Tread pool Worker Thread
//
//----------------------------------------------------------------------------
class WorkerThread : public Thread { // The WorkerThread Thread
friend class WorkerPool;

//----------------------------------------------------------------------------
// WorkerThread::Attributes
//----------------------------------------------------------------------------
protected:
bool                   operational; // TRUE while operational
std::mutex             mutex;       // State switch mutex
Worker*                worker;      // The current Worker

//----------------------------------------------------------------------------
// WorkerThread::Constructors/destructor
//----------------------------------------------------------------------------
public:
   WorkerThread(                    // Constructor
     Worker*           _worker= nullptr) // Associated Worker
:  Thread(), operational(true), mutex(), worker(_worker)
{  ++WorkerPool::new_workers;

   if( HCDM )
     traceh("WorkerThread(%p)!(%p)\n", this, worker);
   if( USE_ITRACE )
     Trace::trace(".WRK", "=NEW", this, worker);

   start(ITS_DETACHED);
}

virtual
   ~WorkerThread( void )            // Destructor
{  ++WorkerPool::del_workers;

   if( HCDM )
     traceh("WorkerThread(%p)~(%p)\n", this, worker);
   if( USE_ITRACE )
     Trace::trace(".WRK", "=DEL", this, worker);
}

//----------------------------------------------------------------------------
// WorkerThread::debug
//----------------------------------------------------------------------------
virtual void
   debug(const char* info= "") const
{
   {{{{ // The Debug lock provides sequential debugf outputs
     std::lock_guard<Debug> debug(*Debug::get());

     debugf("WorkerThread(%p)::debug(%s)\n", this, info);
     if( to_void(this) == nullptr ) {
       debugf("..no information available\n");
       return;
     }

     debugf("..worker(%p) operational(%s)\n", info, b2c(operational));
     Thread::debug(info);
   }}}}
}

//----------------------------------------------------------------------------
// WorkerThread::Accessors
//----------------------------------------------------------------------------
inline bool
   is_operational( void )
{  return operational; }

//----------------------------------------------------------------------------
// WorkerThread::done(): Handle work completion.
//----------------------------------------------------------------------------
inline void
   done( void )                      // Work complete
{  if( HCDM )
     traceh("WorkerThread(%p).done(%p)\n", this, worker);
   if( USE_ITRACE )
     Trace::trace(".WRK", "DONE", this, worker);

   WorkerThread* thread= this;
   unsigned now_used= 0;            // (Avoids exchange if pool_size unchanged)

   if( USE_IDEBUG && !operational ) { // This condition should never occur
     // This is an internal logic error. A non-operational thread is going to
     // or already has deleted itself. If this occurs, debugging is needed.
     debugh("%4d %s (invalid state) !operational\n", __LINE__, __FILE__);
     throwf("Worker: (invalid state) !operational");
   }

   {{{{ // PERFORMANCE CRITICAL ==============================================
     std::lock_guard<decltype(pool_mutex)> lock(pool_mutex);

     if( pool_used < pool_size ) {
       pool[pool_used++]= thread;
       thread= nullptr;
       now_used= pool_used;
     }
   }}}} // PERFORMANCE CRITICAL ==============================================

   if( thread )
     thread->stop();                // (Trace in stop indicates pool full)
   else if( USE_ITRACE )
     Trace::trace(".WRK", "POOL", this);

   size_t was_maxi= 0;              // (Avoids exchange if pool_size unchanged)
   while( now_used > was_maxi ) {
     if( WorkerPool::max_used.compare_exchange_weak(was_maxi, now_used) )
       break;
   }
}

//----------------------------------------------------------------------------
// WorkerThread::reuse()
//
// Call this function once for each reuse of the WorkerThread.
//----------------------------------------------------------------------------
void
   reuse(                           // Reuse this WorkerThread
     Worker*           worker)      // Using this Worker
{  if( HCDM )
     traceh("WorkerThread(%p).reuse(%p)\n", this, worker);
   if( USE_ITRACE )
     Trace::trace(".WRK", "=USE", this, worker);

   this->worker= worker;
   mutex.unlock();
}

//----------------------------------------------------------------------------
// (protected:) WorkerThread::run()
//
// Operate the WorkerThread.
//----------------------------------------------------------------------------
protected:
void
   run( void )                      // Operate the Thread
{  if( HCDM )                       // (Trace first iteration)
     traceh("WorkerThread(%p).run(%p)\n", this, worker);
   if( USE_ITRACE )
     Trace::trace(".WRK", "=RUN", this, worker);

   WorkerPool::inc_threads();

   while( operational ) {
     if( worker == nullptr ) {      // (Should not occur, but ignorable)
       if( USE_IDEBUG )
         debugh("%4d %s operational but NO WORKER\n", __LINE__, __FILE__);
     } else {
       WorkerPool::inc_running();
       try {
         worker->work();
       } catch(Exception& X) {
         debugh("WorkerException: %s\n", X.to_string().c_str());
         utility::report_exception(X.to_string());
       } catch(std::exception& X) {
         debugh("WorkerException: what(%s)\n", X.what());
         utility::report_exception(X.what());
       } catch(...) {
         debugh("WorkerException: ...\n");
         utility::report_exception("...");
       }
       WorkerPool::dec_running();
     }

     worker= nullptr;
     mutex.lock();
     done();
     {{{{
       std::lock_guard<std::mutex> lock(mutex);
     }}}}

     if( HCDM )
       traceh("WorkerThread(%p).post(%p)\n", this, worker);
     if( USE_ITRACE )
       Trace::trace(".WRK", "POST", this, worker);
   }

   if( HCDM )
     traceh("WorkerThread(%p).INOP(%p)\n", this, worker);
   if( USE_ITRACE )
     Trace::trace(".WRK", "INOP", this, worker);

   WorkerPool::dec_threads();
   delete this;
}

//----------------------------------------------------------------------------
// (protected:) WorkerThread::stop()
//
// Terminate Thread processing. [Only invoked from done().]
//----------------------------------------------------------------------------
virtual void
   stop( void )                     // Terminate processing
{  if( HCDM )
     traceh("WorkerThread(%p).stop(%p)\n", this, worker);
   if( USE_ITRACE )
     Trace::trace(".WRK", "STOP", this, worker);

   operational= false;
   mutex.unlock();
}
}; // class WorkerThread

//----------------------------------------------------------------------------
//
// Method-
//       WorkerPool::get_size (static attribute)
//       WorkerPool::get_used (static attribute)
//
// Purpose-
//       Getter: Get pool_size
//       Getter: Get pool_used
//
//----------------------------------------------------------------------------
size_t                              // The current thread pool size
   WorkerPool::get_size( void )     // Get current thread pool size
{  return pool_size; }

size_t                              // The current thread pool used count
   WorkerPool::get_used( void )     // Get current thread pool used count
{  return pool_used; }

//----------------------------------------------------------------------------
//
// Protected method-
//       WorkerPool::inc_running
//
// Purpose-
//       Increment the running count, possibly setting max_running
//
//----------------------------------------------------------------------------
void
   WorkerPool::inc_running( void )  // Increment the running count
{
   size_t was_running= ++running;
   size_t was_maximum= max_running.load();
   while( was_running > was_maximum ) {
     if( max_running.compare_exchange_weak(was_maximum, was_running) )
       break;
   }
}

//----------------------------------------------------------------------------
//
// Protected method-
//       WorkerPool::inc_threads
//
// Purpose-
//       Increment the threads count, possibly setting max_threads
//
//----------------------------------------------------------------------------
void
   WorkerPool::inc_threads( void )  // Increment the threads count
{
   size_t was_threads= ++threads;
   size_t was_maximum= max_threads.load();
   while( was_threads > was_maximum ) {
     if( max_threads.compare_exchange_weak(was_maximum, was_threads) )
       break;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WorkerPool::set_size (static attribute)
//
// Purpose-
//       Setter: Set the pool_size
//
//----------------------------------------------------------------------------
void
   WorkerPool::set_size(            // Set the thread pool size
     size_t            _size)       // The updated thread pool_size
{  std::lock_guard<decltype(pool_mutex)> lock(pool_mutex);

   for(unsigned i= 0; i<pool_used; i++) {
     WorkerThread* thread= pool[i];
     thread->stop();
     pool[i]= nullptr;              // (Not strictly necessary)
   }
   running.store(0);
   pool_used= 0;

   if( pool != (WorkerThread**)&static_pool )
     free(pool);

   pool_size= _size;
   if( pool_size <= MAX_THREADS )
     pool= (WorkerThread**)&static_pool;
   else {
     pool= (WorkerThread**)malloc(pool_size * sizeof(WorkerThread*));
   }
}

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
   WorkerPool::debug(               // Debugging display
     const char*       info,        // Caller information
     bool              detail)      // Add pooled thread information?
{
   debugf("WorkerPool::debug(%s)\n", info ? info : "");

   debugf("%'16zd max_pooled\n",  max_used.load());
   debugf("%'16zd max_running\n", max_running.load());
   debugf("%'16zd max_threads\n", max_threads.load());
   debugf("%'16zd running\n",     running.load());
   debugf("%'16zd threads\n",     threads.load());
   debugf("%'16zd new_workers\n", new_workers.load());
   debugf("%'16zd del_workers\n", del_workers.load());
   debugf("%'16zd workers\n",     workers.load());

   // Static local, protected by mutex
   debugf("%'16zd pool_size\n",   pool_size);
   debugf("%'16zd pool_used\n",   pool_used);

   if( detail ) {
     std::lock_guard<decltype(pool_mutex)> lock(pool_mutex);
     for(unsigned i= 0; i<pool_used; i++) {
       WorkerThread* thread= pool[i];
       debugf("\n");
       debugf("[%4d] %#.14zx\n", i, intptr_t(thread));
       thread->debug("(idle) WorkerThread");
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
{  std::lock_guard<decltype(pool_mutex)> lock(pool_mutex);

   for(unsigned i= 0; i<pool_used; i++) {
     WorkerThread* thread= pool[i];
     thread->stop();
   }
   pool_used= 0;

   // Reset the statistics
   del_workers.store(0);
   new_workers.store(0);
   max_running.store(0);
   max_threads.store(0);
   max_used.store(0);
   running.store(0);
   threads.store(0);
   workers.store(0);
}

//----------------------------------------------------------------------------
//
// Static method-
//       WorkerPool::work
//
// Purpose-
//       Drive the Worker, either reusing a pool Thread or creating a new one.
//
//----------------------------------------------------------------------------
void
   WorkerPool::work(                // Process work
     Worker*           worker)      // Using this Worker
{  ++workers;                       // (Invocation counter)

   if( HCDM )
     traceh("WorkerPool.work(%p) running(%zd)\n", worker, running.load());
   if( USE_ITRACE )
     Trace::trace(".WRK", "WORK", worker, running.load());

   WorkerThread* thread= nullptr;

   {{{{ // PERFORMANCE CRITICAL ==============================================
     std::lock_guard<decltype(pool_mutex)> lock(pool_mutex);

     if( pool_used > 0 )
       thread= pool[--pool_used];
   }}}} // PERFORMANCE CRITICAL ==============================================

   if( thread )
     thread->reuse(worker);
   else
     new WorkerThread(worker);
}
}  // namespace _LIBPUB_NAMESPACE
