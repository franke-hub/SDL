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
//       2026/01/21
//
// Implementation note-
//       We use Thread Local Storage to maintain the Thread::tlss state.
//       TLS is only really needed by Thread::current, because we also keep a
//       pointer to the state in Thread::_tlss and in automatic storage while
//       running Thread::drive. But, since Thread::current *really needs* it
//       we'll continue to use it.
//
//       If _PUBLIB_THREAD_DEBUG is defined in Thread.h, the debugging version
//       of certain methods are implemented here.
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic<> statistics
#include <chrono>                   // For std::chrono::microseconds
#include <mutex>                    // For std::lock_guard, mutex
#include <stdexcept>                // For std::runtime_error
#include <string>                   // For std::string
#include <cerrno>                   // For errno
#include <cstring>                  // For strerror, ...

#include <pub/Debug.h>              // For debugging
#include <pub/Exception.h>          // For debugging
#include "pub/Latch.h"              // For pub::Latch
#include <pub/Named.h>              // For pub::Named
#include "pub/Thread.h"             // For pub::Thread, implemened
#include "pub/Trace.h"              // For pub::Trace
#include "pub/utility.i"            // For conversion subroutines

#define PUB _LIBPUB_NAMESPACE
using namespace PUB::debugging;     // For debugging methods
using std::atomic_size_t;           // For convenience
using std::string;                  // For convenience

typedef PUB::Thread    Thread;      // For convenience
typedef Thread::tlss   tlss;        // For convenience
typedef pthread_t      handle_t;    // For convenience

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
// Production mode settings: HCDM= false; VERBOSE= 1;
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

// Production mode settings: USE_CHECK= true;
,  USE_CHECK=  true                 // Use self-checking code?

// Production mode settings: USE_ITRACE= false; USE_TIMING= false
,  USE_ITRACE= false                // Use internal trace?
,  USE_TIMING= false                // Use timing code?
}; // generic enum

enum FSM                            // The tlss Finite State Machine states
{  FSM_RESET= 0                     // 0 RESET, initial state
,  FSM_DRIVE                        // 1 DRIVE, "Owned" by Thread::drive
,  FSM_OWNER                        // 2 OWNER, "Owned" by Thread's owner
,  FSM_DETACHED                     // 3 DETACHED, "Owned" by Thread::drive
,  FSM_JOINING                      // 4 JOINING, "Owned" by Thread's owner
,  FSM_COUNT                        // 5 COUNT, Number of states
}; // enum FSM

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
// The null handle, and id
const handle_t         Thread::null_handle{}; // Native handle

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static __thread tlss*  tl_tlss= nullptr; // Thread-local storage

// Statistics
static atomic_size_t   completed= 0; // Number of completed Threads
static atomic_size_t   detached= 0; // Number of detached Threads
static atomic_size_t   max_run= 0;  // Maximum running Thread count
static atomic_size_t   running= 0;  // Number of running Threads
static atomic_size_t   started= 0;  // Number of started Threads

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Global initialization/termination
namespace {                         // Anonymous namespace
static struct StaticGlobal {
   StaticGlobal( void )             // Initialize main()'s tl_tlss
{  if( HCDM ) traceh("Thread::StaticGlobal!\n");
}

