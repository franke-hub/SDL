//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2023 Frank Eskesen.
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
//       (Pseudo)-random number generator.
//
// Last change date-
//       2023/11/13
//
// Usage notes-
//       The Random object is thread-safe and lock free. It uses an algorithm
//       that cycles through a 63-bit sequence without repeating, returning
//       the current low-order 32 bits of the current sequence.
//
//       The "standard" Random object is useful for applications that need a
//       simple, shared object. Sample usage:
//           static Random& RNG= Random::standard; // Sample declaration
//           if( RNG.isTrue(0.5) ) {do something}  // Sample usage
//
//----------------------------------------------------------------------------
#ifndef _LIBPUB_RANDOM_H_INCLUDED
#define _LIBPUB_RANDOM_H_INCLUDED

#include <atomic>                   // For std::atomic<uint64_t>
#include <cstdint>                  // For integer types, UINT32_MAX

#include "config.h"                 // For _LIBPUB_ macros

_LIBPUB_BEGIN_NAMESPACE_VISIBILITY(default)
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
std::atomic<uint64_t>  seed;        // The current random number

public:
static Random          standard;    // A standard Random object

//----------------------------------------------------------------------------
// Random::Constructors
//----------------------------------------------------------------------------
public:
   Random( void );                  // Default Constructor
   ~Random( void ) = default;       // Destructor

//----------------------------------------------------------------------------
// Random::Inline methods
//----------------------------------------------------------------------------
double                              // Resultant, range 0.0 .. 1.0
   get_double( void )               // Return a random double value
{  return (double(get()) / UINT32_MAX); }

bool                                // Resultant (TRUE || FALSE)
   is_true(                         // Return TRUE
     double            p)           // With this probability (range 0..1)
{  return (p*UINT32_MAX) >= double(get()); }

uint32_t                            // Resultant, range 0 .. (M-1)
   modulus(                         // Return a random integer value
     uint32_t          M)           // For this M(aximum)
{  return (get() % M); }

//----------------------------------------------------------------------------
// Random::Methods
//----------------------------------------------------------------------------
uint32_t                            // The next random value
   get( void )                      // Get the next random value
{  return uint32_t(get64()); }

void
   randomize( void );               // Randomize the seed

void
   set_seed(                        // Set the seed value
     uint64_t          seed);       // To this

static int                          // Undefined
   _self_test(int);                 // Self-test, undefined parameter

protected:
uint64_t                            // The next random value
   get64( void );                   // Get the next 64-bit random value
}; // class Random
_LIBPUB_END_NAMESPACE
#endif // _LIBPUB_RANDOM_H_INCLUDED
