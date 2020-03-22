//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       testThread.cpp
//
// Purpose-
//       Test std::thread
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <thread>
#include <unistd.h>

#include "Main.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Class-
//       Thread
//
// Purpose-
//       Define a pseudo-thread class.
//
//----------------------------------------------------------------------------
namespace fne {
class Thread {
protected:
std::thread            _thread;     // The thread (while active)

public:
virtual
   ~Thread( void ) {}
   Thread( void ) : _thread() {}

void
   start( void )
{
   std::thread t(Thread::_run, this);
   _thread= std::move(t);
}

void
   join( void )
{
   _thread.join();
   std::thread t= std::move(_thread);
}

protected:
static void
   _run( void* that )
{  ((Thread*)that)->run(); }

virtual void
   run( void ) = 0;                 // Override this method

virtual void
   test(                            // Run a test
     const char*       name)        // Thread specifier
{
   wtlc(LevelInfo, "Thread test(%s)\n", name);

   for(int i= 0; i<5; i++)
   {
     wtlc(LevelInfo, "Thread %s blip\n", name);
     sleep(1);
   }
}
}; // class Thread
}  // namespace fne

//----------------------------------------------------------------------------
//
// Subroutine-
//       testThread
//
// Purpose-
//       Test std::thread
//
//----------------------------------------------------------------------------
extern void
   testThread( void )
{
   wtlc(LevelStd, "testThread()\n");

   class : public fne::Thread {
       protected:
       virtual void run( void ) {
           test("one");
       }
   } one;

   class : public fne::Thread {
       protected:
       virtual void run( void ) {
           test("two");
       }
   } two;

   one.start();
   two.start();
   one.join();
   two.join();
}

