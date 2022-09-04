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
//       Define the Thread Object.
//
// Last change date-
//       2022/09/02
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
//       A standard Thread Object.
//
// Implementation notes-
//       Invoking the Thread destructor DOES NOT terminate thread execution,
//       but does remove the Thread from the active map.
//
//----------------------------------------------------------------------------
class Thread : public Object {      // The Thread Object
//----------------------------------------------------------------------------
// Thread::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
typedef std::thread::id  id;
static const id        null_id;

//----------------------------------------------------------------------------
// Thread::Attributes
//----------------------------------------------------------------------------
protected:
std::thread            thread;      // The thread (while active)

//----------------------------------------------------------------------------
// Thread::Constructors/Destructors
//----------------------------------------------------------------------------
public:
virtual
   ~Thread( void );
   Thread( void );

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
      const char*      info=nullptr); // Caller information (also adds detail)

//----------------------------------------------------------------------------
// Thread::Object methods
//----------------------------------------------------------------------------
virtual size_t                      // A hash code value for this Object
   hashf( void ) const              // Create hash code value for Object
{  std::hash<std::thread::id> builtin;
   return builtin(thread.get_id()); // Use std::hash built-in function
}

//----------------------------------------------------------------------------
// Thread::Accessor methods
//----------------------------------------------------------------------------
id                                  // The Thread ID (when active)
   get_id( void ) const             // Get Thread ID
{  return thread.get_id(); }

std::thread::native_handle_type     // The Thread handle
   get_handle( void )               // Get Thread (native) handle
{  return thread.native_handle(); }

static std::string                  // Associated string
   get_id_string(                   // Represent id as a string
     const id&         thread_id)   // The thread id
{  return utility::to_string(thread_id); }

std::string                         // Associated string
   get_id_string( void ) const      // Represent id as a string
{  return utility::to_string(thread.get_id()); }

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
   detach( void );                  // Detach execution thread from this Object

virtual void
   join( void )                     // Wait for this Thread to complete
{  thread.join(); }

// OVERRIDE this method
virtual void
   run( void ) = 0;                 // Operate this thread

void
   start( void );                   // Start this Thread
}; // class Thread
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_THREAD_H_INCLUDED
