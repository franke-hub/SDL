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
//       Random.cpp
//
// Purpose-
//       Random implementation.
//
// Last change date-
//       2003/06/18
//
//----------------------------------------------------------------------------
#include <stdio.h>

#include "Random.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "RANDOM  " // Source filename

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
unsigned long        Random::seed= 0x7fffffff; // Current seed

//----------------------------------------------------------------------------
//
// Method-
//       RandomP::RandomP
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   RandomP::RandomP(                // Constructor
     double          p)             // Probability, range (0..1)
{
   set(p);
}

//----------------------------------------------------------------------------
//
// Method-
//       RandomP::get
//
// Purpose-
//       Get the probability value
//
//----------------------------------------------------------------------------
double                              // The current probability, range 0..1
   RandomP::get( void ) const       // Get the probability value
{
   return (double)(p)/((double)(unsigned)(MaxP));
}

//----------------------------------------------------------------------------
//
// Method-
//       RandomP::set
//
// Purpose-
//       Set the probability value
//
//----------------------------------------------------------------------------
void
   RandomP::set(                    // Set the probability value
     double          p)             // Probability, range (0..1)
{
   if( p <= 0.0 )
   {
     this->p= 0;
     return;
   }

   if( p >= 1.0 )
   {
     this->p= MaxP;
     return;
   }

   this->p= (unsigned long)((double)p * (double)((unsigned)MaxP));
}

