//----------------------------------------------------------------------------
//
//       Copyright (C) 2022-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       _Include.cpp
//
// Purpose-
//       Compile bringup
//
// Last change date-
//       2023/07/20
//
// Implementation notes-
//       Use make _include to run the compile test.
//
//----------------------------------------------------------------------------
#if true
  #define IT "pub/http/_Client.h"
#elif true
  #define IT "pub/http/_Server.h"
#elif true
  #define IT "pub/http/_Stream.h"
#elif true
  #define IT "pub/http/_StreamSet.h"
#elif false //----------------------// NOT IMPLEMENTED -----------------------
  #define IT "cstdio"               // Include something
#else
  #define IT "cstdio"               // Include something
#endif

//----------------------------------------------------------------------------
// Include (only) the selected header + cstdio for printf
//----------------------------------------------------------------------------
#include IT
#include <cstdio>                   // For printf

extern int                          // Return code
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{  printf("#include \"%s\" // compiles OK\n", IT); return 0; }
