//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Test_Bin.cpp
//
// Purpose-
//       Test the Binary object.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
// #include "../../../../src/cpp/lib/com/Binary.cpp"
#include <inttypes.h>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/Debug.h>
#include <com/Interval.h>
#include <com/Random.h>
#include <com/Verify.h>

#include "com/Binary.h"

using namespace std;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#define ITERATIONS 100000

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator

//----------------------------------------------------------------------------
//
// Subroutine-
//       testFormat
//
// Purpose-
//       Binary object format test
//
//----------------------------------------------------------------------------
static void
   testFormat(                      // Test Binary::out()
     const char*       binFormat,   // Binary format string
     Binary&           binObject,   // Binary object
     const char*       intFormat,   // Integer format string
     int64_t           intObject)   // Integer object
{
   char                binString[256]; // Binary string
   char                intString[256]; // Integer string

   strcpy(binString, binObject.out(binFormat).c_str());
   sprintf(intString, intFormat, intObject);
   if( !verify(strcmp(binString,intString) == 0) )
   {
     debugf("(%s) binString(%s)\n", binFormat, binString);
     debugf("(%s) intString(%s)\n", intFormat, intString);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testBringup
//
// Purpose-
//       Binary object bringup test
//
//----------------------------------------------------------------------------
static void
   testBringup( void )              // Bringup test
{
   debugf("\n");
   verify_info(); debugf("testBringup\n");

   int64_t             siU= 0x8796a5b4c3d2e1f0LL;
   int64_t             uiU= 0x0f1e2d3c4b5a6978LL;

   SignedBinary<8>     sbU= siU;
   UnsignedBinary<8>   ubU= uiU;

   verify( sbU == 0x8796a5b4c3d2e1f0LL );
   verify( ubU == 0x0f1e2d3c4b5a6978LL );

   sbU= -9876543210LL;
   ubU=  1234567890LL;

   verify( sbU == -9876543210LL );
   verify( ubU ==  1234567890LL );

   if( error_count() != 0 )
   {
     VerifyEC::exit("Bringup failure");
     exit(1);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testOutput
//
// Purpose-
//       Binary object output test
//
//----------------------------------------------------------------------------
static void
   testOutput( void )               // Test Binary::out()
{
   debugf("\n");
   verify_info(); debugf("testOutput\n");

   Interval            interval;
   char                binString[64]; // Binary string
   char                intString[64]; // Integer string

   int64_t             sINT= 0x8796a5b4c3d2e1f0LL;
   int64_t             uINT= 0x0f1e2d3c4b5a6978LL;

   SignedBinary<8>     sBIN= sINT;
   UnsignedBinary<8>   uBIN= uINT;

   // Simple HEX format
   verify( sBIN == 0x8796a5b4c3d2e1f0LL );
   verify( uBIN == 0x0f1e2d3c4b5a6978LL );

   strcpy(binString, sBIN.out("%x").c_str());
   sprintf(intString, "%" PRIx64, sINT);
   if( !verify(strcmp(binString,intString) == 0) )
   {
     debugf("binString(%s)\n", binString);
     debugf("intString(%s)\n", intString);
   }

   #ifdef SCDM
     cout << "sBIN: (0x8796a5b4c3d2e1f0) (" << sBIN.out("%x") << ")" << endl;
     cout << "uBIN: (0x0f1e2d3c4b5a6978) (" << uBIN.out("%x") << ")" << endl;
     debugf("(%" PRIx64 ",%" PRId64 "), (%s)\n",
            uINT, uINT, uBIN.out("%x,%u").c_str());
   #endif

   sBIN= sINT= -12345678901234LL;
   uBIN= uINT= 987654321012345LL;
   verify( sBIN == -12345678901234LL );
   verify( uBIN == 987654321012345LL );

   strcpy(binString, sBIN.out("%d").c_str());
   sprintf(intString, "%" PRId64, sINT);
   if( !verify(strcmp(binString,intString) == 0) )
   {
     debugf("binString(%s)\n", binString);
     debugf("intString(%s)\n", intString);
   }

   #ifdef SCDM
     cout << "sBIN: (-12345678901234) (" << sBIN.out("%d") << ")" <<  endl;
     cout << "uBIN: (987654321012345) (" << uBIN.out("%d") << ")" <<  endl;
     debugf("(%" PRIx64 ",%" PRId64 "), (%s)\n",
            sINT, sINT, sBIN.out("%x,%d").c_str());
   #endif

   interval.start();
   for(sINT= (-1000); sINT<=1000; sINT++)
   {
     sBIN= sINT;
     testFormat("Hello %d world!", sBIN, "Hello %" PRId64 " world!", sINT);
     testFormat("%d", sBIN, "%" PRId64, sINT);
     testFormat("%i", sBIN, "%" PRIi64, sINT);
     testFormat("%u", sBIN, "%" PRIu64, sINT);
     testFormat("%o", sBIN, "%" PRIo64, sINT);
     testFormat("%x", sBIN, "%" PRIx64, sINT);
     testFormat("%X", sBIN, "%" PRIX64, sINT);

     testFormat("%24d", sBIN, "%24" PRId64, sINT);
     testFormat("%24i", sBIN, "%24" PRIi64, sINT);
     testFormat("%24u", sBIN, "%24" PRIu64, sINT);
     testFormat("%24o", sBIN, "%24" PRIo64, sINT);
     testFormat("%24x", sBIN, "%24" PRIx64, sINT);
     testFormat("%24X", sBIN, "%24" PRIX64, sINT);

     testFormat("%.24d", sBIN, "%.24" PRId64, sINT);
     testFormat("%.24i", sBIN, "%.24" PRIi64, sINT);
     testFormat("%.24u", sBIN, "%.24" PRIu64, sINT);
     testFormat("%.24o", sBIN, "%.24" PRIo64, sINT);
     testFormat("%.24x", sBIN, "%.24" PRIx64, sINT);
     testFormat("%.24X", sBIN, "%.24" PRIX64, sINT);

     testFormat("%.d", sBIN, "%." PRId64, sINT);
     testFormat("%.i", sBIN, "%." PRIi64, sINT);
     testFormat("%.u", sBIN, "%." PRIu64, sINT);
     testFormat("%.o", sBIN, "%." PRIo64, sINT);
     testFormat("%.x", sBIN, "%." PRIx64, sINT);
     testFormat("%.X", sBIN, "%." PRIX64, sINT);

     testFormat("%.0d", sBIN, "%.0" PRId64, sINT);
     testFormat("%.0i", sBIN, "%.0" PRIi64, sINT);
     testFormat("%.0u", sBIN, "%.0" PRIu64, sINT);
     testFormat("%.0o", sBIN, "%.0" PRIo64, sINT);
     testFormat("%.0x", sBIN, "%.0" PRIx64, sINT);
     testFormat("%.0X", sBIN, "%.0" PRIX64, sINT);

     testFormat("%#+.0d", sBIN, "%#+.0" PRId64, sINT);
     testFormat("%#+.0i", sBIN, "%#+.0" PRIi64, sINT);
     testFormat("%#+.0u", sBIN, "%#+.0" PRIu64, sINT);
     testFormat("%#+.0o", sBIN, "%#+.0" PRIo64, sINT);
     testFormat("%#+.0x", sBIN, "%#+.0" PRIx64, sINT);
     testFormat("%#+.0X", sBIN, "%#+.0" PRIX64, sINT);

     testFormat("%28.24d", sBIN, "%28.24" PRId64, sINT);
     testFormat("%28.24i", sBIN, "%28.24" PRIi64, sINT);
     testFormat("%28.24u", sBIN, "%28.24" PRIu64, sINT);
     testFormat("%28.24o", sBIN, "%28.24" PRIo64, sINT);
     testFormat("%28.24x", sBIN, "%28.24" PRIx64, sINT);
     testFormat("%28.24X", sBIN, "%28.24" PRIX64, sINT);

     testFormat("%-24d", sBIN, "%-24" PRId64, sINT);
     testFormat("%-24i", sBIN, "%-24" PRIi64, sINT);
     testFormat("%-24u", sBIN, "%-24" PRIu64, sINT);
     testFormat("%-24o", sBIN, "%-24" PRIo64, sINT);
     testFormat("%-24x", sBIN, "%-24" PRIx64, sINT);
     testFormat("%-24X", sBIN, "%-24" PRIX64, sINT);

     testFormat("% -24d", sBIN, "% -24" PRId64, sINT);
     testFormat("% -24i", sBIN, "% -24" PRIi64, sINT);
     testFormat("% -24u", sBIN, "% -24" PRIu64, sINT);
     testFormat("% -24o", sBIN, "% -24" PRIo64, sINT);
     testFormat("% -24x", sBIN, "% -24" PRIx64, sINT);
     testFormat("% -24X", sBIN, "% -24" PRIX64, sINT);

     testFormat("%+-24d", sBIN, "%+-24" PRId64, sINT);
     testFormat("%+-24i", sBIN, "%+-24" PRIi64, sINT);
     testFormat("%+-24u", sBIN, "%+-24" PRIu64, sINT);
     testFormat("%+-24o", sBIN, "%+-24" PRIo64, sINT);
     testFormat("%+-24x", sBIN, "%+-24" PRIx64, sINT);
     testFormat("%+-24X", sBIN, "%+-24" PRIX64, sINT);

     testFormat("%+024d", sBIN, "%+024" PRId64, sINT);
     testFormat("%+024i", sBIN, "%+024" PRIi64, sINT);
     testFormat("%+024u", sBIN, "%+024" PRIu64, sINT);
     testFormat("%+024o", sBIN, "%+024" PRIo64, sINT);
     testFormat("%+024x", sBIN, "%+024" PRIx64, sINT);
     testFormat("%+024X", sBIN, "%+024" PRIX64, sINT);

     testFormat("%#24d", sBIN, "%#24" PRId64, sINT);
     testFormat("%#24i", sBIN, "%#24" PRIi64, sINT);
     testFormat("%#24u", sBIN, "%#24" PRIu64, sINT);
     testFormat("%#24o", sBIN, "%#24" PRIo64, sINT);
     testFormat("%#24x", sBIN, "%#24" PRIx64, sINT);
     testFormat("%#24X", sBIN, "%#24" PRIX64, sINT);

     if( error_count() > 0 )
       break;
   }
   interval.stop();
   verify_info(); debugf("%8.4f Seconds\n", interval.toDouble());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testSigned
//
// Purpose-
//       Binary object signed value test
//
//----------------------------------------------------------------------------
static void
   testSigned( void )               // Test SignedBinary
{
   debugf("\n");
   verify_info(); debugf("testSigned\n");

   #ifdef INSTRUMENTATION
     verify( Binary::objectCount == 0 );
   #endif

   Interval            interval;
   int64_t             intA;
   int64_t             intL;
   int64_t             intR;
   int64_t             intD;
   int                 intS;
   int                 intM;

   int                 i;

   interval.start();
   for(i= 0; i<ITERATIONS; i++)
   {
     intL= RNG.get() + RNG.get() + RNG.get();
     intR= RNG.get() + RNG.get() + RNG.get();
     intD= intR & 0x7fffffff;
     if( intD == 0 )
       intD= 1;

     intS= RNG.get() % 160 - 80;
     intM= intS;
     if( intM == 0 )
       intM= 1;

     SignedBinary<8>   binA(intL);
     SignedBinary<12>  binB(binA);
     SignedBinary<8>   binL(binA);
     SignedBinary<8>   binR= intR;
     SignedBinary<8>   binD= intD;

     // Verify constructors
     verify( binL == intL && !(binL != intL) );
     verify( intL == binL && !(intL != binL) );
     verify( binR == intR && !(binR != intR) );
     verify( intR == binR && !(intR != binR) );
     verify( intL == binB && (binB == intL ) );

     VarBinary         varA(intL);
     VarBinary         varL(varA);
     VarBinary         varR(binR);

     verify( varA == intL && varL == intL && varR == intR );
     verify( varA.getData() != varL.getData() );
     verify( varA.getData() != varR.getData() );
     verify( varL.getData() != varR.getData() );
     verify( varL.getData() != binR.getData() );

     verify( intL == binB.toInt() );
     verify( intL == binL.toInt() );
     verify( intR == binR.toInt() );

     // Verify comparison operators
     if( intL < intR )
     {
       verify( (binL < binR) && (binL <= binR) );
       verify( (intL < binR) && (intL <= binR) );
       verify( (binL < intR) && (binL <= intR) );
       verify( !(binL == binR) && !(binL >= binR) && !(binL > binR) );
       verify( !(intL == binR) && !(intL >= binR) && !(intL > binR) );
       verify( !(binL == intR) && !(binL >= intR) && !(binL > intR) );
     }
     else
     {
       verify( !(binL < binR) && (binL >= binR) );
       verify( !(intL < binR) && (intL >= binR) );
       verify( !(binL < intR) && (binL >= intR) );
       if( intL == intR )
       {
         verify( (binL <= binR) && (binL == binR) && !(binL > binR) );
         verify( (intL <= binR) && (intL == binR) && !(intL > binR) );
         verify( (binL <= intR) && (binL == intR) && !(binL > intR) );
       }
       else
       {
         verify( !(binL <= binR) && !(binL == binR) && (binL > binR) );
         verify( !(intL <= binR) && !(intL == binR) && (intL > binR) );
         verify( !(binL <= intR) && !(binL == intR) && (binL > intR) );
       }
     }
     verify( (intL <  intS) == (binL <  intS) );
     verify( (intL <= intS) == (binL <= intS) );
     verify( (intL == intS) == (binL == intS) );
     verify( (intL >= intS) == (binL >= intS) );
     verify( (intL >  intS) == (binL >  intS) );

     verify( (intS <  intR) == (intS <  binR) );
     verify( (intS <= intR) == (intS <= binR) );
     verify( (intS == intR) == (intS == binR) );
     verify( (intS >= intR) == (intS >= binR) );
     verify( (intS >  intR) == (intS >  binR) );

     // Verify bitwise operators
     verify( (intL&intR) == (binL&binR) );
     verify( (intL&intR) == (intL&binR) );
     verify( (intL&intR) == (binL&intR) );
     verify( (intR&intL) == (intR&binL) );
     verify( (intR&intL) == (binR&intL) );
     verify( (intL&intS) == (binL&intS) );

     verify( (intL|intR) == (binL|binR) );
     verify( (intL|intR) == (intL|binR) );
     verify( (intL|intR) == (binL|intR) );
     verify( (intR|intL) == (intR|binL) );
     verify( (intR|intL) == (binR|intL) );
     verify( (intL|intS) == (binL|intS) );

     verify( (intL^intR) == (binL^binR) );
     verify( (intL^intR) == (intL^binR) );
     verify( (intL^intR) == (binL^intR) );
     verify( (intR^intL) == (intR^binL) );
     verify( (intR^intL) == (binR^intL) );
     verify( (intL^intS) == (binL^intS) );

     // Verify unary operators
     verify( (+intR) == (+binR) );
     verify( (-intR) == (-binR) );
     verify( (~intR) == (~binR) );
     verify( (!intR) == (!binR) );
     verify( +(+intR) == +(+binR) );
     verify( -(-intR) == -(-binR) );
     verify( ~(~intR) == ~(~binR) );
     verify( !(!intR) == !(!binR) );

     // Verify shift operators
     #ifdef _CC_GCC
       verify( (intL << intS) == (binL << intS) );
       verify( (intR >> intS) == (binR >> intS) );
     #else
       if( intS >= 0 && intS < 64 )
       {
         verify( (intL << intS) == (binL << intS) );
         verify( (intR >> intS) == (binR >> intS) );
       }
     #endif

     // Verify addition operators
     verify( (intL+intR) == (binL+binR) );
     verify( (intL+intR) == (intL+binR) );
     verify( (intL+intR) == (binL+intR) );
     verify( (intR+intL) == (intR+binL) );
     verify( (intR+intL) == (binR+intL) );
     verify( (intL+intS) == (binL+intS) );

     binB= binL + binR;
     binA= binB;
     verify( binA == (intL + intR) );

     // Verify subtraction operators
     verify( (intL-intR) == (binL-binR) );
     verify( (intL-intR) == (intL-binR) );
     verify( (intL-intR) == (binL-intR) );
     verify( (intR-intL) == (intR-binL) );
     verify( (intR-intL) == (binR-intL) );
     verify( (intL-intS) == (binL-intS) );

     binB= binL - binR;
     binA= binB;
     verify( binA == (intL - intR) );

     // Verify multiplication operators
     verify( (intL*intR) == (binL*binR) );
     verify( (intL*intR) == (intL*binR) );
     verify( (intL*intR) == (binL*intR) );
     verify( (intR*intL) == (intR*binL) );
     verify( (intR*intL) == (binR*intL) );
     verify( (intL*intS) == (binL*intS) );

     binB= binL * binR;
     binA= binB;
     verify( binA == (intL * intR) );

     // Verify division operators
     verify( (intL/intD) == (binL/binD) );
     verify( (intL/intD) == (intL/binD) );
     verify( (intL/intD) == (binL/intD) );
     verify( (intL/intM) == (binL/intM) );

     binB= binL / binD;
     binA= binB;
     verify( binA == (intL / intD) );

     // Verify modulus operators
     if( (intL%intD) != (binL%binD) ) verify("(intL%intD) == (binL%binD)");
     if( (intL%intD) != (intL%binD) ) verify("(intL%intD) == (intL%binD)");
     if( (intL%intD) != (binL%intD) ) verify("(intL%intD) == (binL%intD)");
     if( (intL%intM) != (binL%intM) ) verify("(intL%intM) == (binL%intM)");

     binB= binL % binD;
     binA= binB;
     if( binA != (intL%intD) ) verify("binA == (intL%intD)");

     // Verify VarBinary.div method
     varA= intL;
     if( (intL%intM) != varA.div(intM) )
       verify("(intL%intM) == varA.div(intM)");
     verify( (intL/intM) == varA.toInt() );

     if( error_count() > 0 )
     {
       debugf("intS(%d) intM(%d)\n", intS, intM);
       debugf("intL: %#.16" PRIx64 ", %+24" PRId64 "\n", intL, intL);
       debugf("binL: %s\n", binL.out("%#.16x, %+24d").c_str());
       debugf("intR: %#.16" PRIx64 ", %+24" PRId64 "\n", intR, intR);
       debugf("binR: %s\n", binR.out("%#.16x, %+24d").c_str());
       debugf("intD: %#.16" PRIx64 ", %+24" PRId64 "\n", intD, intD);
       debugf("binD: %s\n", binD.out("%#.16x, %+24d").c_str());

       intA= intL << intS;
       binA= binL << intS;
       debugf("L<<S: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intR >> intS;
       binA= binR >> intS;
       debugf("R>>S: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intL + intR;
       binA= binL + binR;
       debugf(" L+R: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intL - intR;
       binA= binL - binR;
       debugf(" L-R: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intL * intR;
       binA= binL * binR;
       debugf(" L*R: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intL / intD;
       binA= binL / binD;
       debugf(" L/D: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intL % intD;
       binA= binL % binD;
       debugf(" L%%D: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intL + intR;
       binA= binL + binR;
       debugf(" L+R: %s int(%24" PRId64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24d").c_str());

       intA= intL - intR;
       binA= binL - binR;
       debugf(" L-R: %s int(%24" PRId64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24d").c_str());

       intA= intL * intR;
       binA= binL * binR;
       debugf(" L*R: %s int(%24" PRId64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24d").c_str());

       intA= intL / intD;
       binA= binL / binD;
       debugf(" L/D: %s int(%24" PRId64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24d").c_str());

       intA= intL % intD;
       binA= binL % binD;
       debugf(" L%%D: %s int(%24" PRId64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24d").c_str());

       intA= intL * intS;
       binA= binL * intS;
       debugf(" L*S: %s int(%24" PRId64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24d").c_str());

       intA= intL / intM;
       binA= binL / intM;
       debugf(" L/M: %s int(%24" PRId64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24d").c_str());

       intA= intL % intM;
       binA= binL % intM;
       debugf(" L%%M: %s int(%24" PRId64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24d").c_str());

       varA= intL;
       intA= intL % intM;
       binA= varA.div(intM);
       debugf("div%%: %s int(%24" PRId64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24d").c_str());

       intA= intL / intM;
       binA= varA;
       debugf("div/: %s int(%24" PRId64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24d").c_str());
       break;
     }
   }
   interval.stop();
   verify_info(); debugf("%8.4f Seconds\n", interval.toDouble());

   #ifdef INSTRUMENTATION
     verify( Binary::objectCount == 0 );
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testUnsigned
//
// Purpose-
//       Binary object unsigned value test
//
//----------------------------------------------------------------------------
static void
   testUnsigned( void )             // Test UnsignedBinary
{
#ifdef _CC_GCC
   #pragma GCC diagnostic ignored "-Wsign-compare"
#endif

   debugf("\n");
   verify_info(); debugf("testUnsigned\n");

   #ifdef INSTRUMENTATION
     verify( Binary::objectCount == 0 );
   #endif

   Interval            interval;
   uint64_t            intA;
   uint64_t            intL;
   uint64_t            intR;
   uint64_t            intD;
   unsigned int        intS;
   unsigned int        intM;

   int                 i;

   interval.start();
   for(i= 0; i<ITERATIONS; i++)
   {
     intL= RNG.get() + RNG.get() + RNG.get();
     intR= RNG.get() + RNG.get() + RNG.get();
     intD= intR & 0x7fffffff;
     if( intD == 0 )
       intD= 1;

     intS= RNG.get() % 80;
     intM= intS;
     if( intM == 0 )
       intM= 1;

     UnsignedBinary<8>   binA(intL);
     UnsignedBinary<12>  binB(binA);
     UnsignedBinary<8>   binL(binA);
     UnsignedBinary<8>   binR= intR;
     UnsignedBinary<8>   binD= intD;

     // Verify constructors
     verify( binL == intL && !(binL != intL) );
     verify( intL == binL && !(intL != binL) );
     verify( binR == intR && !(binR != intR) );
     verify( intR == binR && !(intR != binR) );
     verify( binA == binB && (binB == binA ) );

     VarBinary         varA(intL);
     VarBinary         varL(varA);
     VarBinary         varR(binR);
     varA.setSigned(FALSE);
     varL.setSigned(FALSE);
     varR.setSigned(FALSE);

     verify( varA == intL && varL == intL && varR == intR );
     verify( varA.getData() != varL.getData() );
     verify( varA.getData() != varR.getData() );
     verify( varL.getData() != varR.getData() );
     verify( varL.getData() != binR.getData() );

     verify( intL == binB.toInt() );
     verify( intL == binL.toInt() );
     verify( intR == binR.toInt() );

     // Verify comparison operators
     if( intL < intR )
     {
       verify( (binL < binR) && (binL <= binR) );
       verify( (intL < binR) && (intL <= binR) );
       verify( (binL < intR) && (binL <= intR) );
       verify( !(binL == binR) && !(binL >= binR) && !(binL > binR) );
       verify( !(intL == binR) && !(intL >= binR) && !(intL > binR) );
       verify( !(binL == intR) && !(binL >= intR) && !(binL > intR) );
     }
     else
     {
       verify( !(binL < binR) && (binL >= binR) );
       verify( !(intL < binR) && (intL >= binR) );
       verify( !(binL < intR) && (binL >= intR) );
       if( intL == intR )
       {
         verify( (binL <= binR) && (binL == binR) && !(binL > binR) );
         verify( (intL <= binR) && (intL == binR) && !(intL > binR) );
         verify( (binL <= intR) && (binL == intR) && !(binL > intR) );
       }
       else
       {
         verify( !(binL <= binR) && !(binL == binR) && (binL > binR) );
         verify( !(intL <= binR) && !(intL == binR) && (intL > binR) );
         verify( !(binL <= intR) && !(binL == intR) && (binL > intR) );
       }
     }
     verify( (intL <  intS) == (binL <  intS) );
     verify( (intL <= intS) == (binL <= intS) );
     verify( (intL == intS) == (binL == intS) );
     verify( (intL >= intS) == (binL >= intS) );
     verify( (intL >  intS) == (binL >  intS) );

     verify( (intS <  intR) == (intS <  binR) );
     verify( (intS <= intR) == (intS <= binR) );
     verify( (intS == intR) == (intS == binR) );
     verify( (intS >= intR) == (intS >= binR) );
     verify( (intS >  intR) == (intS >  binR) );

     // Verify bitwise operators
     verify( (intL&intR) == (binL&binR) );
     verify( (intL&intR) == (intL&binR) );
     verify( (intL&intR) == (binL&intR) );
     verify( (intR&intL) == (intR&binL) );
     verify( (intR&intL) == (binR&intL) );
     verify( (intL&intS) == (binL&intS) );

     verify( (intL|intR) == (binL|binR) );
     verify( (intL|intR) == (intL|binR) );
     verify( (intL|intR) == (binL|intR) );
     verify( (intR|intL) == (intR|binL) );
     verify( (intR|intL) == (binR|intL) );
     verify( (intL|intS) == (binL|intS) );

     verify( (intL^intR) == (binL^binR) );
     verify( (intL^intR) == (intL^binR) );
     verify( (intL^intR) == (binL^intR) );
     verify( (intR^intL) == (intR^binL) );
     verify( (intR^intL) == (binR^intL) );
     verify( (intL^intS) == (binL^intS) );

     // Verify unary operators
     verify( (+intR) == (+binR) );
     verify( (-intR) == (-binR) );
     verify( (~intR) == (~binR) );
     verify( (!intR) == (!binR) );
     verify( +(+intR) == +(+binR) );
     verify( -(-intR) == -(-binR) );
     verify( ~(~intR) == ~(~binR) );
     verify( !(!intR) == !(!binR) );

     // Verify shift operators
     #ifdef _CC_GCC
       verify( (intL << intS) == (binL << intS) );
       verify( (intR >> intS) == (binR >> intS) );
     #else
       if( intS >= 0 && intS < 64 )
       {
         verify( (intL << intS) == (binL << intS) );
         verify( (intR >> intS) == (binR >> intS) );
       }
     #endif

     // Verify addition operators
     verify( (intL+intR) == (binL+binR) );
     verify( (intL+intR) == (intL+binR) );
     verify( (intL+intR) == (binL+intR) );
     verify( (intR+intL) == (intR+binL) );
     verify( (intR+intL) == (binR+intL) );
     verify( (intL+intS) == (binL+intS) );

     binB= binL + binR;
     binA= binB;
     verify( binA == (intL+intR) );

     // Verify subtraction operators
     verify( (intL-intR) == (binL-binR) );
     verify( (intL-intR) == (intL-binR) );
     verify( (intL-intR) == (binL-intR) );
     verify( (intR-intL) == (intR-binL) );
     verify( (intR-intL) == (binR-intL) );
     verify( (intL-intS) == (binL-intS) );

     binB= binL - binR;
     binA= binB;
     verify( binA == (intL-intR) );

     // Verify multiplication operators
     verify( (intL*intR) == (binL*binR) );
     verify( (intL*intR) == (intL*binR) );
     verify( (intL*intR) == (binL*intR) );
     verify( (intR*intL) == (intR*binL) );
     verify( (intR*intL) == (binR*intL) );
     verify( (intL*intS) == (binL*intS) );

     binB= binL * binR;
     binA= binB;
     verify( binA == (intL*intR) );

     // Verify division operators
     verify( (intL/intD) == (binL/binD) );
     verify( (intL/intD) == (intL/binD) );
     verify( (intL/intD) == (binL/intD) );
     verify( (intL/intM) == (binL/intM) );

     binB= binL / binD;
     binA= binB;
     verify( binA == (intL/intD) );

     // Verify modulus operators
     if( (intL%intD) != (binL%binD) ) verify("(intL%intD) == (binL%binD)");
     if( (intL%intD) != (intL%binD) ) verify("(intL%intD) == (intL%binD)");
     if( (intL%intD) != (binL%intD) ) verify("(intL%intD) == (binL%intD)");
     if( (intL%intM) != (binL%intM) ) verify("(intL%intM) == (binL%intM)");

     binB= binL % binD;
     binA= binB;
     if( binA != (intL%intD) ) verify("binA == (intL%intD)");

     // Verify VarBinary.div method
     varA= intL;
     if( (intL%intM) != varA.div(intM) )
       verify("(intL%intM) == varA.div(intM)");
     verify( (intL/intM) == varA.toInt() );

     // Verify signed attribute
     verify( binA.getSigned() == FALSE );
     verify( binL.getSigned() == FALSE );
     verify( binR.getSigned() == FALSE );
     verify( varA.getSigned() == FALSE );
     verify( varL.getSigned() == FALSE );
     verify( varR.getSigned() == FALSE );

     if( error_count() > 0 )
     {
       debugf("intS(%u) intM(%u)\n", intS, intM);
       debugf("intL: %#.16" PRIx64 ", %24" PRIu64 "\n", intL, intL);
       debugf("binL: %s\n", binL.out("%#.16x, %24u").c_str());
       debugf("intR: %#.16" PRIx64 ", %24" PRIu64 "\n", intR, intR);
       debugf("binR: %s\n", binR.out("%#.16x, %24u").c_str());
       debugf("intD: %#.16" PRIx64 ", %24" PRIu64 "\n", intD, intD);
       debugf("binD: %s\n", binD.out("%#.16x, %24u").c_str());

       intA= intL << intS;
       binA= binL << intS;
       debugf("L<<S: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intR >> intS;
       binA= binR >> intS;
       debugf("R>>S: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intL + intR;
       binA= binL + binR;
       debugf(" L+R: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intL - intR;
       binA= binL - binR;
       debugf(" L-R: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intL * intR;
       binA= binL * binR;
       debugf(" L*R: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intL / intD;
       binA= binL / binD;
       debugf(" L/D: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intL % intD;
       binA= binL % binD;
       debugf(" L%%D: %s int(0x%.16" PRIx64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("0x%.16x").c_str());

       intA= intL + intR;
       binA= binL + binR;
       debugf(" L+R: %s int(%24" PRIu64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24u").c_str());

       intA= intL - intR;
       binA= binL - binR;
       debugf(" L-R: %s int(%24" PRIu64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24u").c_str());

       intA= intL * intR;
       binA= binL * binR;
       debugf(" L*R: %s int(%24" PRIu64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24u").c_str());

       intA= intL / intD;
       binA= binL / binD;
       debugf(" L/D: %s int(%24" PRIu64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24u").c_str());

       intA= intL % intD;
       binA= binL % binD;
       debugf(" L%%D: %s int(%24" PRIu64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24u").c_str());

       intA= intL * intS;
       binA= binL * intS;
       debugf(" L*S: %s int(%24" PRIu64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24u").c_str());

       intA= intL / intM;
       binA= binL / intM;
       debugf(" L/M: %s int(%24" PRIu64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24u").c_str());

       intA= intL % intM;
       binA= binL % intM;
       debugf(" L%%M: %s int(%24" PRIu64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24u").c_str());

       varA= intL;
       intA= intL % intM;
       binA= varA.div(intM);
       debugf("div%%: %s int(%24" PRIu64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24u").c_str());

       intA= intL / intM;
       binA= varA;
       debugf("div/: %s int(%24" PRIu64 ") bin(%s)\n",
              intA == binA ? "OK" : "NG",
              intA, binA.out("%24u").c_str());
       break;
     }
   }
   interval.stop();
   verify_info(); debugf("%8.4f Seconds\n", interval.toDouble());

   #ifdef INSTRUMENTATION
     verify( Binary::objectCount == 0 );
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testMixed
//
// Purpose-
//       Binary object mixed signed and unsigned value test
//
//----------------------------------------------------------------------------
static void
   testMixed( void )                // Test mixed Signed and UnsignedBinary
{
   debugf("\n");
   verify_info(); debugf("testMixed\n");

   #ifdef INSTRUMENTATION
     verify( Binary::objectCount == 0 );
   #endif

   SignedBinary<8>     sbL;
   SignedBinary<8>     sbR;
   int64_t             siL;
   int64_t             siR;

   UnsignedBinary<8>   ubL;
   UnsignedBinary<8>   ubR;
   uint64_t            uiL;
   uint64_t            uiR;

   Interval            interval;

   int                 i;

   interval.start();
   for(i= 0; i<ITERATIONS; i++)
   {
     ubL= sbL= uiL= siL= RNG.get() + RNG.get() + RNG.get();
     ubR= sbR= uiR= siR= RNG.get() + RNG.get() + RNG.get();

     if( siL < uiR )
       verify( sbL < ubR );

     if( siL <= uiR )
       verify( sbL <= ubR );

     if( siL >= uiR )
       verify( sbL >= ubR );

     if( siL > uiR )
       verify( sbL > ubR );

     if( uiL < siR )
       verify( ubL < sbR );

     if( uiL <= siR )
       verify( ubL <= sbR );

     if( uiL >= siR )
       verify( ubL >= sbR );

     if( uiL > siR )
       verify( ubL > sbR );

     if( uiL == siL )
     {
       verify( ubL == sbL );
       verify( ubL == siL );
       verify( uiL == sbL );

       verify( sbL == ubL );
       verify( sbL == uiL );
       verify( siL == ubL );
     }

     verify( (siL+uiR) == (sbL+ubR) );
     verify( (siL+uiR) == (sbL+uiR) );
     verify( (siL+uiR) == (siL+ubR) );

     verify( (uiL+siR) == (ubL+sbR) );
     verify( (uiL+siR) == (ubL+siR) );
     verify( (uiL+siR) == (uiL+sbR) );

     verify( (siL-uiR) == (sbL-ubR) );
     verify( (siL-uiR) == (sbL-uiR) );
     verify( (siL-uiR) == (siL-ubR) );

     verify( (uiL-siR) == (ubL-sbR) );
     verify( (uiL-siR) == (ubL-siR) );
     verify( (uiL-siR) == (uiL-sbR) );

     if( error_count() > 0 )
       break;
   }
   interval.stop();
   verify_info(); debugf("%8.4f Seconds\n", interval.toDouble());

   #ifdef INSTRUMENTATION
     verify( Binary::objectCount == 4 );
   #endif
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
   #ifdef HCDM
     debugSetIntensiveMode();      // Hard Core Debug Mode
     verify_info(); debugf("HCDM\n");
   #endif

   //-------------------------------------------------------------------------
   // TRY wrapper
   //-------------------------------------------------------------------------
   try
   {
     //-----------------------------------------------------------------------
     // Prerequisite tests
     //-----------------------------------------------------------------------
     if( 1 )
     {
       testBringup();
       testOutput();
     }

     //-----------------------------------------------------------------------
     // Object tests
     //-----------------------------------------------------------------------
     if( 1 )
     {
       testSigned();
       testUnsigned();
       testMixed();
     }
   }

   //-------------------------------------------------------------------------
   // Error handler
   //-------------------------------------------------------------------------
   catch(const char* X)
   {
     error_found();
     verify_info(); debugf("EXCEPTION(const char*((%s))\n", X);
   }
   catch(...)
   {
     verify("EXCEPTION(...)");
   }

//----------------------------------------------------------------------------
// Testing complete
//----------------------------------------------------------------------------
   verify_exit();
}

