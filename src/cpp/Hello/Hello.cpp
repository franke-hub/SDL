//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
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
//       2020/10/04
//
//----------------------------------------------------------------------------
#include <iostream>
#include "Hello.h"

using namespace std;

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
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
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

