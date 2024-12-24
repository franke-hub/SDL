//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2024 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Thread.h
//
// Purpose-
//       Define the Thread control object.
//
// Last change date-
//       2024/11/18
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_THREAD_H_INCLUDED
#define _LIBPUB_THREAD_H_INCLUDED

#include <string>                   // For std::string
// #include <thread>                // Note:: std::thread not used

#include <pthread.h>                // For pthread

#include "pub/Event.h"              // For pub::Event
#include "pub/Latch.h"              // For pub::Basic_latch

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Thread
//
// Purpose-
//       A standard Thread representation.
//
// TEMPORARY implementation notes:
//       handle_t The native handle. std::thread.get_handle()
//       NOTE: Latches are BASIC_LATCH, NOT RECURSIVE!
//
//----------------------------------------------------------------------------
class Thread {                      // The Thread object
public:
//----------------------------------------------------------------------------
// Thread::tlss || Thread Local Storage struct
//----------------------------------------------------------------------------
struct tlss {                       // Thread Local Storage
typedef pthread_t      handle_t;    // The native handle type

RecursiveLatch         mutex;       // Protects this struct
int                    fsm= 0;      // Finite State Machine

Thread*                pub_thread= {}; // The current pub::Thread
handle_t               std_thread= {}; // The associated system thread

// This Event is used during Thread startup
pub::Event             drive_initialized; // Thread::drive init complete

// Constructor/destructor
   tlss(Thread* thread);            // TODO: REPLACE WITH INLINE VERSION
   ~tlss( void );

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
typedef tlss::handle_t handle_t;    // Import tlss::handle_t

private:
mutable RecursiveLatch mutex;       // Mutex, protects _tlss *ONLY*
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
{  return _tlss ? _tlss->std_thread : null_handle; }

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
}; // class Thread
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_THREAD_H_INCLUDED
