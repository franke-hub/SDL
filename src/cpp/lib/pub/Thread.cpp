//----------------------------------------------------------------------------
//
//       Copyright (C) 2018-2025 Frank Eskesen.
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
//       2025/01/11
//
// Implementation note-
//       We use Thread Local Storage to maintain the Thread::tlss state.
//       TLS is only really needed by Thread::current, because we also keep a
//       pointer to the state in Thread::_tlss and in automatic storage while
//       running Thread::drive. But, since Thread::current *really needs* it
//       we'll continue to use it.
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
typedef tlss::handle_t handle_t;    // For convenience

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
// USE_HCDM_DEBUGGING uses debugging versions of Thread.h inlines.
#define USE_HCDM_DEBUGGING          // Use Thread.h debugging implementations?

// Production mode settings: HCDM= false; VERBOSE= 1;
//   USE_CHECK= false; USE_ITRACE= false; USE_TIMING=false
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 1                       // Verbosity, higher is more verbose

// Production mode settings: USE_CHECK= true; USE_TIMING= false
,  USE_CHECK= true                  // Use self-checking code?
,  USE_ITRACE= false                // Use internal trace?
,  USE_TIMING= false                // Use timing code?
}; // generic enum

enum FSM                            // Finite State Machine states
{  FSM_START= 0                     // START, initial state (in Thread::start)
,  FSM_DRIVE                        // DRIVE, "Owned" by Thread::drive
,  FSM_OWNER                        // OWNER, "Owned" by Thread's owner
,  FSM_DETACHED                     // DETACHED, "Owned" by Thread::drive
,  FSM_JOINING                      // JOINING, "Owned" by Thread's owner
,  FSM_COUNT                        // COUNT, Number of states
}; // enum FSM

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
// The null handle, and id
const handle_t         Thread::null_handle= {}; // Native handle

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

#if 0  // Currently unused - - - - - - - - - - - - - - - - - - - - - - - - - -
// Global initialization/termination
namespace {                         // Anonymous namespace
static struct StaticGlobal {
   StaticGlobal( void )             // Initialize main()'s tl_tlss
{  if( HCDM ) debugh("Thread::StaticGlobal!\n");
   // (Placeholder)
}

