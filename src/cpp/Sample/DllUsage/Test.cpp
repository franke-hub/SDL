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
//       Test.cpp
//
// Purpose-
//       Library container.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "SampleFactory.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#define HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static SampleFactory   factory;     // Our Factory

#ifdef HCDM
static char            buffer[1024];// Test buffer
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       test
//
// Purpose-
//       Simple test routine.
//
//----------------------------------------------------------------------------
extern "C" Interface* DLL_make( void ); // (Not very far) Forward reference
extern "C"
Interface*                          // Our Interface (and Factory)
   DLL_make( void )                 // Get Interface (and Factory)
{
   return &factory;
}

extern "C" void DLL_take(Interface*); // (Not very far) Forward reference
extern "C"
void
   DLL_take(                        // Recycle
     Interface*        object)      // This Interface Object
{
   (void)object; // STATIC OBJECT, DELETED ON EXIT.
}

#if( 1 )
//----------------------------------------------------------------------------
//
// Section-
//       INIT
//
// Purpose-
//       Various OS dependent initialization functions
//
//----------------------------------------------------------------------------
#ifdef _OS_BSD
__attribute__((constructor))
static void my_init()
{
   #ifdef HCDM
     printf("Inside my_init()\n");
     printf("Factory(%p) DLL_make(%p)\n", &factory, &DLL_make);

     time_t v0= time(NULL);
     struct tm v1= *localtime(&v0);
     strcpy(buffer, asctime(&v1));
     printf("Buffer(%p) '%s'\n", buffer, buffer);
   #endif
} 

__attribute__((destructor))
static void my_fini()
{
   #ifdef HCDM
     printf("Inside my_fini()\n");
     printf("Buffer(%p) '%s'\n", buffer, buffer);
   #endif
}
#endif

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
#endif

