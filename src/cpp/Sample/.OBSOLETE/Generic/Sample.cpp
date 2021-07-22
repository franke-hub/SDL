//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Sample.cpp
//
// Purpose-
//       Instantiate Sample object methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <iostream>
using namespace std;

#include "Sample.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "SAMPLE  " // Source file

//----------------------------------------------------------------------------
//
// Method-
//       Sample::~Sample
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Sample::~Sample( void )          // Destructor
{
   cout << "!" << endl;
}

//----------------------------------------------------------------------------
//
// Method-
//       Sample::Sample
//
// Purpose-
//       Default Constructor.
//
//----------------------------------------------------------------------------
   Sample::Sample( void )           // Default constructor
{
   cout << "Hello ";
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       sample
//
// Purpose-
//       SAMPLE test
//
//----------------------------------------------------------------------------
int sample(int, char**);            // (Not very far) Forward reference
int                                 // Error count
   sample(int, char**)              // MALLOC test
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{
   int                 errorCount= 0; // Error count
   Sample              object;      // Sample object

   //-------------------------------------------------------------------------
   // Drive the tests
   //-------------------------------------------------------------------------
   cout << "sample world";

   return errorCount;
}

