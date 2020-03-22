//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//--------------------------------------------------------------------------
//
// Title-
//       Neuron.h
//
// Purpose-
//       Define the Neuron object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NEURON_H_INCLUDED
#define NEURON_H_INCLUDED

//----------------------------------------------------------------------------
//
// Struct-
//       Neuron
//
// Purpose-
//       Neuron descriptor.
//
//----------------------------------------------------------------------------
struct Neuron                       // Neuron descriptor
{
//----------------------------------------------------------------------------
// Neuron::Enumerations and typedefs
//----------------------------------------------------------------------------
enum                                // Generic constants
{
   MIN_TRIGGER=                   3,// Default minTrigger value
   MAX_TRIGGER=                  33,// Default maxTrigger value
   MAX_CYCLE=                   255,// Default maxCycle value
   LOAD=                         32 // Default Trigger load
}; // enum

//----------------------------------------------------------------------------
// Neuron::Attributes
//----------------------------------------------------------------------------
   unsigned char       cycle;       // Cycle number, 0..MAX_CYCLE
   unsigned char       prior;       // Prior trigger count

static unsigned long   minTrigger;  // Minimum trigger cycle
static unsigned long   maxTrigger;  // Maximum trigger cycle
static unsigned long   maxCycle;    // Maximum cycle number
static unsigned long   load;        // Trigger load

//----------------------------------------------------------------------------
// Neuron::Methods
//----------------------------------------------------------------------------
}; // class Neuron

#endif // NEURON_H_INCLUDED
