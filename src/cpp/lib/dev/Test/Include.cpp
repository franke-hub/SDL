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
//       Include.cpp
//
// Purpose-
//       Compile bringup
//
// Last change date-
//       2023/07/29
//
// Implementation notes-
//       Use make include to run the compile test.
//
//----------------------------------------------------------------------------
#if true
  #define IT "pub/http/Agent.h"
#elif true
  #define IT "pub/http/Client.h"
#elif true
  #define IT "pub/http/Codec.h"
#elif true
  #define IT "pub/http/Exception.h"
#elif true
  #define IT "pub/http/Frame.h"
#elif true
  #define IT "pub/http/Listen.h"
#elif true
  #define IT "pub/http/Options.h"
#elif true
  #define IT "pub/http/Request.h"
#elif true
  #define IT "pub/http/Response.h"
#elif true
  #define IT "pub/http/Server.h"
#elif true
  #define IT "pub/http/Stream.h"
#elif true
  #define IT "pub/http/utility.h"
#elif false //----------------------// NOT IMPLEMENTED -----------------------
  #define IT "pub/http/Layer.h"
#elif false
  #define IT "pub/http/Protocol.h"
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
