//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
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
//       Implement the Random objects.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdio.h>

#include <com/Atomic.h>
#include <com/Debug.h>
#include <com/Hardware.h>
#include <com/Thread.h>

#include "com/Random.h"

//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
// N, Log2(N)
//       N 0, 1, 2, 4, 8, 16, 32, 64, 128, 256
// Log2(N) *, 0, 1, 2, 3,  4,  5,  6,   7,   8

enum                                // Bit/Register specifiers
{  REG_MASK= (PerfectRandom::REG_COUNT - 1) // Register mask
}; // enum

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
const double           Random::MINIMUM= 0.0;
const double           Random::MAXIMUM= double(0x7fffffffffffffffLL);
Random                 Random::standard; // A standard random object

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const uint64_t  MAX63= 0x7fffffffffffffffLL;
static uint64_t        internal_seed= 0x4567cdef012389abLL;
static volatile int32_t shift_1s=  0; // Low-order bits of getTSC
static volatile int32_t shift_ix= 32; // Shift counter

//----------------------------------------------------------------------------
//
// Subroutine-
//       getShiftTSC
//
// Purpose-
//       Calculate the number of constant TSC bits.
//
//----------------------------------------------------------------------------
static int                          // The number of constant TSC bits
   getShiftTSC( void )              // Get number of constant TSC bits
{
   int ix= shift_ix;                // Current shift index
   if( ix > 0 )                     // If minimal shift index not found
   {
     int32_t ones= shift_1s;
     int32_t temp= (int32_t)Hardware::getTSC();
     ones |= temp;

     int ix;
     for(ix= 0; (temp&1) == 0; ix++)
       temp >>= 1;

     if( ix < shift_ix )
     {
       int32_t oldValue, newValue;  // Compare and swap values
       int cc;                      // Compare and swap condition code

       do {
         oldValue= shift_1s;
         newValue= shift_1s | ones;
         cc= csw((ATOMIC32*)&shift_1s, oldValue, newValue);
       } while( cc != 0 );

       do {
         oldValue= shift_ix;
         newValue= ix;
         cc= 0;
         if( ix < shift_ix )
           cc= csw((ATOMIC32*)&shift_ix, oldValue, newValue);
       } while( cc != 0 );
     }
   }

   return ix;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       next
//
// Function-
//       Get the next simple pseudo-random value.
//
//----------------------------------------------------------------------------
static uint64_t                     // The next pseudo-random value
   next(                            // Get the next pseudo-random value
     uint64_t          seed)        // For this seed
{
   seed= seed^(seed>>29);
   seed ^= seed<<34;
   seed &= MAX63;
   return seed;
}

//----------------------------------------------------------------------------
//
// Class-
//       Initializer
//
// Purpose-
//       Initialize the getShiftTSC values to a reasonable minimum value.
//
//----------------------------------------------------------------------------
#define Initializer Random_StaticInitializer_ujMiwUvwwtL8
static class Initializer {
public:
   Initializer( void )
{
   for(int i= 0; i<128; i++)
     getShiftTSC();
}
}                      static_initializer;

//----------------------------------------------------------------------------
//
// Method-
//       Random::~Random
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Random::~Random( void )          // Destructor
{
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
:  seed(0x0123456789abcdefLL)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Random::get
//
// Function-
//       Get next simple pseudo-random value.
//
//----------------------------------------------------------------------------
uint64_t                            // The next pseudo-random value
   Random::get( void )              // Get the next pseudo-random value
{
   seed= next(seed);
   return seed;
}

//----------------------------------------------------------------------------
//
// Method-
//       Random::getBit
//
// Function-
//       Get a random value. While we attempt to make the entire value random,
//       externally only the low order bit is guaranteed to be random.
//
//----------------------------------------------------------------------------
uint64_t                            // The hardware random value
   Random::getBit( void )           // Get hardware random value
{
   uint64_t            result;      // Resultant

   do {
     result= internal_seed= next(internal_seed);
     result ^= next(Hardware::getTSC() >> getShiftTSC());
   } while( result == 0 );

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Random::randomize
//
// Purpose-
//       Initialize the random number generator.
//
//----------------------------------------------------------------------------
void
   Random::randomize( void )        // Randomize
{
   setSeed(getBit());
}

//----------------------------------------------------------------------------
//
// Method-
//       Random::setSeed
//
// Function-
//       Set the pseudo-random value seed.
//
//----------------------------------------------------------------------------
void
   Random::setSeed(                 // Set the next pseudo-random value
     uint64_t          seed)        // To this
{
   seed &= MAX63;
   if( seed == 0 )
     seed= 0x0123456789abcdefLL;

   this->seed= seed;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       MT19937 helpers
//
// Purpose-
//       MT19937 implementation helpers
//
//----------------------------------------------------------------------------
inline uint32_t topOf(uint32_t x) { return x & 0x80000000U; }
inline uint32_t botOf(uint32_t x) { return x & 0x00000001U; }
inline uint32_t lowOf(uint32_t x) { return x & 0x7fffffffU; }
inline uint32_t mixOf(uint32_t x, uint32_t y) { return topOf(x) | lowOf(y); }
inline uint32_t twist(uint32_t x, uint32_t y, uint32_t z)
{ return x ^ (mixOf(y,z)>>1) ^ (-botOf(z) & 0x9908b0dfU); }

//----------------------------------------------------------------------------
//
// Method-
//       PseudoRandom::~PseudoRandom
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   PseudoRandom::~PseudoRandom( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       PseudoRandom::PseudoRandom
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   PseudoRandom::PseudoRandom( void ) // Default constructor
:  Random()
,  index(0)
{
   setSeed(0x0123456789abcdefLL);
}

//----------------------------------------------------------------------------
//
// Method-
//       PseudoRandom::get
//
// Purpose-
//       Get the next random number.
//
//----------------------------------------------------------------------------
uint64_t                            // The next random number
   PseudoRandom::get( void )        // Get the next random number
{
   uint64_t result= get32();
   result <<= 32;
   result  |= get32();
   result  &= MAX63;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PseudoRandom::get32
//
// Purpose-
//       Get the next unsigned 32 bit random number.
//
//----------------------------------------------------------------------------
uint32_t                            // The next random number
   PseudoRandom::get32( void )      // Get the next random number
{
   if(index >= DIM)
   {
     index=0;

     // Update the twist registers
     int32_t* p= MT;
     for(int i= (DIM-PER); i--; ++p )
       *p= twist(p[PER], p[0], p[1]);

     for(int i= PER; --i; ++p )
       *p= twist(p[PER-DIM], p[0], p[1]);

     *p= twist(p[PER-DIM], p[0], MT[0]);
   }

   // Get the next random number per MT19937 algorithm
   uint32_t result= MT[index++];
   result ^=  (result >> 11);
   result ^= ((result <<  7) & 2636928640U); // 0x9d2c5680
   result ^= ((result << 15) & 4022730752U); // 0xefc60000
   result ^=  (result >> 18);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PseudoRandom::setSeed
//
// Purpose-
//       Set the random number seed.
//
//----------------------------------------------------------------------------
void
   PseudoRandom::setSeed(           // Set the random number seed
     uint64_t          seed)        // To this
{
   MT[0]= seed ^ (seed >> 32);      // 32 bit value, zero allowed
   for(uint32_t i= 1; i<DIM; i++)
     MT[i]= uint32_t(1812433253 * (MT[i-1] ^ (MT[i-1]>>30)) + i);

   index= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       PerfectRandom::~PerfectRandom
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   PerfectRandom::~PerfectRandom( void ) // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       PerfectRandom::PerfectRandom
//
// Purpose-
//       Default constructor
//
//----------------------------------------------------------------------------
   PerfectRandom::PerfectRandom( void ) // Default constructor
:  Random()
{
   randomize();
}

//----------------------------------------------------------------------------
//
// Method-
//       PerfectRandom::get
//
// Function-
//       Get the next random value.
//
//----------------------------------------------------------------------------
uint64_t                            // The next random value
   PerfectRandom::get( void )       // Get the next random value
{
   uint64_t            clock;       // The clock register's content
   int                 regX;        // Register index

   // Update the seed
   seed= next(seed);

   // Update the clock register
   clock= hidden[REG_CLOCK];
   clock ^= getBit();
   hidden[REG_CLOCK]= clock;

   // Randomly modify a hidden register (possibly the clock register)
   regX= int(clock & REG_MASK);
   hidden[regX] ^= getBit();

   // Return (seed ^ hidden[rand])
   clock >>= REG_SHIFT;             // (Probably a different register)
   regX= int(clock & REG_MASK);
   return ((seed ^ hidden[regX]) & MAX63);
}

//----------------------------------------------------------------------------
//
// Method-
//       PerfectRandom::setSeed
//
// Purpose-
//       Set the random number seed.
//
//----------------------------------------------------------------------------
void
   PerfectRandom::setSeed(          // Set the random number seed
     uint64_t          seed)        // To this
{
   if( seed == 0 )
     seed= 0xfedcba9876543210LL;

   for(int i= 0; i<REG_COUNT; i++)  // Randomize the hidden registers
     hidden[i]= getBit();
}

