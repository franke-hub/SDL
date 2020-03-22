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
#include "Dendrite.h"
#include "Neuron.h"

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
unsigned long          Neuron::minTrigger= Neuron::MIN_TRIGGER;
unsigned long          Neuron::maxTrigger= Neuron::MAX_TRIGGER;
unsigned long          Neuron::maxCycle=   Neuron::MAX_CYCLE;
unsigned long          Neuron::load=       Neuron::LOAD;

//----------------------------------------------------------------------------
//
// Method-
//       Neuron::Neuron
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
// None defined

