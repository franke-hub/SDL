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
//       Neuron.cpp
//
// Purpose-
//       Neural Net Compiler - Neuron methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/define.h>

#include "NC_com.h"
#include "Neuron.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NEURON  " // Source file, for debugging

//----------------------------------------------------------------------------
//
// Method-
//       Neuron::index
//
// Purpose-
//       Compute Vaddr of Neuron[x]
//
//----------------------------------------------------------------------------
NN::Vaddr                           // Resultant offset
   Neuron::index(                   // Compute Vaddr(Neuron[index])
     NN::Vaddr         base,        // Base Neuron address
     unsigned          index)       // Element index
{
   return base + index * sizeof(Neuron);
}

