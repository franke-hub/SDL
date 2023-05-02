//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       include.cpp
//
// Purpose-
//       Test include files.
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <com/Verify.h>

#include "com/ASCII.h"
#include "com/KeyCode.h"
#include "com/ScanCode.h"

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#define SHOW(x) show(#x, x)

//----------------------------------------------------------------------------
//
// Subroutine-
//       show
//
// Purpose-
//       Show value.
//
//----------------------------------------------------------------------------
inline void
   show(                            // Show a value
     char*           name,          // Argument name
     long            value)         // Argument value
{
   verify_info(); debugf("%10ld='%s'\n", value, name);
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
{
   printf("Compile-only test\n");
   verify_exit();
}

