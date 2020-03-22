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
//       Fanin.i
//
// Purpose-
//       Fanin inline functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef FANIN_I_INCLUDED
#define FANIN_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Fanin::~Fanin
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Fanin::~Fanin( void )            // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Fanin::Fanin
//
// Purpose-
//       Default constructor
//
//----------------------------------------------------------------------------
   Fanin::Fanin( void )             // Constructor
:  neuron(NULL)
,  weight(1.0)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Fanin::Fanin(Neuron*, Weight)
//
// Purpose-
//       Valued constructor
//
//----------------------------------------------------------------------------
   Fanin::Fanin(                    // Valued constructor
     Neuron*         neuron,        // -> Source Neuron
     Weight          weight)        // Weight
:  neuron(neuron)
,  weight(weight)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Fanin::resolve
//
// Purpose-
//       Return (neuron->resolve() * weight)
//
//----------------------------------------------------------------------------
Value
   Fanin::resolve( void )           // Resolve Fanin Value
{
   return neuron->resolve() * weight;
}

//----------------------------------------------------------------------------
//
// Method-
//       Fanin::set
//
// Purpose-
//       Initialize the Fanin
//
//----------------------------------------------------------------------------
void
   Fanin::set(                      // Valued constructor
     Neuron*         neuron,        // -> Source Neuron
     Weight          weight)        // Weight
{
   this->neuron= neuron;
   this->weight= weight;
}

#endif // FANIN_I_INCLUDED
