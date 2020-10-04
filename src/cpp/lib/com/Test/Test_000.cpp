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
//       Test_000.cpp
//
// Purpose-
//       Quick and dirty test.
//
// Last change date-
//       2020/10/03
//
// Usage-
//       1) Modify this file, the "quick and dirty test"
//       2) Run the test until satisfied.
//       3) Remove this file
//       4) cvs update Test_000.cpp (to restore this file)
//
//----------------------------------------------------------------------------
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <com/Debug.h>
#include <com/Verify.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       test000
//
// Purpose-
//       Quick and dirty test.
//
//----------------------------------------------------------------------------
static void
   test000( void )                  // Quick and dirty test
{
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
   test000();
   verify_exit();
}

