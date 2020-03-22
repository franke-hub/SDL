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
//       NC_op.i
//
// Purpose-
//       NC_op inline functions.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NC_OP_I_INCLUDED
#define NC_OP_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       ::operator<<(ostream&, const NC_op&)
//
// Purpose-
//       Write the list.
//
//----------------------------------------------------------------------------
inline ostream&
   operator<<(
     ostream&          os,
     const NC_op&      it)
{
   return it.toStream(os);
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFixed::getFixed
//
// Purpose-
//       Get the current resultant.
//
//----------------------------------------------------------------------------
int                                 // Resultant
   NC_opFixed::getFixed( void ) const // Get Resultant
{
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opFloat::getFloat
//
// Purpose-
//       Get the current resultant.
//
//----------------------------------------------------------------------------
double                              // Resultant
   NC_opFloat::getFloat( void ) const // Get Resultant
{
   return resultant;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opNeuronAddr::getFileId
//
// Purpose-
//       Get the current resultant.
//
//----------------------------------------------------------------------------
inline NN::FileId                   // Resultant.FileId
   NC_opNeuronAddr::getFileId( void ) const // Get resultant
{
   return resultant.f;
}

//----------------------------------------------------------------------------
//
// Method-
//       NC_opNeuronAddr::getOffset
//
// Purpose-
//       Get the current resultant.
//
//----------------------------------------------------------------------------
inline NN::Offset                   // Resultant.Offset
   NC_opNeuronAddr::getOffset( void ) const // Get resultant
{
   return resultant.o;
}

#endif // NC_OP_I_INCLUDED
