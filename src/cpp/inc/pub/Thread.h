//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2026 Frank Eskesen.
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
//       Thread.h
//
// Purpose-
//       Define the Thread control object.
//
// Last change date-
//       2026/01/20
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_THREAD_H_INCLUDED
#define _LIBPUB_THREAD_H_INCLUDED

#include <string>                   // For std::string
#include <thread>                   // For std::this_thread::yield

#include <pthread.h>                // For pthread

#include "pub/Event.h"              // For pub::Event
#include "pub/Latch.h"              // For pub::RecursiveLatch

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
// MACRO _IF_PUBLIB_THREAD_INLINE, controlled by _PUBLIB_THREAD_DEBUG
//----------------------------------------------------------------------------
#ifndef   _PUBLIB_THREAD_DEBUG
#  undef  _PUBLIB_THREAD_DEBUG      // (Last for INLINE compilation)
#  define _PUBLIB_THREAD_DEBUG      // (Last for OUTLINE compilation)
#endif

#ifndef _PUBLIB_THREAD_DEBUG
#  define _IF_PUBLIB_THREAD_INLINE(x) x
#else
#  define _IF_PUBLIB_THREAD_INLINE(x) ;
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Thread
//
// Purpose-
//       A pthread based Thread representation.
//
//----------------------------------------------------------------------------
class Thread {                      // The Thread object
public:
//----------------------------------------------------------------------------
// Thread::Typedefs and enumerations
//----------------------------------------------------------------------------
typedef pthread_t      handle_t;    // Import tlss::handle_t

//----------------------------------------------------------------------------
// Thread::tlss || Thread Local Storage struct
//----------------------------------------------------------------------------
struct tlss {                       // Thread Local Storage
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Attributes
Latch                  mutex;       // Protects this struct
int                    fsm= 0;      // Finite State Machine
int                    ___= 0;      // (For alignment)

Thread*                pub_thread{}; // The current pub::Thread
pthread_t              pth_handle{}; // The associated system pthread handle

// These Events are each used once during Thread startup and never reused.
pub::Event             drive_initialized; // Thread::drive init complete
pub::Event             start_completed; // Thread::start complete

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Constructor/destructor
   tlss(Thread* thread)
_IF_PUBLIB_THREAD_INLINE(
:  pub_thread(thread)
{  })

   ~tlss( void )
_IF_PUBLIB_THREAD_INLINE(
{  })

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Methods
void
   debug(                           // Write debugging message
     const char*       info= "") const; // (Optional) caller informationn

void
   set_fsm(int);                    // Update the state
}; // struct tlss

//----------------------------------------------------------------------------
// Thread::Attributes
//----------------------------------------------------------------------------
private:
mutable Latch          mutex;       // Mutex, protects _tlss ONLY)
tlss*                  _tlss= nullptr; // (Internal, valid only while running)

public:
static const handle_t  null_handle; // The handle of a non-executing thread

//----------------------------------------------------------------------------
// Thread::Constructors/Destructors
//----------------------------------------------------------------------------
   Thread( void );

// Disallowed: Copy constructor, assignment operator
   Thread(const Thread&) = delete;
Thread& operator=(const Thread&) = delete;

virtual
   ~Thread( void );

//----------------------------------------------------------------------------
// Thread::Debugging methods
//----------------------------------------------------------------------------
virtual void
   debug(                           // Thread debugging display
      const char*      info="") const; // Caller information

static void
   static_debug(                    // Thread(*) debugging display
      const char*      info="");    // Caller information

//----------------------------------------------------------------------------
// Thread::Accessor methods
//----------------------------------------------------------------------------
handle_t                            // The native Thread handle (when active)
   get_handle( void ) const         // Get native Thread handle
{  return _tlss ? _tlss->pth_handle : null_handle; }

bool                                // TRUE iff Thread is joinable
   joinable( void ) const;          // Is this Thread joinable?

//----------------------------------------------------------------------------
// Thread::Static methods
//----------------------------------------------------------------------------
static Thread*                      // The current Thread*
   current( void );                 // Get current Thread*

static void
   sleep(                           // Delay current Thread
     double            seconds);    // For this many seconds

static void
   yield( void )                    // Give up time slice
{  std::this_thread::yield(); }

//----------------------------------------------------------------------------
// Thread::Methods
//----------------------------------------------------------------------------
void
   detach( void );                  // Detach execution thread

void
   join( void );                    // Wait for this Thread to complete

int                                 // Return code, 0 or errno
   join(double);                    // Join with timeout

// OVERRIDE this method. (There is no default implementation.)
virtual void
   run( void ) = 0;                 // Operate this thread

// Thread::start creates the system thread that drives the run method.
void
   start( void );                   // Start this Thread

//----------------------------------------------------------------------------
// Thread::Internal methods
//----------------------------------------------------------------------------
protected:
static void*                        // (Thread return code, always nullptr)
   drive(                           // Drive (run)
     void*             _thread);    // This Thread

[[noreturn]]
void
   start_failure( void );           // Handle start failure
}; // class Thread
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_THREAD_H_INCLUDED
