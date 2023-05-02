//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
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
//       2018/01/01
//
// Implementation note-
//       The global Thread synchronization mutex is used to insure:
//         1) When a Thread starts, Thread::thread represents the running
//            thread BEFORE thread->run invocation.
//         2) When a running Thread terminates, thread.detach() is only
//            called once. The thread.detach() method is idempotent.
//
//----------------------------------------------------------------------------
#include <mutex>                    // Used by Thread::start() sequence
#include <sstream>                  // Used by get_id_string

#include "obj/Thread.h"

namespace _OBJ_NAMESPACE {
//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::mutex      mutex;       // Global Thread synchronization mutex
const std::thread::id  Thread::null_id; // Null Thead id

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
   {{{{
     std::lock_guard<std::mutex> lock(mutex); // Insure move complete
   }}}}

   thread->run();
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::~Thread
//
// Purpose-
//       Destructor
//
// Implementaiton note-
//       The Thread constuctor is inline
//
//----------------------------------------------------------------------------
   Thread::~Thread( void )          // Destroy this Thread
{
   detach();
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
{
   std::lock_guard<std::mutex> lock(mutex);

   if( thread.joinable() )
     thread.detach();
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::get_id_string
//
// Purpose-
//       std::thread::id.to_string()
//
//----------------------------------------------------------------------------
std::string                         // Associated string
   Thread::get_id_string(           // Represent std::thread::id as a string
     const id&         thread_id)   // The thread id
{
   std::stringstream ss;
   ss << thread_id;
   return ss.str();
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
   std::lock_guard<std::mutex> lock(mutex);

   std::thread t(thread_start, this);
   thread= std::move(t);
}
} // namespace _OBJ_NAMESPACE

