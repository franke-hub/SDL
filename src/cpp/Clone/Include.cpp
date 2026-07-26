//----------------------------------------------------------------------------
//
//       Copyright (c) 2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       Include.cpp
//
// Purpose-
//       Test compile one or all include files
//
// Last change date-
//       2026/07/05
//
// Implementation notes-
//       Invoke from "doit filename.h" script
//
//----------------------------------------------------------------------------
#include <cstdio>                   // For printf

// The current include file (from environment)
#ifdef INCLUDE
  #include INCLUDE
#else
  #define INCLUDE "All include files"
  #include "ClientThread.h"
  #include "CommonThread.h"
  #include "IoCommon.h"
  #include "ListenThread.h"
  #include "RdCommon.h"
  #include "ServerThread.h"
#endif


//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int main(int, char*[])
{
   printf("%s compiled OK\n", INCLUDE);
   return 0;
}