   ~StaticGlobal( void )            // Initialize main()'s tl_tlss
{  if( HCDM ) debugh("Thread::StaticGlobal~\n");
   // (Placeholder)
}
}  static_global;
}  // Anonymous namespace
#endif // Currently unused - - - - - - - - - - - - - - - - - - - - - - - - - -

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
//       m2c
//
// Purpose-
//       Return (RecursiveLatch&) mutex state
//
//----------------------------------------------------------------------------
static const char*                  // "free" || "held"
   m2c(                             // Get mutex state
     const RecursiveLatch&
                       mutex)       // For this mutex
{
   if( mutex.is_held() )
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
//       Construtor
//       Destrutor
//
//----------------------------------------------------------------------------
#ifdef USE_HCDM_DEBUGGING           // Debugging version
   Thread::tlss::tlss(              // Constructor
     Thread*           thread)      // Associated Thread
:  pub_thread(thread)
{  if( HCDM ) debugh("Thread::tlss(%p)!(%p)\n", this, thread); }

   Thread::tlss::~tlss( void )      // Destructor
{  if( HCDM ) debugh("Thread::tlss(%p)~\n", this); }
#endif

/* _LIBPUB_THREAD_DEBUGGING constructor/destructor ***************************
   tlss(Thread* thread);            // TODO: REPLACE WITH INLINE VERSION
   ~tlss( void );
*****************************************************************************/

/* Production constructor/destructor *****************************************
   tlss(                            // Constructor
     Thread*           thread)      // Associated Thread
:  pub_thread(thread) { }

   ~tlss( void ) = default;         // (Destructor does nothing)
*****************************************************************************/

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

   debugf("..mutex(%s) fsm(%s) pub_thread(%p) std_thread(0x%zx)\n"
         , m2c(mutex), f2c(fsm), pub_thread, intptr_t(std_thread));
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
{  if( HCDM && VERBOSE )
     debugh("Thread::tlss(%p)::set_fsm %s=>%s\n", this, f2c(fsm), f2c(_fsm));

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
{  if( HCDM || USE_TIMING ) debugh("Thread(%p)!\n", this);

   if( USE_ITRACE )
     Trace::trace(".THR", "=NEW", this, nullptr);
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
{  if( HCDM || USE_TIMING ) debugh("Thread(%p)~ _tlss(%p)\n", this, _tlss);

   if( USE_ITRACE )
     Trace::trace(".THR", "=DEL", this, _tlss);

   if( _tlss )                      // (Avoid NOP detach call)
     detach();
//debugh("%4d Thread~ EXIT\n", __LINE__);
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
   std::lock_guard<decltype(mutex)> lock(mutex);

   if( _tlss ) {                    // Once JOINED || DETACHED, _tlss= nullptr
     std::lock_guard<decltype(tlss::mutex)> lock(_tlss->mutex);

     switch(_tlss->fsm) {
       case FSM_DRIVE:              // Thread is running (default joinable)
       case FSM_OWNER:              // Thread completed  (default joinable)
         return true;

// When detached, _tlss == nullptr (so DETACHED is an invalid state)
//     case FSM_DETACHED:           // Thread already detached
       case FSM_JOINING:            // (Can't be joined again, so FALSE)
         return false;

       default:
         debugf("%4d Thread invalid fsm(%s)\n", __LINE__, f2c(_tlss->fsm));
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
{  if( HCDM || USE_TIMING )
     debugh("Thread(%p)::detach _tlss(%p)\n", this, _tlss);

   if( USE_ITRACE )
     Trace::trace(".THR", "=DET", this, _tlss);

   // The Thread::mutex lock is held until this method exits
   std::lock_guard<decltype(mutex)> lock(mutex); // Protects _tlss *only*
   tlss* _tlss= this->_tlss;
   int fsm= -1;

if( _tlss == nullptr ) {
// We usually *silently* ignore multiple detaches or detach after join.
// But, we're debugging and making some noise here. It's not an error.
debugh("%4d Thread(%p) _tlss == nullptr (DUPLICATE) DETACH\n", __LINE__, this);
}

   if( _tlss ) {{{{
     std::lock_guard<decltype(tlss::mutex)> lock(_tlss->mutex);
//debugh("Thread(%p,0x%zx).detach\n", this, intptr_t(_tlss->std_thread));

     fsm= _tlss->fsm;
     if( fsm != FSM_DRIVE && fsm != FSM_OWNER ) {
       debugh("%4d Thread(%p)::detach rejected, FSM(%s)\n", __LINE__
             , this, f2c(fsm));
       if( USE_ITRACE )
         Trace::trace(".THR", "DREJ", this, i2v(fsm));
       return;
     }

     // At this point the Thread should be detachable (or joinable.)
     // If (somehow) it's not, join will return an error, which indicates a
     // problem in this code
//_tlss->debug("Before detach");
//debugh("%4d Thread.detach HCDM TIMING\n", __LINE__);
     int rc= pthread_detach(_tlss->std_thread);
     if( rc ) {
       if( USE_ITRACE )
         Trace::trace(".THR", "DERR", this, i2v(i2i(fsm)<<32 | rc));

       errorh("pthread_detach(0x%zx) error %d:%s\b"
             , intptr_t(_tlss->std_thread), rc, strerror(rc));
//debugh("%4d Thread.detach HCDM TIMING\n", __LINE__);
     }

     if( USE_TIMING )
       traceh("%4d Thread(%p).detach _tlss(%p) fsm(%s) detached\n", __LINE__
             , this, _tlss, f2c(fsm));

     if( fsm == FSM_DRIVE ) {       // If Thread::run hasn't returned
       _tlss->set_fsm(FSM_DETACHED); // Thread::drive owns the tlss
       ++detached;                  // Thread::drive will decrement this
       _tlss->pub_thread= nullptr;  // (Thread cannot be referenced)
       _tlss->std_thread= null_handle; // (Thread ID meaningless)
//_tlss->std_thread= pthread_t(intptr_t(-1)); // (Thread ID meaningless)
//debug("detach DRIVE=>DETACHED"); // _tlss not zeroed yet, so full debug
       this->_tlss= nullptr;        // We do not own the tlss

//Thread::static_debug("Detach[DRIVE] exit");
//debugh("%4d detach EXIT, _tlss==nullptr\n", __LINE__);
       if( USE_ITRACE )
         Trace::trace(".THR", "=FSM", this, i2v(fsm));
       return;
     }
   }}}} // (End of scope: lock_guard tlss::mutex)

   // (We couldn't delete the tlss while holding the tlss mutex)
   if( _tlss ) {                    // If deferred tlss delete
     if( USE_CHECK && _tlss->fsm != FSM_OWNER )
       abortf("Thread.cpp: _tlss->fsm!=FSM_OWNER");

//debug("detach DRIVE=>FSM_OWNER [tlss delete]");
     if( USE_ITRACE )
       Trace::trace(".THR", "-TLS", this, _tlss);
     delete _tlss;                // (We own it, so we delete it)
     this->_tlss= nullptr;        // The tlss is no longer meaningful
   }

//debug("Detach exit");
//Thread::static_debug("Detach exit");
   if( USE_ITRACE )
     Trace::trace(".THR", "DXIT", this, _tlss);
   if( USE_TIMING ) {
     traceh("%4d Thread(%p).detach _tlss(%p,%p) fsm(%s) EXIT\n", __LINE__
           , this, _tlss, this->_tlss, "N/A");
   }
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
     debugh("Thread(%p)::join _tlss(%p)->fsm(%s)\n", this, _tlss
           , _tlss ? f2c(_tlss->fsm) : "N/A" );

   if( USE_ITRACE )
     Trace::trace(".THR", "JOIN", this, _tlss);

   tlss* _tlss= nullptr;            // (Not valid yet)

   {{{{
     std::lock_guard<decltype(mutex)> thread_lock(mutex);

     _tlss= this->_tlss;
     if( _tlss == nullptr )         // If Thread detached/joined
       throw std::runtime_error("Invalid join state");

     std::lock_guard<decltype(tlss::mutex)> tlss_lock(_tlss->mutex);

     if( USE_CHECK && _tlss->pub_thread != this )
       abortf("Thread.cpp: _tlss->pub_thread!=this");

     if( USE_TIMING ) {
       traceh("%4d Thread(%p,0x%zx).join  _tlss(%p) join...\n", __LINE__, this
             , intptr_t(_tlss->std_thread), _tlss);
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

// If we map Threads, how do we handle state after run exit but before Thread
// either joins or detaches? Need to remove from map after join/detach then.
// TODO: DEBUGGING: MAKE IT A TIMED JOIN IN A LOOP

   // Once we set the FSM to FSM_JOINING nobody's going to change it (except
   // possibly to OWNER.) In any case nobody gets to delete the tlss but us.

//_tlss->debug("before join");        // TODO: REMOVE
//debugf("JOINING(%p,0x%zx)...\n", this, intptr_t(_tlss->std_thread));

   int rc= pthread_join(_tlss->std_thread, nullptr);
   if( rc ) {                       // Handle join error
     debugh("Thread(%p)::join ERROR %d:%s\n", this, rc, strerror(rc));

     // So much for error recovery. We're ignoring the error
   }

   if( USE_TIMING )
     traceh("%4d Thread(%p).join  _tlss(%p) ...joined\n", __LINE__, this
           , _tlss );

   // Join doesn't complete until Thread::drive completes. (Note that it could
   // have completed before we began the join.)

   // Even though we "own" _tlss and *_tlss, we still need Thread::mutex
   // protection to set it to nullptr.
   std::lock_guard<decltype(mutex)> lock(mutex); // Protects this->_tlss *only*

   if( USE_ITRACE )
     Trace::trace(".THR", "-TLS", this, _tlss);
   delete _tlss;                  // (The thread's tl_tlss is already null)
   this->_tlss= nullptr;

   if( USE_TIMING )
     traceh("%4d Thread(%p).join  _tlss(%p) EXIT\n", __LINE__, this
           , this->_tlss);
}

//----------------------------------------------------------------------------
int                                 // Return code, 0 or errno
   Thread::join(                    // Wait for this thread to complete
     double            timeout)     // With this maximum timeout, in seconds
{  if( HCDM || USE_TIMING )
     debugh("Thread(%p)::join(%.2f) _tlss(%p)->fsm(%s)\n", this, timeout
           , _tlss, _tlss ? f2c(_tlss->fsm) : "N/A" );

   // NOT CODED YET - Regular join must work first
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
//       Thread::start needs: Thread::drive's
//           tl_tlss= thread->_tlss to complete before it can exit.
//
//----------------------------------------------------------------------------
void
   Thread::start( void )            // Start this Thread
{  if( HCDM || USE_TIMING ) debugh("Thread(%p)::start\n", this);

   if( _tlss )                      // If this Thread is already running
     throw Exception("USER ERROR: Duplicate Thread::start");

   tlss* _tlss= new tlss(this);     // Allocate a new tlss
   this->_tlss= _tlss;              // (Pass its address to Thread::drive)
   if( USE_ITRACE )
     Trace::trace(".THR", "+TLS", this, _tlss);

   if( USE_TIMING )
     traceh("%4d Thread(%p).start _tlss(%p)\n", __LINE__, this, _tlss);

   unsigned count= 0;               // Retry counter
   for(;;) {                        // (Retry if resource unavailable)
     int rc= pthread_create         // Create the thread
         ( &_tlss->std_thread       // The pthread_t
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

       if( USE_ITRACE )
         Trace::trace(".THR", "-TLS", this, _tlss);
       this->_tlss= nullptr;
       delete _tlss;
       throw std::runtime_error("Thread::start EAGAIN retry count exceeded");
     }

     if( USE_ITRACE )
       Trace::trace(".THR", "-TLS", this, _tlss);
     this->_tlss= nullptr;
     delete _tlss;
     throw std::runtime_error("Thread::start error: "
                             + std::to_string(rc)+":"+strerror(rc));
   }

   // Thread create successful
   // Before exiting, wait for Tread::drive to copy thread->_tlss into tl_tlss
   // (the thread-local storage pointer to our tlss)
   _tlss->drive_initialized.wait(); // Wait for drive completion
   _tlss->drive_initialized.reset(); // (Reset for possible reuse)
}

//----------------------------------------------------------------------------
//
// Static protected method-
//       Thread::drive
//
// Purpose-
//       Start the Thread, insuring Thread::thread initialized first.
//
// Implementation note-
//       Thread::drive is essentially Thread::run with a prefix and a suffix.
//       (We are in the same thread as Thread::run throughout this method.)
//
//----------------------------------------------------------------------------
void*                               // (Always nullptr)
   Thread::drive(                   // Start
     void*             _thread)     // This Thread
{  Thread* thread= (Thread*)_thread;

   if( HCDM || USE_TIMING )
     debugh("Thread(%p)::drive _tlss(%p)\n", thread, thread->_tlss);

   if( USE_CHECK && tl_tlss ) {     // (Did we forget to clear it somewhere?)
     // Through no fault of its own, this Thread's luck just ran out.
     abortf("Thread.cpp: tl_tlss!=nullptr");
   }

   tlss* _tlss= thread->_tlss;      // (Stack storage copy)
   tl_tlss= _tlss;                  // (Our protected operation)
   _tlss->set_fsm(FSM_DRIVE);       // FSM_START => FSM_DRIVE (no locking)
   _tlss->drive_initialized.post(); // Protected operation is complete

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
       traceh("%4d Thread(%p).drive _tlss(%p,%p) run's done\n", __LINE__
             , thread, _tlss, tl_tlss);

       if( VERBOSE ) {
         traceh("%4d Thread(%p).drive NOTE: Thread state unknown, "
                "it may have been deleted\n", __LINE__, thread);
       }
     }

     // As far as std::thread is concerened, we haven't returned from
     // Thread::drive. We're still running and still "own" tl_tlss (and
     // keeping a copy of that pointer in stack storage _tlss.)
     // We don't necessarily own `thread` or anything thread points to.

     // We still can and do use `thread` in messages. This Thread* is still
     // correct whether or not the Thread still exists.

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
         traceh("%4d Thread(%p) tlss(%p,%p) drive FSM(%s) (run exit)\n"
               , __LINE__, thread, _tlss, tl_tlss, f2c(fsm));
//_tlss->debug("run exit");

       if( USE_ITRACE )
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
         tl_tlss= nullptr;          // tl_tlss available for re-use

// We hold the tlss::mutex, so `thread` and thread->_tlss` are still valid.
//Thread::static_debug("FSM==OWNER");
//thread->debug("FSM==OWNER");

// Last message before exit <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//debugh("%4d HCDM Thread(%p).drive EXIT FSM==OWNER\n", __LINE__, thread);
         if( USE_ITRACE )
           Trace::trace(".THR", "<xit", thread, nullptr);
         return nullptr;
       }
     }}}}

     //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     // Return: fsm == DETACHED; tlss::mutex latch is not held
     //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     --detached;                    // (A detached Thread completed)

// TODO: These checks can be removed after production test succeeds
// if( use_check && false && ... ) Maybe we'll need them again
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
       traceh("%4d Thread(%p) tlss(%p,%p) drive (detached thread completed)\n"
             , __LINE__, thread, _tlss, tl_tlss);

     if( USE_ITRACE )
       Trace::trace(".THR", "-TLS", thread, _tlss);
     delete _tlss;                  // Delete the tlss
     tl_tlss= nullptr;              // Reset the thread local storage pointer
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
   return nullptr;
}
} // namespace _LIBPUB_NAMESPACE
