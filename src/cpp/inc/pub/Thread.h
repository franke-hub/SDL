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
//       2026/02/20
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_THREAD_H_INCLUDED
#define _LIBPUB_THREAD_H_INCLUDED

#include <string>                   // For std::string
#include <thread>                   // For std::this_thread::yield

#include <pthread.h>                // For pthread

#include "pub/Event.h"              // For pub::Event
#include "pub/Latch.h"              // For pub::Latch

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
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
typedef pthread_t      handle_t;    // The tlss::handle_t

// The start method parameter
enum ITS                            // Initial Thread State
{  ITS_JOINABLE= 0                  // Joinable (default)
,  ITS_DETACHED                     // Detached
};

//----------------------------------------------------------------------------
// Thread::tlss_event || Thread Local Storage Struct Event container
//----------------------------------------------------------------------------
struct tlss_event {                 // TLSS startup events
pub::Event             drive_initialized; // Thread::drive init complete
pub::Event             start_completed; // Thread::start complete
}; // struct tlss_event

//----------------------------------------------------------------------------
// Thread::tlss || Thread Local Storage Struct
//----------------------------------------------------------------------------
struct tlss {                       // Thread Local Storage
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Attributes
RecursiveLatch         mutex;       // Protects this struct
int                    fsm= 0;      // Finite State Machine
int                    ___= 0;      // (For alignment)

Thread*                thread= nullptr; // The current pub::Thread
handle_t               handle= null_handle; // The associated pthread handle

// The tlss_events are only used during startup.
tlss_event*            E= nullptr;  // (Events only used during startup)
char                   TES[sizeof(tlss_event)]; // The Event storage

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Constructor/destructor
   tlss(Thread* thread);

   ~tlss( void );

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
mutable Latch          tlss_latch;  // Latch, only protects Thread::tlss_
tlss*                  tlss_= nullptr; // (Internal, valid only while running)

static size_t          max_threads; // Maximum allowed running Threads

public:
static const handle_t  null_handle; // The handle of a non-executing thread

//----------------------------------------------------------------------------
// Thread::Constructors/Destructor
//----------------------------------------------------------------------------
   Thread( void );                  // Constructor

virtual                             // VIRTUAL
   ~Thread( void );                 // Destructor

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Disallowed: copy constructor; assignment operator
   Thread(const Thread&) = delete; // Disallowed copy constructor

Thread&
   operator=(const Thread&) = delete; // Disallowed assignment operator

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
{  return tlss_ ? tlss_->handle : null_handle; }

static size_t                       // The maximum number active Threads
   get_max_threads( void )
{  return max_threads; }

bool                                // TRUE iff Thread is joinable
   joinable( void ) const;          // Is this Thread joinable?

static void
   set_max_threads(size_t);           // Set maximum number active Threads

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

// OVERRIDE this method. (There is no default implementation.)
virtual void
   run( void ) = 0;                 // Operate this thread

// Thread::start creates the system thread that drives the run method.
// Note that a Thread without explicit sequencing controls might be deleted
// even before the start method returns.
void
   start(                           // Start this Thread
     ITS               its= ITS_JOINABLE); // Joinable Thread

//----------------------------------------------------------------------------
// Thread::Internal methods
//----------------------------------------------------------------------------
protected:
static void*
   drive(                           // Drive (run)
     void*             _thread);    // This Thread

[[noreturn]]
void
   start_failure( void );           // Handle start failure
}; // class Thread
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_THREAD_H_INCLUDED
