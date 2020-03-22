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
//       MinMax.h
//
// Purpose-
//       Container for minimum/maximum values.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef MINMAX_H_INCLUDED
#define MINMAX_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       MinMax
//
// Purpose-
//       MinMax descriptor.
//
//       A MinMax object keeps the minimum and maximum of all examined values.
//
//----------------------------------------------------------------------------
class MinMax {                      // MinMax descriptor
//----------------------------------------------------------------------------
// MinMax::Attributes
//----------------------------------------------------------------------------
protected:
double                 minimum;     // Minimum value
double                 maximum;     // Maximum value

//----------------------------------------------------------------------------
// MinMax::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~MinMax( void );                 // Destructor
inline
   MinMax( void );                  // Default constructor

//----------------------------------------------------------------------------
// MinMax::Methods
//----------------------------------------------------------------------------
public:
inline void
   reset( void );                   // Reset the MinMax object

inline double                       // The value
   sample(                          // Set sample value
     double            value);      // Using this value

inline double                       // The minimum value
   getMinimum( void ) const;        // Get minimum value

inline double                       // The maximum value
   getMaximum( void ) const;        // Get maximum value
}; // class MinMax

#include "MinMax.i"

#endif // MINMAX_H_INCLUDED