   ~StaticGlobal( void )            // Initialize main()'s tl_tlss
{  if( HCDM ) traceh("Thread::StaticGlobal~\n");
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
     case FSM_RESET:                // Thread::start running
       name= "RESET";
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
//       m2c
//
// Purpose-
//       Return (RecursiveLatch&) mutex state
//
//----------------------------------------------------------------------------
static inline const char*           // "free" || "held"
   m2c(                             // Get mutex state
     const RecursiveLatch&
                       mutex)       // For this mutex
{  if( mutex.is_held() )
     return "held";

   return "free";
}

static inline const char*           // "free" || "held"
   m2c(                             // Get mutex state
     const Latch&      mutex)       // For this mutex
{  if( mutex.is_held() )
     return "held";

   return "free";
}

// NOT CODED YET, NEED INTERNALS (WHICH MAY VARY) [CURRENTLY UNUSED]
static inline const char*           // "free" || "held"
   m2c(                             // Get mutex state
     const std::recursive_mutex&)   // For this (ignored) mutex
{  return "std"; }

//----------------------------------------------------------------------------
//
// Method-
//       Thread::tlss::tlss
//       Thread::tlss::~tlss
//
// Purpose-
//       Construtor
//       Destrutor
//
//----------------------------------------------------------------------------
#ifdef _PUBLIB_THREAD_DEBUG         // Debugging (outline) version
   Thread::tlss::tlss(              // Constructor
     Thread*           thread)      // Associated Thread
:  pub_thread(thread)
{  if( HCDM )
     traceh("Thread::tlss(%p)!(%p)\n", this, thread);
   else if( USE_ITRACE )
     Trace::trace(".NEW", "TLSS", this, pub_thread);
}

   Thread::tlss::~tlss( void )      // Destructor
{  if( HCDM )
     traceh("Thread::tlss(%p)~\n", this);
   else if( USE_ITRACE )
     Trace::trace(".DEL", "TLSS", this, pub_thread);
}
#endif

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

   debugf("..mutex(%s) fsm(%s) pub_thread(%p) pth_handle(0x%zx)\n"
         , m2c(mutex), f2c(fsm), pub_thread, intptr_t(pth_handle));
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
{  if( HCDM && VERBOSE ) {
     traceh("Thread(%p)::tlss(%p)::set_fsm %s=>%s\n", pub_thread, this
           , f2c(fsm), f2c(_fsm));
   } else if( USE_ITRACE ) {
     void* voidptr= (void*)(uintptr_t(fsm) << 32 | _fsm);
     Trace::trace(".THR", "=FSM", pub_thread, voidptr);
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
{  if( HCDM || USE_TIMING )
     traceh("Thread(%p)!\n", this);
   else if( USE_ITRACE )
     Trace::trace(".THR", "=NEW", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::~Thread
//
// Purpose-
//       Destructor
//
// Implementation notes-
//       TODO: Must agree with run completion.
//
//----------------------------------------------------------------------------
   Thread::~Thread( void )          // Destructor
{  if( HCDM || USE_TIMING )
     traceh("Thread(%p)~ _tlss(%p)\n", this, _tlss);
   else if( USE_ITRACE )
     Trace::trace(".THR", "=DEL", this, _tlss);

   if( _tlss )                      // (Avoid NOP detach call)
     detach();

   if( HCDM || USE_TIMING )
     traceh("Thread~ EXIT\n");
   else if( USE_ITRACE )
     Trace::trace(".THR", "EXIT", this, _tlss);
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
// Implementation notes-
//       It's not a good idea to call static_debug in production mode
//
//----------------------------------------------------------------------------
void
   Thread::debug(                   // Debugging display
     const char*       info) const  // Caller information
{
   std::string name= "";
   const Named* named= dynamic_cast<const Named*>(this);
   if( named )
     name= " Named(" + named->get_name() + ")";

   {{{{ // The Debug lock insures sequential debug statement
     std::lock_guard<Debug> debug(*Debug::get());

     debugf("Thread(%p)::debug(%s)%s\n", this, info, s2c(name));
     debugf("..mutex(%s) joinable(%d) _tlss(%p)\n"
           , m2c(mutex), joinable(), _tlss);
     if( _tlss )
       _tlss->debug(info);
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

     debugf("%'16zd started\n",   started.load());
     debugf("%'16zd completed\n", completed.load());
     debugf("%'16zd running\n",   running.load());
     debugf("%'16zd detached\n",  detached.load());
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
   std::lock_guard<decltype(mutex)> lock(mutex); // (Protects _tlss ONLY)

   if( _tlss ) {                    // Once JOINED || DETACHED, _tlss= nullptr
     std::lock_guard<decltype(tlss::mutex)> lock(_tlss->mutex);

     switch(_tlss->fsm) {
       case FSM_DRIVE:              // Thread is running (default joinable)
       case FSM_OWNER:              // Thread completed  (default joinable)
         return true;

       // When detached, _tlss == nullptr (so DETACHED is an invalid state)
       // case FSM_DETACHED:        // Thread already detached
       case FSM_JOINING:            // (Can't be joined again, so FALSE)
         return false;

       default:
         debugf("Thread invalid fsm(%s)\n", f2c(_tlss->fsm));
         break;
     }
   }

   return false;
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
//       If running in a pthread not controlled by pub::Thread,
//       tl_tlss will always be null (and nullptr will be returned).
//
//----------------------------------------------------------------------------
Thread*                             // The current Thread
   Thread::current( void )          // Get current Thread
{  return tl_tlss ? tl_tlss->pub_thread : nullptr; }

//----------------------------------------------------------------------------
//
// Method-
//       Thread::detach
//
// Purpose-
//       Detach execution thread from this object
//
// Implementation notes-
//       Idempotent: detach may be called if already detached or (in error)
//       if join has already completed. If called while joining, an error
//       message is written but detach returns normally.
//
//----------------------------------------------------------------------------
void
   Thread::detach( void )           // Detach excution thread from this object
{  if( HCDM )
     traceh("Thread(%p)::detach _tlss(%p)\n", this, _tlss);
   else if( USE_ITRACE )
     Trace::trace(".THR", "=DET", this, _tlss);

   // The Thread::mutex lock is held until this method exits
   std::lock_guard<decltype(mutex)> lock(mutex); // (Protects _tlss ONLY)
   tlss* _tlss= this->_tlss;
   int fsm= -1;

   if( HCDM ) {
     if( _tlss == nullptr ) {
       // We usually *silently* ignore multiple detaches or detach after join.
       // Even though it's an *ignorable* error, it's still an error.
       // We're debugging and going to complain.
       debugh("Thread(%p) _tlss == nullptr (DUPLICATE) DETACH\n", this);
     }
   }

   if( _tlss ) {{{{
     std::lock_guard<decltype(tlss::mutex)> lock(_tlss->mutex);

     if( HCDM )
       traceh("Thread(%p,0x%zx).detach\n", this, intptr_t(_tlss->pth_handle));

     fsm= _tlss->fsm;
     if( fsm != FSM_DRIVE && fsm != FSM_OWNER ) {
       debugh("Thread(%p)::detach rejected, FSM(%s)\n", this, f2c(fsm));
       if( USE_ITRACE )
         Trace::trace(".THR", "DREJ", this, i2v(fsm));
       return;
     }

     // At this point the Thread should be detachable (or joinable.)
     // If (somehow) it's not, join will return an error indicating there's a
     // problem in this code
     int rc= pthread_detach(_tlss->pth_handle);
     if( rc ) {
       if( USE_ITRACE )
         Trace::trace(".THR", "DERR", this, i2v(i2i(fsm)<<32 | rc));

       errorh("pthread_detach(0x%zx) error %d:%s\b"
             , intptr_t(_tlss->pth_handle), rc, strerror(rc));
     }

     if( USE_TIMING )
       traceh("Thread(%p).detach _tlss(%p) fsm(%s) detached\n" , this
             , _tlss, f2c(fsm));

     if( fsm == FSM_DRIVE ) {       // If Thread::run hasn't returned
       _tlss->set_fsm(FSM_DETACHED); // Thread::drive owns the tlss
       ++detached;                  // Thread::drive will decrement this
       _tlss->pub_thread= nullptr;  // (Thread cannot be referenced)
       _tlss->pth_handle= null_handle; // (Thread ID meaningless)
       this->_tlss= nullptr;        // We do not own the tlss

       if( USE_ITRACE )             // Hard Core Debug Detach
         Trace::trace(".THR", "HCDD", this, i2v(__LINE__));
       return;
     }
   }}}} // (End of scope: lock_guard tlss::mutex)

   // (We couldn't delete the tlss while holding _tlss->mutex)
   if( _tlss ) {                    // If deferred tlss delete
     if( USE_CHECK && _tlss->fsm != FSM_OWNER )
       abortf("Thread.cpp: _tlss->fsm!=FSM_OWNER");

     delete _tlss;                // (We own it, so we delete it)
     this->_tlss= nullptr;        // The tlss is no longer meaningful
   }

   if( USE_TIMING )
     traceh("Thread(%p).detach _tlss(%p,%p) fsm(%s) EXIT\n", this
           , _tlss, this->_tlss, "N/A");
   else if( USE_ITRACE )
     Trace::trace(".THR", "HCDD", this, i2v(__LINE__));
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
{  if( HCDM || USE_TIMING )
     traceh("Thread(%p)::join _tlss(%p)->fsm(%s)\n", this, _tlss
           , _tlss ? f2c(_tlss->fsm) : "N/A" );
   else if( USE_ITRACE )
     Trace::trace(".THR", "JOIN", this, _tlss);

   tlss* _tlss= nullptr;            // (Not valid yet)

   {{{{ // (Thread::mutex protects _tlss ONLY)
     std::lock_guard<decltype(mutex)> thread_lock(mutex);

     _tlss= this->_tlss;
     if( _tlss == nullptr )         // If Thread detached/joined
       throw std::runtime_error("Invalid join state");

     std::lock_guard<decltype(tlss::mutex)> tlss_lock(_tlss->mutex);

     if( USE_CHECK && _tlss->pub_thread != this )
       abortf("Thread.cpp: _tlss->pub_thread!=this");

     if( USE_TIMING ) {
       traceh("Thread(%p,0x%zx).join  _tlss(%p) join...\n", this
             , intptr_t(_tlss->pth_handle), _tlss);
     }

     int fsm= _tlss->fsm;
     if( fsm != FSM_DRIVE && fsm != FSM_OWNER ) {
       if( USE_ITRACE )
         Trace::trace(".THR", "JREJ", this, i2v(fsm));
       debugh("Thread(%p)::join rejected, FSM(%s)\n", this, f2c(fsm));
       throw std::runtime_error("join rejected");
     }

     _tlss->set_fsm(FSM_JOINING); // (This state prevents detach)
     if( USE_ITRACE )
       Trace::trace(".THR", "JFSM", this, i2v(_tlss->fsm));
   }}}}

   // We set the FSM to FSM_JOINING so nobody's allowed to change it (except
   // possibly to OWNER.) In any case nobody gets to delete the tlss but us.
   int rc= EFAULT;                  // Default, bad address
   if( _tlss )                      // (Should *ALWAYS* be valid)
     rc= pthread_join(_tlss->pth_handle, nullptr);
   if( rc ) {                       // Handle join error
     // Except for this message, we ignore the error.
     debugh("Thread(%p)::join ERROR %d:%s\n", this, rc, strerror(rc));
   }

   if( USE_TIMING )
     traceh("Thread(%p).join  _tlss(%p) ...joined\n", this, _tlss );

   // Join doesn't complete until Thread::drive completes. (Note that it could
   // have completed before we began the join.)

   // Even though we "own" _tlss and *_tlss, we still need Thread::mutex
   // protection to set it to nullptr.
   std::lock_guard<decltype(mutex)> lock(mutex); // (Protects _tlss ONLY)

   delete _tlss;                  // (The thread's tl_tlss is already null)
   this->_tlss= nullptr;

   if( USE_TIMING )
     traceh("Thread(%p).join _tlss(%p) EXIT\n", this, this->_tlss);
}

//----------------------------------------------------------------------------
int                                 // Return code, 0 or errno
   Thread::join(                    // Wait for this thread to complete
     double            timeout)     // With this maximum timeout, in seconds
{  if( HCDM || USE_TIMING )
     traceh("Thread(%p)::join(%.2f) _tlss(%p)->fsm(%s)\n", this, timeout
           , _tlss, _tlss ? f2c(_tlss->fsm) : "N/A" );

   // NOT CODED YET - Regular join not 100% reliable yet
   join();
   return 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::run
//
// Purpose-
//       Run is a pure virtual function, and is not implemented here.
//       If not overridden, the compiler and system loader complain.
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
//----------------------------------------------------------------------------
void
   Thread::sleep(                   // Delay the current Thread
     double            seconds)     // For this many seconds
{
   // (Using microsecond resolution)
   int64_t us= int64_t(seconds * 1'000'000 + 500'000); // (Rounded up)
   if( us > 0 && double(us) >= seconds ) // If a positive and valid delay
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
//       We need Thread->_tlss initialized in start() because we're using
//       _tlss->drive_initialized and (now) _tlss->start_completed there.
//
//       We can't set (thread local) tl_tlss until drive() since before that
//       we aren't actually running under the thread.
//
//       We can't let start() exit until the tl_tlss is set, because current()
//       won't work. Ergo, method start() must wait for drive_initialized.
//       We discovered that this wasn't enough.
//
//       We can't let start() continue after drive() posts drive_initialized,
//       because the Thread can run to completion (deleting the tlss) before
//       start()'s drive_initialized.wait() completes. If this should occur,
//       _tlss->drive_initialized.wait() refers to undefined storage. Bad!
//       >>>>>>>>>>>>>>>>>>>>>>>>>> THIS WAS A BUG. <<<<<<<<<<<<<<<<<<<<<<<<<<
//       (It happened. This explains the fix and why fixing it was necessary.)
//
//       To prevent this, we added another Event in the tlss, start_completed.
//       In start_completed.post(), once the post() action is performed the
//       start() method won't access the tlss again. It doesn't matter how
//       many instructions remain before start() exits or how long they take.
//       The last tlss reference in start() is `_tlss->start_completed(post)`.
//
//       Method drive() waits for this Event before invoking run().
//
//----------------------------------------------------------------------------
void
   Thread::start( void )            // Start this Thread
{  if( HCDM || USE_TIMING ) traceh("Thread(%p)::start\n", this);

   if( _tlss ) {                    // If this Thread is already running
     debugf("pub::Thread(%p)::start but already running\n", this);
     throwf("USER ERROR: Duplicate Thread::start");
   }

   this->_tlss= new tlss(this);     // Allocate a new tlss, fsm==FSM_RESET

   unsigned count= 0;               // Retry counter
   for(;;) {                        // (Retry if resource unavailable)
     int rc= pthread_create         // Create the thread
         ( &_tlss-> pth_handle      // The pthread_t
         , nullptr                  // The pthread_attr_t
         , Thread::drive            // The pthread entry point
         , this);                   // The entry point's parameter
     if( rc == 0 )                  // If successful
       break;

     // Handle pthread_create error
     if( rc == EAGAIN ) {           // If retryable error
       if( ++count < 128 ) {        // If retry count exceeded
         sleep(0.001);              // One millisecond delay before retry
         continue;
       }
     }

     start_failure();
   }

   _tlss->drive_initialized.wait(); // Wait for tl_tlss= _tlss
   _tlss->start_completed.post();   // We are exiting now
   if( USE_ITRACE )                 // (This can occur after thread deletion)
     Trace::trace(".THR", "INIT", this, _tlss); // (OK to use addresses)
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
//----------------------------------------------------------------------------
void*                               // (Always nullptr)
   Thread::drive(                   // Start
     void*             _thread)     // This Thread
{  if( HCDM || USE_TIMING )
     traceh("Thread::drive(%p)\n", _thread);

   Thread* thread= (Thread*)_thread;
   Thread::tlss* _tlss= thread->_tlss;
   tl_tlss= _tlss;
   if( USE_ITRACE )
     Trace::trace(".THR", "+LCL", thread, _tlss);

   _tlss->set_fsm(FSM_DRIVE);       // FSM_RESET => FSM_DRIVE (no locking)
   _tlss->drive_initialized.post(); // We've initialized tl_tlss= _tlss
   _tlss->start_completed.wait();   // We can't continue until start completes
   // At this point start() is done with *_tlss
// _tlss->drive_initialized.reset(); // (Not needed, not reused)
// _tlss->start_completed.reset();  // (Not needed, not reused)

   try {
     // Update statistics
     ++started;
     std::size_t was_running= ++running;
     std::size_t old_running= max_run.load();
     while( was_running > old_running )
       max_run.compare_exchange_weak(old_running, was_running);

     // Run the Thread, catching exceptions
     try {
       if( USE_ITRACE )
         Trace::trace(".THR", ">run", thread, _tlss);
       thread->run();
     } catch(Exception& X) {         // Exceptions get message, but complete
       debugh("%4d Thread(%p)::run, Exception: %s\n", __LINE__
             , thread, s2c(X.to_string()));
     } catch(std::exception& X) {
       debugh("%4d Thread(%p)::run, std::exception what(%s)\n", __LINE__
             , thread, X.what());
     } catch(...) {
       debugh("%4d Thread(%p)::run, catch(...)\n", __LINE__, thread);
     }

     //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // Run return: fsm == (DRIVE || DETACHED || JOINING)
     //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     ++completed;                   // The Thread's completed and
     --running;                     // It's done running

     if( USE_TIMING ) {
       traceh("Thread(%p).drive _tlss(%p,%p) run's done\n", thread
             , _tlss, tl_tlss);

       if( VERBOSE ) {
         traceh("%4d Thread(%p).drive NOTE: Thread state unknown, "
                "it may have been deleted\n", __LINE__, thread);
       }
     }

     // Implementation notes:
     // We still can and do use `thread` in messages and internal trace.
     // This Thread* is still correct whether or not the Thread still exists.
     //- * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - *
     // Note that deleted Threads can be re-allocated at the same address.
     //- * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - * - *

     // As far as std::thread is concerened, we haven't returned from
     // Thread::drive. We're still running and still "own" tl_tlss (and are
     // keeping a copy of that pointer using _tlss (stack storage).)
     // We don't necessarily own `thread` or anything thread points to.

     // All the abortf calls are (most likely) due to internal logic errors.
     // We could omit these checks and let the SEGFAULTs and wild stores go,
     // but that would make debugging a lot harder should they actually occur.
     // (These checks have a negligable impact on Thread performance.)
     if( USE_CHECK && _tlss != tl_tlss)
       abortf("Thread.cpp: _tlss!=tl_tlss");

     {{{{
       std::lock_guard<decltype(tlss::mutex)> lock(_tlss->mutex);

       int fsm= _tlss->fsm;
       if( USE_TIMING )
         traceh("Thread(%p) tlss(%p,%p) drive FSM(%s) (run exit)\n", thread
               , _tlss, tl_tlss, f2c(fsm));

       if( USE_ITRACE )             // Thread is done running
         Trace::trace(".THR", "<run", thread, i2v(fsm));
       if( fsm != FSM_DETACHED ) {  // If the Thread isn't detached
         if( USE_CHECK ) {
           if( _tlss != thread->_tlss)
             abortf("Thread.cpp: _tlss!=thread->_tlss");
           if( _tlss->pub_thread != thread )
             abortf("Thread.cpp: _tlss->pub_thread!=thread");
           if( fsm != FSM_DRIVE && fsm != FSM_JOINING )
             abortf("Thread.cpp: invalid state(%s)", f2c(fsm));
         }

         // The Thread's owner hasn't returned from detach or join yet.
         // We can't delete tlss since it's needed for delete or join, so we
         // tranfer tlss ownership to the Thread.
         // If everything goes according to plan, either Thread::detach or
         // Thread::join will delete the tlss using Thread::_tlss.
         _tlss->set_fsm(FSM_OWNER); // Pass tlss ownership to Thread
         tl_tlss= nullptr;          // tl_tlss available for reuse
         if( USE_ITRACE ) {
           Trace::trace(".THR", "-LCL", thread, _tlss);
           Trace::trace(".THR", "HCDR", thread, i2v(__LINE__));
         }

         return nullptr;
       }
     }}}}

     //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // Return: fsm == DETACHED; tlss::mutex latch is not held
     //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     --detached;                    // (A detached Thread completed)

     if( USE_CHECK ) {
       if( _tlss != tl_tlss )
         abortf("Thread.cpp: _tlss!=tl_tlss");
       if( _tlss->pub_thread != nullptr )
         abortf("Thread.cpp: _tlss->pub_thread!=nullptr");
       if( _tlss->mutex.is_held() )
         abortf("Thread.cpp: _tlss->mutex.is_held()==true");
     }

     // We have exclusive control of the tlss, but we're about to delete it.
     if( USE_TIMING )
       traceh("Thread(%p) tlss(%p,%p) drive (detached thread completed)\n"
             , thread, _tlss, tl_tlss);

     delete _tlss;                  // Delete the tlss
     tl_tlss= nullptr;              // Reset the thread local storage pointer
     if( USE_ITRACE )
       Trace::trace(".THR", "-LCL", thread, _tlss);
     return nullptr;
   } catch(Exception& X) {          // (Exception handling)
     debugh("%4d Thread(%p)::drive, Exception: %s\n", __LINE__
           , thread, s2c(X.to_string()));
   } catch(std::exception& X) {
     debugh("%4d Thread(%p)::drive, std::exception what(%s)\n", __LINE__
           , thread, X.what());
   } catch(...) {
     debugh("%4d Thread(%p)::drive, catch(...)\n", __LINE__, thread);
   }
   Thread::static_debug("Exception"); // (We don't know if thread is valid)

   if( USE_ITRACE )
     Trace::trace(".THR", "HCDR", thread, i2v(__LINE__));
   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::start_failure
//
// Purpose-
//       Handle Thread::start failure
//
//----------------------------------------------------------------------------
[[noreturn]]
void
   Thread::start_failure( void )    // Handle start failure
{
   if( USE_ITRACE )
     Trace::trace(".THR", "-ERR", this, _tlss);
   delete _tlss;
   _tlss= nullptr;

   debugf("Thread::start failure %d:%s\n", errno, strerror(errno));
   throwf("Thread::start failure");
}
} // namespace _LIBPUB_NAMESPACE
