//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TestUnit.cpp
//
// Purpose-
//       Test the Unit objects.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <new.h>
#include <stdio.h>
#include <stdlib.h>

#include <Unused/Unit.h>
#include <Unused/UnitCallBack.h>
#include <Unused/UnitManager.h>
#include <Unused/ManagerData.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "TestUnit" // Source file

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
     int             argc,          // Argument count
     char*           argv[])        // Argument array
{
   printf("Compile-only test\n");

   return 0;                        // Normal completion
}

