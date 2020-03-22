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
//       NN_psv.h
//
// Purpose-
//       (NN) Neural Net: Process State Vector
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NN_PSV_H_INCLUDED
#define NN_PSV_H_INCLUDED

#ifndef NNTYPE_H_INCLUDED
#include "NNtype.h"
#endif

//----------------------------------------------------------------------------
// FPO location of the process state vector
//----------------------------------------------------------------------------
#define PSV_FILE                  0 // The PSV file number
#define PSV_PART                  2 // The PSV part number
#define PSV_OFFSET                0 // The PSV part offset

//----------------------------------------------------------------------------
// NN_PSV: Process State Vector
//----------------------------------------------------------------------------
struct NN_psv                       // Process State Vector
{
   unsigned char       psvcbid[8];  // Control Block Identifier
#define PSV_CBID "NN_UFILE"         // Control block identifier

   //-------------------------------------------------------------------------
   // Current position
   //-------------------------------------------------------------------------
   NN::FileId          psvfileno;   // File number
   NN::PartId          psvpartno;   // Part(ition) number
   NN::Offset          psvoffset;   // Offset

   //-------------------------------------------------------------------------
   // Current clock
   //-------------------------------------------------------------------------
   NN::Tick            clock;       // Current clock tick
   NN::Tick            train;       // Current training subtick
}; // struct NN_psv

#endif // NN_PSV_H_INCLUDED
