//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Random.cpp
//
// Purpose-
//       Implement Random.h
//
// Last change date-
//       2023/12/02
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

,  USE_SELF_TEST= true              // Use _self_test method?
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
// Macro-
//       verify_info
//
//----------------------------------------------------------------------------
#define verify_info debugf("\n%4d %s: ", __LINE__, __FILE__)

//----------------------------------------------------------------------------
//
// Subroutine-
//       bit_counter
//
// Purpose-
//       For each bit in a word, count its occurance in a counter array.
//
//----------------------------------------------------------------------------
static inline void
   bit_counter(                     // Count bit occurances.
     uint64_t          word,        // In this word
     uint64_t*         array)       // Counter array
{
   uint64_t mask= 1;
   for(int i= 0; i<64; i++) {
     if( (word&mask) != 0 )
       array[i]++;

     mask <<= 1;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       bit_checker (Used by Random::_self_test)
//
// Purpose-
//       Display the number of occurances for each bit.
//
//----------------------------------------------------------------------------
static inline void
   bit_checker(                     // Check bit occurances.
     const char*       type,        // The function used to set the bits
     int               count,       // The number of tests
     uint64_t*         array)       // Counter array[64]
{
   int minCount= count/2 - count/512;
   int maxCount= count/2 + count/512;
   debugf("bit_checker(%s) {%'d; %'d; %'d}\n", type
         , minCount, count/2, maxCount);

   for(int i= 0; i<63; i++) {
     int x= 62 - i;
     int ones= int(array[x]);
     debugf("[%2d] %'8d of %'8d ", x, ones, count);
     if( ones >= minCount && ones <= maxCount )
       debugf("OK\n");
     else
       debugf("!! NG !!\n");
   }
}

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

//----------------------------------------------------------------------------
//
// Method-
//       Random::_self_test
//
// Function-
//       Run self test
//
//----------------------------------------------------------------------------
int                                 // Error count
   Random::_self_test(              // Run the self test
     int               opt_verbose) // Verbosity
{
   enum
   { DIM_ARRAY=        64
   , ITERATIONS=         uint64_t(100'000'000) // Range:    10,000,000 and up
   , CHECK_ITERATIONS= uint64_t(5'000'000'000) // Range: 1,000,000,000 and up
   };

   int error_count= 0;
   if( ! USE_SELF_TEST ) {
     if( opt_verbose )
       debugf("\ntest_Random disabled\n");
   } else {
     static Random& RNG= Random::standard;

     if( opt_verbose )
       debugf("\ntest_Random\n");

     //-----------------------------------------------------------------------
     // Quick test for duplicates
     uint64_t            array[DIM_ARRAY]; // Random value array
     int                 count;     // Iteration/result counter

     for(int i=0; i<DIM_ARRAY; i++)
       array[i]= RNG.get64();

     for(int i=0; i<DIM_ARRAY; i++) {
       for(int j=i+1; j<DIM_ARRAY; j++) {
         if( array[i] == array[j] ) {
           debugf("Random::get64() repeats [%d]==[%d]\n", i, j);
           for(int x= 0; x<DIM_ARRAY; ++x) {
             debugf("[%'5d]: %'zu\n", x, array[x]);
           }
           return 1;                // Quick loop detected
         }
       }
     }

     //-----------------------------------------------------------------------
     // Test for duplicates. Duplicates will repeat sequence
     uint64_t checker= array[DIM_ARRAY-1];
     if( opt_verbose )
       debugf("Pass 1\n");
     for(uint64_t i= 1; i <= CHECK_ITERATIONS; ++i) {
       if( RNG.get64() == checker ) { // Oh no! Algorithm failure
         debugf("Random::get64() repeats: %'zd loops, value %'zu\n", i, checker);
         return ++error_count;        // Slow loop detected
       }
       if( opt_verbose && (i % 1'000'000'000) == 0 )
         printf("Iteration %'16zd of %'16zd\n", i, CHECK_ITERATIONS);
     }

     checker= RNG.get64();            // We might have skidded into a loop
     if( opt_verbose )
       debugf("Pass 2\n");
     for(uint64_t i= 1; i <= CHECK_ITERATIONS; ++i) {
       if( RNG.get64() == checker ) { // Oh no! Algorithm failure
         debugf("Random::get64() repeats: %'zd loops, value %'zu\n", i, checker);
         return ++error_count;        // Slow loop detected
       }
       if( opt_verbose && (i % 1'000'000'000) == 0 )
         printf("Iteration %'16zd of %'16zd\n", i, CHECK_ITERATIONS);
     }
     if( opt_verbose )
       debugf("No duplicate found in %'zu iterations\n", 2 * CHECK_ITERATIONS);

     //-----------------------------------------------------------------------
     // Distribution tests
     if( opt_verbose && DIM_ARRAY >= 64 ) {
       // Testing get
       int prior[64];               // Prior bit value
       int  cur0[64];               // Sequentially zero (current sequence)
       int  cur1[64];               // Sequentially ones (current sequence)
       int  max0[64];               // Sequentially zero (longest sequence)
       int  max1[64];               // Sequentially ones (longest sequence)
       int  seq0[64];               // Sequentially zero
       int  seq1[64];               // Sequentially ones

       for(int i=0; i<64; i++) {
         prior[i]= (-1);            // Neither one nor zero
         cur0[i]= 0;
         cur1[i]= 0;
         max0[i]= 0;
         max1[i]= 0;
         seq0[i]= 0;
         seq1[i]= 0;
       }

       for(int i=0; i<DIM_ARRAY; i++)   // For bitCounter
         array[i]= 0;

       for(count= 0; count<ITERATIONS; count++) {
         uint64_t temp= RNG.get64();

         uint64_t mask= 1;
         for(int i= 0; i<64; i++) {
           if( (temp & mask) == 0 ) { // If bitvalue zero
             if( prior[i] == 0 ) {
               cur0[i]++;
               if( cur0[i] > max0[i] )
                 max0[i]= cur0[i];

               seq0[i]++;
             } else {
               prior[i]= 0;
               cur0[i]= 0;
               cur1[i]= 1;
             }
           } else {
             if( prior[i] == 1 ) {
               cur1[i]++;
               if( cur1[i] > max1[i] )
                 max1[i]= cur1[i];

               seq1[i]++;
             } else {
               prior[i]= 1;
               cur0[i]= 1;
               cur1[i]= 0;
             }
           }

           mask <<= 1;
         }

         bit_counter(temp, array);
       }
       verify_info; bit_checker("get", count, array);

       // Testing randomize
       debugf("\n BIT         Seq0    :    Seq1 Max0 Max1 ITERATIONS(%'zd)\n"
             , ITERATIONS);
       for(int i= 0; i<63; i++) {
         int x= 62 - i;
         debugf("[%2d] %'12d %'12d %'4d %'4d\n", x
               , seq0[x], seq1[x], max0[x], max1[x]);
       }

       for(int i=0; i<64; i++)      // For bitCounter
         array[i]= 0;

       for(count= 0; count<ITERATIONS; count++) {
         RNG.randomize();
         uint64_t temp= RNG.get64();
         bit_counter(temp, array);
       }
       verify_info; bit_checker("randomize", count, array);
     }
   }

   return error_count;
}
_LIBPUB_END_NAMESPACE
