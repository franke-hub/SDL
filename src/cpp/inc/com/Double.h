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
//       Double.h
//
// Purpose-
//       Explicit double, prevents automatic conversion to (double).
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef DOUBLE_H_INCLUDED
#define DOUBLE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Double
//
// Purpose-
//       Floating point double value.
//
//----------------------------------------------------------------------------
class Double {
//----------------------------------------------------------------------------
// Double::Attributes
//----------------------------------------------------------------------------
private:
double                 value;       // The value

//----------------------------------------------------------------------------
// Double::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~Double( void );                 // Destructor

inline
   Double( void );                  // Default constructor

inline
   Double(                          // Value constructor
     double            source);     // Source value

inline Double&                      // Return (reference)
   operator=(                       // Assignment operator
     double            source);     // Source value

inline
   operator double( void ) const;   // (double) cast operator
}; // class Double

#include "Double.i"

#endif // DOUBLE_H_INCLUDED
