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
//       NN_store.cpp
//
// Purpose-
//       NeuralNet: Store functions.
//
// Last change date-
//       2007/01/01
//
// Entry points-
//       NN_store_V  nn_rdval(neuron[store])
//
//----------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>

#include <com/Debug.h>

#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NN_STORE" // Source file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_store_V
//
// Function-
//       NN_RDVAL(neuron[store]).
//
//----------------------------------------------------------------------------
extern NN::Value NN_store_V(Neuron*, NN::FileId); // (Defined where used)
extern NN::Value
   NN_store_V(                      // nn_rdval(neuron[store])
     Neuron*           ptrN,        // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   Fanin*              ptrF;        // -> Fanin (Internal address)
   Neuron*             setN;        // -> Neuron (Internal address)

   unsigned            fanix;       // Fanin index
   NN::Offset          offset;      // -> Fanin (Internal, current)

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   offset= ptrN->faninVaddr;       // Address the 1st fanin

   //-------------------------------------------------------------------------
   // Set each neuron to the associated weight
   //-------------------------------------------------------------------------
   for( fanix=0; fanix < ptrN->faninCount; fanix++) // Calculate resultant
   {
     ptrF= ref_fanin(fileN, offset); // Access the fanin
     if( ptrF == NULL )
     {
       nndamage(fileN, ptrN, offset); // Indicate neuron damage
       break;
     }

     setN= chg_neuron(ptrF->fileId, ptrF->neuron); // Access the neuron
     if( setN != NULL )             // If access succeeded
     {
       setN->value= ptrF->weight;   // Reset the neuron's output value
       rel_neuron(ptrF->fileId, ptrF->neuron); // Release the neuron
     }

     rel_fanin(fileN, offset);      // Release the current FANIN
     offset += sizeof(Fanin);       // Address the next FANIN
   }

   //-------------------------------------------------------------------------
   // Output value is constant
   //-------------------------------------------------------------------------
   return(ptrN->value);
}

