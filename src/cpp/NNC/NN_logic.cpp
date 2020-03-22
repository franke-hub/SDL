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
//       NN_logic.cpp
//
// Purpose-
//       NeuralNet: Logic control functions.
//
// Last change date-
//       2007/01/01
//
// Entry points-
//       NN_until_V  nn_rdval(neuron[until])
//       NN_while_V  nn_rdval(neuron[while])
//
//----------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>

#include <com/Debug.h>

#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NN_LOGIC" // Source file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_until_V
//
// Function-
//       NN_RDVAL(neuron[until]).
//
//----------------------------------------------------------------------------
extern NN::Value
   NN_until_V(                      // nn_rdval(neuron[until])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   NN::Value           resultant;   // Resultant

   //-------------------------------------------------------------------------
   // Until loop
   //-------------------------------------------------------------------------
   for(;;)
   {
     //-----------------------------------------------------------------------
     // Read the fanin list
     //-----------------------------------------------------------------------
     nnfinop(ptr_N, fileN);         // Read the fanin list

     //-----------------------------------------------------------------------
     // Read the control neuron
     //-----------------------------------------------------------------------
     resultant= nnfanin(ptr_N, fileN, 0); // Read fanin[0]
     if( resultant <= 0 )
       break;

     //-----------------------------------------------------------------------
     // Prevent useless recursion
     //-----------------------------------------------------------------------
//   if( (ptr_N->clock == NN_COM.clock
//        &&ptr_N->train == NN_COM.train ) // If resultant already computed
//       ||ptr_N->ex.disabled )     // or the neuron is disabled
     if( ptr_N->clock == NN_COM.clock // If resultant already computed
         ||ptr_N->ex.disabled )     // or the neuron is disabled
       break;
     }

   //-------------------------------------------------------------------------
   // Return resultant
   //-------------------------------------------------------------------------
   return(resultant);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_WHILE_V
//
// Function-
//       NN_RDVAL(neuron[while]).
//
//----------------------------------------------------------------------------
extern NN::Value
   NN_while_V(                      // nn_rdval(neuron[while])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
  {
   NN::Value           resultant;   // Resultant

   //-------------------------------------------------------------------------
   // While loop
   //-------------------------------------------------------------------------
   for(;;)
   {
     //-----------------------------------------------------------------------
     // Read the control neuron
     //-----------------------------------------------------------------------
     resultant= nnfanin(ptr_N, fileN, 0); // Read fanin[0]
     if( resultant <= 0 )
       break;

     //-----------------------------------------------------------------------
     // Read the fanin list
     //-----------------------------------------------------------------------
     nnfinop(ptr_N, fileN);         // Read the fanin list

     //-----------------------------------------------------------------------
     // Prevent useless recursion
     //-----------------------------------------------------------------------
//   if( (ptr_N->clock == NN_COM.clock
//        &&ptr_N->train == NN_COM.train ) // If resultant already computed
//       ||ptr_N->ex.disabled )     // or the neuron is disabled
     if( ptr_N->clock == NN_COM.clock // If resultant already computed
         ||ptr_N->ex.disabled )     // or the neuron is disabled
       break;
   }

   //-------------------------------------------------------------------------
   // Return resultant
   //-------------------------------------------------------------------------
   return(resultant);
}

