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
//       2026/01/20
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_WORKER_H_INCLUDED
#define _LIBPUB_WORKER_H_INCLUDED

#include "pub/bits/pubconfig.h"     // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
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
//       Manage the Worker thread pool.
//
// Implementation notes-
//       All methods are static.
//
//       The Worker thread pool is a set of Worker threads saved for reuse
//       instead of being allocated and deleted for each work() invocation.
//
//       When work() is invoked, it runs under control of a Worker thread.
//       This Worker thread is either created (if the thread pool is empty)
//       or removed from the pool and reused.
//
//       When work() returns, if a pool slot is available the Worker thread
//       is added to the pool. If the pool is full, the thread is deleted.
//
//       Applications invoke work without consideration of whether the Worker
//       is created or reused. The number of pooled threads starts at zero
//       and only increases when when work() returns.
//
//       Invoking set_size(0) is valid. This both empties the current pool and
//       causes running Worker threads to be deleted when work() returns.
//
//----------------------------------------------------------------------------
class WorkerPool {
public:
//----------------------------------------------------------------------------
// WorkerPool::Methods
//----------------------------------------------------------------------------
static unsigned                      // The current number of running threads
   get_running( void );              // Get current number of running threads

static unsigned                      // The thread pool size
   get_size( void );                 // Get thread pool size

static void
   set_size(                         // Set the thread pool size
     unsigned          size);        // The updated thread pool size

static void
   debug(                            // Debugging display (statistics)
     const char*       info= nullptr); // Caller info (adds detail)

static void
   reset( void );                    // Reset (Empty) the WorkerThread pool

static void
   work(                             // Process work
     Worker*           worker);      // Using this Worker
}; // class WorkerPool
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_WORKER_H_INCLUDED
