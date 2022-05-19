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
//       2022/05/06
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::lock_guard

#include <pub/Debug.h>              // For debugging
#include <pub/Latch.h>              // For mutex substitute
#include <pub/Semaphore.h>
#include <pub/Thread.h>

#include "pub/Worker.h"
using namespace _PUB_NAMESPACE::debugging;

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

namespace _PUB_NAMESPACE {
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
static unsigned        max_running= 0; // Maximum number of running threads
// tic unsigned        max_size= MAX_THREADS; // Maximum size of thread pool
static unsigned        max_used= 0; // Maximum number of used threads
static unsigned        running= 0;  // Current number of running threads
static const unsigned  size= MAX_THREADS; // Size of thread pool
static unsigned        used= 0;     // Current number of  used threads
static unsigned        workers= 0;  // Number of WorkerPool::work() invocations

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
// WorkerThread::done()
//
// Handle work completion.
//----------------------------------------------------------------------------
static inline void
   done(                             // Work complete
     WorkerThread*     thread)       // For this WorkerThread
{
   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     running--;

     if( used < size ) {
       pool[used++]= thread;
       if( used > max_used )
         max_used= used;
       thread= nullptr;
     }
   }}}}

   if( thread ) {
     thread->detach();
     thread->stop();
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
       } catch(pub::Exception& X) {
         debugging::debugh("WorkerException: %s\n", X.to_string().c_str());
       } catch(std::exception& X) {
         debugging::debugh("WorkerException: what(%s)\n", X.what());
       } catch(...) {
         debugging::debugh("WorkerException: ...\n");
       }
     }
     worker= nullptr;

     done(this);
     sem.wait();
   }
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
#if 0 // Implementation not exposed ==========================================
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
#endif // Implementation not exposed =========================================

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
   WorkerPool::debug( void )        // Debugging display
{
   debugf("WorkerPool::debug()\n");

   setlocale(LC_NUMERIC, "");       // Activates ' thousand separator
   debugf("%'16d max_running\n", max_running);
// debugf("%'16d max_size\n",    max_size);
   debugf("%'16d max_used\n",    max_used);
   debugf("%'16d running\n",     running);
// debugf("%'16d size\n",        size);
   debugf("%'16d used\n",        used);
   debugf("%'16d workers\n",     workers);
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
   max_running= 0;
   max_used= 0;
   running= 0;
   used= 0;
   workers= 0;
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
   WorkerThread* thread= nullptr;

   {{{{
     std::lock_guard<decltype(mutex)> lock(mutex);

     workers++;
     running++;
     if( running > max_running )
       max_running= running;

     if( used > 0 )
       thread= pool[--used];
   }}}}

   if( thread )
     thread->drive(worker);
   else
     new WorkerThread(worker);
}
}  // namespace _PUB_NAMESPACE
