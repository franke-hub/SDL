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
//       NoisyNice.h
//
// Purpose-
//       Describes a "nice" class as defined by the STL.
//
// Last change date-
//       2007/01/01
//
// Implementation notes-
//       This class logs all of its operations.
//
//----------------------------------------------------------------------------
#ifndef NOISYNICE_H_INCLUDED
#define NOISYNICE_H_INCLUDED

#ifndef NICE_H_INCLUDED
#include "Nice.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       NoisyNice
//
// Purpose-
//       A "nice," buy noisy class.
//
//----------------------------------------------------------------------------
class NoisyNice : public Nice       // A "nice" class, by definition
{
//----------------------------------------------------------------------------
// NoisyNice::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~NoisyNice( void );              // Destructor
inline
   NoisyNice( void );               // Default constructor

inline
   NoisyNice(                       // Copy constructor, implies equality
     const NoisyNice&  source);     // Source NoisyNice&

//----------------------------------------------------------------------------
// NoisyNice::Operators
//----------------------------------------------------------------------------
public:
inline NoisyNice&                   // Resultant, always (*this)
   operator=(                       // Assignment operator, implies equality
     const NoisyNice&  source);     // Source NoisyNice&

inline int                          // Resultant (TRUE || FALSE)
   operator==(                      // Equality operator
     const NoisyNice&  source) const; // Source NoisyNice&

inline int                          // Resultant (TRUE || FALSE)
   operator!=(                      // Inequality operator
     const NoisyNice&  source) const; // Source NoisyNice&

//----------------------------------------------------------------------------
// NoisyNice::Additional operators, if ordering is required
//----------------------------------------------------------------------------
public:
inline int                          // Resultant (*this < source)
   operator<(                       // Less than operator
     const NoisyNice&  source) const; // Source NoisyNice&

//----------------------------------------------------------------------------
// NoisyNice::Sample (and optional) "equality preserving" member function
//----------------------------------------------------------------------------
public:
inline int                          // Resultant, equality preserving
   s( void ) const;                 // Iff NoisyNice& a == NoisyNice& b; a.s() == b.s()

//----------------------------------------------------------------------------
// NoisyNice::Without this member function, this class would be "extra-nice"
//----------------------------------------------------------------------------
public:
inline int                          // Resultant
   i( void );                       // Change equality
}; // class NoisyNice

#include "NoisyNice.i"

#endif // NOISYNICE_H_INCLUDED
