//----------------------------------------------------------------------------
//
//       Copyright (c) 2012 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       DLLStartup.cpp
//
// Purpose-
//       Container for diagnostie DLL initialization routines.
//
// Last change date-
//       2012/01/01
//
// Implementation notes-
//       Container for diagnostic DLL initialization routines.
//       (Currently only for diagnostic purposes.)
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       my_init
//
// Purpose-
//       _OS_BSD DLL initialization function.
//
//----------------------------------------------------------------------------
#ifdef _OS_BSD
__attribute__((constructor))
static void my_init()
{
   #ifdef HCDM
     printf("Inside my_init()\n");
   #endif
} 
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       my_fini
//
// Purpose-
//       _OS_BSD DLL termination function.
//
//----------------------------------------------------------------------------
#ifdef _OS_BSD
__attribute__((destructor))
static void my_fini()
{
   #ifdef HCDM
     printf("Inside my_fini()\n");
   #endif
}
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       DllMain
//
// Purpose-
//       _OS_WIN DLL initialization/termination functions.
//
// Implementation notes-
//       RESTRICTED FUNCTIONALITY.
//       For example, printf cannot be used if the DLL is statically linked.
//
//----------------------------------------------------------------------------
#if defined(_OS_WIN)
#include <windows.h>

extern "C"
BOOL WINAPI DllMain(
    HINSTANCE hinstDLL,  // handle to DLL module
    DWORD fdwReason,     // reason for calling function
    LPVOID lpReserved )  // reserved
{
   #ifdef HCDM
     printf("Inside DllMain()\n"); // BREAKS if statically linked
   #endif

   // Perform actions based on the reason for calling.
   switch( fdwReason )
   {
     case DLL_PROCESS_ATTACH:
       // Initialize once for each new process.
       // Return FALSE to fail DLL load.
       break;

     case DLL_THREAD_ATTACH:
       // Do thread-specific initialization.
       break;

     case DLL_THREAD_DETACH:
       // Do thread-specific cleanup.
       break;

     case DLL_PROCESS_DETACH:
       // Perform any necessary cleanup.
       break;

     default:
       return FALSE;
       break;
   }

   return TRUE;
}
#endif

