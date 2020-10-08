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
//       NNfanin.cpp
//
// Purpose-
//       Neural Net: FANIN utility functions
//
// Last change date-
//       2007/01/01
//
// Entry points-
//       nndamage   Indicate neuron damage
//       nnfanin    weight[i] * read_val(neuron[i])
//       nnfinop    Read but ignore fanin values
//       nnsigma    SUM( weight[i] * read_val(neuron[i]) )
//       nnsigm1    SUM( weight[i] * read_val(neuron[i]) )
//                  Exclude element 0
//
//----------------------------------------------------------------------------
#include <stdio.h>

#include <com/Debug.h>

#include "NN_com.h"
#include "NNtype.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NNFANIN " // Source file, for debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM                        // If not already defined
#define HCDM                   TRUE // Hard-Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define C2H(ptr_C) (*(int16_t*)(ptr_C)) // Convert CBID to int16_t

//----------------------------------------------------------------------------
//
// Subroutine-
//       nndamage
//
// Purpose-
//       Indicate NEURON damage.
//
//----------------------------------------------------------------------------
extern NN::Value
   nndamage(                        // Indicate NEURON damage
     NN::FileId        fileN,       // Source file identifier
     Neuron*           ptrN,        // -> Source neuron
     NN::Vaddr         offset)      // Damage offset
{
   printf("\n");
   printf("%.2ld:0x%.8lX FANIN damaged\n",
          (long)fileN, (long)offset);

   ptrN->ex.disabled= TRUE;         // Indicate disabled
   ptrN->ex.any= TRUE;
   ptrN->ex.eof= TRUE;

   return(0);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nnfanin
//
// Purpose-
//       Read WEIGHT[i] * read_val(neuron[i])
//
//----------------------------------------------------------------------------
extern NN::Value
   nnfanin(                         // Read weight[i] * fanin[i]
     Neuron*           ptrN,        // -> Source neuron
     NN::FileId        fileN,       // Source file identifier
     unsigned          index)       // Fanin index
{
   NN::Value           resultant;   // Resultant
   Fanin*              ptrF;        // -> Fanin (Internal address)

   NN::Offset          offset;      // -> Fanin (External, current)

   //-------------------------------------------------------------------------
   // Locate, then read the fanin value
   //-------------------------------------------------------------------------
   if( index >= ptrN->faninCount )
   {
     ptrN->ex.eof= TRUE;
     return 0;
   }

   resultant= 0;                    // Set default resultant
   offset= Fanin::index(ptrN->faninVaddr, index);

   ptrF= ref_fanin(fileN, offset);  // Access the Fanin
   if( ptrF == NULL )
     return(nndamage(fileN, ptrN, offset));

   resultant= ptrF->weight * nnreadv(ptrF->fileId, ptrF->neuron);
   rel_fanin(fileN, offset);        // Release the Fanin

   return(resultant);               // Return the resultant
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nnfinop
//
// Purpose-
//       Read (but ignore) all fanin values.
//
//----------------------------------------------------------------------------
extern void
   nnfinop(                         // Read (but ignore) fanins
     Neuron*           ptrN,        // -> Source neuron
     NN::FileId        fileN)       // Source file identifier
{
   Fanin*              ptrF;        // -> Fanin (Internal address)

   unsigned            fanix;       // Current FANIN index
   NN::Offset          offset;      // -> Fanin (Internal, current)

   //-------------------------------------------------------------------------
   // Read (but ignore) the fanin values
   //-------------------------------------------------------------------------
   offset= ptrN->faninVaddr;        // Address the 1st fanin
   for(fanix= 0; fanix < ptrN->faninCount; fanix++)
   {
     ptrF= ref_fanin(fileN, offset);// Access the fanin
     if( ptrF == NULL )             // If invalid queue
     {
       nndamage(fileN, ptrN, offset); // Indicate damage
       return;
     }

     nnreadv(ptrF->fileId, ptrF->neuron); // Read the NEURON
     rel_fanin(fileN, offset);      // Release the current FANIN

     offset += sizeof(Fanin);       // Address the next FANIN
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nnsigma
//
// Purpose-
//       Calculate sum(value[i] * weight[i])
//
//----------------------------------------------------------------------------
extern NN::Value
   nnsigma(                         // Sum(Value[i] * Weight[i]
     Neuron*           ptrN,        // -> Source neuron
     NN::FileId        fileN)       // Source file identifier
{
   NN::Value           resultant;   // Resultant
   Fanin*              ptrF;        // -> Fanin (Internal address)

   unsigned            fanix;       // Current FANIN index
   NN::Offset          offset;      // -> Fanin (Internal, current)

   //-------------------------------------------------------------------------
   // Resultant= sum(weight[i]) * READV(fanin[i]))
   //-------------------------------------------------------------------------
   resultant= 0;                    // Initialize the resultant

   offset= ptrN->faninVaddr;        // Address the 1st fanin
   for( fanix=0; fanix < ptrN->faninCount; fanix++) // Calculate resultant
   {
     ptrF= ref_fanin(fileN, offset);// Access the fanin
     if( ptrF == NULL )
       return(nndamage(fileN, ptrN, offset));

     resultant += ptrF->weight * nnreadv(ptrF->fileId, ptrF->neuron);
     rel_fanin(fileN, offset);     // Release the current FANIN
     offset += sizeof(Fanin);      // Address the next FANIN
   }

   return(resultant);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       nnsigm1
//
// Purpose-
//       Calculate sum(value[i] * weight[i]), exclude element[0]
//
// Notes-
//       Element[0] is not read.
//
//----------------------------------------------------------------------------
extern NN::Value
   nnsigm1(                         // Sum(Value[i] * Weight[i]
                                    // (Excluding element[0])
     Neuron*           ptrN,        // -> Source neuron
     NN::FileId        fileN)       // Source file identifier
{
   NN::Value           resultant;   // Resultant
   Fanin*              ptrF;        // -> Fanin (Internal address)

   unsigned            fanix;       // Current FANIN index
   NN::Offset          offset;      // -> Fanin (Internal, current)

   //-------------------------------------------------------------------------
   // Resultant= sum(weight[i]) * READV(fanin[i]))
   //-------------------------------------------------------------------------
   resultant= 0;                    // Initialize the resultant

   offset= ptrN->faninVaddr + sizeof(Fanin); // Address the 2nd fanin
   for( fanix=1; fanix < ptrN->faninCount; fanix++) // Calculate resultant
   {
     ptrF= ref_fanin(fileN, offset);// Access the fanin
     if( ptrF == NULL )
       return(nndamage(fileN, ptrN, offset));

     resultant += ptrF->weight * nnreadv(ptrF->fileId, ptrF->neuron);
     rel_fanin(fileN, offset);     // Release the current FANIN
     offset += sizeof(Fanin);      // Address the next FANIN
   }

   return(resultant);
}

