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
//       NNdisp.cpp
//
// Purpose-
//       Neural net storage access informational display.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
//
// Associated display-
//       *---*------------------------------------------*
//       |OK?|                                          |<-Full
//       *---*------------------------------------------*  state
//       |   |                                          |
//       |   |                                          |
//       |   |                                          |
//       |R/W|                 *Neuron[n]               |<-Area[n]
//       |   |                                          |
//       |   |                                          |
//       |   |                                          |
//       |   |                                          |
//       *---*------------------------------------------*
//
//       Full= used=  <- light blue -><- Yellow -><-Red->
//             empty= Background
//
//       RW=     Idle=   Background
//               Rd=     Green
//               Wr=     Yellow
//
//       Neuron= InFile= Background
//               Read=   Green
//               Write=  Yellow
//               Clear=  Light blue
//
//----------------------------------------------------------------------------
#include "NN_com.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "NNDISP  " // Source file name

//----------------------------------------------------------------------------
//
// Subroutine-
//       nndisp
//
// Purpose-
//       Storage access display control
//
//----------------------------------------------------------------------------
extern void
   nndisp(                          // Storage access display
     int               fc,          // Function code
     NN::FileId        area,        // Target area identifier
     int               blockid)     // Target frame identifier
{
}
