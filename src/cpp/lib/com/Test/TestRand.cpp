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
//       TestRand.cpp
//
// Purpose-
//       Miscellaneous tests.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/Interval.h>
#include <com/Verify.h>

#include "com/Hardware.h"
#include "com/Random.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#define SCDM                        // If defined, Soft Core Debug Mode
#endif

// Basic test iteration counters
#ifndef TEST_ITERATIONS
  #define TEST_ITERATIONS 1000000   // Number of test iterations
#endif

#ifndef ERROR_TOLERANCE
  #define ERROR_TOLERANCE 0.00175   // Maximum error tolerance for iterations
#endif

//----------------------------------------------------------------------------
// Note that these defaults can be overridden at compile time
//   e.g. using export CDEFS=-DTEST_LEVEL=1
//----------------------------------------------------------------------------
#ifndef TEST_LEVEL
  #define TEST_LEVEL 0              // Default: VERIFICATION test 2
#endif

//----------------------------------------------------------------------------
#if( TEST_LEVEL == 0 )              // Debugging test
  #define DIM_ARRAY   64            // (MINIMUM VALUE!)
  #define RAND_ITERATIONS     ((int64_t)1000000)

//----------------------------------------------------------------------------
#elif( TEST_LEVEL == 1 )            // Verification test 1
  #define DIM_ARRAY 1024
  #define RAND_ITERATIONS     ((int64_t)1000000000)

//----------------------------------------------------------------------------
#elif( TEST_LEVEL == 2 )            // Verification test 2
  #define DIM_ARRAY  128
  #define RAND_ITERATIONS     ((int64_t)10000000000)

//----------------------------------------------------------------------------
#elif( TEST_LEVEL == 3 )            // Timing test
  #define DIM_ARRAY  128
  #define RAND_ITERATIONS     ((int64_t)100000000)

//----------------------------------------------------------------------------
#else                               // Verification test
  #ifndef DIM_ARRAY                 // User specified
    #define DIM_ARRAY  128
  #endif

  #ifndef RAND_ITERATIONS
    #define RAND_ITERATIONS   10000000000LL
  #endif
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard;

