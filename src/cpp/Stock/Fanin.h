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
//       Fanin.h
//
// Purpose-
//       Define the Fanin object.
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       Fanin is a flat class.
//       Constructors, destructors and virtual methods are not allowed.
//
//----------------------------------------------------------------------------
#ifndef FANIN_H_INCLUDED
#define FANIN_H_INCLUDED

#ifndef TYPES_H_INCLUDED
#include "Types.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Neuron;

//----------------------------------------------------------------------------
//
// Class-
//       Fanin
//
// Purpose-
//       Fanin descriptor.
//
//----------------------------------------------------------------------------
class Fanin                         // Fanin descriptor
{
//----------------------------------------------------------------------------
// Fanin::Constructor/Destructor
//----------------------------------------------------------------------------
public:
inline
   ~Fanin( void );                  // Destructor

inline
   Fanin( void );                   // Default constructor

inline
   Fanin(                           // Valued constructor
     Neuron*         neuron,        // -> Source Neuron
     Weight          weight);       // Weight

//----------------------------------------------------------------------------
// Fanin::Methods
//----------------------------------------------------------------------------
public:
inline Value                        // neuron->resolve() * weight
   resolve( void );                 // Return neuron->resolve() * weight

inline void
   set(                             // Set Neuron and weight
     Neuron*         neuron,        // -> Source Neuron
     Weight          weight);       // Weight

//----------------------------------------------------------------------------
// Fanin::Attributes
//----------------------------------------------------------------------------
public:
   Neuron*           neuron;        // 0000 -> Neuron
   Weight            weight;        // 0004 Input weight
}; // class Fanin                   // 0008

#endif // FANIN_H_INCLUDED
