//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.cpp
//
// Purpose-
//       Mainline code -- test driver.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <com/Debug.h>

#include "SampleFactory.h"
#include "Loader.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#define SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       test
//
// Purpose-
//       Run the tests.
//
//----------------------------------------------------------------------------
void
   test( void )
{
   {{{{
     Loader L1("libTest.so.1.0");
     SampleFactory* F1= (SampleFactory*)L1.make();
     SampleFactory::Object* O1= (SampleFactory::Object*)F1->make();

     F1->talk("This is only a test. (one)");
     O1->doSomething("Object talks. (one)");

     F1->take(O1);
     L1.take(F1);
   }}}}

   {{{{
     Loader L2("libTest.so.1.1");
     SampleFactory* F2= (SampleFactory*)L2.make();
     SampleFactory::Object* O2= (SampleFactory::Object*)F2->make();

     F2->talk("This is only a test. (one)");
     O2->doSomething("Object talks. (one)");

     F2->take(O2);
     L2.take(F2);
   }}}}

   {{{{
     Loader L1("libTest.so.1.0");
     SampleFactory* F1= (SampleFactory*)L1.make();
     SampleFactory::Object* O1= (SampleFactory::Object*)F1->make();

     Loader L2("libTest.so.1.1");
     SampleFactory* F2= (SampleFactory*)L2.make();
     SampleFactory::Object* O2= (SampleFactory::Object*)F2->make();

     Loader L3("libTest.so.1.0");
     SampleFactory* F3= (SampleFactory*)L3.make();
     SampleFactory::Object* O3= (SampleFactory::Object*)F3->make();

     #ifdef HCDM
       printf("Pause...\n");
       getchar();
     #endif

     F1->talk("This is only a test. (one)");
     O1->doSomething("Object talks. (one)");

     F2->talk("This is only a test. (two)");
     O2->doSomething("Object talks. (two)");

     F3->talk("This is only a test. (301)");
     O3->doSomething("Object talks. (301)");

     F1->take(O1);
     L1.take(F1);

     F2->take(O2);
     L2.take(F2);

     F3->take(O3);
     L3.take(F3);
   }}}}
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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   #ifdef SCDM
     printf("%4d Main.main() %p\n", __LINE__, main);
   #endif

   //-------------------------------------------------------------------------
   // Perform the test
   //-------------------------------------------------------------------------
   try {
     test();
   } catch(const char* X) {
     fprintf(stderr, "Exception(%s) const char*\n", X);
   } catch(...) {
     fprintf(stderr, "Exception(...)\n");
   }

   return 0;
}

