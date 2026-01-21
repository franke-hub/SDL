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
//       2026/01/20
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
using std::atomic_uint;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum                                // Generic enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

// Production mode settings: USE_ITRACE= false;
,  USE_ITRACE= false                // Use internal trace?
}; // (Generic) enum

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class WorkerThread;                 // The Worker thread

//----------------------------------------------------------------------------
// Static attributes
//----------------------------------------------------------------------------
enum { MAX_THREADS= 16 };           // The default pool_size

static Latch           pool_mutex;  // Pool access mutex
static WorkerThread*   static_pool[MAX_THREADS]; // The built-in thread pool
static WorkerThread**  pool= (WorkerThread**)&static_pool; // The current thread pool

// Statistical counters
static atomic_size_t   del_workers(0); // The number of deleted workers
static atomic_size_t   new_workers(0); // The number of allocated workers
static atomic_size_t   max_running(0); // Maximum number of running threads
static atomic_uint     max_used(0); // Maximum number of pool threads
static atomic_size_t   running(0);  // Current number of running threads
static unsigned        pool_size= MAX_THREADS; // Size of thread pool
static unsigned        pool_used= 0;     // Current number of pool threads
static atomic_size_t   workers(0);  // Number of WorkerPool::work() invocations

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

   ++new_workers;

   if( false ) {                    // TODO: REMOVE
     // DIAGNOSTIC: Check for excess threads, report once
     //   MAX_THREADS limits the number of WorkerThreads in the reuse pool.
     //   The number of running WorkerThreads is not limited.
     //   This check was added because WorkerThreads were created in error
     //   in some sort of loop. The debugh is a convienent gdb breakpoint.
     size_t was_running= running.load();
     if( was_running > MAX_THREADS ) {
       if( true ) {                 // Report every occurance
         debugh("%4d %s INFO: %zd threads running\n", __LINE__, __FILE__
               , was_running);
       } else {                     // Report once
         static atomic_uint reported= 0;
         unsigned int was_reported= reported.load();
         while( was_reported == 0 ) {
           if( reported.compare_exchange_weak(was_reported, 1) ) {
             debugh("%4d %s INFO: %zd threads running\n", __LINE__, __FILE__
                   , was_running);
           }
         }
       }
     }
   }

   start();
}

virtual
   ~WorkerThread( void )            // Destructor
{  if( HCDM )
     traceh("WorkerThread(%p)~(%p)\n", this, worker);
   else if( USE_ITRACE )
     Trace::trace(".WRK", "=DEL", this, worker);

   ++del_workers;
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

   --running;
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


   unsigned was_maxi= 0;            // (Avoids load if pool_size unchanged)
   while( now_used > was_maxi ) {
     if( max_used.compare_exchange_weak(was_maxi, now_used) )
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
     if( worker ) {
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
       else debugh("%4d %s operational but NO WORKER\n", __LINE__, __FILE__); // TODO: REMOVE

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
// (Static) method-
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
// (Static) method-
//       WorkerPool::get_poolsize
//
// Purpose-
//       Accessor: pool_size
//
//----------------------------------------------------------------------------
unsigned                            // The WorkerPool size
   WorkerPool::get_size( void )     // Get WorkerPool size
{  return pool_size; }

//----------------------------------------------------------------------------
//
// Method-
//       WorkerPool::set_size
//
// Purpose-
//       Accessor: Set the pool_size
//
//----------------------------------------------------------------------------
void
   WorkerPool::set_size(            // Set the thread pool size
     unsigned          _size)       // The updated thread pool_size
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
     const char*       info)      // Caller information (adds thread list)
{
   debugf("WorkerPool::debug(%s)\n", info ? info : "");

   debugf("%'16zd max_running\n", max_running.load());
   debugf("%'16d max_pooled\n",   max_used.load());
   debugf("%'16zd running\n",     running.load());
   debugf("%'16d pool_size\n",    pool_size);
   debugf("%'16zd new_workers\n", new_workers.load());
   debugf("%'16zd del_workers\n", del_workers.load());
   debugf("%'16d pooled\n",       pool_used);
   debugf("%'16zd workers\n",     workers.load());

   if( info ) {
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

   // Reset the statistics
   max_running.store(0);
   max_used.store(0);
   running.store(0);
   pool_used= 0;
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
