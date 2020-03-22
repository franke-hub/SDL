//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Normalizer.i
//
// Purpose-
//       Normalizer inline methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NORMALIZER_I_INCLUDED
#define NORMALIZER_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       Normalizer::normalize
//
// Function-
//       Normalize a value.
//
//----------------------------------------------------------------------------
double                              // The normalized value
   Normalizer::normalize(           // Normalize
     double            value) const // This value
{
   return nomNorm + ((value - nomData) * toNormal);
}

//----------------------------------------------------------------------------
//
// Method-
//       Normalizer::restore
//
// Function-
//       Unnormalize a value.
//
//----------------------------------------------------------------------------
double                              // The unnormalized value
   Normalizer::restore(             // Restore (unnormalize)
     double            value) const // This value
{
   return nomData + ((value - nomNorm) * unNormal);
}

#endif // NORMALIZER_I_INCLUDED
