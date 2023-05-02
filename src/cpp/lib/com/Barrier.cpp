//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Barrier.cpp
//
// Purpose-
//       Barrier object methods.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <com/Atomic.h>
#include <com/Debug.h>
#include <com/Thread.h>

#include "com/Barrier.h"

//----------------------------------------------------------------------------
//
// Method-
//       Barrier::attempt
//
// Purpose-
//       Conditionally obtain exclusive access to a resource.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 iff successful
   Barrier::attempt( void )         // Attempt to obtain the Barrier latch
{
   int32_t oldValue= barrier;
   while( oldValue == 0 )
   {
     int cc= csw(&barrier, oldValue, (-1));
     if( cc == 0 )
       break;

     oldValue= barrier;
   }

   return oldValue;
}

//----------------------------------------------------------------------------
//
// Method-
//       Barrier::obtain
//
// Purpose-
//       Unconditionally obtain exclusive access to a resource.
//
//----------------------------------------------------------------------------
void
   Barrier::obtain( void )          // Obtain the Barrier latch
{
   for(unsigned count= 1;;count++)
   {
     int oldValue= attempt();
     if( oldValue == 0 )
       break;

     Thread::yield();               // Let the thread holding the barrier run
     if( (count%1000) == 0 )        // If it's been a while
     {
       double delay= double(count) / 100000.0;
       if( delay > 0.1 )
         delay= 0.1;

       Thread::sleep(delay);
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Barrier::release
//
// Purpose-
//       Release the Barrier latch.
//
//----------------------------------------------------------------------------
void
   Barrier::release( void )         // Release the Barrier latch
{
   barrier= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Barrier::reset
//
// Purpose-
//       Reset the barrier (perhaps in lieu of constructing it.)
//
//----------------------------------------------------------------------------
void
   Barrier::reset( void )           // Reset the Barrier
{
   barrier= 0;
}

