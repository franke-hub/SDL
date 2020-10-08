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
//       factor.cpp
//
// Purpose-
//       Factor (64 bit integer)
//
// Last change date-
//       2007/01/01
//
// Largest calculatable prime-
//       9,223,372,036,854,775,783 (0x7fff.ffff.ffff.ffe7)
//       Caclulation takes about 9 minutes on a 166Mhz Pentium.
//
//----------------------------------------------------------------------------
#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>

#include <com/Binary.h>
#include <com/Debug.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define SIEVESIZE             30030 // 2 * 3 * 5 * 7 * 11 * 13
#define SIEVEMAXP                13 // The highest prime in the sieve

//----------------------------------------------------------------------------
// Static areas
//----------------------------------------------------------------------------
unsigned char          sieve[SIEVESIZE];

//----------------------------------------------------------------------------
//
// Subroutine-
//       factor
//
// Purpose-
//       Factor a number
//
//----------------------------------------------------------------------------
static int                          // Return code (0 if prime)
   factor(                          // Factor a number
     int64_t           value,       // The number
     int64_t&          fact1,       // Lowest factor (1 if prime)
     int64_t&          fact2)       // Remaining factor (value if prime)
{
   int64_t             factor;
   int64_t             minfact;
   int64_t             maxfact;
   int64_t             root;
   int                 i;

   if (value <= 3)
   {
     fact1= 1;
     fact2= value;
     return FALSE;
   }

   if ((value & 1) == 0)
   {
     fact1= 2;
     fact2= value/2;
     return TRUE;
   }

   // Determine approximate square root
   root= value / 2;
   do
   {
     minfact= root;
     maxfact= value / root;
     root= (minfact + maxfact) >> 1;
   } while(maxfact < minfact);

   maxfact += (SIEVESIZE);

   // Search for factors
   for(minfact=0; minfact<=maxfact; minfact+=SIEVESIZE)
   {
     for(i=1; i<SIEVESIZE; i+=2)
     {
       if (sieve[i])                // If this is a possible prime
       {
         factor= minfact + i;
         if ((value % factor) == 0)
         {
           if (factor >= value)
             break;

           fact1= factor;
           fact2= value / factor;
           return TRUE;
         }
       }
     }
   }

   // The value is a prime
   fact1= 1;
   fact2= value;
   return FALSE;
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
int                                 // Return code
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   UnsignedBinary<8>   param;
   int64_t             value;
   int64_t             remainder;
   int32_t             factors= 0;
   int                 rc;
   int                 i, j;

   // Initialize the sieve
   for(i=0; i<SIEVESIZE; i++)       // Default, all of sieve contains primes
     sieve[i]= TRUE;

   sieve[0]= FALSE;                 // (Always divisible by 2)
   sieve[1]= FALSE;                 // 1 not a prime, 30031 divisible by 59
   for(i= 2; i<=SIEVEMAXP; i++)     // Clear the known primes
   {
     for(j= i+i; j<SIEVESIZE; j+=i)
     {
       sieve[j]= FALSE;
     }
   }

   try {
     for(int i= 1; i<argc; i++) {
       param.inp(argv[i]);          // Get the parameter value
       value= param.toInt();
       printf("%" PRId64 ":", value);
       for(;;)
       {
         rc= factor(value, remainder, value);
         if (rc == FALSE)
           break;

         factors= 1;
         printf(" %" PRId64, remainder);
       }

       if (factors)
         printf(" %" PRId64 "\n", value);
       else
         printf(" is a PRIME\n");
     }
   } catch(const char* X) {
     printf("Error(%s)\n", X);
     return 1;
   } catch(...) {
     printf("Error(...)\n");
     return 1;
   }

   return 0;
}

