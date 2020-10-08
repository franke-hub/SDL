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
//       testAsync.cpp
//
// Purpose-
//       Test std::async
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <future>
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
//       Async
//
// Purpose-
//       Define a pseudo-thread classm=, using underlying std::async
//
//----------------------------------------------------------------------------
namespace fne {
class Async {
protected:
std::future<void>      _thread;     // The async (while active)

public:
virtual
   ~Async( void ) {}
   Async( void ) : _thread() {}

void
   start( void )
{
   std::future<void> t= std::async(std::launch::async, Async::_run, this);
   _thread= std::move(t);
}

void
   join( void )
{
   _thread.get();
   std::future<void> t= std::move(_thread);
}

protected:
static void
   _run( void* that )
{  ((Async*)that)->run(); }

virtual void
   run( void ) = 0;                 // Override this method

virtual void
   test(                            // Run a test
     const char*       name)        // Async specifier
{
   wtlc(LevelInfo, "Async test(%s)\n", name);

   for(int i= 0; i<5; i++)
   {
     wtlc(LevelInfo, "Async %s blip\n", name);
     sleep(1);
   }
}
}; // class Async
}  // namespace fne

//----------------------------------------------------------------------------
//
// Subroutine-
//       testAsync
//
// Purpose-
//       Test std::async
//
//----------------------------------------------------------------------------
extern void testAsync( void );      // (Not very far) Forward reference
extern void
   testAsync( void )
{
   wtlc(LevelStd, "testAsync()\n");

   class : public fne::Async {
       protected:
       virtual void run( void ) {
           test("one");
       }
   } one;

   class : public fne::Async {
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

