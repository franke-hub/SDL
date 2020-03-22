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

#ifndef NN_H_INCLUDED
#include "NN.h"
#endif

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
// Fanin::Compile-time methods
//----------------------------------------------------------------------------
public:
static inline NN::Vaddr             // Resultant offset
   index(                           // Compute Vaddr(Fanin[index])
     NN::Vaddr         base,        // Base Fanin address
     unsigned          index);      // Element index

//----------------------------------------------------------------------------
// Fanin::Methods
//----------------------------------------------------------------------------
public:
NN::Value                           // Current value
   resolve( void );                 // Resolve value * weight

//----------------------------------------------------------------------------
// Fanin::Attributes
//----------------------------------------------------------------------------
public:
   NN::Offset          neuron;      // 0000 -> Neuron Offset
   NN::FileId          fileId;      // 0008 -> Neuron FileId
   NN::Weight          weight;      // 000c Input weight
}; // class Fanin                   // 0010

#include "Fanin.i"

#endif // FANIN_H_INCLUDED
