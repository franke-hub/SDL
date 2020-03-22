//----------------------------------------------------------------------------
//
//       Copyright (c) 2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Include.cpp
//
// Purpose-
//       Test driver.
//
// Last change date-
//       2018/01/01
//
// Implemntation notes-
//       Trace file "debug.log" is append-only.
//
//----------------------------------------------------------------------------
#include "Debug.h"

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Test include files.
//
//----------------------------------------------------------------------------
int
   main(                            // Mainline code
     int               argc,        // Argument count
     const char*       argv[])      // Argument list
{
   tracef("This only goes to debug.log\n");
   debugf("This goes to stdout and debug.log\n");

   return 0;
}

