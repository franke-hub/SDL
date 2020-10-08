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
//       NN_arith.cpp
//
// Purpose-
//       NeuralNet: Arithmetic functions.
//
// Last change date-
//       2007/01/01
//
// Entry points-
//       NN_const_V  nn_rdval(neuron[constant])
//       NN_incr1_V  nn_rdval(neuron[inc])
//       NN_decr1_V  nn_rdval(neuron[dec])
//       NN_add_V    nn_rdval(neuron[add])
//       NN_sub_V    nn_rdval(neuron[sub])
//       NN_mul_V    nn_rdval(neuron[mul])
//       NN_div_V    nn_rdval(neuron[div])
//     * NN_abs_V    nn_rdval(neuron[abs])
//     * NN_neg_V    nn_rdval(neuron[neg])
//       NN_sigmd_V  nn_rdval(neuron[sigmoid]).
//     * (Not implemented yet)
//
//----------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>

#include <com/Debug.h>

#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NN_ARITH" // Source file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_const_V
//
// Function-
//       NN_RDVAL(neuron[constant]).
//
//----------------------------------------------------------------------------
extern NN::Value NN_const_V(Neuron*, NN::FileId); // (Defined where used)
extern NN::Value
   NN_const_V(                      // nn_rdval(neuron[constant])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   //-------------------------------------------------------------------------
   // Read (but ignore) the inputs
   //-------------------------------------------------------------------------
   nnfinop(ptr_N, fileN);

   //-------------------------------------------------------------------------
   // Return the (unchanged) output value
   //-------------------------------------------------------------------------
   return(ptr_N->value);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_incr1_V
//
// Function-
//       NN_RDVAL(neuron[inc]).
//
// Notes-
//       resultant= (fanin[0]+fanin[1]+fanin[2]+...+fanin[n])+1
//
//----------------------------------------------------------------------------
extern NN::Value NN_incr1_V(Neuron*, NN::FileId); // (Defined where used)
extern NN::Value
   NN_incr1_V(                      // nn_rdval(neuron[inc])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   NN::Value           inpsignal;   // Weighted input signal
   NN::Value           resultant;   // Resultant

   //-------------------------------------------------------------------------
   // Read the inputs
   //-------------------------------------------------------------------------
   inpsignal= nnsigma(ptr_N, fileN);// Get the input signal

   //-------------------------------------------------------------------------
   // Calculate new output value
   //-------------------------------------------------------------------------
   resultant= inpsignal + 1.0;      // Compute the resultant

   return(resultant);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_decr1_V
//
// Function-
//       NN_RDVAL(neuron[dec]).
//
// Notes-
//       resultant= (fanin[0]+fanin[1]+fanin[2]+...+fanin[n])-1
//
//----------------------------------------------------------------------------
extern NN::Value NN_decr1_V(Neuron*, NN::FileId); // (Defined where used)
extern NN::Value
   NN_decr1_V(                      // nn_rdval(neuron[dec])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   NN::Value           inpsignal;   // Weighted input signal
   NN::Value           resultant;   // Resultant

   //-------------------------------------------------------------------------
   // Read the inputs
   //-------------------------------------------------------------------------
   inpsignal= nnsigma(ptr_N, fileN);// Get the input signal

   //-------------------------------------------------------------------------
   // Calculate new output value
   //-------------------------------------------------------------------------
   resultant= inpsignal - 1.0;      // Compute the resultant

   return(resultant);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_add_V
//
// Function-
//       NN_RDVAL(neuron[add]).
//
// Notes-
//       resultant= fanin[0]+(fanin[1]+fanin[2]+...+fanin[n])
//
//----------------------------------------------------------------------------
extern NN::Value NN_add_V(Neuron*, NN::FileId); // (Defined where used)
extern NN::Value
   NN_add_V(                        // nn_rdval(neuron[add])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   NN::Value           resultant;   // Resultant

   //-------------------------------------------------------------------------
   // Read the inputs
   //-------------------------------------------------------------------------
   resultant= nnsigma(ptr_N, fileN);// Get the input signal

   return(resultant);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_sub_V
//
// Function-
//       NN_RDVAL(neuron[sub]).
//
// Notes-
//       resultant= fanin[0]-(fanin[1]+fanin[2]+...+fanin[n])
//
//----------------------------------------------------------------------------
extern NN::Value NN_sub_V(Neuron*, NN::FileId); // (Defined where used)
extern NN::Value
   NN_sub_V(                        // nn_rdval(neuron[sub])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   NN::Value           resultant;   // First element
   NN::Value           inpsignal;   // Weighted input signal

   //-------------------------------------------------------------------------
   // Read the inputs
   //-------------------------------------------------------------------------
   resultant= nnfanin(ptr_N, fileN, 0);// Get the first element
   inpsignal= nnsigm1(ptr_N, fileN);// Get the input signal

   return(resultant-inpsignal);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_mul_V
//
// Function-
//       NN_RDVAL(neuron[mul]).
//
// Notes-
//       resultant= fanin[0]*(fanin[1]+fanin[2]+...+fanin[n])
//
//----------------------------------------------------------------------------
extern NN::Value NN_mul_V(Neuron*, NN::FileId); // (Defined where used)
extern NN::Value
   NN_mul_V(                        // nn_rdval(neuron[mul])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   NN::Value           resultant;   // First element
   NN::Value           inpsignal;   // Weighted input signal

   //-------------------------------------------------------------------------
   // Read the inputs
   //-------------------------------------------------------------------------
   resultant= nnfanin(ptr_N, fileN, 0);// Get the first element
   inpsignal= nnsigm1(ptr_N, fileN);// Get the input signal

   return(resultant*inpsignal);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_div_V
//
// Function-
//       NN_RDVAL(neuron[div]).
//
// Notes-
//       resultant= fanin[0]/(fanin[1]+fanin[2]+...+fanin[n])
//
//----------------------------------------------------------------------------
extern NN::Value NN_div_V(Neuron*, NN::FileId); // (Defined where used)
extern NN::Value
   NN_div_V(                        // nn_rdval(neuron[div])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   NN::Value           resultant;   // First element
   NN::Value           inpsignal;   // Weighted input signal

   //-------------------------------------------------------------------------
   // Read the inputs
   //-------------------------------------------------------------------------
   resultant= nnfanin(ptr_N, fileN, 0);// Get the first element
   inpsignal= nnsigm1(ptr_N, fileN);// Get the input signal

   return(resultant/inpsignal);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_sigmd_V
//
// Function-
//       NN_RDVAL(neuron[sigmoid]).
//
//----------------------------------------------------------------------------
extern NN::Value NN_sigmd_V(Neuron*, NN::FileId); // (Defined where used)
extern NN::Value
   NN_sigmd_V(                      // nn_rdval(neuron[sigmoid])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   NN::Value           inpsignal;   // Weighted input signal
   NN::Value           resultant;   // Resultant

   //-------------------------------------------------------------------------
   // Read the inputs
   //-------------------------------------------------------------------------
   inpsignal= nnsigma(ptr_N, fileN);// Get the input signal

   //-------------------------------------------------------------------------
   // Calculate new output value
   //-------------------------------------------------------------------------
   resultant= 1.0 / (1.0 + exp(-inpsignal)); // Compute the resultant

   return(resultant);
}

