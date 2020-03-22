//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Random.h
//
// Purpose-
//       Random number generator.
//
// Last change date-
//       2014/01/01
//
// Usage notes-
//       The Random objects are not thread-safe. A separate Random object is
//       required for each thread.
//
//       Random:get()
//           Returns the next random or pseudo-random number.
//
//       Random:getBit( void )
//           Returns a value whose low-order bit cannot be reliably predicted.
//           (This value comes from the hardware clock.)
//
//       Random:isTrue(double probability)
//           Returns (TRUE or FALSE) with the specified TRUE probability.
//
//       Random:randomize( void )
//           Sets the seed to a completely unpredictable value.
//
//       Random:setSeed(int64_t s)
//           Sets the seed (and the random sequence) to a predictable value.
//           (In PerfectRandom, setting the seed randomizes.)
//
//       The "standard" Random object is useful for applications that need a
//       simple, shared object. Sample usage:
//           static Random& RNG= Random::standard; // Sample declaration
//           if( RNG.isTrue(0.5) ) {do something}  // Sample usage
//
//----------------------------------------------------------------------------
#ifndef RANDOM_H_INCLUDED
#define RANDOM_H_INCLUDED

#include <stdint.h>

//----------------------------------------------------------------------------
//
// Class-
//       Random
//
// Purpose-
//       Simple pseudo-random number generator.
//
//----------------------------------------------------------------------------
class Random {                      // Simple pseudo-random number generator
//----------------------------------------------------------------------------
// Random::Static attributes
//----------------------------------------------------------------------------
protected:
uint64_t               seed;        // The current random number

public:
static const double    MINIMUM;     // Minimum resultant from get()
static const double    MAXIMUM;     // Maximum resultant from get()
static Random          standard;    // A standard Random object

//----------------------------------------------------------------------------
// Random::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Random( void );                 // Destructor
   Random( void );                  // Default Constructor

//----------------------------------------------------------------------------
// Random::Inline methods
//----------------------------------------------------------------------------
public:
double                              // Resultant, range 0.0 .. 1.0
   getDouble( void )                // Return a random double value
{  return (double(get()) / MAXIMUM); }

int                                 // Resultant (TRUE || FALSE)
   isTrue(                          // Return TRUE
     double            p)           // With this probability
{  return (p*MAXIMUM) >= double(get()); }

unsigned long                       // Resultant, range 0 .. (M-1)
   modulus(                         // Return a random integer value
     unsigned long     M)           // For this M(aximum)
{  return (get() % M); }

//----------------------------------------------------------------------------
// Random::Methods
//----------------------------------------------------------------------------
public:
virtual uint64_t                    // The next random value
   get( void );                     // Get the next random value

virtual void
   randomize( void );               // Set a randomized seed value

virtual void
   setSeed(                         // Set the seed value
     uint64_t          seed);       // To this

//----------------------------------------------------------------------------
// Random::Static methods
//----------------------------------------------------------------------------
public:
static uint64_t                     // Resultant, low order bit unpredictable
   getBit( void );                  // Get unpredicatble bit
}; // class Random

//----------------------------------------------------------------------------
//
// Class-
//       PseudoRandom
//
// Purpose-
//       Pseudo-random number generator using Mersenne Twister algorithm,
//       version MT19937.
//
//----------------------------------------------------------------------------
class PseudoRandom : public Random { // Pseudo-Random number generator
//----------------------------------------------------------------------------
// PseudoRandom::Attributes
//----------------------------------------------------------------------------
protected:
enum { DIM=624 };                   // The size of the data array
enum { PER=397 };                   // The period of the data array

int32_t                index;       // The data array index
int32_t                MT[DIM];     // The data array

//----------------------------------------------------------------------------
// PseudoRandom::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~PseudoRandom( void );                 // Destructor
   PseudoRandom( void );                  // Default Constructor

private:                            // Bitwise copy is prohibited
   PseudoRandom(const PseudoRandom&); // Disallowed copy constructor
PseudoRandom& operator=(const PseudoRandom&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// PseudoRandom::Methods
//----------------------------------------------------------------------------
public:
virtual uint64_t                    // The next random value
   get( void );                     // Get next random value

virtual uint32_t                    // The next 32 bit random value
   get32( void );                   // Get next 32 bit random value

virtual void
   setSeed(                         // Set the seed value
     uint64_t          seed);       // To this
}; // class PseudoRandom

//----------------------------------------------------------------------------
//
// Class-
//       PerfectRandom
//
// Purpose-
//       Perfect random number generator: No sequence will ever be duplicated.
//       The seed consists of multiple uint64_t words, one of which is adjusted
//       from the hardware clock on each get() method call.
//
//----------------------------------------------------------------------------
class PerfectRandom : public Random { // Perfect random number generator
//----------------------------------------------------------------------------
// PerfectRandom::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum
{  REG_COUNT= 32                    // Number of "hidden" registers
,  REG_SHIFT= 5                     // log2(REG_COUNT)
,  REG_CLOCK= (REG_COUNT-1)         // The "clock" register
}; // enum

//----------------------------------------------------------------------------
// PerfectRandom::Attributes
//----------------------------------------------------------------------------
protected:
uint64_t               hidden[REG_COUNT]; // The "hidden" register array

//----------------------------------------------------------------------------
// PerfectRandom::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~PerfectRandom( void );          // Destructor
   PerfectRandom( void );           // Default Constructor

//----------------------------------------------------------------------------
// PerfectRandom::Methods
//----------------------------------------------------------------------------
public:
virtual uint64_t                    // The next random value
   get( void );                     // Get the next random value

virtual void
   setSeed(                         // Set the seed value
     uint64_t          seed);       // To this
}; // class PerfectRandom

#endif // RANDOM_H_INCLUDED
