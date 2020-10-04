//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Latch.cpp
//
// Purpose-
//       Instantiate Latch methods.
//
// Last change date-
//       2020/10/03
//
// Latchword format-
//       xxxx.ssss
//           xxxx 16bit reserve count, the number of exclusive reserves
//           ssss 16bit shared count, the number of sharers
//
//       A Latch cannot be obtained shared if an exclusive reserve exists.
//
// Latchword transitions-
//       Obtain shared:
//           0000.nnnn => 0000.(nnnn+1) increment the number of sharers.
//                        Capacity Error if nnnn == 0xfffe
//
//       Release shared:
//           mmmm.nnnn => mmmm.(nnnn-1) decrement the number of sharers.
//                        Error if nnnn == 0xffff || nnnn == 0x0000
//
//       Obtain exclusive(without reserve):
//           0000.0000 => 0001.ffff     Reserve and obtain the latch
//
//           mmmm.nnnn => (mmmm+1).nnnn Reserve the latch (nnnn != 0x0000)
//                        Capacity Error if mmmm == 0xfffe
//
//       Obtain exclusive(already reserved):
//           mmmm.0000 => mmmm.ffff Obtain the latch
//                        Error if mmmm == 0x0000
//
//       Release exclusive:
//           mmmm.nnnn => (mmmm-1).0000 Release the latch
//                        Error if nnnn != 0xffff || mmmm == 0x0000
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/Atomic.h>
#include <com/Debug.h>
#include <com/Exception.h>
#include <com/SharedMem.h>
#include <com/Thread.h>

#include "com/Latch.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef USE_TIMEOUT_ABORT
#undef  USE_TIMEOUT_ABORT           // If defined, enable timeout abort
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       spinDelay
//
// Purpose-
//       Delay before re-attempting to obtain a Latch
//
//----------------------------------------------------------------------------
inline unsigned                     // The next interval
   spinDelay(                       // Delay
     unsigned          count)       // Interval count
{
   Thread::yield();
   if( (++count % 256) == 0 )
   {
     #ifdef USE_TIMEOUT_ABORT
       if( count > 1048576 )
         throwf("%4d Latch::spinDelay TIMEOUT", __LINE__);
     #endif

     double delay= double(count) / 1048576.0;
     if( delay > 0.015625 )
       delay= 0.015625;

     Thread::sleep(delay);
   }

   return count;
}

