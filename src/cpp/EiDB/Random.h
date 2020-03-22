//----------------------------------------------------------------------------
//
//       Copyright (C) 2004 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Random.h
//
// Description-
//       Random number generator.
//
// Last change date-
//       2004/01/01
//
// Classes-
         class Random;              // Random number
         class RandomP;             // Random probability
//
//----------------------------------------------------------------------------
#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED

//----------------------------------------------------------------------------
//
// Class-
//       Random
//
// Purpose-
//       Random number generator
//
//----------------------------------------------------------------------------
class Random {                      // Random number generator
//----------------------------------------------------------------------------
// Random::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum
{
   MINIMUM= 1,                      // Minimum resultant from get()
   MAXIMUM= 0x7fffffff              // Maximum resultant from get()
}; // enum

//----------------------------------------------------------------------------
// Random::Constructors
//----------------------------------------------------------------------------
public:
   ~Random( void ) {}               // Destructor

private:
   Random( void );                  // NO Constructor (static class)

//----------------------------------------------------------------------------
// Random::Methods
//----------------------------------------------------------------------------
public:
static inline unsigned long         // The next pseudo-random value
   get( void );                     // Get the next pseudo-random value

static inline unsigned long         // Get the seed value
   getSeed( void );                 // Get the seed value

static inline void
   setSeed(                         // Set the seed value
     unsigned long   seed);         // To this value

//----------------------------------------------------------------------------
// Random::Attributes
//----------------------------------------------------------------------------
protected:
static unsigned long seed;          // Current seed
}; // class Random

//----------------------------------------------------------------------------
//
// Class-
//       RandomP
//
// Purpose-
//       Random probability.
//
// Notes-
//       The purpose of this class is to define the high-performance method
//       isTrue(), which will return TRUE with the defined probability.
//       Note that (!isTrue()) returns TRUE with a probability of 1-p.
//
//       If an attempt is made to set a probability < 0.0, it is set to 0.0
//       If an attempt is made to set a probability > 1.0, it is set to 1.0
//
//----------------------------------------------------------------------------
class RandomP {                     // Random
//----------------------------------------------------------------------------
// RandomP::Enumerations and Typedefs
//----------------------------------------------------------------------------
private:
enum
{
   MinP=                          0,// Minimum probability
   MaxP=               0x80000000UL,// Maximum probability
   MaxMask=            0x7FFFFFFFUL // Maximum probability - 1
};

//----------------------------------------------------------------------------
// RandomP::Constructors
//----------------------------------------------------------------------------
public:
   RandomP(                         // Construct a probability
     double          p=0.0);        // Probability, range (0..1)

//----------------------------------------------------------------------------
// RandomP::Methods
//----------------------------------------------------------------------------
public:
inline int                          // TRUE || FALSE
   isTrue( void ) const;            // This will be TRUE with probability P

double                              // Probability, range (0..1)
   get( void ) const;               // Get probability

inline unsigned long                // The integer ratio (n * p)
   ratio(                           // Get probability ratio
     unsigned long  n) const;       // Number of elements

void
   set(                             // Set probability
     double          p);            // Probability, range (0..1)

//----------------------------------------------------------------------------
// RandomP::Attributes
//----------------------------------------------------------------------------
private:
   unsigned long     p;             // The integer probability value
}; // class RandomP

#include "Random.i"

#endif // RANDOM_H_INCLUDED
