//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2014 Frank Eskesen.
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
//       User view of a Thread.
//
// Last change date-
//       2014/01/01
//
// Notes-
//       1) A Thread is a separate unit of execution.  Multiple Threads
//          may run concurrently within a Process.  However, Threads
//          must not require this level of concurrent operation.  Threads
//          must periodically yield control either by waiting for an event
//          (such as an I/O event) or explicity with Thread::yield().
//
//       2) A Thread runs with a separate stack, but otherwise shares
//          Process resources.
//          In particular, the system calls "::exit()" and "::abort()"
//          cause the Process (not just the Thread) to exit or abort.
//
//       3) A Thread begins when the start() method is invoked.  Control
//          begins in the run() method, which must be supplied in a
//          derived class.  A Thread terminates either when the run()
//          method completes, or when Thread::exit() is invoked.
//
//       4) Once started, a Thread continues to run even if the Thread object
//          is destroyed.  In this case the Thread is called a zombie.
//          Zombie threads must not access the Thread object itself, but
//          may use the static thread methods.  Thread::current() returns
//          NULL for a zombie thread.
//
//       5) The notify method is user replaceable, and is provided as a
//          mechanism for interrupting a Thread. The base class implementation
//          ignores notifications and returns -1.
//
//       6) A Thread's priority is relative to other Threads. Positive values
//          indicate higher priority, negative values lower.
//
//----------------------------------------------------------------------------
#ifndef THREAD_H_INCLUDED
#define THREAD_H_INCLUDED

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class _SystemThread;                // The system Thread (OS Dependent)

//----------------------------------------------------------------------------
//
// Class-
//       Thread
//
// Purpose-
//       Thread descriptor.
//
//----------------------------------------------------------------------------
class Thread {                      // Thread descriptor
friend class _SystemThread;         // The system Thread

//----------------------------------------------------------------------------
// Thread::Attributes
//----------------------------------------------------------------------------
protected:
void*                  object;      // Hidden Object

//----------------------------------------------------------------------------
// Thread::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Thread( void );                 // Destructor
   Thread( void );                  // Constructor

private:                            // Bitwise copy is prohibited
   Thread(const Thread&);           // Disallowed copy constructor
   Thread& operator=(const Thread&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// Thread::Accessor methods
//----------------------------------------------------------------------------
public:
int                                 // The Thread's relative priority
   getPriority( void ) const;       // Get Thread's priority (positive higher)

void
   setPriority(                     // Change the Thread's priority
     int               priority);   // Relative priority (negative lower)

unsigned long                       // The Thread's stack size
   getStackSize( void ) const;      // Get Thread's stack size

void
   setStackSize(                    // Set Thread's stack size
     unsigned long     size);       // To this value

unsigned long                       // The Thread's unique identifier
   getThreadID( void ) const;       // Get Thread's unique identifier

//----------------------------------------------------------------------------
// Thread::cancel()
//
// Terminate Thread processing.
//----------------------------------------------------------------------------
public:
virtual void
   cancel( void );                  // Terminate the Thread

//----------------------------------------------------------------------------
// Thread::notify()
//
// This method is called from outside of the Thread. There is no intrinsic
// meaning for this notification, usage is entirely up to the user program.
//----------------------------------------------------------------------------
public:
virtual int                         // Return code (Thread dependent)
   notify(                          // Notify the Thread
     int               code);       // Using this notification code

//----------------------------------------------------------------------------
// Thread::run()
//
// This user-supplied method performs the Thread function.
// It is called (under control of a new operating system thread) by start().
//----------------------------------------------------------------------------
protected:
virtual long                        // The Thread's return code
   run( void ) = 0;                 // The Thread's user-supplied method

//----------------------------------------------------------------------------
// Thread::start()
//
// This method creates a physical thread, associates it with this Object,
// then drives Thread::run()
//----------------------------------------------------------------------------
public:
void
   start( void );                   // Start thread operation

//----------------------------------------------------------------------------
// Thread::wait()
//
// This method waits for Thread::run() to complete and returns its return code
//----------------------------------------------------------------------------
public:
virtual long                        // The Thread's return code
   wait( void );                    // Wait for Thread to complete

//----------------------------------------------------------------------------
// Thread::Static methods (operate upon the current Thread)
//----------------------------------------------------------------------------
public:
static Thread*                      // -> current Thread
   current( void );                 // Extract the current Thread

static void
   exit(                            // Exit the current Thread
     long              returnCode); // The Thread's return code

static void
   sleep(                           // Suspend the current Thread
     double            secs);       // For this many seconds

static void
   yield( void );                   // Yield control to another Thread
}; // class Thread

//----------------------------------------------------------------------------
//
// Class-
//       NamedThread
//
// Purpose-
//       Define Thread with a name attribute
//
//----------------------------------------------------------------------------
class NamedThread : public Thread {
//----------------------------------------------------------------------------
// NamedThread::Attributes
//----------------------------------------------------------------------------
protected:
const char*            name;        // The name of the Thread

//----------------------------------------------------------------------------
// NamedThread::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~NamedThread( void )             // Destructor
{ }

inline
   NamedThread(                     // Constructor
     const char*       name)        // The name of the Thread
:  Thread(),  name(name)
{ }

//----------------------------------------------------------------------------
// NamedThread::Accessors
//----------------------------------------------------------------------------
public:
inline const char*                  // The name of the Thread
   getName( void ) const            // Get name of the Thread
{  return name; }
}; // class NamedThread

#endif // THREAD_H_INCLUDED
