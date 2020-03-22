//----------------------------------------------------------------------------
//
//       Copyright (C) 2003 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Random.i
//
// Purpose-
//       Random number inline methods.
//
// Last change date-
//       2003/03/16
//
//----------------------------------------------------------------------------
#ifndef RANDOM_I_INCLUDED
#define RANDOM_I_INCLUDED

#include <assert.h>

//----------------------------------------------------------------------------
//
// Method-
//       Random::get
//
// Function-
//       Get the next pseudo-random value.
//
//----------------------------------------------------------------------------
unsigned long                       // The next pseudo-random value
   Random::get( void )              // Get the next pseudo-random value
{
   unsigned long     temp;

   temp= seed;

   assert(temp != 0);

   temp= temp^(temp>>13);
   temp ^= temp<<18;
   temp &= 0x7fffffffUL;
   seed= temp;
   return temp;
}

//----------------------------------------------------------------------------
//
// Method-
//       Random::getSeed
//
// Function-
//       Get the current pseudo-random value.
//
//----------------------------------------------------------------------------
unsigned long                       // The current pseudo-random value
   Random::getSeed( void )          // Get the current pseudo-random value
{
   if ( seed == 0 )                 // (Zero hashes to zero)
     seed= 0x7fffffffUL;            // Default seed

   return seed;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Random::setSeed
//
// Function-
//       Set the pseudo-random seed value.
//
//----------------------------------------------------------------------------
void
   Random::setSeed(                 // Set the seed value
     unsigned long   seed)          // To this value
{
   seed &= 0x7fffffffUL;            // The high-order bit is always zero
   if ( seed == 0 )                 // (Zero hashes to zero)
     seed= 0x7fffffffUL;            // Default seed

   Random::seed= seed;
}

//----------------------------------------------------------------------------
//
// Method-
//       RandomP::isTrue
//
// Purpose-
//       Return TRUE with probability P.
//
//----------------------------------------------------------------------------
int                                 // TRUE || FALSE
   RandomP::isTrue( void ) const    // This will be TRUE with probability P
{
   unsigned long     r= Random::get(); // Get a random number, 0..(Max-1)

// #if RandomP::MaxMask != 0x7FFFFFFF
//    r &= MaxMask;
// #endif

   return ( p > r );
}

//----------------------------------------------------------------------------
//
// Method-
//       RandomP::ratio
//
// Purpose-
//       Return the integer probability ratio (n * p)
//
//----------------------------------------------------------------------------
unsigned long                       // The integer ratio (n * p)
   RandomP::ratio(                  // Get probability ratio
     unsigned long  n) const        // Number of elements
{
   return (unsigned long)((double)n * (double)p / ((double)(unsigned)MaxP));
}

#endif // RANDOM_I_INCLUDED
