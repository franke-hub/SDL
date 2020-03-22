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
//       NC_dim.h
//
// Purpose-
//       (NC) Neural Net Compiler: Dimensionality control
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NC_DIM_H_INCLUDED
#define NC_DIM_H_INCLUDED

//----------------------------------------------------------------------------
//
// Struct-
//       NC_dim
//
// Purpose-
//       Dimensionality control
//
//----------------------------------------------------------------------------
struct NC_dim                       // Dimensionality
{
//----------------------------------------------------------------------------
// NC_dim::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum
{
   MAX_DIM=                      32 // Maximum dimensionality
}; // enum

//----------------------------------------------------------------------------
// NC_dim::Attributes
//----------------------------------------------------------------------------
public:
   unsigned            elements;    // Number of elements
   unsigned            dim;         // Number of dimensions
   unsigned            bound[MAX_DIM]; // Bound or value for dimension
}; // struct NC_dim

#endif // NC_DIM_H_INCLUDED
