//----------------------------------------------------------------------------
//
//       Copyright (c) 2011 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Position.h
//
// Purpose-
//       X, Y, and Z position container.
//
// Last change date-
//       2011/01/01
//
//----------------------------------------------------------------------------
#ifndef POSITION_H_INCLUDED
#define POSITION_H_INCLUDED

#include "XYZ.h"

class Position : public XYZ {       // X, Y, and Z position
public:
inline
   ~Position( void ) {}             // Destructor

inline
   Position( void )                 // Constructor
:  XYZ() {}                         // Zero value

inline
   Position(                        // Constructor
     float             x,           // X position
     float             y,           // Y position
     float             z)           // Z position
:  XYZ(x,y,z) {}                    // Value

inline
   Position(                        // Copy constructor
     const XYZ&        p)           // Source XYZ
:  XYZ(p) {}

inline Position&                    // Resultant
   operator=(                       // Assignment operator
     const XYZ&        p)           // Source XYZ
{
   XYZ::operator=(p);

   return *this;
}
}; // class Position

#endif // POSITION_H_INCLUDED
