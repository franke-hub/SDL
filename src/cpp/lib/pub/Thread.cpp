//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2026 Frank Eskesen.
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
//       Thread.cpp
//
// Purpose-
//       Thread method implementations.
//
// Last change date-
//       2026/03/09
//
// Implementation notes-
//       Thread::tlss is used to maintain the Thread state. There are three
//       tlss pointers:
//         tlss* _tlss; (A stack tlss* copy)
//         tlss* Thread::tlss_; (For Thread access to the tlss)
//         static __thread tlss* tl_tlss; (For Thread-local access)
//
//       Thread::latch only protects Thread::tlss_. When using Thread::tlss_
//       to locate the tlss, this latch must be held before obtaining the
//       Thread::tlss::mutex.
//
//       Thread::tlss::mutex protects Thread::tlss content.
//
//       Thread::current() uses tl_tlss to find the running Thread. It does
//       not obtain Thread::tlss::mutex.
//
// Implementation notes (Thread::join)-
//       Note that detach and join are mutually exclusive operations.
//       Since join does not return until Thread completion, a running thread
//       is not permitted to join itself. (A deadlock would occur.)
//
// Implementation notes (Thread::detach and Thread::~Thread)-
//       Running Threads *can* delete themselves but, while a Thread is
//       running, Thread delete from a different thread is not allowed.
//       When a running Thread self-deletes, if in a joinable state it also
//       performs a self-detach.
//
//       Similarly, running Threads *can* detach themselves, but detach from
//       a different Thread is never permitted. Threads that complete without
//       detaching first must be joined before they are deleted. Thread
//       delete
//
//
// Implementation notes (Thread::cancel)-
//       Thread::cancel is not implemented. Within the pub library, derived
//       classes generally use a stop() method to terminate Threads.
//       See Worker.cpp, WorkerThread::stop for a sample implementation.
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic<> statistics
#include <chrono>                   // For std::chrono::microseconds
#include <mutex>                    // For std::lock_guard, mutex
#include <stdexcept>                // For std::runtime_error
#include <string>                   // For std::string
#include <cerrno>                   // For errno, perror
#include <cstring>                  // For strerror, ...

#include <pub/Debug.h>              // For debugging
#include "pub/Event.h"              // For pub::Yield_event
#include <pub/Exception.h>          // For debugging
#include "pub/Latch.h"              // For pub::Latch, pub::RecursiveLatch
#include <pub/Named.h>              // For pub::Named
#include "pub/Semaphore.h"          // For pub::Semaphore (max_threads)
#include "pub/System.h"             // For namespace pub::System
#include "pub/Thread.h"             // For pub::Thread, implemened
#include "pub/Trace.h"              // For pub::Trace
#include "pub/utility.h"            // For pub::utility::to_null
#include "pub/utility.i"            // For conversion subroutines

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods
using std::atomic_size_t;           // For convenience
using std::string;                  // For convenience
using PUB::utility::to_void;        // For if( to_void(this) == nullptr )
using PUB::System::log;             // For convenience

typedef PUB::Thread    Thread;      // For convenience
typedef Thread::tlss   tlss;        // For convenience
typedef pthread_t      handle_t;    // For convenience

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
// Production mode settings: HCDM= false; VERBOSE= 0;
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

// Production mode settings: USE_ICHECK= true; USE_ITRACE= false
,  USE_ICHECK= true                 // Use internal self-checking?
,  USE_ITRACE= false                // Use internal trace?

// Import System Log Level values   // System::log(level) meaning
,  LL_NONE=  System::LL_NONE        // (Log to stdout if log_level == LL_NONE)
,  LL_INFO=  System::LL_INFO        // (Log to stdout if log_level <= LL_INFO)
,  LL_ERROR= System::LL_ERROR       // (Log to stdout if log_level <= LL_ERROR)
,  LL_HCDM=  System::LL_HCDM        // (Log to stdout if log_level <= LL_HCDM)
,  LL_ALL=   System::LL_ALL         // (Always log to stdout)
}; // generic enum

enum FSM                            // The tlss Finite State Machine states
{  FSM_START= 0                     // 0 START, initial state
,  FSM_DRIVE                        // 1 DRIVE, "Owned" by Thread::drive
,  FSM_OWNER                        // 2 OWNER, "Owned" by Thread's owner
,  FSM_DETACHED                     // 3 DETACHED, "Owned" by Thread::drive
,  FSM_JOINING                      // 4 JOINING, "Owned" by Thread's owner
,  FSM_COUNT                        // 5 COUNT, Number of states
}; // enum FSM

