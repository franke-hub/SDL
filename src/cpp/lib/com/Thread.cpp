//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2023 Frank Eskesen.
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
//       Thread object methods.
//
// Last change date-
//       2023/05/12
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include <com/Atomic.h>
#include <com/Barrier.h>
#include <com/define.h>
#include <com/Debug.h>
#include <com/List.h>
#include <com/Mutex.h>

#include "com/Thread.h"

#ifdef _OS_WIN
#include <windows.h>
#endif

#ifdef _OS_BSD
#include <pthread.h>                // Must be first
#include <sched.h>
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard-Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Class-
//       _SystemThread
//
// Purpose-
//       Define the system Thread, a Thread friend
//
//----------------------------------------------------------------------------
class _SystemThread : public Thread { // _SystemThread descriptor
using Thread::cancel;               // (Avoids compiler error messages)
using Thread::wait;                 // (Avoids compiler error messages)

public:
virtual
   ~_SystemThread( void );          // Destructor
   _SystemThread( void );           // Constructor

//----------------------------------------------------------------------------
// _SystemThread::Methods
//----------------------------------------------------------------------------
virtual long                        // Always 0
   run( void );                     // Does nothing

//----------------------------------------------------------------------------
// _SystemThread::Static methods (operate upon the current Thread)
//----------------------------------------------------------------------------
static Thread*                      // -> current Thread
   current( void );                 // Extract the current Thread

static void
   exit(                            // Exit the current Thread
     long              returnCode); // The Thread's return code

static void
   sleep(                           // Suspend the current Thread
     unsigned          secs,        // For this many seconds
     unsigned          nsec);       // and this many nanoseconds

static void
   yield( void );                   // Yield control to another Thread

//----------------------------------------------------------------------------
// _SystemThread::Static methods (operate upon the supplied Thread)
//----------------------------------------------------------------------------
static int                          // The Thread's priority
   getPriority(                     // Get Thread's priority
     const Thread*     thread);     // For this Thread

static void
   setPriority(                     // Set Thread's priority
     Thread*           thread,      // For this Thread
     int               priority);   // To this value

static unsigned long                // The Thread's stack size
   getStackSize(                    // Get Thread's stack size
     const Thread*     thread);     // For this Thread

static void
   setStackSize(                    // Set Thread's stack size
     Thread*           thread,      // For this Thread
     unsigned long     size);       // To this value

static unsigned long                // The Thread's identifier
   getThreadID(                     // Get Thread's identifier
     const Thread*     thread);     // For this Thread

static void
   cancel(                          // Cancel
     Thread*           thread);     // This Thread

static void
   create(                          // Create
     Thread*           thread);     // This Thread

static void
   destroy(                         // Destroy
     Thread*           thread);     // This Thread

static long                         // The Thread's return code
   run(                             // Run
     Thread*           thread);     // This Thread

static void
   start(                           // Start
     Thread*           thread);     // This Thread

static long                         // The Thread's return code
   wait(                            // Wait for
     Thread*           thread);     // This Thread to complete
}; // class _SystemThread

#if   defined(_OS_WIN)
#include "OS/WIN/Thread.cpp"

#elif defined(_OS_BSD)
#include "OS/BSD/Thread.cpp"

#else
#error "Invalid OS"
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       Thread::~Thread
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Thread::~Thread()                // Destructor
{
   _SystemThread::destroy(this);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Thread::Thread
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Thread::Thread()                 // Constructor
{
   _SystemThread::create(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::getPriority
//
// Purpose-
//       Get the priority.
//
//----------------------------------------------------------------------------
int                                  // The priority
   Thread::getPriority( void ) const // Get the priority
{
   return _SystemThread::getPriority(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::setPriority
//
// Purpose-
//       Update the priority.
//
//----------------------------------------------------------------------------
void
   Thread::setPriority(             // Update the priority
     int               priority)    // To this value
{
   _SystemThread::setPriority(this, priority);
}


//----------------------------------------------------------------------------
//
// Method-
//       Thread::getStackSize
//
// Purpose-
//       Get the stack size.
//
//----------------------------------------------------------------------------
unsigned long                        // The stack size
   Thread::getStackSize( void ) const// Get the stack size
{
   return _SystemThread::getStackSize(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::setStackSize
//
// Purpose-
//       Set the stack size.
//
//----------------------------------------------------------------------------
void
   Thread::setStackSize(            // Set the stack size
     unsigned long     size)        // To this value
{
   _SystemThread::setStackSize(this, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::getThreadID
//
// Purpose-
//       Get the thread identifier
//
//----------------------------------------------------------------------------
unsigned long                       // The thread identifier
   Thread::getThreadID( void ) const// Get thread identifier
{
   return _SystemThread::getThreadID(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::cancel
//
// Purpose-
//       Cancel the Thread.
//
//----------------------------------------------------------------------------
void
   Thread::cancel( void )           // Cancel the Thread
{
   _SystemThread::cancel(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::notify
//
// Purpose-
//       Notify the Thread.
//
//----------------------------------------------------------------------------
int                                 // Return code (UNDEFINED)
   Thread::notify(                  // Notify the Thread
     int               nid)         // Using this notify identifier
{
   (void)nid;                       // Notify identifier ignored
   return (-1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::run
//
// Purpose-
//       Operate a Thread.
//
//----------------------------------------------------------------------------
long                                // Return code (UNDEFINED)
   Thread::run( void )              // Operate the Thread
{
   return (-1);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::start
//
// Purpose-
//       Start the Thread.
//
//----------------------------------------------------------------------------
void
   Thread::start( void )            // Start the Thread
{
   _SystemThread::start(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::wait
//
// Purpose-
//       Wait for Thread completion.
//
//----------------------------------------------------------------------------
long                                // The Thread's return code
   Thread::wait( void )             // Wait for Thread completion
{
   return _SystemThread::wait(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Thread::Static Methods
//
// Purpose-
//       Implement static methods.
//
//----------------------------------------------------------------------------
Thread*                             // -> current Thread
   Thread::current( void )          // Extract the current Thread
{  return _SystemThread::current();
}

void
   Thread::exit(                    // Exit the current Thread
     long              returnCode)  // The Thread's return code
{  _SystemThread::exit(returnCode);
}

void
   Thread::sleep(                   // Suspend the current Thread
     double            time)        // For this many seconds
{
   unsigned secs= unsigned(time);
   unsigned nsec= unsigned((time - secs) * 1000000000.0);
   _SystemThread::sleep(secs, nsec);
}

void
   Thread::yield( void )            // Yield control to another Thread
{  _SystemThread::yield();
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::~_SystemThread
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   _SystemThread::~_SystemThread()  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::_SystemThread
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   _SystemThread::_SystemThread()   // Contructor
:  Thread()
{
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemThread::run
//
// Purpose-
//       Allow object to be created.
//
//----------------------------------------------------------------------------
long
   _SystemThread::run( void )       // Operate the Thread
{
   return 0;
}

