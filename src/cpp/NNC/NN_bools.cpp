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
//       NN_bools.cpp
//
// Purpose-
//       NeuralNet: Boolean functions.
//
// Last change date-
//       2007/01/01
//
// Entry points-
//       NN_and_V    nn_rdval(neuron[and])
//       NN_or_V     nn_rdval(neuron[or])
//       NN_nand_V   nn_rdval(neuron[nand])
//       NN_nor_V    nn_rdval(neuron[nor])
//
//----------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>

#include <com/Debug.h>

#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NN_BOOLS" // Source file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_and_V
//
// Function-
//       nn_rdval(neuron[and]).
//
// Notes-
//       resultant= (fanin[0]&fanin[1]&fanin[2]&...&fanin[n])
//
//----------------------------------------------------------------------------
extern NN::Value
   NN_and_V(                        // nn_rdval(neuron[and])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   NN::Value           inpsignal;   // Input signal
   NN::Value           resultant;   // Resultant
   int                 index;       // Element index

   //-------------------------------------------------------------------------
   // Compute the resultant
   //-------------------------------------------------------------------------
   ptr_N->ex.eof= FALSE;            // Initialize
   resultant= TRUE;

   for(index= 0; ; index++)
   {
     inpsignal= nnfanin(ptr_N, fileN, index);
     if (ptr_N->ex.eof == TRUE)
       break;
     if (inpsignal == 0)
       resultant= FALSE;
   }

   return(resultant);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_or_V
//
// Function-
//       nn_rdval(neuron[or]).
//
// Notes-
//       resultant= (fanin[0]|fanin[1]|fanin[2]|...|fanin[n])
//
//----------------------------------------------------------------------------
extern NN::Value
   NN_or_V(                         // nn_rdval(neuron[or])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   NN::Value           inpsignal;   // Input signal
   NN::Value           resultant;   // Resultant
   int                 index;       // Element index

   //-------------------------------------------------------------------------
   // Compute the resultant
   //-------------------------------------------------------------------------
   ptr_N->ex.eof= FALSE;            // Initialize
   resultant= FALSE;

   for(index= 0; ; index++)
   {
     inpsignal= nnfanin(ptr_N, fileN, index);
     if (ptr_N->ex.eof == TRUE)
       break;
     if (inpsignal != 0)
       resultant= TRUE;
   }

   return(resultant);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_nand_V
//
// Function-
//       nn_rdval(neuron[nand]).
//
// Notes-
//       resultant= !(fanin[0]&fanin[1]&fanin[2]&...&fanin[n])
//
//----------------------------------------------------------------------------
extern NN::Value
   NN_nand_V(                       // nn_rdval(neuron[nand])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   NN::Value           inpsignal;   // Input signal
   NN::Value           resultant;   // Resultant
   int                 index;       // Element index

   //-------------------------------------------------------------------------
   // Compute the resultant
   //-------------------------------------------------------------------------
   ptr_N->ex.eof= FALSE;            // Initialize
   resultant= FALSE;

   for(index= 0; ; index++)
   {
     inpsignal= nnfanin(ptr_N, fileN, index);
     if (ptr_N->ex.eof == TRUE)
       break;
     if (inpsignal == 0)
       resultant= TRUE;
   }

   return(resultant);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_nor_V
//
// Function-
//       nn_rdval(neuron[nor]).
//
// Notes-
//       resultant= !(fanin[0]|fanin[1]|fanin[2]|...|fanin[n])
//
//----------------------------------------------------------------------------
extern NN::Value
   NN_nor_V(                        // nn_rdval(neuron[nor])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   NN::Value           inpsignal;   // Input signal
   NN::Value           resultant;   // Resultant
   int                 index;       // Element index

   //-------------------------------------------------------------------------
   // Compute the resultant
   //-------------------------------------------------------------------------
   ptr_N->ex.eof= FALSE;            // Initialize
   resultant= TRUE;

   for(index= 0; ; index++)
   {
     inpsignal= nnfanin(ptr_N, fileN, index);
     if (ptr_N->ex.eof == TRUE)
       break;
     if (inpsignal != 0)
       resultant= FALSE;
   }

   return(resultant);
}

