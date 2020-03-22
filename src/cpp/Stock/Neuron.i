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
//       Neuron.i
//
// Purpose-
//       Neuron inline functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NEURON_I_INCLUDED
#define NEURON_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Neuron::getValue
//
// Purpose-
//       Extract current Value.
//
//----------------------------------------------------------------------------
Value
   Neuron::getValue( void ) const   // Extract Neuron Value
{
   return(value);                   // Return resultant
}

//----------------------------------------------------------------------------
//
// Method-
//       Neuron::setValue
//
// Purpose-
//       Set the Neuron value.
//
//----------------------------------------------------------------------------
void
   Neuron::setValue(                // Set the Neuron value
     Value           value)         // to this
{
   this->value= value;
}

//----------------------------------------------------------------------------
//
// Method-
//       Neuron::resolve
//
// Purpose-
//       Resolve Neuron value.
//
//----------------------------------------------------------------------------
Value
   Neuron::resolve( void )          // Resolve Neuron Value
{
   Value             value;         // Working value

   #ifdef STATISTICS
     globalCount++;
   #endif

   if( clock == globalClock )       // If already computed
     return this->value;            // Return precomputed value

   clock= globalClock;              // Prevent infinite recursion

   value= compute();                // Compute resultant
   this->value= value;              // Set resultant
   return(value);                   // Return resultant
}

//----------------------------------------------------------------------------
//
// Method-
//       Neuron::setFanin
//
// Purpose-
//       Set the Fanin array.
//
//----------------------------------------------------------------------------
void
   Neuron::setFanin(                // Set the Fanin array
     unsigned        count,         // Number of elements in array
     Fanin*          array)         // -> Fanin array
{
   faninCount= count;
   faninArray= array;
}

//----------------------------------------------------------------------------
//
// Method-
//       Neuron::sigma
//
// Purpose-
//       Compute sum of inputs.
//
//----------------------------------------------------------------------------
Value
   Neuron::sigma( void )            // Compute sum of inputs
{
   Value             resultant;     // Resultant

   unsigned          i;

   //-------------------------------------------------------------------------
   // Read the inputs
   //-------------------------------------------------------------------------
   resultant= 0.0;
   for(i= 0; i<faninCount; i++)
   {
     resultant += faninArray[i].resolve();
   }

   return(resultant);
}

#endif // NEURON_I_INCLUDED
