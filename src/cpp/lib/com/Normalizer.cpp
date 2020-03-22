//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Normalizer.cpp
//
// Purpose-
//       Normalizer object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>

#include "com/Normalizer.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NMALIZER" // Source file name, for debugging

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define EPSILON             1.0E-16 // A small value

//----------------------------------------------------------------------------
//
// Method-
//       Normalizer::~Normalizer
//
// Function-
//       Destructor.
//
//----------------------------------------------------------------------------
   Normalizer::~Normalizer( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Normalizer::Normalizer
//
// Function-
//       Constructor.
//
//----------------------------------------------------------------------------
   Normalizer::Normalizer( void )   // Constructor
:  nomNorm(0.0)
,  nomData(0.0)
,  toNormal(0.0)
,  unNormal(0.0)
{
}

   Normalizer::Normalizer(          // Constructor
     double            minNorm,     // Minimum normalized value
     double            maxNorm,     // Maximum normalized value
     double            minData,     // Minimum data value
     double            maxData)     // Maximum data value
{
   initialize(minNorm, maxNorm, minData, maxData);
}

//----------------------------------------------------------------------------
//
// Method-
//       Normalizer::initialize
//
// Function-
//       Initialize the Normalizer.
//
//----------------------------------------------------------------------------
void
   Normalizer::initialize(          // Initialize the normalizer
     double            minNorm,     // Minimum normalized value
     double            maxNorm,     // Maximum normalized value
     double            minData,     // Minimum data value
     double            maxData)     // Maximum data value
{
   #ifdef HCDM
     printf("%s %4d: Normalizer(%p)::initialize(%g,%g,%g,%g)\n",
            __SOURCE__, __LINE__, this,
            minNorm, maxNorm, minData, maxData);
   #endif

   nomData= minData/2.0 + maxData/2.0;
   nomNorm= minNorm/2.0 + maxNorm/2.0;

   if( fabs(minNorm-maxNorm) < EPSILON || fabs(minData-maxData) < EPSILON )
   {
     toNormal= 0.0;
     unNormal= 1.0;
   }
   else
   {
     toNormal= (maxNorm - minNorm) / (maxData - minData);
     unNormal= (maxData - minData) / (maxNorm - minNorm);
   }

   #ifdef HCDM
     printf("%s %4d: nomData(%g) nomNorm(%g) toNorm(%g) unNorm(%g)\n",
            __SOURCE__, __LINE__, nomData, nomNorm, toNormal, unNormal);
   #endif
}

