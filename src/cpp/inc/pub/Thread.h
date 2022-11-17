//----------------------------------------------------------------------------
//
//       Copyright (c) 2018-2022 Frank Eskesen.
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
//       2022/11/08
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_THREAD_H_INCLUDED
#define _LIBPUB_THREAD_H_INCLUDED

#include <thread>                   // For std::thread

#include <pub/Object.h>             // For pub::Object, base class
#include <pub/utility.h>            // For utility::to_string(std::thread::id)

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
//----------------------------------------------------------------------------
//
// Class-
//       Thread
//
// Purpose-
//       A standard Thread representation.
//
// Implementation notes-
//       Invoking the Thread destructor for a running Thread:
//         detaches the Thread (if it wasn't already detached,) and causes
//         Thread::current() to return nullptr for that Thread.
//         Note that, once deleted, a deleted Thread must not be accessed.
//       Invoking detach for a running thread detaches the thread, but
//         Thread::current() still returns the Thread*.
//
//----------------------------------------------------------------------------
class Thread {                      // The Thread object
//----------------------------------------------------------------------------
// Thread::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef std::string    string;
typedef std::thread    thread_t;
typedef thread_t::id   id_t;

//----------------------------------------------------------------------------
// Thread::Attributes
//----------------------------------------------------------------------------
protected:
id_t                   id;          // The thread id
thread_t               thread;      // The std::thread (while active)
void*                  _tls= nullptr; // (Internal)

public:
static const id_t      null_id;     // The thread id of a non-executing thread

//----------------------------------------------------------------------------
// Thread::Constructors/Destructors
//----------------------------------------------------------------------------
   Thread( void );

virtual
   ~Thread( void );

// Disallowed: Copy constructor, assignment operator
   Thread(const Thread&) = delete;
Thread& operator=(const Thread&) = delete;

//----------------------------------------------------------------------------
// Thread::Debugging methods
//----------------------------------------------------------------------------
virtual void
   debug(                           // Thread debugging display
      const char*      info="") const; // Caller information

static void
   static_debug(                    // Thread(*) debugging display
      const char*      info=nullptr); // Caller information

//----------------------------------------------------------------------------
// Thread::Accessor methods
//----------------------------------------------------------------------------
id_t                                // The Thread ID (when active)
   get_id( void ) const             // Get Thread ID
{  return id; }

std::thread::native_handle_type     // The Thread handle
   get_handle( void )               // Get Thread (native) handle
{  return thread.native_handle(); }

static string                       // Associated string
   get_id_string(                   // Represent id as a string
     const id_t&         id);       // The thread id

std::string                         // Associated string
   get_id_string( void ) const      // Represent id as a string
{  return get_id_string(id); }

bool                                // TRUE iff Thread is joinable
   joinable( void ) const           // Is this Thread joinable?
{  return thread.joinable(); }

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

virtual void
   join( void )                     // Wait for this Thread to complete
{  thread.join(); }

// OVERRIDE this method
virtual void
   run( void ) = 0;                 // Operate this thread

void
   start( void );                   // Start this Thread

//----------------------------------------------------------------------------
// Thread::Internal methods
//----------------------------------------------------------------------------
protected:
static void
   drive(                           // Drive (start)
     Thread*           thread);     // This Thread
}; // class Thread
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_THREAD_H_INCLUDED
