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
//       TestWild.cpp
//
// Purpose-
//       Test Wildchar object.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>

#include "com/Wildchar.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TESTWILD" // Source file, for debugging

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Function-
//       Initialize a Wildchar object.
//
//----------------------------------------------------------------------------
static int                          // Error count
   init(                            // Initialize
     Wildchar&         object)      // Wildchar object
{
   object.set('N', "ACTG");
   object.set('V', "ACG");
   object.set('H', "ACT");
   object.set('D', "AGT");
   object.set('B', "CGT");
   object.set('M', "AC");
   object.set('R', "AG");
   object.set('W', "AT");
   object.set('S', "CG");
   object.set('Y', "CT");
   object.set('K', "GT");

   return 0;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       tcSTRCMP
//
// Purpose-
//       Test Wildchar::compare(char*,char*)
//
//----------------------------------------------------------------------------
static int                          // Error counter
   tcSTRCMP(                        // Mainline code
     Wildchar&       object,        // Test object
     int             expect,        // Expected resultant
     const char*     source,        // Source string
     const char*     target)        // Target substring
{
   int               actual;        // Actual resultant

   actual= object.compare(source,target);
   if( expect == 0 && actual == 0 )
     return 0;

   if( expect < 0 && actual < 0 )
     return 0;

   if( expect > 0 && actual > 0 )
     return 0;

   printf("%4d: source(%s)\n", __LINE__, source);
   printf("%4d: target(%s)\n", __LINE__, target);
   printf("%4d: expect(%d)\n", __LINE__, expect);
   printf("%4d: actual(%d)\n", __LINE__, actual);
   return 1;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       tcSTRSTR
//
// Purpose-
//       Test Wildchar::strstr
//
//----------------------------------------------------------------------------
static int                          // Error counter
   tcSTRSTR(                        // Mainline code
     Wildchar&       object,        // Test object
     const char*     expect,        // Expected resultant
     const char*     string,        // Source string
     const char*     substr)        // Target substring
{
   char*             actual;        // Actual resultant

   actual= object.strstr(string,substr);
   if( expect == actual )
     return 0;

   printf("%4d: string(%s)\n", __LINE__, string);
   printf("%4d: substr(%s)\n", __LINE__, substr);
   printf("%4d: expect(%s)\n", __LINE__, expect);
   printf("%4d: actual(%s)\n", __LINE__, actual);
   return 1;
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
extern int                          // Return code
   main(int, char**)                // Mainline code
//   int             argc,          // Argument count
//   char*           argv[])        // Argument array
{
   unsigned          errorCount;    // Error counter
   Wildchar          object;        // The test object

   const char*       string;        // Search string
   const char*       substr;        // Search substring
   const char*       expect;        // Expected result

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   errorCount= init(object);

   //-------------------------------------------------------------------------
   // Test STRCMP
   //-------------------------------------------------------------------------
   errorCount += tcSTRCMP(object, +1, "DAA", "CCC");
   errorCount += tcSTRCMP(object,  0, "AKA", "ASA");
   errorCount += tcSTRCMP(object, -1, "CCC", "DAA");

   errorCount += tcSTRCMP(object, -1, "AAA", "CCC");
   errorCount += tcSTRCMP(object,  0, "AAA", "AAA");
   errorCount += tcSTRCMP(object, +1, "CCC", "AAA");

   errorCount += tcSTRCMP(object,  0, "AAA", "MRW");
   errorCount += tcSTRCMP(object,  0, "MRW", "AAA");
   errorCount += tcSTRCMP(object,  0, "MRW", "RWM");

   //-------------------------------------------------------------------------
   // Test STRSTR
   //-------------------------------------------------------------------------
   string= "AAACCCKKKGGG";
   expect= string + 6;

   substr= "TTT";
   errorCount += tcSTRSTR(object, expect, string, substr);
   substr= "GGG";
   errorCount += tcSTRSTR(object, expect, string, substr);
   substr= "GTG";
   errorCount += tcSTRSTR(object, expect, string, substr);
   substr= "TGT";
   errorCount += tcSTRSTR(object, expect, string, substr);
   substr= "TST";
   errorCount += tcSTRSTR(object, expect, string, substr);
   substr= "TYT";
   errorCount += tcSTRSTR(object, expect, string, substr);

   string= "AAACCCTTTGGG";
   expect= string + 6;

   substr= "TTT";
   errorCount += tcSTRSTR(object, expect, string, substr);
   substr= "TNT";
   errorCount += tcSTRSTR(object, expect, string, substr);
   substr= "THT";
   errorCount += tcSTRSTR(object, expect, string, substr);
   substr= "TDT";
   errorCount += tcSTRSTR(object, expect, string, substr);
   substr= "TBT";
   errorCount += tcSTRSTR(object, expect, string, substr);
   substr= "TWT";
   errorCount += tcSTRSTR(object, expect, string, substr);
   substr= "TYT";
   errorCount += tcSTRSTR(object, expect, string, substr);
   substr= "TKT";
   errorCount += tcSTRSTR(object, expect, string, substr);

   expect= string + 9;
   substr= "GGG";
   errorCount += tcSTRSTR(object, expect, string, substr);

   //-------------------------------------------------------------------------
   // Return
   //-------------------------------------------------------------------------
   if( errorCount == 0 )
     printf("NO errors detected\n");
   else if( errorCount == 1 )
     printf("1 error detected\n");
   else
     printf("%d errors detected\n", errorCount);

   if( errorCount == 0 )
     return 0;
   return 1;
}

