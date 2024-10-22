//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Main.cpp
//
// Purpose-
//       Run the HttpListener
//
// Last change date-
//       2024/10/08
//
//----------------------------------------------------------------------------
#include "HttpListen.h"             // For HttpListen

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
{
   //-------------------------------------------------------------------------
   // Run the Listener
   HttpListen listener;             // The HTTP listener
   listener.start();                // Start the listener
   listener.join();                 // Run (until interrupted)

   return 0;
}
