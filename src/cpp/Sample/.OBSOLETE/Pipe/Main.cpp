//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
// SPDX-License-Identifier: MIT
//----------------------------------------------------------------------------
//
// Title-
//       Main.cpp
//
// Purpose-
//       Sample main routine.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <unistd.h>

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define __SOURCE__       "MAIN    " // Source file, for debugging

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
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
//       main
//
// Purpose-
//       Mainline code.
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(int, char**)                // Mainline code
//   int             argc,          // Argument count (Unused)
//   char*           argv[])        // Argument array (Unused)
{
   #ifdef SCDM
     printf("%4d Main::main()\n", __LINE__);
   #endif

   //-------------------------------------------------------------------------
   // Return to caller
   //-------------------------------------------------------------------------
   return 0;
}

