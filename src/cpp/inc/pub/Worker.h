//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2026 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
// SPDX-License-Identifier: LGPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Worker.h
//
// Purpose-
//       Define a Worker used to handle discrete units of work.
//
// Last change date-
//       2026/02/09
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_WORKER_H_INCLUDED
#define _LIBPUB_WORKER_H_INCLUDED

#include <atomic>                   // For std::atomic<>, std::atomic_size_t

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Forward reference-
//       WorkerThread
//
// Purpose-
//       The WorkerThread is an internal Thread used to invoke Worker.work().
//
//----------------------------------------------------------------------------
class WorkerThread;                 // The Worker thread

//----------------------------------------------------------------------------
//
// Interface class-
//       Worker
//
// Purpose-
//       The Worker interface.
//
//----------------------------------------------------------------------------
class Worker {                      // The Worker interface
public:
virtual void                        // ** OVERRIDE THIS METHOD **
   work( void ) = 0;                // The Worker method
}; // class Worker

//----------------------------------------------------------------------------
//
// (Static) class-
//       WorkerPool
//
// Purpose-
//       Manage the WorkerThread pool.
//
// Implementation notes-
//       All methods are static.
//
//       The WorkerThread pool is a set of WorkerThreads saved for reuse
//       instead of being allocated and deleted for each work() invocation.
//
//       The number of pooled WorkerThreads starts at zero. There are no
//       pre-allocated WorkerThreads.
//
//       When work() is invoked, it runs under control of a WorkerThread.
//       This WorkerThread is either created (if the thread pool is empty)
//       or removed from the pool and reused.
//
//       When work() returns, if a pool slot is available the WorkerThread
//       is added to the pool. If the pool is full, the Thread is deleted.
//
//       Invoking set_size(0) is valid. This both empties the current pool and
//       causes running WorkerThreads to be deleted when work() returns.
//       The set_size method is thread-safe and can be invoked at any time.
//
//----------------------------------------------------------------------------
class WorkerPool {
friend class WorkerThread;

//----------------------------------------------------------------------------
// WorkerPool::Attributes
//----------------------------------------------------------------------------
typedef std::atomic_size_t          atomic_size_t;

static atomic_size_t   del_workers; // The number of deleted WorkerThreads
static atomic_size_t   new_workers; // The number of created WorkerThreads
static atomic_size_t   max_running; // Maximum number of running WorkerThreads
static atomic_size_t   max_used;    // Maximum number of pooled WorkerThreads
static atomic_size_t   running;     // Current number of running WorkerThreads
static atomic_size_t   workers;     // Number of WorkerPool::work() invocations

//----------------------------------------------------------------------------
// WorkerPool::Accessors
//----------------------------------------------------------------------------
public:
static size_t                       // The maximum number of running Workers
   get_max_running( void )          // Get maximum number of running Workers
{  return max_running.load(); }

static size_t                       // The current thread pool count
   get_max_used( void )             // Get current thread pool count
{  return max_used.load(); }

static size_t                       // The current number of running Workers
   get_running( void )              // Get current number of running Workers
{  return running.load(); }

static size_t                       // The current thread pool size
   get_size( void );                // Get current thread pool size

static size_t                       // The current thread pool used count
   get_used( void );                // Get current thread pool used count

static size_t                       // The number of Workers created
   get_worker_creates( void )       // Get number of Workers created
{  return new_workers.load(); }

static size_t                       // The number of Workers deleted
   get_worker_deletes( void )       // Get number of Workers deleted
{  return del_workers.load(); }

static size_t                       // The work invocation count
   get_workers( void )              // Get work invocation count
{  return workers.load(); }

static void
   set_size(                        // Set the thread pool size
     size_t            size);       // The updated thread pool size

//----------------------------------------------------------------------------
// WorkerPool::Methods
//----------------------------------------------------------------------------
static void
   debug(                           // Debugging display (statistics)
     const char*       info= "",    // Caller info
     bool              detail= false); // Add pooled thread information?

static void
   reset( void );                   // Reset (Empty) the WorkerThread pool

static void
   work(                            // Process work
     Worker*           worker);     // Using this Worker
}; // class WorkerPool
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_WORKER_H_INCLUDED
