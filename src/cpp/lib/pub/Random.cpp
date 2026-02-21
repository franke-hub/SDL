//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Random.cpp
//
// Purpose-
//       Implement Random.h
//
// Last change date-
//       2026/02/18
//
//----------------------------------------------------------------------------
#include <atomic>                   // For std::atomic<uint64_t>
#include <cstdint>                  // For integer types, UINT64_MAX

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Hardware.h>           // For pub::Hardware::getTSC
#include "pub/Random.h"             // For pub::Random, implemented

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // Using PUB library
using namespace PUB::debugging;     // For debugging methods

namespace _LIBPUB_NAMESPACE {
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbiosity, higher is more verbose
};

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
Random                 Random::standard; // A standard random object

//----------------------------------------------------------------------------
// Internal data area: constants
//----------------------------------------------------------------------------
static const uint64_t  initializer= uint64_t(0x0123456789ABCDEF);

//----------------------------------------------------------------------------
//
// Method-
//       Random::Random
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Random::Random( void )           // Default constructor
:  seed(initializer)                // (Default seed)
{  }

//----------------------------------------------------------------------------
//
// Method-
//       Random::get
//
// Function-
//       Get next simple pseudo-random value.
//
// Implementation note-
//       Thread safe and lock free.
//
//----------------------------------------------------------------------------
uint64_t                            // The next pseudo-random value
   Random::get64( void )            // Get the next pseudo-random value
{
   uint64_t old_seed= seed.load();  // The current seed value
   uint64_t new_seed;               // The updated seed value
   do {                             // Update the seed value
     new_seed= old_seed^(old_seed>>29);
     new_seed ^= new_seed<<34;
     new_seed &= INT64_MAX;
   } while( ! seed.compare_exchange_strong(old_seed, new_seed) );

   return new_seed;
}

//----------------------------------------------------------------------------
//
// Method-
//       Random::randomize
//
// Purpose-
//       Initialize the random number generator, somewhat unpredictably.
//
//----------------------------------------------------------------------------
void
   Random::randomize( void )        // Randomize
{
   uint64_t new_seed= Hardware::getTSC();
   new_seed ^= Hardware::getTSC();
   new_seed ^= Hardware::getTSC();

   set_seed(new_seed);

   get64();
   get64();
}

//----------------------------------------------------------------------------
//
// Method-
//       Random::set_seed
//
// Function-
//       Set the pseudo-random value seed.
//
//----------------------------------------------------------------------------
void
   Random::set_seed(                // Set the next pseudo-random value
     uint64_t          new_seed)    // To this
{
   if( new_seed == 0 )              // (Zero value locks at zero)
     new_seed= initializer;         // Convert zero to default initializer

   seed.store(new_seed);
}
_LIBPUB_END_NAMESPACE
