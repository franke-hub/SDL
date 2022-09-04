//----------------------------------------------------------------------------
//
//       Copyright (c) 2021-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_num.cpp
//
// Purpose-
//       Test the Number object.
//
// Last change date-
//       2022/09/02
//
//----------------------------------------------------------------------------
#include <inttypes.h>               // For PRId64, PRIx64 printf format macros
#include <iostream>                 // For cout
#include <stdio.h>                  // For sprintf
#include <string.h>                 // For strcpy

#include <pub/TEST.H>               // For VERIFY macro
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Interval.h>           // For pub::Interval
#include "pub/Number.h"             // For pub::Number, tested
#include <com/Random.h>             // For com::Random
#include <pub/Wrapper.h>            // For pub::Wrapper

#define PUB _LIBPUB_NAMESPACE
using namespace PUB;                // For Number
using namespace PUB::debugging;     // For debugging functions
using PUB::Wrapper;                 // For pub::Wrapper class
using namespace std;

#define opt_hcdm       PUB::Wrapper::opt_hcdm
#define opt_verbose    PUB::Wrapper::opt_verbose

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= false                      // Hard Core Debug Mode?
,  SCDM= false                      // Soft Core Debug Mode?
,  ITERATIONS= 100'000              // Iteration count
};

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator
static const intmax_t  sONE= 0x8796a5b4c3d2e1f0LL;
static const intmax_t  sTWO= 0x0f1e2d3c4b5a6978LL;
static const uintmax_t uONE= 0x8796a5b4c3d2e1f0LL;
static const uintmax_t uTWO= 0x0f1e2d3c4b5a6978LL;

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_bringup
//
// Purpose-
//       Bringup test
//
//----------------------------------------------------------------------------
static int                          // Error count
   test_bringup( void )             // Bringup test
{
   if( opt_verbose ) {
     debugf("\ntest_bringup\n");

     debugf("%4zd= sizeof(Number)\n", sizeof(Number));
     debugf("%4zd= sizeof(Number::Byte)\n", sizeof(Number::Byte));
     debugf("%4zd= sizeof(Number::Word)\n", sizeof(Number::Word));
     debugf("%4zd= Number::MIN_SIZE\n", Number::get_minsize());
   }

   Number one= sONE;
   Number two= sTWO;

   int error_count= 0;
   error_count += VERIFY( one == sONE );
   error_count += VERIFY( two == sTWO );
   error_count += VERIFY( two != one );

   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Woverflow"
   error_count += VERIFY( (two + one) == (sTWO + sONE) );
   error_count += VERIFY( (two - one) == (sTWO - sONE) );
   error_count += VERIFY( (two * one) == (sTWO * sONE) );
   error_count += VERIFY( (two / one) == (sTWO / sONE) );
   error_count += VERIFY( (two % one) == (sTWO % sONE) ); // (Problem with % in verify macro)
   error_count += VERIFY( (two & one) == (sTWO & sONE) );
   error_count += VERIFY( (two | one) == (sTWO | sONE) );
   error_count += VERIFY( (two ^ one) == (sTWO ^ sONE) );
   error_count += VERIFY( (     ~one) == ~sONE );
   error_count += VERIFY( (     +one) == +sONE );
   error_count += VERIFY( (     -one) == -sONE );
   #pragma GCC diagnostic pop

   // Simple assignment
   one= -9876543210LL;
   two=  1234567890LL;
   error_count += VERIFY( one == -9876543210 );
   error_count += VERIFY( two ==  1234567890 );

   two= one;
   error_count += VERIFY( two == -9876543210 );
   error_count += VERIFY( one == two );

   // Hard Core Debug Mode: Shift operators
   if( false ) {
     intmax_t imax= 0xFEDCBA9876543210LL;
     one= imax;
//   two= imax;
     Number zero; zero.reset(nullptr);
     for(int i= 0; i<64; i++) {
       error_count += VERIFY( (one << i) == (imax << i) );
       error_count += VERIFY( (one >> i) == (imax >> i) );
//     error_count += VERIFY( (two << i) == (((uintmax_t)imax) << i) );
//     error_count += VERIFY( (two >> i) == (((uintmax_t)imax) >> i) );
       error_count += VERIFY( (zero >> i) == 0 );
       error_count += VERIFY( (zero << i) == 0 );
       if( error_count ) {
         debugf("     i: %d\n", i);
         debugf("one<<i: %s\n", (one << i).out().c_str());
         debugf("one>>i: %s\n", (one >> i).out().c_str());
//       debugf("two<<i: %s\n", (two << i).out().c_str());
//       debugf("two>>i: %s\n", (two >> i).out().c_str());
         debugf("iii>>i: %lx\n", (imax >> i));
         debugf("iii<<i: %lx\n", (imax << i));
//       debugf("uuu>>i: %lx\n", (((uintmax_t)imax) >> i));
//       debugf("uuu<<i: %lx\n", (((uintmax_t)imax) << i));
         break;
       }
     }
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_format
//
// Purpose-
//       Number object format test
//
//----------------------------------------------------------------------------
static int                          // Error count
   test_format(                     // Test Number::out()
     const char*       numFormat,   // Number format string
     Number&           numObject,   // Number object
     const char*       intFormat,   // Integer format string
     intmax_t          intObject)   // Integer object
{
   char                numString[256]; // Number string
   char                intString[256]; // Integer string

   int error_count= 0;

   strcpy(numString, numObject.out(numFormat).c_str());
   sprintf(intString, intFormat, intObject);
   if( VERIFY( strcmp(numString,intString) == 0) ) {
     ++error_count;
     debugf("numString(%s) (%s)\n", numString, numFormat);
     debugf("intString(%s) (%s)\n", intString, intFormat);
   }

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Number
//
// Purpose-
//       Number object value tests
//
//----------------------------------------------------------------------------
static int                          // Error count
   test_Number( void )              // Test Number object
{
   if( opt_verbose )
     debugf("\ntest_Number\n");

   int error_count= 0;
   Interval interval;
   interval.start();
   for(int i= 0; i<ITERATIONS; i++) {
     intmax_t intL= RNG.get() + RNG.get() + RNG.get();
     intmax_t intR= RNG.get() + RNG.get() + RNG.get();
     intmax_t intD= intR & 0x7fffffff;
     if( intD == 0 )
       intD= 1;

     int intS= RNG.get() % 160 - 80;
     int intM= intS;
     if( intM == 0 )
       intM= 1;

     Number numA(intL);
     Number numB(nullptr, 12);
     numB= numA;
     Number numL(numA);
     Number numR= intR;
     Number numD= intD;

     // Verify constructors
     error_count += VERIFY( numL == intL && !(numL != intL) );
     error_count += VERIFY( intL == numL && !(intL != numL) );
     error_count += VERIFY( numR == intR && !(numR != intR) );
     error_count += VERIFY( intR == numR && !(intR != numR) );
     error_count += VERIFY( intL == numB && (numB == intL ) );

//   VarBinary         varA(intL);
//   VarBinary         varL(varA);
//   VarBinary         varR(binR);
//
//   error_count += VERIFY( varA == intL && varL == intL && varR == intR );
//   error_count += VERIFY( varA.getData() != varL.getData() );
//   error_count += VERIFY( varA.getData() != varR.getData() );
//   error_count += VERIFY( varL.getData() != varR.getData() );
//   error_count += VERIFY( varL.getData() != numR.getData() );

     // (Assumes Number endian is the same as hardware endian)
     error_count += VERIFY( intL == *(intmax_t*)numB.get_data() );
     error_count += VERIFY( intL == *(intmax_t*)numL.get_data() );
     error_count += VERIFY( intR == *(intmax_t*)numR.get_data() );

     // Verify comparison operators
     if( intL < intR ) {
       error_count += VERIFY( (numL < numR) && (numL <= numR) );
       error_count += VERIFY( (intL < numR) && (intL <= numR) );
       error_count += VERIFY( (numL < intR) && (numL <= intR) );
       error_count += VERIFY( !(numL == numR) && !(numL >= numR) && !(numL > numR) );
       error_count += VERIFY( !(intL == numR) && !(intL >= numR) && !(intL > numR) );
       error_count += VERIFY( !(numL == intR) && !(numL >= intR) && !(numL > intR) );
     } else {
       error_count += VERIFY( !(numL < numR) && (numL >= numR) );
       error_count += VERIFY( !(intL < numR) && (intL >= numR) );
       error_count += VERIFY( !(numL < intR) && (numL >= intR) );
       if( intL == intR ) {
         error_count += VERIFY( (numL <= numR) && (numL == numR) && !(numL > numR) );
         error_count += VERIFY( (intL <= numR) && (intL == numR) && !(intL > numR) );
         error_count += VERIFY( (numL <= intR) && (numL == intR) && !(numL > intR) );
       } else {
         error_count += VERIFY( !(numL <= numR) && !(numL == numR) && (numL > numR) );
         error_count += VERIFY( !(intL <= numR) && !(intL == numR) && (intL > numR) );
         error_count += VERIFY( !(numL <= intR) && !(numL == intR) && (numL > intR) );
       }
     }
     error_count += VERIFY( (intL <  intS) == (numL <  intS) );
     error_count += VERIFY( (intL <= intS) == (numL <= intS) );
     error_count += VERIFY( (intL == intS) == (numL == intS) );
     error_count += VERIFY( (intL >= intS) == (numL >= intS) );
     error_count += VERIFY( (intL >  intS) == (numL >  intS) );

     error_count += VERIFY( (intS <  intR) == (intS <  numR) );
     error_count += VERIFY( (intS <= intR) == (intS <= numR) );
     error_count += VERIFY( (intS == intR) == (intS == numR) );
     error_count += VERIFY( (intS >= intR) == (intS >= numR) );
     error_count += VERIFY( (intS >  intR) == (intS >  numR) );

     // Verify bitwise operators
     error_count += VERIFY( (intL&intR) == (numL&numR) );
     error_count += VERIFY( (intL&intR) == (intL&numR) );
     error_count += VERIFY( (intL&intR) == (numL&intR) );
     error_count += VERIFY( (intR&intL) == (intR&numL) );
     error_count += VERIFY( (intR&intL) == (numR&intL) );
     error_count += VERIFY( (intL&intS) == (numL&intS) );

     error_count += VERIFY( (intL|intR) == (numL|numR) );
     error_count += VERIFY( (intL|intR) == (intL|numR) );
     error_count += VERIFY( (intL|intR) == (numL|intR) );
     error_count += VERIFY( (intR|intL) == (intR|numL) );
     error_count += VERIFY( (intR|intL) == (numR|intL) );
     error_count += VERIFY( (intL|intS) == (numL|intS) );

     error_count += VERIFY( (intL^intR) == (numL^numR) );
     error_count += VERIFY( (intL^intR) == (intL^numR) );
     error_count += VERIFY( (intL^intR) == (numL^intR) );
     error_count += VERIFY( (intR^intL) == (intR^numL) );
     error_count += VERIFY( (intR^intL) == (numR^intL) );
     error_count += VERIFY( (intL^intS) == (numL^intS) );

     // Verify unary operators
     error_count += VERIFY( (+intR) == (+numR) );
     error_count += VERIFY( (-intR) == (-numR) );
     error_count += VERIFY( (~intR) == (~numR) );
     error_count += VERIFY( (!intR) == (!numR) );
     error_count += VERIFY( +(+intR) == +(+numR) );
     error_count += VERIFY( -(-intR) == -(-numR) );
     error_count += VERIFY( ~(~intR) == ~(~numR) );
     error_count += VERIFY( !(!intR) == !(!numR) );

     // Verify shift operators
     intmax_t verI= intL << (intS%64);
     Number   verN= numL << (intS%64);
     error_count += VERIFY( verI == verN );
     verI= intR >> (intS%64);
     verN= numR >> (intS%64);
     error_count += VERIFY( verI == verN );

     // Verify addition operators
     error_count += VERIFY( (intL+intR) == (numL+numR) );
     error_count += VERIFY( (intL+intR) == (intL+numR) );
     error_count += VERIFY( (intL+intR) == (numL+intR) );
     error_count += VERIFY( (intR+intL) == (intR+numL) );
     error_count += VERIFY( (intR+intL) == (numR+intL) );
     error_count += VERIFY( (intL+intS) == (numL+intS) );

     numB= numL + numR;
     numA= numB;
     error_count += VERIFY( numA == (intL + intR) );

     // Verify subtraction operators
     error_count += VERIFY( (intL-intR) == (numL-numR) );
     error_count += VERIFY( (intL-intR) == (intL-numR) );
     error_count += VERIFY( (intL-intR) == (numL-intR) );
     error_count += VERIFY( (intR-intL) == (intR-numL) );
     error_count += VERIFY( (intR-intL) == (numR-intL) );
     error_count += VERIFY( (intL-intS) == (numL-intS) );

     numB= numL - numR;
     numA= numB;
     error_count += VERIFY( numA == (intL - intR) );

     // Verify multiplication operators
     error_count += VERIFY( (intL*intR) == (numL*numR) );
     error_count += VERIFY( (intL*intR) == (intL*numR) );
     error_count += VERIFY( (intL*intR) == (numL*intR) );
     error_count += VERIFY( (intR*intL) == (intR*numL) );
     error_count += VERIFY( (intR*intL) == (numR*intL) );
     error_count += VERIFY( (intL*intS) == (numL*intS) );

     numB= numL * numR;
     numA= numB;
     error_count += VERIFY( numA == (intL * intR) );

     // Verify division operators
     error_count += VERIFY( (intL/intD) == (numL/numD) );
     error_count += VERIFY( (intL/intD) == (intL/numD) );
     error_count += VERIFY( (intL/intD) == (numL/intD) );
     error_count += VERIFY( (intL/intM) == (numL/intM) );

     numB= numL / numD;
     numA= numB;
     error_count += VERIFY( numA == (intL / intD) );

     // Verify modulus operators
     error_count += VERIFY( (intL%intD) == (numL%numD) );
     error_count += VERIFY( (intL%intD) == (intL%numD) );
     error_count += VERIFY( (intL%intD) == (numL%intD) );
     error_count += VERIFY( (intL%intM) == (numL%intM) );

     numB= numL % numD;
     numA= numB;
     error_count += VERIFY( numA == (intL%intD) );

//   // Verify Varbinary.div method
//   varA= intL;
//   if( (intL%intM) != varA.div(intM) )
//     error_count += VERIFY("(intL%intM) == varA.div(intM)");
//   error_count += VERIFY( (intL/intM) == varA.toInt() );

     if( error_count ) {
       debugf("intS(%d) intM(%d)\n", intS, intM);
       debugf("intL: %#.16" PRIx64 ", %+24" PRId64 "\n", intL, intL);
       debugf("numL: %s\n", numL.out("%#.16x, %+24d").c_str());
       debugf("intR: %#.16" PRIx64 ", %+24" PRId64 "\n", intR, intR);
       debugf("numR: %s\n", numR.out("%#.16x, %+24d").c_str());
       debugf("intD: %#.16" PRIx64 ", %+24" PRId64 "\n", intD, intD);
       debugf("numD: %s\n", numD.out("%#.16x, %+24d").c_str());

       intmax_t intA= intL << (intS%64);
       numA= numL << (intS%64);
       debugf("L<<S: %s int(0x%.16" PRIx64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("0x%.16x").c_str());

       intA= intR >> (intS%64);
       numA= numR >> (intS%64);
       debugf("R>>S: %s int(0x%.16" PRIx64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("0x%.16x").c_str());

       intA= intL + intR;
       numA= numL + numR;
       debugf(" L+R: %s int(0x%.16" PRIx64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("0x%.16x").c_str());

       intA= intL - intR;
       numA= numL - numR;
       debugf(" L-R: %s int(0x%.16" PRIx64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("0x%.16x").c_str());

       intA= intL * intR;
       numA= numL * numR;
       debugf(" L*R: %s int(0x%.16" PRIx64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("0x%.16x").c_str());

       intA= intL / intD;
       numA= numL / numD;
       debugf(" L/D: %s int(0x%.16" PRIx64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("0x%.16x").c_str());

       intA= intL % intD;
       numA= numL % numD;
       debugf(" L%%D: %s int(0x%.16" PRIx64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("0x%.16x").c_str());

       intA= intL + intR;
       numA= numL + numR;
       debugf(" L+R: %s int(%24" PRId64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("%24d").c_str());

       intA= intL - intR;
       numA= numL - numR;
       debugf(" L-R: %s int(%24" PRId64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("%24d").c_str());

       intA= intL * intR;
       numA= numL * numR;
       debugf(" L*R: %s int(%24" PRId64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("%24d").c_str());

       intA= intL / intD;
       numA= numL / numD;
       debugf(" L/D: %s int(%24" PRId64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("%24d").c_str());

       intA= intL % intD;
       numA= numL % numD;
       debugf(" L%%D: %s int(%24" PRId64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("%24d").c_str());

       intA= intL * intS;
       numA= numL * intS;
       debugf(" L*S: %s int(%24" PRId64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("%24d").c_str());

       intA= intL / intM;
       numA= numL / intM;
       debugf(" L/M: %s int(%24" PRId64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("%24d").c_str());

       intA= intL % intM;
       numA= numL % intM;
       debugf(" L%%M: %s int(%24" PRId64 ") num(%s)\n",
              intA == numA ? "OK" : "NG",
              intA, numA.out("%24d").c_str());

//     varA= intL;
//     intA= intL % intM;
//     numA= varA.div(intM);
//     debugf("div%%: %s int(%24" PRId64 ") num(%s)\n",
//            intA == numA ? "OK" : "NG",
//            intA, numA.out("%24d").c_str());
//
//     intA= intL / intM;
//     numA= varA;
//     debugf("div/: %s int(%24" PRId64 ") num(%s)\n",
//            intA == numA ? "OK" : "NG",
//            intA, numA.out("%24d").c_str());
       break;
     }
   }
   interval.stop();
   if( opt_verbose )
     debugf("%8.4f Seconds\n", interval.to_double());

   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Number8
//
// Purpose-
//       Test Number object vs intmax_t
//
//----------------------------------------------------------------------------
static int                          // Error count
   test_Number8( void )             // Test Number object
{
   if( opt_verbose )
     debugf("\ntest_Number8\n");

   int error_count= 0;
   if( false ) {
     debugf("%4d HCDM test skipped\n", __LINE__);
     return error_count;
   }

   intmax_t i1;
   intmax_t i2;
   intmax_t tI;

   Number n1;
   Number n2;
   Number tN;

   Interval interval;
   interval.start();
   for(size_t iteration= 0; iteration<ITERATIONS; ++iteration) {
     n1= i1= RNG.get() + RNG.get() + RNG.get();
     n2= i2= RNG.get() + RNG.get() + RNG.get();
     if( i1 == 0 )
       n1= i1= 1;
     if( i2 == 0 )
       n2= i2= 1;

     error_count += VERIFY( ~n1 == ~i1 );
     error_count += VERIFY( +n1 == +i1 );
     error_count += VERIFY( -n1 == -i1 );

     error_count += VERIFY( n1 + n2 == i1 + i2 );
     error_count += VERIFY( n1 - n2 == i1 - i2 );
     error_count += VERIFY( n1 * n2 == i1 * i2 );
     error_count += VERIFY( n1 / n2 == i1 / i2 );
     tN= n1 % n2; tI= i1 % i2;
     error_count += VERIFY( tN      == tI );

     tN= n1;
     error_count += VERIFY( ~n1 == ~i1 );
     error_count += VERIFY( +n1 == +i1 );
     error_count += VERIFY( -n1 == -i1 );

     tN= n1;
     error_count += VERIFY( tN++ == n1 );
     error_count += VERIFY( tN   == n1 + 1 );
     error_count += VERIFY( ++tN == n1 + 2 );
     error_count += VERIFY( tN   == n1 + 2 );

     tN= n1;
     error_count += VERIFY( tN-- == n1 );
     error_count += VERIFY( tN   == n1 - 1 );
     error_count += VERIFY( --tN == n1 - 2 );
     error_count += VERIFY( tN   == n1 - 2 );

     error_count += VERIFY( (n1 & n2) == (i1 & i2) );
     error_count += VERIFY( (n1 | n2) == (i1 | i2) );
     error_count += VERIFY( (n1 ^ n2) == (i1 ^ i2) );

     error_count += VERIFY( n1 + i2 == i1 + i2 );
     error_count += VERIFY( n1 - i2 == i1 - i2 );
     error_count += VERIFY( n1 * i2 == i1 * i2 );
     error_count += VERIFY( n1 / i2 == i1 / i2 );
     tN= n1 % i2;
     error_count += VERIFY( tN      == tI);

     error_count += VERIFY( (n1 & i2) == (i1 & i2) );
     error_count += VERIFY( (n1 | i2) == (i1 | i2) );
     error_count += VERIFY( (n1 ^ i2) == (i1 ^ i2) );

     error_count += VERIFY( i1 + n2 == i1 + i2 );
     error_count += VERIFY( i1 - n2 == i1 - i2 );
     error_count += VERIFY( i1 * n2 == i1 * i2 );
     error_count += VERIFY( i1 / n2 == i1 / i2 );
     tN= i1 % n2;
     error_count += VERIFY( tN      == tI );

     error_count += VERIFY( (i1 & n2) == (i1 & i2) );
     error_count += VERIFY( (i1 | n2) == (i1 | i2) );
     error_count += VERIFY( (i1 ^ n2) == (i1 ^ i2) );

     if( error_count > 0 ) {
       debugf("Error: Iteration %zd\n", iteration);
       debugf("i1: 0x%.16lx, #%ld\n", i1, i1);
       debugf("i2: 0x%.16lx, #%ld\n", i2, i2);
       break;
     }
   }
   interval.stop();
   if( opt_verbose )
     debugf("%8.4f Seconds\n", interval.to_double());
   return error_count;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test_Number8_out
//
// Purpose-
//       Number object output test
//
//----------------------------------------------------------------------------
static int                          // Number of errors encountered
   test_Number8_out( void )         // Test Number::out() (size == 8)
{
   if( opt_verbose )
     debugf("\ntest_Number8_out\n");

   int error_count= 0;              // Error counter

   if( false ) {
     debugf("%4d HCDM test skipped\n", __LINE__);
     return error_count;
   }

   Interval            interval;
   char                numString[64]; // Number string
   char                intString[64]; // Integer string

   intmax_t            sINT= 0x8796a5b4c3d2e1f0LL;
   intmax_t            uINT= 0x0f1e2d3c4b5a6978LL;

   Number              sNUM= sINT;
   Number              uNUM= uINT;

   // Simple HEX format
   error_count += VERIFY( sNUM == 0x8796a5b4c3d2e1f0LL );
   error_count += VERIFY( uNUM == 0x0f1e2d3c4b5a6978LL );

   strcpy(numString, sNUM.out("%x").c_str());
   sprintf(intString, "%" PRIx64, sINT);
   if( VERIFY( strcmp(numString,intString) == 0) ) {
     error_count++;
     debugf("numString(%s)\n", numString);
     debugf("intString(%s)\n", intString);
   }

   if( SCDM ) {
     cout << "sNUM: (0x8796a5b4c3d2e1f0) (" << sNUM.out("%x") << ")" << endl;
     cout << "uNUM: (0x0f1e2d3c4b5a6978) (" << uNUM.out("%x") << ")" << endl;
     debugf("(%" PRIx64 ",%" PRId64 "), (%s)\n",
            uINT, uINT, uNUM.out("%x,%u").c_str());
   }

   sNUM= sINT= -12345678901234LL;
   uNUM= uINT= 987654321012345LL;
   error_count += VERIFY( sNUM == -12345678901234LL );
   error_count += VERIFY( uNUM == 987654321012345LL );

   strcpy(numString, sNUM.out("%d").c_str());
   sprintf(intString, "%" PRId64, sINT);
   if( VERIFY( strcmp(numString,intString) == 0) ) {
     error_count++;
     debugf("numString(%s)\n", numString);
     debugf("intString(%s)\n", intString);
   }

   if( SCDM ) {
     cout << "sNUM: (-12345678901234) (" << sNUM.out("%d") << ")" <<  endl;
     cout << "uNUM: (987654321012345) (" << uNUM.out("%d") << ")" <<  endl;
     debugf("(%" PRIx64 ",%" PRId64 "), (%s)\n",
            sINT, sINT, sNUM.out("%x,%d").c_str());
   }

   interval.start();
   for(sINT= (-1000); sINT<=1000; sINT++) {
     sNUM= sINT;
     error_count += test_format("Hello %d world!", sNUM, "Hello %" PRId64 " world!", sINT);
     error_count += test_format("%d", sNUM, "%" PRId64, sINT);
     error_count += test_format("%i", sNUM, "%" PRIi64, sINT);
     error_count += test_format("%u", sNUM, "%" PRIu64, sINT);
     error_count += test_format("%o", sNUM, "%" PRIo64, sINT);
     error_count += test_format("%x", sNUM, "%" PRIx64, sINT);
     error_count += test_format("%X", sNUM, "%" PRIX64, sINT);

     error_count += test_format("%24d", sNUM, "%24" PRId64, sINT);
     error_count += test_format("%24i", sNUM, "%24" PRIi64, sINT);
     error_count += test_format("%24u", sNUM, "%24" PRIu64, sINT);
     error_count += test_format("%24o", sNUM, "%24" PRIo64, sINT);
     error_count += test_format("%24x", sNUM, "%24" PRIx64, sINT);
     error_count += test_format("%24X", sNUM, "%24" PRIX64, sINT);

     error_count += test_format("%.24d", sNUM, "%.24" PRId64, sINT);
     error_count += test_format("%.24i", sNUM, "%.24" PRIi64, sINT);
     error_count += test_format("%.24u", sNUM, "%.24" PRIu64, sINT);
     error_count += test_format("%.24o", sNUM, "%.24" PRIo64, sINT);
     error_count += test_format("%.24x", sNUM, "%.24" PRIx64, sINT);
     error_count += test_format("%.24X", sNUM, "%.24" PRIX64, sINT);

     error_count += test_format("%.d", sNUM, "%." PRId64, sINT);
     error_count += test_format("%.i", sNUM, "%." PRIi64, sINT);
     error_count += test_format("%.u", sNUM, "%." PRIu64, sINT);
     error_count += test_format("%.o", sNUM, "%." PRIo64, sINT);
     error_count += test_format("%.x", sNUM, "%." PRIx64, sINT);
     error_count += test_format("%.X", sNUM, "%." PRIX64, sINT);

     error_count += test_format("%.0d", sNUM, "%.0" PRId64, sINT);
     error_count += test_format("%.0i", sNUM, "%.0" PRIi64, sINT);
     error_count += test_format("%.0u", sNUM, "%.0" PRIu64, sINT);
     error_count += test_format("%.0o", sNUM, "%.0" PRIo64, sINT);
     error_count += test_format("%.0x", sNUM, "%.0" PRIx64, sINT);
     error_count += test_format("%.0X", sNUM, "%.0" PRIX64, sINT);

     error_count += test_format("%#+.0d", sNUM, "%#+.0" PRId64, sINT);
     error_count += test_format("%#+.0i", sNUM, "%#+.0" PRIi64, sINT);
     error_count += test_format("%#+.0u", sNUM, "%#+.0" PRIu64, sINT);
     error_count += test_format("%#+.0o", sNUM, "%#+.0" PRIo64, sINT);
     error_count += test_format("%#+.0x", sNUM, "%#+.0" PRIx64, sINT);
     error_count += test_format("%#+.0X", sNUM, "%#+.0" PRIX64, sINT);

     error_count += test_format("%28.24d", sNUM, "%28.24" PRId64, sINT);
     error_count += test_format("%28.24i", sNUM, "%28.24" PRIi64, sINT);
     error_count += test_format("%28.24u", sNUM, "%28.24" PRIu64, sINT);
     error_count += test_format("%28.24o", sNUM, "%28.24" PRIo64, sINT);
     error_count += test_format("%28.24x", sNUM, "%28.24" PRIx64, sINT);
     error_count += test_format("%28.24X", sNUM, "%28.24" PRIX64, sINT);

     error_count += test_format("%-24d", sNUM, "%-24" PRId64, sINT);
     error_count += test_format("%-24i", sNUM, "%-24" PRIi64, sINT);
     error_count += test_format("%-24u", sNUM, "%-24" PRIu64, sINT);
     error_count += test_format("%-24o", sNUM, "%-24" PRIo64, sINT);
     error_count += test_format("%-24x", sNUM, "%-24" PRIx64, sINT);
     error_count += test_format("%-24X", sNUM, "%-24" PRIX64, sINT);

     error_count += test_format("% -24d", sNUM, "% -24" PRId64, sINT);
     error_count += test_format("% -24i", sNUM, "% -24" PRIi64, sINT);
     error_count += test_format("% -24u", sNUM, "% -24" PRIu64, sINT);
     error_count += test_format("% -24o", sNUM, "% -24" PRIo64, sINT);
     error_count += test_format("% -24x", sNUM, "% -24" PRIx64, sINT);
     error_count += test_format("% -24X", sNUM, "% -24" PRIX64, sINT);

     error_count += test_format("%+-24d", sNUM, "%+-24" PRId64, sINT);
     error_count += test_format("%+-24i", sNUM, "%+-24" PRIi64, sINT);
     error_count += test_format("%+-24u", sNUM, "%+-24" PRIu64, sINT);
     error_count += test_format("%+-24o", sNUM, "%+-24" PRIo64, sINT);
     error_count += test_format("%+-24x", sNUM, "%+-24" PRIx64, sINT);
     error_count += test_format("%+-24X", sNUM, "%+-24" PRIX64, sINT);

     error_count += test_format("%+024d", sNUM, "%+024" PRId64, sINT);
     error_count += test_format("%+024i", sNUM, "%+024" PRIi64, sINT);
     error_count += test_format("%+024u", sNUM, "%+024" PRIu64, sINT);
     error_count += test_format("%+024o", sNUM, "%+024" PRIo64, sINT);
     error_count += test_format("%+024x", sNUM, "%+024" PRIx64, sINT);
     error_count += test_format("%+024X", sNUM, "%+024" PRIX64, sINT);

     error_count += test_format("%#24d", sNUM, "%#24" PRId64, sINT);
     error_count += test_format("%#24i", sNUM, "%#24" PRIi64, sINT);
     error_count += test_format("%#24u", sNUM, "%#24" PRIu64, sINT);
     error_count += test_format("%#24o", sNUM, "%#24" PRIo64, sINT);
     error_count += test_format("%#24x", sNUM, "%#24" PRIx64, sINT);
     error_count += test_format("%#24X", sNUM, "%#24" PRIX64, sINT);

     if( error_count )
       break;
   }
   interval.stop();
   if( opt_verbose )
     debugf("%8.4f Seconds\n", interval.to_double());
   return error_count;
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
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   Wrapper  tc;                     // The test case wrapper
   Wrapper* tr= &tc;                // A test case wrapper pointer

   tc.on_main([tr](int, char*[])
   {
     if( opt_verbose )
       debugf("%s: %s %s\n", __FILE__, __DATE__, __TIME__);

     int error_count= test_bringup();
     if( error_count == 0 ) {
       test_Number();
       test_Number8();
       test_Number8_out();
     }

     if( opt_verbose ) {
       debugf("\n");
       tr->report_errors(error_count);
     }
     return error_count != 0;
   });

   //-------------------------------------------------------------------------
   // Run the test
   return tc.run(argc, argv);
}

