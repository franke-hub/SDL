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
//       NNback.cpp
//
// Purpose-
//       Back propagation training algorithm.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <com/Debug.h>

#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NNBACK  " // Source file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       nnback
//
// Purpose-
//       Back propagation training algorigthm
//
//----------------------------------------------------------------------------
#if 0
extern NN::Value nnback(Neuron*, NN::FO*); // (Defined where used)
extern NN::Value                    // Resultant
   nnback(                          // Back propagation training
     Neuron*           ptrN,        // -> Neuron (Internal address)
     NN::FO*           fxoN)        // -> Neuron (External address)
{                                   // TODO: REMOVE: UNUSED??
   (void)ptrN; (void)fxoN;          // (Parameters unused)
   return(0.0);                     // Function not defined
}
#endif

