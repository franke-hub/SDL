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
//       RecursiveBarrier.cpp
//
// Purpose-
//       RecursiveBarrier object methods.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <com/Atomic.h>
#include <com/Debug.h>
#include <com/Software.h>
#include <com/Thread.h>

#include "com/RecursiveBarrier.h"

//----------------------------------------------------------------------------
//
// Method-
//       RecursiveBarrier::attempt
//
// Purpose-
//       Conditionally obtain exclusive access to a resource.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 iff successful
   RecursiveBarrier::attempt( void ) // Attempt to obtain the RecursiveBarrier latch
{
   volatile void* oldValue= barrier;
   void* newValue= (void*)Software::getTid();
   while( oldValue == NULL )
   {
     int cc= csp(&barrier, oldValue, newValue);
     if( cc == 0 )
       return 0;

     oldValue= barrier;
   }

   return 1;
}

//----------------------------------------------------------------------------
//
// Method-
//       RecursiveBarrier::obtain
//
// Purpose-
//       Unconditionally obtain exclusive access to a resource.
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 iff first holder
   RecursiveBarrier::obtain( void ) // Obtain the RecursiveBarrier latch
{
   void* newValue= (void*)Software::getTid();
   for(unsigned count= 1;;count++)
   {
     if( attempt() == 0 )
       return 0;

     if( barrier == newValue )
       return 1;

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
//       RecursiveBarrier::release
//
// Purpose-
//       Release the Barrier latch.
//
//----------------------------------------------------------------------------
void
   RecursiveBarrier::release( void ) // Release the RecursiveBarrier latch
{
   void* oldValue= (void*)Software::getTid();
   if( barrier != oldValue )
     throw "RecursiveBarrier::release latch not held";

   barrier= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       RecursiveBarrier::reset
//
// Purpose-
//       Reset the barrier (perhaps in lieu of constructing it.)
//
//----------------------------------------------------------------------------
void
   RecursiveBarrier::reset( void )  // Reset the RecursiveBarrier
{
   barrier= NULL;
}

