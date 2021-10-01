//----------------------------------------------------------------------------
//
//       Copyright (c) 2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_Num.cpp
//
// Purpose-
//       Test the Number object.
//
// Last change date-
//       2021/10/01
//
//----------------------------------------------------------------------------
#include <inttypes.h>               // For PRId64, PRIx64 printf format macros
#include <iostream>                 // For cout
#include <stdio.h>                  // For sprintf
#include <string.h>                 // For strcpy

#include <com/Debug.h>              // For debugging classes and functions
#include <com/Interval.h>           // For timing classes and functions
#include <com/Random.h>             // For random number generation
#include <com/Verify.h>             // For test verification

#include "pub/Number.h"             // Implementation test class

using namespace pub;                // For Number
// using namespace pub::debugging;     // For debugging functions
using namespace std;

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
static void
   test_bringup( void )             // Bringup test
{
   debugf("\n");
   verify_info(); debugf("test_bringup\n");

   debugf("%4zd= sizeof(Number)\n", sizeof(Number));
   debugf("%4zd= sizeof(Number::Byte)\n", sizeof(Number::Byte));
   debugf("%4zd= sizeof(Number::Word)\n", sizeof(Number::Word));
   debugf("%4zd= Number::MIN_SIZE\n", Number::get_minsize());

   Number one= sONE;
   Number two= sTWO;

   verify( one == sONE );
   verify( two == sTWO );
   verify( two != one );

   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Woverflow"
   verify( (two + one) == (sTWO + sONE) );
   verify( (two - one) == (sTWO - sONE) );
   verify( (two * one) == (sTWO * sONE) );
   verify( (two / one) == (sTWO / sONE) );
// verify( (two % one) == (sTWO % sONE) ); // (Problem with % in verify macro)
   verify( (two & one) == (sTWO & sONE) );
   verify( (two | one) == (sTWO | sONE) );
   verify( (two ^ one) == (sTWO ^ sONE) );
   verify( (     ~one) == ~sONE );
   verify( (     +one) == +sONE );
   verify( (     -one) == -sONE );
   #pragma GCC diagnostic pop

   // Simple assignment
   one= -9876543210LL;
   two=  1234567890LL;
   verify( one == -9876543210 );
   verify( two ==  1234567890 );

   two= one;
   verify( two == -9876543210 );
   verify( one == two );

   // Hard Core Debug Mode: Shift operators
   if( false ) {
     intmax_t imax= 0xFEDCBA9876543210LL;
     one= imax;
//   two= imax;
     Number zero; zero.reset(nullptr);
     for(int i= 0; i<64; i++) {
       verify( (one << i) == (imax << i) );
       verify( (one >> i) == (imax >> i) );
//     verify( (two << i) == (((uintmax_t)imax) << i) );
//     verify( (two >> i) == (((uintmax_t)imax) >> i) );
       verify( (zero >> i) == 0 );
       verify( (zero << i) == 0 );
       if( error_count() ) {
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
static void
   test_format(                     // Test Number::out()
     const char*       numFormat,   // Number format string
     Number&           numObject,   // Number object
     const char*       intFormat,   // Integer format string
     intmax_t          intObject)   // Integer object
{
   char                numString[256]; // Number string
   char                intString[256]; // Integer string

   strcpy(numString, numObject.out(numFormat).c_str());
   sprintf(intString, intFormat, intObject);
   if( !verify(strcmp(numString,intString) == 0) ) {
     debugf("numString(%s) (%s)\n", numString, numFormat);
     debugf("intString(%s) (%s)\n", intString, intFormat);
   }
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
static void
   test_Number( void )              // Test Number object
{
   debugf("\n");
   verify_info(); debugf("test_Number\n");

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
     verify( numL == intL && !(numL != intL) );
     verify( intL == numL && !(intL != numL) );
     verify( numR == intR && !(numR != intR) );
     verify( intR == numR && !(intR != numR) );
     verify( intL == numB && (numB == intL ) );

//   VarBinary         varA(intL);
//   VarBinary         varL(varA);
//   VarBinary         varR(binR);
//
//   verify( varA == intL && varL == intL && varR == intR );
//   verify( varA.getData() != varL.getData() );
//   verify( varA.getData() != varR.getData() );
//   verify( varL.getData() != varR.getData() );
//   verify( varL.getData() != numR.getData() );

     // (Assumes Number endian is the same as hardware endian)
     verify( intL == *(intmax_t*)numB.get_data() );
     verify( intL == *(intmax_t*)numL.get_data() );
     verify( intR == *(intmax_t*)numR.get_data() );

     // Verify comparison operators
     if( intL < intR ) {
       verify( (numL < numR) && (numL <= numR) );
       verify( (intL < numR) && (intL <= numR) );
       verify( (numL < intR) && (numL <= intR) );
       verify( !(numL == numR) && !(numL >= numR) && !(numL > numR) );
       verify( !(intL == numR) && !(intL >= numR) && !(intL > numR) );
       verify( !(numL == intR) && !(numL >= intR) && !(numL > intR) );
     } else {
       verify( !(numL < numR) && (numL >= numR) );
       verify( !(intL < numR) && (intL >= numR) );
       verify( !(numL < intR) && (numL >= intR) );
       if( intL == intR ) {
         verify( (numL <= numR) && (numL == numR) && !(numL > numR) );
         verify( (intL <= numR) && (intL == numR) && !(intL > numR) );
         verify( (numL <= intR) && (numL == intR) && !(numL > intR) );
       } else {
         verify( !(numL <= numR) && !(numL == numR) && (numL > numR) );
         verify( !(intL <= numR) && !(intL == numR) && (intL > numR) );
         verify( !(numL <= intR) && !(numL == intR) && (numL > intR) );
       }
     }
     verify( (intL <  intS) == (numL <  intS) );
     verify( (intL <= intS) == (numL <= intS) );
     verify( (intL == intS) == (numL == intS) );
     verify( (intL >= intS) == (numL >= intS) );
     verify( (intL >  intS) == (numL >  intS) );

     verify( (intS <  intR) == (intS <  numR) );
     verify( (intS <= intR) == (intS <= numR) );
     verify( (intS == intR) == (intS == numR) );
     verify( (intS >= intR) == (intS >= numR) );
     verify( (intS >  intR) == (intS >  numR) );

     // Verify bitwise operators
     verify( (intL&intR) == (numL&numR) );
     verify( (intL&intR) == (intL&numR) );
     verify( (intL&intR) == (numL&intR) );
     verify( (intR&intL) == (intR&numL) );
     verify( (intR&intL) == (numR&intL) );
     verify( (intL&intS) == (numL&intS) );

     verify( (intL|intR) == (numL|numR) );
     verify( (intL|intR) == (intL|numR) );
     verify( (intL|intR) == (numL|intR) );
     verify( (intR|intL) == (intR|numL) );
     verify( (intR|intL) == (numR|intL) );
     verify( (intL|intS) == (numL|intS) );

     verify( (intL^intR) == (numL^numR) );
     verify( (intL^intR) == (intL^numR) );
     verify( (intL^intR) == (numL^intR) );
     verify( (intR^intL) == (intR^numL) );
     verify( (intR^intL) == (numR^intL) );
     verify( (intL^intS) == (numL^intS) );

     // Verify unary operators
     verify( (+intR) == (+numR) );
     verify( (-intR) == (-numR) );
     verify( (~intR) == (~numR) );
     verify( (!intR) == (!numR) );
     verify( +(+intR) == +(+numR) );
     verify( -(-intR) == -(-numR) );
     verify( ~(~intR) == ~(~numR) );
     verify( !(!intR) == !(!numR) );

     // Verify shift operators
     intmax_t verI= intL << (intS%64);
     Number   verN= numL << (intS%64);
     verify( verI == verN );
     verI= intR >> (intS%64);
     verN= numR >> (intS%64);
     verify( verI == verN );

     // Verify addition operators
     verify( (intL+intR) == (numL+numR) );
     verify( (intL+intR) == (intL+numR) );
     verify( (intL+intR) == (numL+intR) );
     verify( (intR+intL) == (intR+numL) );
     verify( (intR+intL) == (numR+intL) );
     verify( (intL+intS) == (numL+intS) );

     numB= numL + numR;
     numA= numB;
     verify( numA == (intL + intR) );

     // Verify subtraction operators
     verify( (intL-intR) == (numL-numR) );
     verify( (intL-intR) == (intL-numR) );
     verify( (intL-intR) == (numL-intR) );
     verify( (intR-intL) == (intR-numL) );
     verify( (intR-intL) == (numR-intL) );
     verify( (intL-intS) == (numL-intS) );

     numB= numL - numR;
     numA= numB;
     verify( numA == (intL - intR) );

     // Verify multiplication operators
     verify( (intL*intR) == (numL*numR) );
     verify( (intL*intR) == (intL*numR) );
     verify( (intL*intR) == (numL*intR) );
     verify( (intR*intL) == (intR*numL) );
     verify( (intR*intL) == (numR*intL) );
     verify( (intL*intS) == (numL*intS) );

     numB= numL * numR;
     numA= numB;
     verify( numA == (intL * intR) );

     // Verify division operators
     verify( (intL/intD) == (numL/numD) );
     verify( (intL/intD) == (intL/numD) );
     verify( (intL/intD) == (numL/intD) );
     verify( (intL/intM) == (numL/intM) );

     numB= numL / numD;
     numA= numB;
     verify( numA == (intL / intD) );

     // Verify modulus operators
     if( (intL%intD) != (numL%numD) ) verify("(intL%intD) == (numL%numD)");
     if( (intL%intD) != (intL%numD) ) verify("(intL%intD) == (intL%numD)");
     if( (intL%intD) != (numL%intD) ) verify("(intL%intD) == (numL%intD)");
     if( (intL%intM) != (numL%intM) ) verify("(intL%intM) == (numL%intM)");

     numB= numL % numD;
     numA= numB;
     if( numA != (intL%intD) ) verify("numA == (intL%intD)");

//   // Verify Varnumary.div method
//   varA= intL;
//   if( (intL%intM) != varA.div(intM) )
//     verify("(intL%intM) == varA.div(intM)");
//   verify( (intL/intM) == varA.toInt() );

     if( error_count() > 0 ) {
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
   verify_info(); debugf("%8.4f Seconds\n", interval.toDouble());
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
static void
   test_Number8( void )             // Test Number object
{
   debugf("\n");
   verify_info(); debugf("test_Number8\n");

   if( false ) {
     debugf("%4d HCDM test skipped\n", __LINE__);
     return;
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

     verify( ~n1 == ~i1 );
     verify( +n1 == +i1 );
     verify( -n1 == -i1 );

     verify( n1 + n2 == i1 + i2 );
     verify( n1 - n2 == i1 - i2 );
     verify( n1 * n2 == i1 * i2 );
     verify( n1 / n2 == i1 / i2 );
     tN= n1 % n2; tI= i1 % i2;
     verify( tN      == tI );

     tN= n1;
     verify( ~n1 == ~i1 );
     verify( +n1 == +i1 );
     verify( -n1 == -i1 );

     tN= n1;
     verify( tN++ == n1 );
     verify( tN   == n1 + 1 );
     verify( ++tN == n1 + 2 );
     verify( tN   == n1 + 2 );

     tN= n1;
     verify( tN-- == n1 );
     verify( tN   == n1 - 1 );
     verify( --tN == n1 - 2 );
     verify( tN   == n1 - 2 );

     verify( (n1 & n2) == (i1 & i2) );
     verify( (n1 | n2) == (i1 | i2) );
     verify( (n1 ^ n2) == (i1 ^ i2) );

     verify( n1 + i2 == i1 + i2 );
     verify( n1 - i2 == i1 - i2 );
     verify( n1 * i2 == i1 * i2 );
     verify( n1 / i2 == i1 / i2 );
     tN= n1 % i2;
     verify( tN      == tI);

     verify( (n1 & i2) == (i1 & i2) );
     verify( (n1 | i2) == (i1 | i2) );
     verify( (n1 ^ i2) == (i1 ^ i2) );

     verify( i1 + n2 == i1 + i2 );
     verify( i1 - n2 == i1 - i2 );
     verify( i1 * n2 == i1 * i2 );
     verify( i1 / n2 == i1 / i2 );
     tN= i1 % n2;
     verify( tN      == tI );

     verify( (i1 & n2) == (i1 & i2) );
     verify( (i1 | n2) == (i1 | i2) );
     verify( (i1 ^ n2) == (i1 ^ i2) );

     if( error_count() > 0 ) {
       debugf("Error: Iteration %zd\n", iteration);
       debugf("i1: 0x%.16lx, #%ld\n", i1, i1);
       debugf("i2: 0x%.16lx, #%ld\n", i2, i2);
       break;
     }
   }
   interval.stop();
   verify_info(); debugf("%8.4f Seconds\n", interval.toDouble());
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
static void
   test_Number8_out( void )         // Test Number::out() (size == 8)
{
   debugf("\n");
   verify_info(); debugf("test_Number8_out\n");

   if( false ) {
     debugf("%4d HCDM test skipped\n", __LINE__);
     return;
   }

   Interval            interval;
   char                numString[64]; // Number string
   char                intString[64]; // Integer string

   intmax_t            sINT= 0x8796a5b4c3d2e1f0LL;
   intmax_t            uINT= 0x0f1e2d3c4b5a6978LL;

   Number              sNUM= sINT;
   Number              uNUM= uINT;

   // Simple HEX format
   verify( sNUM == 0x8796a5b4c3d2e1f0LL );
   verify( uNUM == 0x0f1e2d3c4b5a6978LL );

   strcpy(numString, sNUM.out("%x").c_str());
   sprintf(intString, "%" PRIx64, sINT);
   if( !verify(strcmp(numString,intString) == 0) ) {
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
   verify( sNUM == -12345678901234LL );
   verify( uNUM == 987654321012345LL );

   strcpy(numString, sNUM.out("%d").c_str());
   sprintf(intString, "%" PRId64, sINT);
   if( !verify(strcmp(numString,intString) == 0) ) {
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
     test_format("Hello %d world!", sNUM, "Hello %" PRId64 " world!", sINT);
     test_format("%d", sNUM, "%" PRId64, sINT);
     test_format("%i", sNUM, "%" PRIi64, sINT);
     test_format("%u", sNUM, "%" PRIu64, sINT);
     test_format("%o", sNUM, "%" PRIo64, sINT);
     test_format("%x", sNUM, "%" PRIx64, sINT);
     test_format("%X", sNUM, "%" PRIX64, sINT);

     test_format("%24d", sNUM, "%24" PRId64, sINT);
     test_format("%24i", sNUM, "%24" PRIi64, sINT);
     test_format("%24u", sNUM, "%24" PRIu64, sINT);
     test_format("%24o", sNUM, "%24" PRIo64, sINT);
     test_format("%24x", sNUM, "%24" PRIx64, sINT);
     test_format("%24X", sNUM, "%24" PRIX64, sINT);

     test_format("%.24d", sNUM, "%.24" PRId64, sINT);
     test_format("%.24i", sNUM, "%.24" PRIi64, sINT);
     test_format("%.24u", sNUM, "%.24" PRIu64, sINT);
     test_format("%.24o", sNUM, "%.24" PRIo64, sINT);
     test_format("%.24x", sNUM, "%.24" PRIx64, sINT);
     test_format("%.24X", sNUM, "%.24" PRIX64, sINT);

     test_format("%.d", sNUM, "%." PRId64, sINT);
     test_format("%.i", sNUM, "%." PRIi64, sINT);
     test_format("%.u", sNUM, "%." PRIu64, sINT);
     test_format("%.o", sNUM, "%." PRIo64, sINT);
     test_format("%.x", sNUM, "%." PRIx64, sINT);
     test_format("%.X", sNUM, "%." PRIX64, sINT);

     test_format("%.0d", sNUM, "%.0" PRId64, sINT);
     test_format("%.0i", sNUM, "%.0" PRIi64, sINT);
     test_format("%.0u", sNUM, "%.0" PRIu64, sINT);
     test_format("%.0o", sNUM, "%.0" PRIo64, sINT);
     test_format("%.0x", sNUM, "%.0" PRIx64, sINT);
     test_format("%.0X", sNUM, "%.0" PRIX64, sINT);

     test_format("%#+.0d", sNUM, "%#+.0" PRId64, sINT);
     test_format("%#+.0i", sNUM, "%#+.0" PRIi64, sINT);
     test_format("%#+.0u", sNUM, "%#+.0" PRIu64, sINT);
     test_format("%#+.0o", sNUM, "%#+.0" PRIo64, sINT);
     test_format("%#+.0x", sNUM, "%#+.0" PRIx64, sINT);
     test_format("%#+.0X", sNUM, "%#+.0" PRIX64, sINT);

     test_format("%28.24d", sNUM, "%28.24" PRId64, sINT);
     test_format("%28.24i", sNUM, "%28.24" PRIi64, sINT);
     test_format("%28.24u", sNUM, "%28.24" PRIu64, sINT);
     test_format("%28.24o", sNUM, "%28.24" PRIo64, sINT);
     test_format("%28.24x", sNUM, "%28.24" PRIx64, sINT);
     test_format("%28.24X", sNUM, "%28.24" PRIX64, sINT);

     test_format("%-24d", sNUM, "%-24" PRId64, sINT);
     test_format("%-24i", sNUM, "%-24" PRIi64, sINT);
     test_format("%-24u", sNUM, "%-24" PRIu64, sINT);
     test_format("%-24o", sNUM, "%-24" PRIo64, sINT);
     test_format("%-24x", sNUM, "%-24" PRIx64, sINT);
     test_format("%-24X", sNUM, "%-24" PRIX64, sINT);

     test_format("% -24d", sNUM, "% -24" PRId64, sINT);
     test_format("% -24i", sNUM, "% -24" PRIi64, sINT);
     test_format("% -24u", sNUM, "% -24" PRIu64, sINT);
     test_format("% -24o", sNUM, "% -24" PRIo64, sINT);
     test_format("% -24x", sNUM, "% -24" PRIx64, sINT);
     test_format("% -24X", sNUM, "% -24" PRIX64, sINT);

     test_format("%+-24d", sNUM, "%+-24" PRId64, sINT);
     test_format("%+-24i", sNUM, "%+-24" PRIi64, sINT);
     test_format("%+-24u", sNUM, "%+-24" PRIu64, sINT);
     test_format("%+-24o", sNUM, "%+-24" PRIo64, sINT);
     test_format("%+-24x", sNUM, "%+-24" PRIx64, sINT);
     test_format("%+-24X", sNUM, "%+-24" PRIX64, sINT);

     test_format("%+024d", sNUM, "%+024" PRId64, sINT);
     test_format("%+024i", sNUM, "%+024" PRIi64, sINT);
     test_format("%+024u", sNUM, "%+024" PRIu64, sINT);
     test_format("%+024o", sNUM, "%+024" PRIo64, sINT);
     test_format("%+024x", sNUM, "%+024" PRIx64, sINT);
     test_format("%+024X", sNUM, "%+024" PRIX64, sINT);

     test_format("%#24d", sNUM, "%#24" PRId64, sINT);
     test_format("%#24i", sNUM, "%#24" PRIi64, sINT);
     test_format("%#24u", sNUM, "%#24" PRIu64, sINT);
     test_format("%#24o", sNUM, "%#24" PRIo64, sINT);
     test_format("%#24x", sNUM, "%#24" PRIx64, sINT);
     test_format("%#24X", sNUM, "%#24" PRIX64, sINT);

     if( error_count() > 0 )
       break;
   }
   interval.stop();
   verify_info(); debugf("%8.4f Seconds\n", interval.toDouble());
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
{
   //-------------------------------------------------------------------------
   // Initialization
   //-------------------------------------------------------------------------
   if( HCDM ) {                    // If Hard Core Debug Mode
     debugSetIntensiveMode();
     verify_info();
     debugf("HCDM\n");
   }

   //-------------------------------------------------------------------------
   // TRY wrapper
   //-------------------------------------------------------------------------
   try {
     //-----------------------------------------------------------------------
     // Prerequisite tests
     //-----------------------------------------------------------------------
     if( 1 ) {
       test_bringup();
     }

     //-----------------------------------------------------------------------
     // Object tests
     //-----------------------------------------------------------------------
     if( error_count() == 0 ) {
       test_Number8();
       test_Number8_out();
       test_Number();
     }
   }

   //-------------------------------------------------------------------------
   // Error handler
   //-------------------------------------------------------------------------
   catch(const char* X) {
     error_found();
     verify_info(); debugf("EXCEPTION(const char*((%s))\n", X);
   } catch(std::exception& X) {
     VerifyEC::_verify_(false, __FILE__, __LINE__
                       , "std::exception(%s)", X.what());
   } catch(...) {
     verify("EXCEPTION(...)");
   }

//----------------------------------------------------------------------------
// Testing complete
//----------------------------------------------------------------------------
   verify_exit();
}