//----------------------------------------------------------------------------
//
// Subroutine-
//       isOdd
//
// Purpose-
//       Determine whether a value is odd.
//
//----------------------------------------------------------------------------
static inline int                   // TRUE if the value is odd
   isOdd(                           // Is argument odd
     int64_t           value)       // (The argument)
{
   return value & 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       bitCounter
//
// Purpose-
//       For each bit in a word, count its occurance in a counter array.
//
//----------------------------------------------------------------------------
static inline void
   bitCounter(                      // Count bit occurances.
     uint64_t          word,        // In this word
     uint64_t*         array)       // Counter array
{
   uint64_t            mask;        // Bit mask

   int                 i;

   mask= 1;
   for(i= 0; i<64; i++)
   {
     if( (word&mask) != 0 )
       array[i]++;

     mask <<= 1;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       bitChecker
//
// Purpose-
//       Display the number of occurances for each bit.
//
//----------------------------------------------------------------------------
static inline void
   bitChecker(                      // Check bit occurances.
     const char*       type,        // The function used to set the bits
     int               count,       // The number of tests
     uint64_t*         array)       // Counter array[64]
{
   int                 ones;        // The number of ones encounterd
   int                 minCount;    // Minimum expected
   int                 maxCount;    // Maximum expected

   int                 i;

   debugf("\n");
   verify_info(); debugf("bitChecker(%s)\n", type);
   minCount= count/2 - count/16;
   maxCount= count/2 + count/16;

   for(i= 0; i<63; i++)
   {
     ones= int(array[i]);
     debugf("[%2d] %8d of %8d ", i, ones, count);
     if( ones >= minCount && ones <= maxCount )
       debugf("OK\n");
     else
       debugf("!! NG !!\n");
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       dumpArray
//
// Purpose-
//       Dump an array.
//
//----------------------------------------------------------------------------
static void
   dumpArray(                       // Dump Array[DIM_ARRAY]
     const char*       desc,        // Description
     uint64_t*         array)       // The array
{
   for(int i= 0; i<DIM_ARRAY; i++)
     debugf("[%5d] %.16" PRIx64 " %s\n", i, array[i], desc);

   debugf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testHardware
//
// Purpose-
//       Test the hardware functions used by the Random objects
//
//----------------------------------------------------------------------------
static void
   testHardware( void )             // Test Hardware functions
{
   uint64_t            array[DIM_ARRAY]; // Resultant array

   int                 i;

   debugf("\n");
   verify_info(); debugf("testHardware()\n");

   // Timestamp counter test
   for(i=0; i<DIM_ARRAY; i++)
     array[i]= Hardware::getTSC();

   #if FALSE
     IFSCDM( dumpArray("Hardware::getTSC()", array); )
   #endif

   debugf("%16" PRId64 "= Hardware::getTSC()  (stop)\n", array[DIM_ARRAY-1]);
   debugf("%16" PRId64 "= Hardware::getTSC() (start)\n", array[0]);
   debugf("%16" PRId64 "= cycles\n", array[DIM_ARRAY-1] - array[0]);

   // Each TSC must be greater than the last
   for(i=1; i<DIM_ARRAY; i++)
   {
     if( !verify(array[i] > array[i-1]) )
     {
       verify("Hardware.getTSC() increment failure");
       dumpArray("Hardware::getTSC()", array);

       debugf("[%6d] %.16" PRIx64 "\n", i-1, array[i-1]);
       debugf("[%6d] %.16" PRIx64 "\n", i, array[i]);
       break;
     }
   }

   // Count the number of times each bit is set
   IFSCDM(
     if( DIM_ARRAY >= 64 )
     {
       for(i=0; i<DIM_ARRAY; i++)
         array[i]= 0;

       for(i= 0; i<TEST_ITERATIONS; i++)
         bitCounter(Hardware::getTSC(), array);
       bitChecker("Hardware::getTSC", TEST_ITERATIONS, array);
     }
   )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testRandom
//
// Purpose-
//       Test the Random object basic functions.
//
//----------------------------------------------------------------------------
static void
   testRandom(                      // Test Random object functions
     const char*       name,        // The name of the generator
     Random&           rng)         // The random number generator
{
   uint64_t            array[DIM_ARRAY]; // Random value array
   int                 count;       // Iteration/result counter
   double              isTrueA;     // Actual isTrue probability
   double              isTrueX;     // Expected isTrue probability
   uint64_t            temp;        // Temporary

   int                 i;
   int                 j;

   debugf("\n");
   verify_info(); debugf("testRandom(%s)\n", name);

   // Verify function get() {Short version}
   for(i=0; i<DIM_ARRAY; i++)
     array[i]= rng.get();

   #if FALSE
     IFSCDM( dumpArray("Random::get()", array); )
   #endif

   for(i=0; i<DIM_ARRAY; i++)
   {
     for(j=i+1; j<DIM_ARRAY; j++)
     {
       if( array[i] == array[j] )
       {
         verify("Random::get() repeats");
         dumpArray("Random::get()", array);

         debugf("[%6d] [%6d] %.16" PRIx64 "\n", i, j, array[i]);
         break;
       }
     }
   }

   // Verify function isTrue()
   for(isTrueX=0.0; isTrueX<=1.0; isTrueX+=0.0625)
   {
     count= 0;
     for(i=0; i<TEST_ITERATIONS; i++)
       count += rng.isTrue(isTrueX);
     isTrueA= double(count)/double(TEST_ITERATIONS);

     if( !verify(fabs(isTrueX-isTrueA) <= ERROR_TOLERANCE) )
     {
       debugf("expected(%f) != actual(%f), allowed(%f) actual(%f)\n",
              isTrueX, isTrueA, ERROR_TOLERANCE, fabs(isTrueX-isTrueA));
     }
   }

   // Verify function randomize {short version}
   for(i=0; i<DIM_ARRAY; i++)
   {
     rng.randomize();
     temp= rng.get();
     array[i]= temp;
   }

   for(i=0; i<DIM_ARRAY; i++)
   {
     for(j=i+1; j<DIM_ARRAY; j++)
     {
       if( array[i] == array[j] )
       {
         verify("Random::randomize() repeats");
         dumpArray("Random::randomize()", array);

         debugf("[%6d] [%6d] %.16" PRIx64 "\n", i, j, array[i]);
         break;
       }
     }
   }

   // Count the number of times each bit is set
   IFSCDM(
     if( DIM_ARRAY >= 64 )
     {
       int prior[64];               // Prior bit value
       int  cur0[64];               // Sequentially zero (current sequence)
       int  cur1[64];               // Sequentially ones (current sequence)
       int  max0[64];               // Sequentially zero (longest sequence)
       int  max1[64];               // Sequentially ones (longest sequence)
       int  seq0[64];               // Sequentially zero
       int  seq1[64];               // Sequentially ones

       for(i=0; i<64; i++)
       {
         prior[i]= (-1);            // Neither one nor zero
         cur0[i]= 0;
         cur1[i]= 0;
         max0[i]= 0;
         max1[i]= 0;
         seq0[i]= 0;
         seq1[i]= 0;
       }

       for(i=0; i<DIM_ARRAY; i++)   // For bitCounter
         array[i]= 0;

       for(count= 0; count<TEST_ITERATIONS; count++)
       {
         temp= rng.get();

         uint64_t mask= 1;
         for(i= 0; i<64; i++)
         {
           if( (temp & mask) == 0 ) // If bitvalue zero
           {
             if( prior[i] == 0 )
             {
               cur0[i]++;
               if( cur0[i] > max0[i] )
                 max0[i]= cur0[i];

               seq0[i]++;
             }
             else
             {
               prior[i]= 0;
               cur0[i]= 0;
               cur1[i]= 1;
             }
           }
           else
           {
             if( prior[i] == 1 )
             {
               cur1[i]++;
               if( cur1[i] > max1[i] )
                 max1[i]= cur1[i];

               seq1[i]++;
             }
             else
             {
               prior[i]= 1;
               cur0[i]= 1;
               cur1[i]= 0;
             }
           }

           mask <<= 1;
         }

         bitCounter(temp, array);
       }
       bitChecker("get", count, array);

       debugf("\n");
       debugf(" BIT     Seq0  :  Seq1     Max0     Max1 TEST_ITERATIONS(%d)\n",
              TEST_ITERATIONS);
       for(i= 0; i<64; i++)
       {
         debugf("[%2d] %8d %8d:%8d %8d\n", i,
                seq0[i], seq1[i], max0[i], max1[i]);
       }

       for(i=0; i<DIM_ARRAY; i++)   // For bitCounter
         array[i]= 0;

       for(count= 0; count<(TEST_ITERATIONS/100); count++)
       {
         rng.randomize();
         temp= rng.get();
         bitCounter(temp, array);
       }
       bitChecker("randomize", count, array);
     }
   )
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testRandom_get
//
// Purpose-
//       Test the Random object get function.
//
//----------------------------------------------------------------------------
static void
   testRandom_get(                  // Test Random object get function
     const char*       name,        // The name of the generator
     Random&           rng)         // The random number generator
{
   uint64_t            array[DIM_ARRAY]; // Random value array
   int64_t             count;       // Iteration/result counter
   Interval            interval;    // Interval timer
   int64_t             odd;         // Odd value counter
   uint64_t            temp;        // Temporary

   int                 i;
   int                 j;

   debugf("\n");
   verify_info(); debugf("testRandom_get(%s)\n", name);

   // Verify function get() {Short version}
   for(i=0; i<DIM_ARRAY; i++)
     array[i]= rng.get();

   for(i=0; i<DIM_ARRAY; i++)
   {
     for(j=i+1; j<DIM_ARRAY; j++)
     {
       if( array[i] == array[j] )
       {
         verify("Random::get() repeats");
         dumpArray("Random::get()", array);

         debugf("[%6d] [%6d] %.16" PRIx64 "\n", i, j, array[i]);
         break;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Hard-core test
   //-------------------------------------------------------------------------
   if( error_count() != 0 )
   {
     debugf("Full test SKIPPED: errorCount(%d)\n", error_count());
     return;
   }

   odd= 0;
   interval.start();
   for(count=0; count < RAND_ITERATIONS && error_count() == 0 ; count++)
   {
     temp= rng.get();
     odd += isOdd(temp);

     for(i= 0; i<DIM_ARRAY; i++)
     {
       if( !verify( temp != array[i]) )
       {
         verify("Random::get() repeats");
         dumpArray("Random::get()", array);

         if( i == 0 )
         {
           for(j= i+1; j<DIM_ARRAY; j++)
           {
             temp= rng.get();
             odd += isOdd(temp);

             if( temp == array[j] )
             {
               debugf("Examine: %" PRId64 " Faux wrap [%d][%d]\n", count, i, j);
               break;
             }
           }
           if( j >= DIM_ARRAY )
           {
             debugf("Repeat detected after %" PRId64 ", %" PRId64 " odd",
                    count, odd);
             break;
           }
         }
         else
         {
           debugf("Examine: %" PRId64 " Faux wrap [%d][%d] %.16" PRIx64 "\n",
                  count, i, i, temp);
           break;
         }
       }
     }

     if( (count % (RAND_ITERATIONS/10)) == 0 )
     {
       verify_info();
       debugf("%12" PRId64 " of %12" PRId64 "\n", count, RAND_ITERATIONS);
     }
   }
   interval.stop();

   verify_info();
   debugf("%12" PRId64 " of %12" PRId64 " odd values\n", odd, count);
   verify_info();
   debugf(" Elapsed: %10.4f seconds\n", interval.toDouble());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testRandom_randomize
//
// Purpose-
//       Extensively test the Random object randomize function.
//
//----------------------------------------------------------------------------
static void
   testRandom_randomize(            // Test Random object functions
     const char*       name,        // The name of the generator
     Random&           rng)         // The random number generator
{
   uint64_t            array[DIM_ARRAY]; // Random value array
   int64_t             count;       // Iteration/result counter
   Interval            interval;    // Interval timer
   int64_t             odd;         // Odd value counter
   uint64_t            temp;        // Temporary

   int                 i;
   int                 j;
   int64_t             M;
   int64_t             N;

   debugf("\n");
   verify_info(); debugf("testRandom_randomize(%s)\n", name);

   // Verify function randomize {short version}
   for(i=0; i<DIM_ARRAY; i++)
   {
     rng.randomize();
     temp= rng.get();
     array[i]= temp;
   }

   for(i=0; i<DIM_ARRAY; i++)
   {
     for(j=i+1; j<DIM_ARRAY; j++)
     {
       if( array[i] == array[j] )
       {
         verify("Random::randomize() repeats");
         dumpArray("Random::randomize()", array);

         debugf("[%6d] [%6d] %.16" PRIx64 "\n", i, j, array[i]);
         break;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Hard-core randomizer test
   //-------------------------------------------------------------------------
   if( error_count() != 0 )
   {
     debugf("Full test SKIPPED: errorCount(%d)\n", error_count());
     return;
   }

   odd= 0;
   M= RAND_ITERATIONS/100;          // Number of test iterations
   N= M/10;                         // Number of iterations between prints

   interval.start();
   for(count= 0; count<M && error_count() == 0; count++)
   {
     rng.randomize();
     temp= rng.get();
     odd += isOdd(temp);
     for(i= 0; i<DIM_ARRAY; i++)
     {
       if( temp == array[i] )
       {
         verify("Random::randomize() repeats");
         dumpArray("Random::randomize()", array);

         debugf("%12" PRId64 " [%6d] %.16" PRIx64 "\n", count, i, array[i]);
         break;
       }
     }

     if( (count % N) == 0 )
     {
       verify_info();
       debugf("%12" PRId64 " of %12" PRId64 "\n", count, M);
     }
   }
   interval.stop();

   verify_info();
   debugf("%12" PRId64 " of %12" PRId64 " odd values\n", odd, count);
   verify_info();
   debugf(" Elapsed: %10.4f seconds\n", interval.toDouble());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
extern int
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count
//   char*             argv[])      // Argument array
{
   Random              simpleRandom;  // Base class
   PseudoRandom        pseudoRandom;  // Pseudo random object
   PerfectRandom       perfectRandom; // Perfect random object

   //-------------------------------------------------------------------------
   // Test type
   //-------------------------------------------------------------------------
   verify_info();
   if( TEST_LEVEL == 0 )
     debugf("DEBUG test\n");
   else if( TEST_LEVEL == 1 )
     debugf("VERIFICATION test 1\n");
   else if( TEST_LEVEL == 2 )
     debugf("VERIFICATION test 2\n");
   else if( TEST_LEVEL == 3 )
     debugf("TIMING test\n");
   else
     debugf("VERIFICATION test: DIM_ARRAY(%d) RAND_ITERATIONS(%" PRId64 ")\n",
            DIM_ARRAY, RAND_ITERATIONS);

   //-------------------------------------------------------------------------
   // Basic tests
   //-------------------------------------------------------------------------
   testHardware();
   testRandom("Common",  RNG);
   testRandom("Simple",  simpleRandom);
   testRandom("Pseudo",  pseudoRandom);
   testRandom("Perfect", perfectRandom);

   //-------------------------------------------------------------------------
   // Extended tests
   //-------------------------------------------------------------------------
   testRandom_randomize("Simple", simpleRandom);

   testRandom_get("Simple",  simpleRandom);
   testRandom_get("Pseudo",  pseudoRandom);
   testRandom_get("Perfect", perfectRandom);

   //-------------------------------------------------------------------------
   // Testing complete
   //-------------------------------------------------------------------------
   verify_exit();
}

