//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2022 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Worker.h
//
// Purpose-
//       Define a Worker used to handle discrete units of work.
//
// Last change date-
//       2022/09/02
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_WORKER_H_INCLUDED
#define _LIBPUB_WORKER_H_INCLUDED

#include <pub/bits/pubconfig.h>     // For _LIBPUB_ macros

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
// Class-
//       WorkerPool
//
// Purpose-
//       Manage the Worker thread pool.
//
// Implementation note-
//       Applications may use any number of Threads.
//       The maximum number of WorkerThreads pooled for later re-use is
//       implementation defined.
//
//----------------------------------------------------------------------------
class WorkerPool {
//----------------------------------------------------------------------------
// WorkerPool::Methods
//----------------------------------------------------------------------------
public:
static unsigned                      // The current number of running threads
   get_running( void );              // Get current number of running threads

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