enum                                // Generic enum
{  MAX_THREADS= 8192                // Default max_threads count
}; // Generic enum

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
size_t                 Thread::max_threads= MAX_THREADS;
const handle_t         Thread::null_handle{}; // Native handle

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static __thread tlss*  tl_tlss= nullptr; // Thread-local storage (pointer)

static pthread_attr_t  attr_detached; // Detached attribute
static pthread_attr_t  attr_joinable; // Joinable attribute

static Semaphore       startable(MAX_THREADS);

// Statistics
static atomic_size_t   complete= 0; // Number of complete Threads
static atomic_size_t   detached= 0; // Number of detached Threads
static atomic_size_t   max_run= 0;  // Maximum running Thread count
static atomic_size_t   running= 0;  // Number of running Threads
static atomic_size_t   started= 0;  // Number of started Threads

//----------------------------------------------------------------------------
// Global initialization/termination
//----------------------------------------------------------------------------
namespace {                         // Anonymous namespace
[[noreturn]]
static void
   sno(                             // Should Not Occur handler
     int               line,        // Failing line number
     const char*       op,          // Operation name
     int               rc)          // Failing return code
{
   debugh("%4d %s %d=%s ERR: %d:%s\n", line, __FILE__, rc, op
         , errno, strerror(errno));
   exit(EXIT_FAILURE);
}

static struct StaticGlobal {
   StaticGlobal( void )             // Static constructor
{  if( HCDM ) traceh("Thread::StaticGlobal!\n");

   int
   rc= pthread_attr_init(&attr_detached);
   if( rc ) sno(__LINE__, "pthread_attr_init", rc);
   rc= pthread_attr_setdetachstate(&attr_detached, PTHREAD_CREATE_DETACHED);
   if( rc ) sno(__LINE__, "pthread_setdetachstate", rc);

   rc= pthread_attr_init(&attr_joinable);
   if( rc ) sno(__LINE__, "pthread_attr_init", rc);
}

   ~StaticGlobal( void )            // Static destructor
{  if( HCDM ) traceh("Thread::StaticGlobal~\n");

   int
   rc= pthread_attr_destroy(&attr_detached);
   if( rc ) sno(__LINE__, "pthread_attr_destroy", rc);

   rc= pthread_attr_destroy(&attr_joinable);
   if( rc ) sno(__LINE__, "pthread_attr_destroy", rc);
}
}  static_global;
}  // Anonymous namespace

