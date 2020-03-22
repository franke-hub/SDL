//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Approximately.cpp
//
// Purpose-
//       Approximately object methods.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <stdlib.h>                 // For rand
#include <com/Debug.h>              // For debugging
#include <com/Random.h>             // (Alternate implemenatation)

#include "Approximately.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define USE_ALTERNATE FALSE         // Use alternate implementation?

//----------------------------------------------------------------------------
// Available random number generators
//----------------------------------------------------------------------------
static PseudoRandom    pseudo;      // Pseudo random number generator

#if( USE_ALTERNATE )
static Random          simple;      // Simple random number generator
static PerfectRandom   perfect;     // "Perfect" random number generator
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Approximately::Approximately
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Approximately::Approximately(    // Constructor
     unsigned          count)       // Initial count
:  exponent(0)
{
   while( count > 0 )
   {
     exponent++;
     count >>= 1;
   }

   pseudo.randomize();              // Preferred

#if( USE_ALTERNATE )
   srand((int)Random::getBit());
   simple.randomize();
   perfect.randomize();
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Approximately::event
//
// Purpose-
//       Count an event.
//
// Implementation notes-
//       For pseudo-random numbers, randomly picking the result first appears
//       to help. This has not been rigorously tested.
//
//       The alternate implementations are ordered in terms of reliability
//       using Tester --testApprox.
//
//----------------------------------------------------------------------------
void
   Approximately::event( void )     // Count an event
{
#if( USE_ALTERNATE == FALSE )       // Preferred implementation
   if( exponent < 255 )             // Maximum allowed exponent
   {
     int flip= pseudo.get() & 1;    // (Pragmatically: better than constant)
     for(int i= 0; i<exponent; i++)
     {
       if( (pseudo.get() & 1) != flip )
         return;
     }

     exponent++;
   }

//----------------------------------------------------------------------------
// Alternate implementations: for comparison
//----------------------------------------------------------------------------
#elif FALSE                         // ALTERNATE implementation (poor)
   if( exponent < 63 )              // Maximum allowed exponent
   {
     uint64_t mask= pseudo.get();     // High order bit always zero
     mask >>= (63 - exponent);
     if( mask == 0 )
       exponent++;
   }

#elif FALSE                         // ALTERNATE implementation (OK)
   if( exponent < 63 )              // Maximum allowed exponent
   {
     uint64_t mask= pseudo.get() ^ pseudo.get(); // High order bit always zero
     mask >>= (63 - exponent);
     if( mask == 0 )
       exponent++;
   }

#elif FALSE                         // ALTERNATE implementation (poor)
   if( exponent < 255 )             // Maximum allowed exponent
   {
     int flip= rand() & 1;
     for(int i= 0; i<exponent; i++)
     {
       if( (rand() & 1) == flip )
         return;
     }

     exponent++;
   }

#elif FALSE                         // ALTERNATE implementation (poor)
   if( exponent < 255 )             // Maximum allowed exponent
   {
     int flip= simple.get() & 1;
     for(int i= 0; i<exponent; i++)
     {
       if( (simple.get() & 1) == flip )
         return;
     }

     exponent++;
   }

#elif FALSE                         // ALTERNATE implementation (OK)
   if( exponent < 255 )             // Maximum allowed exponent
   {
     int flip= perfect.get() & 1;
     for(int i= 0; i<exponent; i++)
     {
       if( (perfect.get() & 1) == flip )
         return;
     }

     exponent++;
   }

#elif FALSE                         // ALTERNATE implementation (OK)
   if( exponent < 63 )              // Maximum allowed exponent
   {
     uint64_t mask= perfect.get();    // High order bit always zero
     mask >>= (63 - exponent);
     if( mask == 0 )
       exponent++;
   }

#elif FALSE                         // ALTERNATE implementation (OK)
   if( exponent < 255 )             // Maximum allowed exponent
   {
     int flip= Random::getBit() & 1;
     for(int i= 0; i<exponent; i++)
     {
       if( (Random::getBit() & 1) == flip )
         return;
     }

     exponent++;
   }

#elif FALSE                         // ALTERNATE implementation (mixed)
   if( exponent < 63 )              // Maximum allowed exponent
   {
     uint64_t mask= Random::getBit(); // High order bit always zero
     mask >>= (63 - exponent);
     if( mask == 0 )
       exponent++;
   }

#else
   #error "No implementation defined"
#endif
}