//----------------------------------------------------------------------------
//
// Method-
//       Latch::~Latch
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Latch::~Latch( void )            // Destructor
{
   IFHCDM( debugf("%4d Latch(%p)::~Latch()\n", __LINE__, this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       Latch::Latch
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Latch::Latch( void )             // Constructor
:  latchWord(0)
{
   IFHCDM( debugf("%4d Latch(%p)::Latch()\n", __LINE__, this); )
}

//----------------------------------------------------------------------------
//
// Method-
//       Latch::isHeldSHR
//
// Purpose-
//       Is this latch held in shared mode?
//
//----------------------------------------------------------------------------
int                                 // TRUE if latch held in shared mode
   Latch::isHeldSHR( void ) const   // Is this latch held in shared mode?
{
   int held= (latchWord & 0x0000ffff);

   return (held != 0 && held != 0x0000ffff);
}

//----------------------------------------------------------------------------
//
// Method-
//       Latch::isHeldXCL
//
// Purpose-
//       Is this latch held in exclusive mode?
//
//----------------------------------------------------------------------------
int                                 // TRUE if latch held in exclusive mode
   Latch::isHeldXCL( void ) const   // Is this latch held in exclusive mode?
{
   return ((latchWord & 0x0000ffff) == 0x0000ffff);
}

//----------------------------------------------------------------------------
//
// Method-
//       Latch::attemptSHR
//
// Purpose-
//       Conditionally obtain the Latch in shared mode.
//
//----------------------------------------------------------------------------
int                                 // TRUE if Latch obtained
   Latch::attemptSHR( void )        // Conditionally obtain SHR Latch
{
   IFHCDM( debugf("%4d Latch(%p)::attemptSHR()\n", __LINE__, this); )

   // Attempt to obtain the latch in shared mode
   int cc;
   do
   {
     int32_t oldValue= latchWord;
     if( (oldValue & 0xffff0000) != 0 ) // If a reservation exists
       return FALSE;

     if( oldValue == 0x0000fffe )   // If at capacity
       return FALSE;

     int32_t newValue= oldValue + 1;
     cc= csw((ATOMIC32*)&latchWord, oldValue, newValue);
   } while( cc != 0 );

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       Latch::attemptXCL
//
// Purpose-
//       Conditionally obtain the Latch in exclusive mode.
//
//----------------------------------------------------------------------------
int                                 // TRUE if latch was obtained
   Latch::attemptXCL( void )        // Conditionally obtain XCL Latch
{
   IFHCDM( debugf("%4d Latch(%p)::attemptXCL()\n", __LINE__, this); )

   // Reserve and obtain the latch
   return (csw((ATOMIC32*)&latchWord, 0, 0x0001ffff) == 0);
}

//----------------------------------------------------------------------------
//
// Method-
//       Latch::modifySHR
//
// Purpose-
//       Downgrade the Latch to shared mode.
//
//----------------------------------------------------------------------------
void
   Latch::modifySHR( void )         // Downgrade the Latch to shared mode
{
   IFHCDM( debugf("%4d= Latch(%p)::modifySHR()\n", __LINE__, this); )

   // Downgrade the latch
   int cc;
   do
   {
     int32_t oldValue= latchWord;
     int32_t newValue= oldValue & 0xffff0000;
     assert( (oldValue & 0x0000ffff) == 0x0000ffff ); // Usage error: not XCL
     assert( newValue != 0 );            // Internal error: XCL count zero

     newValue -= 0x00010000;             // Decrement XCL reservations
     newValue |= 0x00000001;             // Set SHR holders to one
     cc= csw((ATOMIC32*)&latchWord, oldValue, newValue);
   } while( cc != 0 );
}

//----------------------------------------------------------------------------
//
// Method-
//       Latch::modifyXCL
//
// Purpose-
//       Upgrade the Latch to exclusive mode.
//
//----------------------------------------------------------------------------
int                                 // TRUE iff successful
   Latch::modifyXCL( void )         // Upgrade the Latch to exclusive mode
{
   IFHCDM( debugf("%4d= Latch(%p)::modifyXCL()\n", __LINE__, this); )

   // Upgrade the latch: Must be only one SHR holder
   int cc;
   do
   {
     int32_t oldValue= latchWord;
     if( (oldValue & 0x0000ffff) != 1 ) // If multiple SHR holders
       return FALSE;                // Cannot upgrade

     uint32_t newValue= oldValue & 0xffff0000; // Reservation count
     if( newValue == 0xfffe0000 )   // If at capacity
       return FALSE;                // Cannot upgrade

     newValue += 0x00010000;        // Increment XCL reservations
     newValue |= 0x0000ffff;        // Indicate held in XCL mode
     cc= csw((ATOMIC32*)&latchWord, oldValue, newValue);
   } while( cc != 0 );

   return TRUE;
}

//----------------------------------------------------------------------------
//
// Method-
//       Latch::obtainSHR
//
// Purpose-
//       Obtain the Latch in shared mode.
//
//----------------------------------------------------------------------------
void
   Latch::obtainSHR( void )         // Obtain the Latch in shared mode
{
   IFHCDM( debugf("%4d Latch(%p)::obtainSHR()\n", __LINE__, this); )

   // Obtain the latch in shared mode
   for(unsigned i= 0;;)
   {
     int32_t oldValue= latchWord;
     if( (oldValue & 0xffff0000) != 0 ) // If an XCL latch reservation exists
       i= spinDelay(i);
     else
     {
       if( oldValue == 0x0000fffe )   // If at capacity
         throwf("%4d Latch(%p)::obtainSHR capacity error", __LINE__, this);

       int32_t newValue= oldValue + 1;
       if( csw((ATOMIC32*)&latchWord, oldValue, newValue) == 0 )
         return;

       i= 0;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Latch::obtainXCL
//
// Purpose-
//       Obtain the Latch in exclusive mode.
//
//----------------------------------------------------------------------------
void
   Latch::obtainXCL( void )         // Obtain the Latch in exclusive mode
{
   int                 cc;          // csw Condition code

   IFHCDM( debugf("%4d Latch(%p)::obtainXCL()\n", __LINE__, this); )

   // Reserve and obtain the latch
   if( csw((ATOMIC32*)&latchWord, 0, 0x0001ffff) == 0 )
     return;

   // Unable to reserve and obtain; reserve the latch
   do
   {
     int32_t oldValue= latchWord;
     if( (oldValue & 0xffff0000) == 0xfffe0000 ) // If at reservation capacity
       throwf("%4d Latch(%p)::obtainXCL capacity error", __LINE__, this);

     int32_t newValue= oldValue + 0x00010000;
     cc= csw((ATOMIC32*)&latchWord, oldValue, newValue);
   } while( cc != 0 );

   // Wait for latch to become available
   for(unsigned i= 0;;)
   {
     int32_t oldValue= latchWord;
     oldValue &= 0xffff0000;        // SHRs must complete; XCL must release
     assert( oldValue != 0 );       // No one else can release our reserve

     int32_t newValue= oldValue | 0x0000ffff;
     if( csw((ATOMIC32*)&latchWord, oldValue, newValue) == 0 )
       return;

     i= spinDelay(i);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Latch::releaseSHR
//
// Purpose-
//       Release the Latch from shared mode.
//
//----------------------------------------------------------------------------
void
   Latch::releaseSHR( void )        // Release the Latch from shared mode
{
   IFHCDM( debugf("%4d Latch(%p)::releaseSHR()\n", __LINE__, this); )

   // Release the latch
   int cc;
   do
   {
     int32_t oldValue= latchWord;
     int32_t newValue= oldValue - 1;
     assert( (oldValue & 0x0000ffff) != 0x00000000 ); // Usage error: not held
     assert( (oldValue & 0x0000ffff) != 0x0000ffff ); // Usage error: held XCL

     cc= csw((ATOMIC32*)&latchWord, oldValue, newValue);
   } while( cc != 0 );
}

//----------------------------------------------------------------------------
//
// Method-
//       Latch::releaseXCL
//
// Purpose-
//       Release the Latch from exclusive mode.
//
//----------------------------------------------------------------------------
void
   Latch::releaseXCL( void )        // Release the Latch from exclusive mode
{
   IFHCDM( debugf("%4d Latch(%p)::releaseXCL()\n", __LINE__, this); )

   // Release the latch
   int cc;
   do
   {
     int32_t oldValue= latchWord;
     int32_t newValue= oldValue & 0xffff0000;
     assert( (oldValue & 0x0000ffff) == 0x0000ffff ); // Usage error: not XCL
     assert( newValue != 0x00000000 ); // Internal error: XCL count zero

     newValue -= 0x00010000;
     cc= csw((ATOMIC32*)&latchWord, oldValue, newValue);
   } while( cc != 0 );
}

