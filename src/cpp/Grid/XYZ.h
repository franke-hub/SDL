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
//       XYZ.h
//
// Purpose-
//       X, Y, and Z value container.
//
// Last change date-
//       2011/01/01
//
//----------------------------------------------------------------------------
#ifndef XYZ_H_INCLUDED
#define XYZ_H_INCLUDED

#include "XY.h"

class XYZ {                         // X, Y, and Z value
public:
float                  x;           // X value
float                  y;           // Y value
float                  z;           // Z value

public:
inline
   ~XYZ( void ) {};                 // Destructor

inline
   XYZ( void )                      // Constructor
:  x(0.0), y(0.0), z(0.0) {}        // Zero value

inline
   XYZ(                             // Constructor
     float             x,           // X value
     float             y,           // Y value
     float             z)           // Z value
:  x(x), y(y), z(z) {}              // Value

inline
   XYZ(                             // Copy constructor
     const XYZ&        p)           // Source XYZ
:  x(p.x), y(p.y), z(p.z) {};

inline XYZ&                         // Resultant
   operator=(                       // Assignment operator
     const XYZ&        p)           // Source XYZ
{
   x= p.x;
   y= p.y;
   z= p.z;

   return *this;
}

//----------------------------------------------------------------------------
// Arithmetic operators
//----------------------------------------------------------------------------
inline XYZ                          // Resultant
   operator+( void ) const          // Unary plus
{
   XYZ r(x,y,z);

   return r;
}

inline XYZ                          // Resultant
   operator+(                       // Addition operator
     const XYZ&        p) const     // Addend
{
   XYZ r(x,y,z);

   r.x += p.x;
   r.y += p.y;
   r.z += p.z;

   return r;
}

inline XYZ                          // Resultant
   operator-( void ) const          // Unary minus
{
   XYZ r(-x,-y,-z);

   return r;
}

inline XYZ                          // Resultant
   operator-(                       // Subtraction operator (from this)
     const XYZ&        p) const     // Subtrahend
{
   XYZ r(x,y,z);

   r.x -= p.x;
   r.y -= p.y;
   r.z -= p.z;

   return r;
}


inline XYZ                          // Resultant
   operator*(                       // Scalar multiplication
     float             s) const     // Scalar multiplicand
{
   XYZ r(x*s,y*s,z*s);

   return r;
}

inline XYZ                          // Resultant
   operator/(                       // Scalar division
     float             s) const     // Scalar divisor
{
   XYZ r(x/s,y/s,z/s);

   return r;
}

//----------------------------------------------------------------------------
// Comparison operators
//----------------------------------------------------------------------------
inline int                          // Resultant
   operator==(                      // Test for equality
     const XYZ&        p) const     // With this
{
   return (x == p.x && y == p.y && z == p.z);
}

inline int                          // Resultant
   operator!=(                      // Test for inequality
     const XYZ&        p) const     // With this
{
   return (x != p.x || y != p.y || z != p.z);
}

//----------------------------------------------------------------------------
//
// Method-
//       cross
//
// Purpose-
//       Compute the cross product.
//
//----------------------------------------------------------------------------
inline XYZ                          // The cross product
   cross(                           // Compute cross product
     const XYZ&        p) const     // Multiplicand
{
   XYZ r(y*p.z - z*p.y, z*p.x - x*p.z, x*p.y - y*p.x);

   return r;
}

//----------------------------------------------------------------------------
//
// Method-
//       dot
//
// Purpose-
//       Compute the dot product.
//
//----------------------------------------------------------------------------
inline float                        // The dot product
   dot(                             // Compute dot product
     const XYZ&        p) const     // Multiplicand
{
   return x*p.x + y*p.y + z*p.z;
}

//----------------------------------------------------------------------------
//
// Method-
//       intersection
//
// Purpose-
//       Determine intersection between a line and a line perpendicular to
//       to that line which passes through this point.
//
//----------------------------------------------------------------------------
inline XYZ                          // Resultant
   intersection(                    // Distance between this XYZ and
     const XYZ&        line0,       // The line that goes through here
     const XYZ&        line1) const // And here
{
   float M= line1.separation(line0);
   float U= (((x - line0.x) * (line1.x - line0.x))
          +  ((y - line0.y) * (line1.y - line0.y))
          +  ((z - line0.z) * (line1.z - line0.z))
            ) / (M*M);

   XYZ r(line0.x + U * (line1.x - line0.x),
         line0.y + U * (line1.y - line0.y),
         line0.z + U * (line1.z - line0.z));

   return r;
}

//----------------------------------------------------------------------------
//
// Method-
//       isWithinSegment
//
// Purpose-
//       Does the line which passes through this point and is perpendicular
//       to a line segment intersect that line segment?
//
//----------------------------------------------------------------------------
inline int                          // Resultant
   isWithinSegment(                 // Is intersection within line segment?
     const XYZ&        line0,       // Line segment origin
     const XYZ&        line1) const // Line segment endpoint
{
   float M= line1.separation(line0);
   float U= (((x - line0.x) * (line1.x - line0.x))
          +  ((y - line0.y) * (line1.y - line0.y))
          +  ((z - line0.z) * (line1.z - line0.z))
            ) / (M*M);

   return (U >= 0.0 && U <= 1.0);
}

//----------------------------------------------------------------------------
//
// Method-
//       separation
//
// Purpose-
//       Determine the distance between two points
//
//----------------------------------------------------------------------------
inline float                        // Resultant
   separation(                      // Distance between this and
     const XYZ&        p) const     // This XYZ
{
   float dX= p.x - x;
   float dY= p.y - y;
   float dZ= p.z - z;
   return sqrt(dX*dX + dY*dY + dZ*dZ);
}

inline float                        // Resultant
   separation(                      // Distance between this and
     const XY&         p) const     // This XY (ignoring Z)
{
   float dX= p.x - x;
   float dY= p.y - y;
   return sqrt(dX*dX + dY*dY);
}
}; // class XYZ

#endif // XYZ_H_INCLUDED