//----------------------------------------------------------------------------
//
// Subroutine-
//       f2c
//
// Purpose-
//       Convert FSM number to name
//
//----------------------------------------------------------------------------
static const char*                  // The FSM name
   f2c(int fsm)                     // The FSM number
{
   const char* name= "UNDEF";       // Default, undefined
   switch( fsm ) {
     case FSM_START:                // Thread::start running
       name= "START";
       break;

     case FSM_DRIVE:                // Thread::drive running
       name= "DRIVE";               // Shared tlss ownership
       break;

     case FSM_OWNER:                // Thread detach/join must delete tlss
       name= "OWNER";               // (Only set in Thread::drive)
       break;

     case FSM_DETACHED:             // Thread drive must delete tlss
       name= "DETACHED";            // (Only set in Thread::detach)
       break;

     case FSM_JOINING:              // Thread join must delete tlss
       name= "JOINING";             // (Only set in Thread::join)
       break;

     default:
       break;
   }

   return name;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       l2c
//
// Purpose-
//       Get Latch state: "free" or "held"
//
//----------------------------------------------------------------------------
static inline const char*           // "free" || "held"
   l2c(                             // Get latch state
     const RecursiveLatch&
                       latch)       // For this latch
{  if( latch.is_held() )
     return "held";

   return "free";
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
static inline const char*           // "free" || "held"
   l2c(                             // Get latch state
     const Latch&      latch)       // For this latch
{  if( latch.is_held() )
     return "held";

   return "free";
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::tlss::tlss
//       Thread::tlss::~tlss
//
// Purpose-
//       Constructor
//       Destructor
//
//----------------------------------------------------------------------------
   Thread::tlss::tlss(              // Constructor
     Thread*           _thread)     // Associated Thread
:  thread(_thread)
{  if( HCDM )
     traceh("Thread::tlss(%p)!(%p)\n", this, thread);
   if( USE_ITRACE )
     Trace::trace(".NEW", "TLSS", this, thread);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   Thread::tlss::~tlss( void )      // Destructor
{  if( HCDM )
     traceh("Thread::tlss(%p)~\n", this);
   if( USE_ITRACE )
     Trace::trace(".DEL", "TLSS", this, thread);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::tlss::debug
//
// Purpose-
//       Display debugging information
//
//----------------------------------------------------------------------------
void
   Thread::tlss::debug(             // Display debugging information
     const char*       info) const  // (Optional) caller information
{  std::lock_guard<Debug> debug(*Debug::get());

   debugf("Thread::tlss(%p)::debug(%s)\n", this, info);

   debugf("..mutex(%s) fsm(%s) thread(%p) handle(0x%zx)\n"
         , l2c(mutex), f2c(fsm), thread, intptr_t(handle));
   debugf("..drive_initialized(%d) start_finalized(%d)\n"
          , drive_initialized.latch.load(), start_finalized.latch.load());
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::tlss::set_fsm
//
// Purpose-
//       Update the state
//
// Implementation notes-
//       Caller is responsible for any required locking.
//
//----------------------------------------------------------------------------
void
   Thread::tlss::set_fsm(           // Update the state
     int               _fsm)        // The new FSM
{  if( HCDM && VERBOSE > 0 )
     traceh("Thread(%p)::tlss(%p)::set_fsm %s=>%s\n", thread, this
           , f2c(fsm), f2c(_fsm));

   if( USE_ITRACE ) {
     // void* voidptr= (void*)(uintptr_t(fsm) << 32 | _fsm);
     uintptr_t uintptr= uintptr_t(fsm) << 32 | _fsm;
     Trace::trace(".THR", "=FSM", thread, uintptr);
   }

   fsm= _fsm;
}

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
{  if( HCDM )
     traceh("Thread(%p)!\n", this);
   if( USE_ITRACE )
     Trace::trace(".NEW", "=THR", this);
}

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
{  if( HCDM )
     traceh("Thread(%p)~ tlss_(%p)\n", this, this->tlss_);

   if( USE_ITRACE )
     Trace::trace(".DEL", "=THR", this, this->tlss_);

   // Thread termination error recovery, avoiding most aborts
   tlss* _tlss= this->tlss_;
   if( _tlss ) {                    // If the thread is still in-use
     // It is *always* a user error to delete a Thread that's still in-use.
     // We *always* document the user error with a System::log message.
     if( this == current() ) {      // If user is running on this Thread
       // OK: detach isn't allowed from any Thread but the current Thread
       // and it's running here. (It can't also be running anywhere else.)
       log(LL_ERROR, "Thread(%p)::~Thread auto-detach\n", this);
       detach();
     } else {                       // If user didn't already join
       // This lock_guard prevents others from changing Thread::tlss_
       std::lock_guard<decltype(this->tlss_latch)> outer(this->tlss_latch);
       _tlss= this->tlss_;          // (Get the latch-protected tlss_)
       if( _tlss == nullptr ) {     // If the join already completed
         // Good news, join completed! We don't have to do anything else.
         log(LL_ERROR, "Thread(%p)::~Thread active join completed\n", this);
       } else {                     // The join hasn't completed
         // Bad news! We've got problems.
         // We'll get the tlss::mutex and figure them out. Note that it's
         // possible that join might eventually run on a different Thread.
         // (If it does, it's going to be using destructed storage.)

         int fsm;                   // FSM, not set until tlss::mutex held
         {{{{
           std::lock_guard<decltype(_tlss->mutex)> inner(_tlss->mutex);

           fsm= _tlss->fsm;         // Obtain the current state

           // This recovery sequence will only succeed if the application just
           // forgot to issue join. If the application does issue join it's
           // going to reference destructed and possibly freed storage,
           // resulting in uncontrolled and unpredictable behavior.

           // We're going to pthread_join here with Thread::tlss_latch and
           // tlss::mutex held, logging the result.
           int rc= pthread_join(_tlss->handle, nullptr);
           if( rc == 0 )            // We succeded. The join method shouldn't.
             log(LL_ERROR, "Thread(%p)::~Thread auto-join\n", this);
           else
             log(LL_ERROR, "Thread(%p)::~Thread auto-join error %d:%s\n", this
                         , errno, strerror(errno));

           _tlss->thread= nullptr; // The Thread is no longer current
         }}}} // (End of scope: tlss::mutex)

         // We're still holding Thread::tlss_latch, but we couldn't delete the
         // tlss while holding tlss::mutex
         if( fsm != FSM_OWNER )     // If unexpected state
           log(LL_INFO, "Thread(%p)::~Thread auto-join unexpected fsm(%s)\n"
                      , this, f2c(fsm));

         delete _tlss;
         startable.post();          // The Thread completed
       }
     } // (End of scope: Thread::tlss_latch)
   } else if( tl_tlss && tl_tlss->thread == this ) { // (Special case check)
     // The current Thread (which must be detached) invoked the destructor
     if( USE_ITRACE )               // Current Thread Destructor
       Trace::trace(".THR", "=CTD", this, tl_tlss);

     tl_tlss->thread= nullptr;      // It's no longer current
   }

   if( USE_ITRACE )                 // (Thread delete exit)
     Trace::trace(".DEL", "<THR", this, this->tlss_);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::debug
//       Thread::static_debug
//
// Purpose-
//       Debugging displays
//
//----------------------------------------------------------------------------
void
   Thread::debug(                   // Debugging display
     const char*       info) const  // Caller information
{
   {{{{ // The Debug lock provides sequential debugf outputs
     std::lock_guard<Debug> debug(*Debug::get());

     debugf("Thread(%p)::debug(%s)\n", this, info);
     if( to_void(this) == nullptr ) {
       debugf("..no information available\n");
       return;
     }

     std::string name= "";
     const Named* named= dynamic_cast<const Named*>(this);
     if( named )
       debugf("..named(%s)\n", s2c(named->get_name()));

     debugf("..tlss_latch(%s) joinable(%d) tlss_(%p)\n"
           , l2c(this->tlss_latch), joinable(), this->tlss_);
     if( this->tlss_ )
       this->tlss_->debug(info);
   }}}}
}

void
   Thread::static_debug(            // Debugging display
     const char*       info)        // Caller information
{
   {{{{
     std::lock_guard<Debug> debug(*Debug::get());

     debugf("Thread::static_debug(%s)\n", info);
     debugf("%'16zd max_running\n", max_run.load());

     debugf("%'16zd started\n",  started.load());
     debugf("%'16zd complete\n", complete.load());
     debugf("%'16zd running\n",  running.load());
     debugf("%'16zd detached\n", detached.load());
   }}}}
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::joinable
//
// Purpose-
//       Is this thread joinable?
//
//----------------------------------------------------------------------------
bool                                // TRUE if this thread is joinable
   Thread::joinable( void ) const   // Is this thread joinable?
{
   std::lock_guard<decltype(this->tlss_latch)> outer(this->tlss_latch);

   tlss* _tlss= this->tlss_;
   if( _tlss ) {                    // If already joined or detached
     std::lock_guard<decltype(tlss::mutex)> inner(_tlss->mutex);

     switch(_tlss->fsm) {
       case FSM_DRIVE:              // Thread is running
       case FSM_OWNER:              // Thread completed
         return true;

       // If in ~Thread destructor recovery, a message is warranted
       // case FSM_DELETE:          // ~Thread invoked, should not occur

       // Once detached, this->tlss_ == nullptr and FSM_DETACHED is implied
       // case FSM_DETACHED:        // Thread already detached
       case FSM_JOINING:            // (Can't be joined again, so FALSE)
         return false;

       default:
         debugf("Thread(%p) invalid fsm(%s)\n", this, f2c(this->tlss_->fsm));
         break;
     }
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::set_max_threads
//
// Purpose-
//       Set the maximum Thread count
//
// Implementation note-
//       When invoked with running Threads, the maximum thread count and the
//       Semaphore can get slightly out of synch. (This hasn't been tested.)
//
//----------------------------------------------------------------------------
void
   Thread::set_max_threads(         // Set the maximum Thread count
     size_t            count)       // The new maximum Thread count
{
   static std::mutex   mutex;       // (Only guards this method)
   std::lock_guard<decltype(mutex)> lock(mutex);

   if( count > max_threads ) {      // If increasing the count
     size_t more= count - max_threads;
     startable.reset(startable.get_count() + more);
   } else if( count < max_threads ) { // If decreasing the count
     size_t less= max_threads - count;
     size_t have= startable.get_count();
     size_t want= have - less;
     if( have < less )
       want= 0;
     startable.reset(want);
   }

   max_threads= count;
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::current
//
// Purpose-
//       Get the current Thread
//
// Implementation notes-
//       If running in a thread not controlled by pub::Thread,
//       tl_tlss will always be null (and nullptr will be returned).
//
//----------------------------------------------------------------------------
Thread*                             // The current Thread
   Thread::current( void )          // Get current Thread
{  tlss* _tlss= tl_tlss; return _tlss ? _tlss->thread : nullptr; }

//----------------------------------------------------------------------------
//
// Method-
//       Thread::detach
//
// Purpose-
//       Detach execution thread from this object
//
// Implementation notes-
//       Detach may be only be called (once) from the running thread.
//       A thread that completes without being detached must be joined.
//
//----------------------------------------------------------------------------
void
   Thread::detach( void )           // Detach excution thread from this object
{  if( HCDM )
     traceh("Thread(%p)::detach tl_tlss(%p)\n", this, tl_tlss);
   if( USE_ITRACE )
     Trace::trace(".THR", "=DET", this, this->tlss_);

   // The tlss_latch prevents interaction with join and possibly ~Thread
   std::lock_guard<decltype(this->tlss_latch)> outer(this->tlss_latch);
   Thread* thread= current();
   if( this != thread )
     throwf("Thread(%p)::detach but current(%p)", this, thread);
   if( this->tlss_ == nullptr )
     throwf("Thread(%p)::detach but already joined or detached", this);

   tlss* _tlss= tl_tlss;            // Local copy of tl_tlss
   if( USE_ICHECK && this->tlss_ != _tlss ) // (Should not occur)
     throwf("Thread(%p)::detach exception, tlss_!=tl_tlss\n", this);

   {{{{
     std::lock_guard<decltype(tlss::mutex)> inner(_tlss->mutex);

     int fsm= _tlss->fsm;
     if( fsm != FSM_DRIVE ) {
       if( USE_ITRACE )
         Trace::trace(".THR", "DREJ", this, fsm);
       throwf("Thread(%p)::detach rejected, FSM(%s)\n", this, f2c(fsm));
     }

     // At this point the Thread should be detachable. If we're wrong, the
     // detach error usually implies a problem somewhere in Thread.cpp.
     int rc= pthread_detach(_tlss->handle);
     if( rc ) {
       // This detach error usually implies a logic error in Thread.cpp
       // We write a log message and pretend that the detach succeeded
       if( USE_ITRACE )
         Trace::trace(".THR", "DERR", this, i2i(fsm)<<32 | rc);

       log(LL_ALL, "Thread(%p)::detach pthread(0x%zx) error %d:%s\n", this
                 , intptr_t(_tlss->handle), rc, strerror(rc));
     }

     _tlss->set_fsm(FSM_DETACHED); // Thread::drive owns the tlss
     ++detached;                  // Thread::drive will decrement this
     this->tlss_= nullptr;        // Thread longer owns or references the tlss
   }}}} // (End of scope: lock_guard tlss::mutex)

   if( USE_ITRACE )
     Trace::trace(".THR", "<DET", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::join
//
// Purpose-
//       Wait for this thread to complete
//
//----------------------------------------------------------------------------
void
   Thread::join( void )             // Wait for this thread to complete
{  if( HCDM )
     traceh("Thread(%p)::join tlss_(%p)->fsm(%s)\n", this, this->tlss_
           , this->tlss_ ? f2c(this->tlss_->fsm) : "N/A" );

   if( USE_ITRACE )
     Trace::trace(".THR", "JOIN", this, this->tlss_);

   if( USE_ICHECK ) {               // (We don't need the tlss_latch here)
     if( current() == this ) {
       log(LL_ERROR, "Thread(%p)::join invoked by this thread\n", this);
       errno= EDEADLK;              // DEADLOCK error
       return;
     }
   }

   tlss* _tlss= nullptr;            // (Not valid yet. Set under lock_guard.)

   {{{{
     std::lock_guard<decltype(this->tlss_latch)> outer(this->tlss_latch);

     _tlss= this->tlss_;
     if( _tlss == nullptr )         // If Thread detached/joined
       throwf("Thread(%p) Invalid join state: inactive", this);

     std::lock_guard<decltype(tlss::mutex)> inner(_tlss->mutex);

     int fsm= _tlss->fsm;
     if( fsm == FSM_DRIVE )         // If the Thread's still running
       _tlss->set_fsm(FSM_JOINING);
     else if( fsm != FSM_OWNER ) {  // If the Thread hasn't completed
       if( USE_ITRACE )
         Trace::trace(".THR", "JREJ", this, fsm);

       throwf("Thread(%p)::join rejected, FSM(%s)\n", this, f2c(fsm));
     }
   }}}}

   // (We're not holding either Thread::tlss_latch or tlss:mutex)
   // We now own the tlss and we'll delete it when we're done with it.
   int rc= EFAULT;                  // Default, bad address
   rc= pthread_join(_tlss->handle, nullptr);
   if( rc == 0 ) {                  // If successful
     // If ~Thread is invoked while we were joining, it's possible that the
     // Thread doesn't even exist now and we're about to reference deleted
     // storage. A System::log message was written to alert us.
     std::lock_guard<decltype(this->tlss_latch)> lock(this->tlss_latch);

     // Join won't successfully complete until Thread::drive completes.
     if( USE_ICHECK && _tlss->fsm != FSM_OWNER )
       throwf("Thread(%p) fsm(%s) != FSM_OWNER", this, f2c(_tlss->fsm));

     if( USE_ITRACE )               // Join normal exit
       Trace::trace(".THR", "JXIT", this, _tlss);

     delete _tlss;                  // Delete the tlss
     this->tlss_= nullptr;          // (Allow Thread reuse)
     startable.post();              // (Thread complete)
     return;
   }

   // If a pthread_join error occurs, we don't know the Thread state.
   // If it occurred because of an interaction with ~Thread, we don't even
   // know whether the Thread still exists, so we'll assume it doesn't.
   // This log message indicates that we're now in an unknown error state.
   log(LL_ALL, "Thread(%p)::join ERROR %d:%s SHOULD NOT OCCUR\n", this
             , rc, strerror(rc));

   // Since we assume that the Thread doesn't exist, we won't modify it.
   // (This leaves existing Threads in a state where they can't be reused.)
   // thread->tlss_= nullptr; <<<<<<<< THIS STATEMENT PURPOSELY COMMENTED OUT

   // Since we didn't try to delete the tlss, it may never get deleted.
   // startable.post();             // We don't know if the Thread completed

   if( USE_ITRACE )                 // Join error exit
     Trace::trace(".THR", "JERR", this, rc);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::run
//
// Purpose-
//       Run is a pure virtual function and not implemented here.
//       If it's not implemented in a subclass, the compiler and/or the system
//       loader detect the error.
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//
// Method-
//       Thread::sleep
//
// Purpose-
//       Delay the current thread
//
// Implementation notes-
//       Delays greater than LONG_LONG_MAX / 1'000'000 won't work properly.
//
//----------------------------------------------------------------------------
void
   Thread::sleep(                   // Delay the current Thread
     double            seconds)     // For this many seconds
{
   if( seconds <= 0.0000005 )       // If very short delay
     return;                        // Don't bother

   // (Using microsecond resolution, rounded up)
   int64_t us= int64_t((seconds+0.0000005) * 1'000'000); // (Rounded up)
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
// Implementation notes-
//       Synchronization between start() and drive() is tricky:
//
//       We need Thread::tlss_ initialized here in start() because we're using
//       tlss::drive_initialized and tlss::start_finalized here and in drive.
//
//       We can't set (thread local) tl_tlss until running drive() since
//       before that we aren't actually running under the new thread.
//
//       We can't let start() exit until the tl_tlss is set, because current()
//       won't work. Ergo, method start() must wait for drive_initialized.
//       We discovered, however, that this wasn't enough.
//
//       We can't let start() continue after drive() posts drive_initialized,
//       because the Thread can run to completion and delete the tlss before
//       start()'s drive_initialized.wait() completes. If this should occur,
//       _tlss->drive_initialized.wait() refers to undefined storage. Bad!
//       >>>>>>>>>>>>>>>>>>>>>>>>>> THIS WAS A BUG. <<<<<<<<<<<<<<<<<<<<<<<<<<
//
//       To prevent this, we added the start_finalized event to the tlss.
//       Method start invokes start_finalized.post as its final reference to
//       the tlss and method drive waits for this event before invoking run.
//
//       The drive_initialized and start_finalized are Yield_event structs.
//       Each Yield_event struct contains a 32 bit latch, but doesn't require
//       any other system resources while not in use.
//
//----------------------------------------------------------------------------
void
   Thread::start(                   // Start this Thread
     ITS               its)         // Initial Thread State
{  if( HCDM )
     traceh("Thread(%p)::start(%s)\n", this, its ? "DETACHED" : "JOINABLE");

   if( this->tlss_ ) {              // If this Thread is already running
     debugf("pub::Thread(%p)::start but already running\n", this);
     throwf("USER ERROR: Duplicate Thread::start");
   }

   tlss* _tlss= new tlss(this);     // Allocate a new tlss, fsm==FSM_START
   this->tlss_= _tlss;

   // Limit the number of actively running Threads.
   for(int retry= 0; ; ++retry) {
     if( startable.wait(0.25) )     // (Does not wait if semaphore available)
       break;

     if( retry == 1 )               // If the first retry
       debugh("Thread::start %'zd threads already active\n", max_threads);
   }

   // Create/drive the Thread
   pthread_attr_t* pthread_attr= &attr_joinable; // (Default attributes)
   if( its == ITS_DETACHED ) {      // If starting detached
     _tlss->set_fsm(FSM_DETACHED);
     pthread_attr= &attr_detached;
   }

   for(int retry= 0; ; ++retry ) {  // (Retry if resource unavailable)
     int rc= pthread_create         // Create the thread
         ( &_tlss->handle           // The pthread_t
         , pthread_attr             // The pthread_attr_t*
         , Thread::drive            // The pthread entry point
         , this);                   // The entry point's parameter
     if( rc == 0 )                  // If successful
       break;

     // Handle pthread_create error
     if( rc == EAGAIN ) {           // If retryable error
       if( retry < 128 ) {          // If retry count exceeded
         sleep(0.001);              // One millisecond delay before retry
         continue;
       }
     }

     //= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
     // Start failure: EXCEPTION
     if( USE_ITRACE )
       Trace::trace(".THR", "-ERR", this, this->tlss_);

     startable.post();              // Semaphore recovery

     delete this->tlss_;            // TLSS storage recovery
     this->tlss_= nullptr;

     // Error diagnostic (throwf writes to console, so log doesn't need to)
     log(LL_NONE, "Thread::start failure %d:%s\n", errno, strerror(errno));
     throwf("Thread::start failure %d:%s\n", errno, strerror(errno));
   }

   //= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
   // Start successful: SYNCHRONIZE with drive()
   _tlss->drive_initialized.wait();
   _tlss->start_finalized.post();

   // While the Thread and tlss cannot be referenced now, addresses can be.
   if( USE_ITRACE )                 // Start exit
     Trace::trace(".THR", "SXIT", this, _tlss);
}

//----------------------------------------------------------------------------
//
// Static protected method-
//       Thread::drive
//
// Purpose-
//       Start the Thread, insuring Thread::thread initialized first.
//
// Implementation notes-
//       Initialization synchronization is a bit tricky: see start() notes
//
//       We don't need to obtain the tlss_latch before invoking run().
//
//----------------------------------------------------------------------------
void*                               // (Always nullptr)
   Thread::drive(                   // Start
     void*             _thread)     // This Thread
{  if( HCDM )
     traceh("Thread::drive(%p)\n", _thread);

   Thread* thread= (Thread*)_thread;
   Thread::tlss* _tlss= thread->tlss_;
   tl_tlss= _tlss;
   if( USE_ITRACE )
     Trace::trace(".THR", "+LCL", thread, _tlss);

   // (We don't need to obtain the tlss_latch before invoking run)
   if( _tlss->fsm == FSM_DETACHED ) { // If started detached
     ++detached;                    // Count the detached Thread
     thread->tlss_= nullptr;        // (tl_tlss can still find the Thread)
   } else {
     _tlss->set_fsm(FSM_DRIVE);     // FSM_START => FSM_DRIVE (no locking)
   }

   _tlss->drive_initialized.post(); // SYNCHRONIZE with drive()
   _tlss->start_finalized.wait();

   try {
     // Update statistics
     ++started;
     std::size_t now_running= ++running;
     std::size_t was_running= max_run.load();
     while( now_running > was_running )
       max_run.compare_exchange_weak(was_running, now_running);

     // Run the Thread, catching exceptions
     if( USE_ITRACE )
       Trace::trace(".THR", ">run", thread, _tlss);
     try {
       thread->run();
     } catch(Exception& X) {         // Exceptions get message, but complete
       debugh("%4d Thread(%p)::run, Exception: %s\n", __LINE__
             , thread, s2c(X.to_string()));
       debug_backtrace();
     } catch(std::exception& X) {
       debugh("%4d Thread(%p)::run, std::exception what(%s)\n", __LINE__
             , thread, X.what());
       debug_backtrace();
     } catch(...) {
       debugh("%4d Thread(%p)::run, catch(...)\n", __LINE__, thread);
       debug_backtrace();
     }
     int fsm= _tlss->fsm;
     if( USE_ITRACE )
       Trace::trace(".THR", "<run", thread, fsm);

     //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // Run return: fsm == (DRIVE || DETACHED || JOINING || DELETE)
     //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     ++complete;                    // The Thread's complete and
     --running;                     // It's done running

     if( HCDM && VERBOSE > 0 )      // (Debugging reminder)
       traceh("%4d Thread(%p).drive NOTE: Thread state unknown, "
               "it may have been deleted\n", __LINE__, thread);

     // Implementation notes:
     // At this point we do not know whether the associated Thread exists.
     // We still can and do use `thread` in messages and internal trace.
     // This Thread* has meaning whether or not the Thread still exists.
     //- * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - *
     // Note: deleted Threads can be newly constructed at the same address.
     //- * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - *

     // As far as pthread is concerened, we haven't returned from drive()
     // We're still running and still "own" tl_tlss (and are keeping a
     // duplicate copy of that pointer in (stack storage) _tlss.)
     // We don't necessarily own `thread` or anything thread points to.

     // All the USE_ICHECK errors indicate internal logic errors. We could
     // omit these checks and let SEGFAULTs and wild stores happen, but that
     // would make debugging a lot harder should they actually occur.
     // (These checks have no measurable impact on Thread performance.)
     if( USE_ICHECK && _tlss != tl_tlss )
       throwf("Thread.cpp: _tlss!=tl_tlss");

     // Only the running Thread can detach. Once the Thread is detached, the
     // Thread state cannot change. (Therefore, we don't need locking here.)
     if( fsm == FSM_DETACHED ) {
       --detached;                  // (A detached Thread completed)

       // We have exclusive control of the tlss, but we're about to delete it.
       if( USE_ITRACE )
         Trace::trace(".THR", "<DET", thread, _tlss);

       delete _tlss;                // Delete the tlss
       tl_tlss= nullptr;            // Reset the thread local storage pointer
       startable.post();            // (Thread complete)
       return nullptr;
     }

     // The Thread needs to be joined. The join operation may already be in
     // progress but cannot complete until drive returns.
     {{{{
       std::lock_guard<decltype(tlss::mutex)> lock(_tlss->mutex);

       fsm= _tlss->fsm;             // (Protected FSM)

       // The tlss is still needed by join, we can't delete it now.
       // Since we are the last point within the Thread where tl_tlss is
       // valid, we set it to nullptr. The Thread is no longer current.
       // If everything goes according to plan, Thread::join finds the tlss
       // using Thread::tlss_ and deletes it when it completes normally.
       // Contingency plans also exist for other situations.
       if( USE_ITRACE )
         Trace::trace(".THR", "<OWN", thread, _tlss);

       _tlss->set_fsm(FSM_OWNER);   // Pass tlss ownership to Thread
       tl_tlss= nullptr;            // Reset the thread local storage pointer
       return nullptr;
     }}}}
   } catch(Exception& X) {          // (Exception handling)
     debugh("%4d Thread(%p)::drive, Exception: %s\n", __LINE__
           , thread, s2c(X.to_string()));
     debug_backtrace();
   } catch(std::exception& X) {
     debugh("%4d Thread(%p)::drive, std::exception what(%s)\n", __LINE__
           , thread, X.what());
     debug_backtrace();
   } catch(...) {
     debugh("%4d Thread(%p)::drive, catch(...)\n", __LINE__, thread);
     debug_backtrace();
   }
   Thread::static_debug("Exception"); // (We don't know if thread is valid)

   if( USE_ITRACE )
     Trace::trace(".THR", "HCDR", thread, __LINE__);
   return nullptr;
}
} // namespace _LIBPUB_NAMESPACE
