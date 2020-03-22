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
//       Master.h
//
// Purpose-
//       Define the Master object.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef MASTER_H_INCLUDED
#define MASTER_H_INCLUDED

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MEMORY_SIZE      0x10000000 // 256M memory
#define MASTER_NCOUNT    0x00100000 // Default number of Neurons
#define MASTER_D_PER_N   0x00000060 // Default number of Dendrites/Neuron
#define MASTER_ICOUNT    0x00000020 // Default number of Inputs

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
struct Dendrite;
struct Neuron;

//----------------------------------------------------------------------------
//
// Struct-
//       Master
//
// Purpose-
//       Master object descriptor.
//
//----------------------------------------------------------------------------
struct Master                       // Master object descriptor
{
   unsigned long       cycle;       // Cycle number

   unsigned long       dPerN;       // Number of Dendrites/Neuron
   unsigned long       iCount;      // Number of Input Axons
   unsigned long       nCount;      // Number of Neurons in array

   unsigned long       aCount;      // Number of Axons
   unsigned long       dCount;      // Number of Dendrites in array

   char*               axon;        // Axon bit map
   Dendrite*           dendrite;    // Dendrite array
   Neuron*             neuron;      // Neuron array
}; // class Master

extern Master*         master;      // -> Master object

#endif // MASTER_H_INCLUDED
