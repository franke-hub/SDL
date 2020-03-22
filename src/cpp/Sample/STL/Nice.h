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
//       Nice.h
//
// Purpose-
//       Describes a "nice" class as defined by the STL.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef NICE_H_INCLUDED
#define NICE_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Nice
//
// Purpose-
//       A "nice" class.
//
// Examples use-
//       Nice a; Nice b; Nice c;
//
//----------------------------------------------------------------------------
class Nice                          // A "nice" class, by definition
{
//----------------------------------------------------------------------------
// Nice::Constructors
//----------------------------------------------------------------------------
public:
// ~Nice( void );                   // Destructor (implied destructor OK)
   Nice( void );                    // Default constructor

// Nice a(b); assert( a == b );
   Nice(                            // Copy constructor
     const Nice&       source);     // Source Nice&

//----------------------------------------------------------------------------
// Nice::Operators
//----------------------------------------------------------------------------
public:
// a= b; assert( a == b );
Nice&                               // Resultant, always (*this)
   operator=(                       // Assignment operator
     const Nice&       source);     // Source Nice&

// If( a == b ) assert( b == a );   // For all a and b
// If( a == b && b == c ) assert( a == c ); // For all a, b, and c
int                                 // Resultant (TRUE || FALSE)
   operator==(                      // Equality operator
     const Nice&       source) const; // Source Nice&

// The inequality operator is the inverse of the equality operator
// If( a == b ) assert( !( a != b ) ); If( a != b ) assert( !( a == b ) );
int                                 // Resultant (TRUE || FALSE)
   operator!=(                      // Inequality operator
     const Nice&       source) const; // Source Nice&

//----------------------------------------------------------------------------
// Nice::Additional operators, if ordering is required
//----------------------------------------------------------------------------
public:
int                                 // Resultant (*this < source)
   operator<(                       // Less than operator
     const Nice&       source) const; // Source Nice&

//----------------------------------------------------------------------------
// Nice::Sample (and optional) "equality preserving" member function
//----------------------------------------------------------------------------
public:
int                                 // Resultant, equality preserving
   s( void ) const;                 // Iff Nice& a == Nice& b; a.s() == b.s()

//----------------------------------------------------------------------------
// Nice::An optional member function which is not equality preserving
//----------------------------------------------------------------------------
public:
int                                 // Resultant
   i( void );                       // Not all member functions preserve equality

//----------------------------------------------------------------------------
// Nice::Attributes -- (Not part of definition)
//----------------------------------------------------------------------------
protected:
static int             generator;   // Serial number generator
   int                 serialNum;   // The Serial Number
}; // class Nice

#endif // NICE_H_INCLUDED
