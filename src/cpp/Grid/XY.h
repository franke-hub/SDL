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
//       XY.h
//
// Purpose-
//       X, and Y value container.
//
// Last change date-
//       2011/01/01
//
//----------------------------------------------------------------------------
#ifndef XY_H_INCLUDED
#define XY_H_INCLUDED

#include <math.h>

class XY {                          // X and Y value
public:
float                  x;           // X value
float                  y;           // Y value

public:
inline
   ~XY( void ) {};                  // Destructor

inline
   XY( void )                       // Constructor
:  x(0.0), y(0.0) {}                // Zero value

inline
   XY(                              // Constructor
     float             x,           // X value
     float             y)           // Y value
:  x(x), y(y) {}                    // Value

inline
   XY(                              // Copy constructor
     const XY&         p)           // Source XY
:  x(p.x), y(p.y) {};

inline XY&                          // Resultant
   operator=(                       // Assignment operator
     const XY&         p)           // Source XY
{
   x= p.x;
   y= p.y;

   return *this;
}

//----------------------------------------------------------------------------
// Arithmetic operators
//----------------------------------------------------------------------------
inline XY                           // Resultant
   operator+( void ) const          // Unary plus
{
   XY r(x,y);

   return r;
}

inline XY                           // Resultant
   operator+(                       // Addition operator
     const XY&         p) const     // Addend
{
   XY r(x,y);

   r.x += p.x;
   r.y += p.y;

   return r;
}

inline XY                           // Resultant
   operator-( void ) const          // Unary minus
{
   XY r(-x,-y);

   return r;
}

inline XY                           // Resultant
   operator-(                       // Subtraction operator (from this)
     const XY&         p) const     // Subtrahend
{
   XY r(x,y);

   r.x -= p.x;
   r.y -= p.y;

   return r;
}

inline XY                           // Resultant
   operator*(                       // Scalar multiplication
     float             s) const     // Scalar multiplicand
{
   XY r(x*s,y*s);

   return r;
}

inline XY                           // Resultant
   operator/(                       // Scalar division
     float             s) const     // Scalar divisor
{
   XY r(x/s,y/s);

   return r;
}

//----------------------------------------------------------------------------
// Comparison operators
//----------------------------------------------------------------------------
inline int                          // Resultant
   operator==(                      // Test for equality
     const XY&         p) const     // With this
{
   return (x == p.x && y == p.y);
}

inline int                          // Resultant
   operator!=(                      // Test for inequality
     const XY&         p) const     // With this
{
   return (x != p.x || y != p.y);
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
     const XY&         p) const     // Multiplicand
{
   return x*p.x + y*p.y;
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
inline XY                           // Resultant
   intersection(                    // Distance between this XY and
     const XY&         line0,       // The line that goes through here
     const XY&         line1) const // And here
{
   float M= line1.separation(line0);
   float U= (((x - line0.x) * (line1.x - line0.x))
          +  ((y - line0.y) * (line1.y - line0.y))
            ) / (M*M);

   XY r(line0.x + U * (line1.x - line0.x),
        line0.y + U * (line1.y - line0.y));

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
     const XY&         line0,       // Line segment origin
     const XY&         line1) const // Line segment endpoint
{
   float M= line1.separation(line0);
   float U= (((x - line0.x) * (line1.x - line0.x))
          +  ((y - line0.y) * (line1.y - line0.y))
            ) / (M*M);

   return (U >= 0.0 && U <= 1.0);
}

//----------------------------------------------------------------------------
//
// Method-
//       separation
//
// Purpose-
//       Determine distance between two points
//
//----------------------------------------------------------------------------
inline float                        // Resultant
   separation(                      // XY Distance between this and
     const XY&         p) const     // This XY
{
   float dX= p.x - x;
   float dY= p.y - y;
   return sqrt(dX*dX + dY*dY);
}
}; // class XY

#endif // XY_H_INCLUDED
