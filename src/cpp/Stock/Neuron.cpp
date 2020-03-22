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
//       Neuron methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/define.h>

#include "Fanin.h"
#include "Neuron.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NEURON  " // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Global data areas
//----------------------------------------------------------------------------
Tick                 Neuron::globalClock= 0;// The current global
unsigned long        Neuron::globalCount= 0;// The number of reads

//----------------------------------------------------------------------------
// Inlines
//----------------------------------------------------------------------------
#include "Fanin.i"
#include "Neuron.i"

//----------------------------------------------------------------------------
//
// Method-
//       Neuron::~Neuron
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Neuron::~Neuron( void )          // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Neuron::Neuron
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Neuron::Neuron( void )           // Constructor
:  cbid(CBID)
,  clock(0)
,  faninCount(0)
,  faninArray(NULL)
,  value(0.0)
{
   #ifdef HCDM
     debugf("Neuron(%p).Neuron()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Neuron::compute
//
// Purpose-
//       SIGMOID: Compute Neuron value.
//
//----------------------------------------------------------------------------
Value
   Neuron::compute( void )          // Compute Neuron Value
{
   Value             resultant;

   resultant= 1.0 / (1.0 + exp(-sigma()));

   #ifdef HCDM
     debugf("%f= Neuron(%p).compute()\n", resultant, this);
   #endif

   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NeuronValue::~NeuronValue
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NeuronValue::~NeuronValue( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       NeuronValue::NeuronValue
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   NeuronValue::NeuronValue( void )  // Constructor
:  Neuron()
{
   #ifdef HCDM
     debugf("NeuronValue(%p).NeuronValue()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       NeuronValue::compute
//
// Purpose-
//       Compute Neuron value.
//
//----------------------------------------------------------------------------
Value
   NeuronValue::compute( void )     // Compute Neuron Value
{
   #ifdef HCDM
     debugf("%f= NeuronValue(%p).compute()\n", value, this);
   #endif

   return value;                    // Return the (unchanged) value
}

