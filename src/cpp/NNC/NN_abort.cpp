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
//       NN_abort.cpp
//
// Purpose-
//       NeuralNet: Abort functions.
//
// Last change date-
//       2007/01/01
//
// Entry points-
//       NN_abort_V  nn_rdval(neuron[abort])
//       NN_abort_S  nn_rdstr(neuron[abort])
//
//----------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <com/Debug.h>

#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NN_ABORT" // Source file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_abort
//
// Function-
//       ABORT: Invalid neuron type.
//
//----------------------------------------------------------------------------
static void
   nn_abort(                        // Neuron[abort]
     const char*       op,          // "str" or "val" operator
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   //-------------------------------------------------------------------------
   // Error abort
   //-------------------------------------------------------------------------
   printf("%s <%d>: neuron read_%s(F:%2d,O:unknown) type[%d], ABORT\n",
          __FILE__, __LINE__, op,
          fileN, ptr_N->type);
   exit(EXIT_FAILURE);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_abort_S
//
// Function-
//       READ_STR(Neuron[abort]).
//
//----------------------------------------------------------------------------
extern NN::String
   NN_abort_S(                      // read_str(neuron[abort])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   //-------------------------------------------------------------------------
   // Error abort
   //-------------------------------------------------------------------------
   nn_abort("str", ptr_N, fileN);
   return(NULL);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       NN_abort_V
//
// Function-
//       NN_RDVAL(neuron[abort]).
//
//----------------------------------------------------------------------------
extern NN::Value
   NN_abort_V(                      // nn_rdval(neuron[abort])
     Neuron*           ptr_N,       // -> Neuron (Internal address)
     NN::FileId        fileN)       // Neuron file identifier
{
   //-------------------------------------------------------------------------
   // Error abort
   //-------------------------------------------------------------------------
   nn_abort("val", ptr_N, fileN);
   return(0);
}

