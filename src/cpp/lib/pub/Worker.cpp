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
//       2026/01/22
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic<>
#include <mutex>                    // For std::lock_guard
#include <cstdlib>                  // For malloc, free

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Exception.h>          // For pub::Exception
#include "pub/Latch.h"              // For pub::Latch objects
#include <pub/Semaphore.h>          // For pub::Semaphore
#include "pub/Thread.h"             // For pub::Thread
#include "pub/Trace.h"              // For pub::Trace
#include "pub/Worker.h"             // For pub:: Worker, implemented
#include <pub/utility.h>            // For pub::utility::report_exception

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods
using pub::Trace;                   // For tracing methods
using std::atomic_size_t;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

// Production mode settings: USE_ITRACE= false;
,  USE_IDEBUG= true                 // Use internal debugging?
,  USE_ITRACE= false                // Use internal trace?
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
atomic_size_t          WorkerPool::max_used(0); // Maximum pool Thread count
atomic_size_t          WorkerPool::running(0);  // Running Thread count
atomic_size_t          WorkerPool::workers(0); // WorkerPool::work() invocations

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
//----------------------------------------------------------------------------
// WorkerThread::Attributes
//----------------------------------------------------------------------------
protected:
bool                   operational; // TRUE while operational
Semaphore              sem;         // State switch event Semaphore
Worker*                worker;      // The current Worker

//----------------------------------------------------------------------------
// WorkerThread::Constructors/destructor
//----------------------------------------------------------------------------
public:
   WorkerThread(                    // Constructor
     Worker*           worker= nullptr) // Associated Worker
:  Thread(), operational(true), sem(), worker(worker)
{  if( HCDM )
     traceh("WorkerThread(%p)!(%p)\n", this, worker);
   else if( USE_ITRACE )
     Trace::trace(".WRK", "=NEW", this, worker);

   ++WorkerPool::new_workers;
   start();
}

virtual
   ~WorkerThread( void )            // Destructor
{  if( HCDM )
     traceh("WorkerThread(%p)~(%p)\n", this, worker);
   else if( USE_ITRACE )
     Trace::trace(".WRK", "=DEL", this, worker);

   ++WorkerPool::del_workers;
}

//----------------------------------------------------------------------------
// WorkerThread::debug
//----------------------------------------------------------------------------
virtual void
   debug(const char* info= nullptr) const
{
   if( info == nullptr )
     info= "WorkerThread";

   debugf("WorkerThread(%p)::debug(%s) worker(%p) operational(%s)\n", this
         , info, worker, operational ? "true" : "false");
   sem.debug("WorkerThread.sem");
   Thread::debug(info);
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
   else if( USE_ITRACE )
     Trace::trace(".WRK", "DONE", this, worker);

   --WorkerPool::running;
   WorkerThread* thread= this;
   unsigned now_used= 0;

   if( operational )
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

   size_t was_maxi= 0;              // (Avoids load if pool_size unchanged)
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
   else if( USE_ITRACE )
     Trace::trace(".WRK", "=USE", this, worker);

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
{  if( HCDM )
     traceh("WorkerThread(%p).stop(%p)\n", this, worker);
   else if( USE_ITRACE )
     Trace::trace(".WRK", "STOP", this, worker);

   operational= false;
   sem.post();
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
   else if( USE_ITRACE )
     Trace::trace(".WRK", "=RUN", this, worker);

   while( operational ) {
     if( worker == nullptr ) {      // (Should not occur, but ignorable)
       if( USE_IDEBUG )
         debugh("%4d %s operational but NO WORKER\n", __LINE__, __FILE__);
     } else {
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
     }

     worker= nullptr;
     done();
     sem.wait();
     sem.reset();

     if( HCDM )
       traceh("WorkerThread(%p).post(%p)\n", this, worker);
     else if( USE_ITRACE )
       Trace::trace(".WRK", "POST", this, worker);
   }

   if( HCDM )
     traceh("WorkerThread(%p).INOP(%p)\n", this, worker);
   else if( USE_ITRACE )
     Trace::trace(".WRK", "INOP", this, worker);

   delete this;
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
   WorkerPool::debug(             // Debugging display
     const char*       info,      // Caller information
     bool              detail)    // Add pooled thread information?
{
   debugf("WorkerPool::debug(%s)\n", info ? info : "");

   debugf("%'16zd max_running\n", max_running.load());
   debugf("%'16zd max_pooled\n",  max_used.load());
   debugf("%'16zd running\n",     running.load());
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
   max_used.store(0);
   running.store(0);
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
   WorkerPool::work(                 // Process work
     Worker*           worker)       // Using this Worker
{  if( HCDM )
     traceh("WorkerPool.work(%p) running(%zd)\n", worker, running.load());
   else if( USE_ITRACE )
     Trace::trace(".WRK", "WORK", worker, i2v(running.load()));

   ++workers;
   size_t was_running= ++running;
   size_t was_maximum= max_running.load();
   while( was_running > was_maximum ) {
     if( max_running.compare_exchange_weak(was_maximum, was_running) )
       break;
   }

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
