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
//       DarwinPlex.i
//
// Purpose-
//       DarwinPlex inline methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef DARWINPLEX_I_INCLUDED
#define DARWINPLEX_I_INCLUDED

#include <assert.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinPlex::getCull
//
// Purpose-
//       Get the number of Units to cull per generation
//
//----------------------------------------------------------------------------
unsigned int                        // The number of units to cull
   DarwinPlex::getCull( void ) const// Get cull count
{
   double              resultant;   // The number of units to cull

   resultant= double(Random::standard.get()) / double(Random::MAXIMUM);
   resultant *= (double) used;

   return (unsigned int)resultant;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinPlex::getGeneration
//
// Purpose-
//       Get the current generation
//
//----------------------------------------------------------------------------
DarwinPlex::Generation              // The current generation
   DarwinPlex::getGeneration( void ) const // Get current generation
{
   return generation;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinPlex::setGeneration
//
// Purpose-
//       Set the current generation
//
//----------------------------------------------------------------------------
void
   DarwinPlex::setGeneration(       // Set current generation
     Generation        generation)    // To this value
{
   this->generation= generation;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinPlex::getMutation
//
// Purpose-
//       Get the mutation counter.
//
//----------------------------------------------------------------------------
unsigned int                        // The mutation count
   DarwinPlex::getMutation( void ) const // Get mutation counter
{
   return mutation;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinPlex::getUsed
//
// Purpose-
//       Get the number of used elements
//
//----------------------------------------------------------------------------
unsigned int                        // The number of used elements
   DarwinPlex::getUsed( void ) const // Get number of used elements
{
   return used;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinPlex::getUnit
//
// Purpose-
//       Extract DarwinUnit
//
//----------------------------------------------------------------------------
DarwinUnit*                         // -> DarwinUnit
   DarwinPlex::getUnit(             // Get DarwinUnit
     unsigned int      index) const // The DarwinUnit index
{
   assert( index < used );

   return unit[index];              // Return the element
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DarwinPlex::setUnit
//
// Purpose-
//       Set DarwinUnit.
//
//----------------------------------------------------------------------------
unsigned int                        // The unit index
   DarwinPlex::setUnit(             // Set DarwinUnit
     DarwinUnit*       element)     // -> element
{
   // All elements must belong to the same class
   if( className == NULL )
     className= element->className();

   assert( className == element->className() );

   // Insert the element
   assert( used < count );
   unit[used]= element;             // Set the element

   return used++;
}

#endif // DARWINPLEX_I_INCLUDED
