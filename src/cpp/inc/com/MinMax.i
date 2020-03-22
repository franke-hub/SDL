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
//       MinMax.i
//
// Purpose-
//       MinMax inline methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef MINMAX_I_INCLUDED
#define MINMAX_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       MinMax::~MinMax
//
// Function-
//       Destructor.
//
//----------------------------------------------------------------------------
   MinMax::~MinMax( void )          // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       MinMax::MinMax
//
// Function-
//       Default constructor.
//
//----------------------------------------------------------------------------
   MinMax::MinMax( void )           // Default constructor
:  minimum(+1.0E300)
,  maximum(-1.0E300)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       MinMax::reset
//
// Function-
//       Reset the MinMax object.
//
//----------------------------------------------------------------------------
void
   MinMax::reset( void )            // Reset the MinMax object
{
   minimum= +1.0E300;
   maximum= -1.0E300;
}

//----------------------------------------------------------------------------
//
// Method-
//       MinMax::sample
//
// Function-
//       Examine sample value
//
//----------------------------------------------------------------------------
double                              // The sample value
   MinMax::sample(                  // Set sample value
     double            value)       // Using this value
{
   if( value < minimum )
     minimum= value;
   if( value > maximum )
     maximum= value;

   return value;
}

//----------------------------------------------------------------------------
//
// Method-
//       MinMax::getMinimum
//
// Function-
//       Extract the minimum value.
//
//----------------------------------------------------------------------------
double                              // The minimum value
   MinMax::getMinimum( void ) const // Extract the minimum value
{
   return minimum;
}

//----------------------------------------------------------------------------
//
// Method-
//       MinMax::getMaximum
//
// Function-
//       Extract the maximum value.
//
//----------------------------------------------------------------------------
double                              // The maximum value
   MinMax::getMaximum( void ) const // Extract the maximum value
{
   return maximum;
}

#endif // MINMAX_I_INCLUDED
