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
//       Normalizer.h
//
// Purpose-
//       Container for information used for normalization.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NORMALIZER_H_INCLUDED
#define NORMALIZER_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Normalizer
//
// Purpose-
//       Normalizer descriptor.
//
//----------------------------------------------------------------------------
class Normalizer {                  // Normalizer descriptor
//----------------------------------------------------------------------------
// Normalizer::Attributes
//----------------------------------------------------------------------------
protected:
double                 nomData;     // Nominal data value
double                 nomNorm;     // Nominal normalized value
double                 toNormal;    // Normalizer multiplier
double                 unNormal;    // Unnormalizer multiplier

//----------------------------------------------------------------------------
// Normalizer::Constructors
//----------------------------------------------------------------------------
public:
   ~Normalizer( void );             // Destructor
   Normalizer( void );              // Default constructor
   Normalizer(                      // Initializing constructor
     double            minNormal,   // Minimum normalized value
     double            maxNormal,   // Maximum normalized value
     double            minInput,    // Minimum data value
     double            maxInput);   // Maximum data value

//----------------------------------------------------------------------------
// Normalizer::Initializer
//----------------------------------------------------------------------------
public:
void
   initialize(                      // Initilize the Normalizer
     double            minNormal,   // Minimum normalized value
     double            maxNormal,   // Maximum normalized value
     double            minInput,    // Minimum data value
     double            maxInput);   // Maximum data value

//----------------------------------------------------------------------------
// Normalizer::Methods used after initialization
//----------------------------------------------------------------------------
public:
inline double                       // The normalized value
   normalize(                       // Normalize
     double            value) const;// This value

inline double                       // The unnormalized value
   restore(                         // Restore (unnormalize)
     double            value) const;// This value
}; // class Normalizer

#include "Normalizer.i"

#endif // NORMALIZER_H_INCLUDED
