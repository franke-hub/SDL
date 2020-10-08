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
//       NN_clock.cpp
//
// Purpose-
//       NeuralNet: Clocking functions.
//
// Last change date-
//       2007/01/01
//
// Entry points-
//       NN_clock_V  nn_rdval(neuron[clock])
//
//----------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>

#include <com/Debug.h>

#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NN_CLOCK" // Source file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_clock_V
//
// Function-
//       NN_RDVAL(neuron[clock]).
//
//----------------------------------------------------------------------------
extern NN::Value NN_clock_V(Neuron*, NN::FileId); // (Defined where used)
extern NN::Value
   NN_clock_V(                      // nn_rdval(neuron[clock])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   //-------------------------------------------------------------------------
   // Update the clock
   //-------------------------------------------------------------------------
   NN_COM.clock++;                  // Update the clock
   NN_COM.train= 0;

   //-------------------------------------------------------------------------
   // Read the fanin list
   //-------------------------------------------------------------------------
   nnfinop(ptr_N, fileN);

   //-------------------------------------------------------------------------
   // Return the updated clock value
   //-------------------------------------------------------------------------
   return(NN_COM.clock);
}

