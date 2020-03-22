//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2018 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Hello.cpp
//
// Purpose-
//       Hello, C++ world.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <iostream>

using namespace std;

//----------------------------------------------------------------------------
// Function prototypes
//----------------------------------------------------------------------------
extern int test00( void );
extern int test01( void );
extern int test02( void );
extern int test03( void );

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
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   unsigned int        errorCount= 0;

   errorCount += test00();
   errorCount += test01();
   errorCount += test02();
   errorCount += test03();

   cout << "Hello C++ World, ";
   cout << "Errors: " << errorCount << endl;

   return errorCount;
}

