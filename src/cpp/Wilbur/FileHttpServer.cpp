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
//       FileHttpServer.cpp
//
// Purpose-
//       FileHttpServer implementation methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <sys/stat.h>
#include <com/Debug.h>
#include <com/istring.h>            // for memicmp
#include <com/Thread.h>

#include "Common.h"
#include "DateParser.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Properties.h"
#include "TextBuffer.h"

#include "HttpServer.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef BRINGUP
#undef  BRINGUP                     // If defined, BRINGUP Mode
#endif

#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Subroutine-
//       DLL_make
//
// Purpose-
//       Allocate and initialize a PostHttpServer object
//
//----------------------------------------------------------------------------
extern "C"
Interface*                          // Our Interface
   DLL_make( void )                 // Get Interface
{
   Interface* result= new HttpServer();
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       DLL_take
//
// Purpose-
//       Finalize and release storage for an Interface Object.
//
//----------------------------------------------------------------------------
extern "C"
void
   DLL_take(                        // Recycle
     Interface*        object)      // This Interface Object
{
   delete object;                   // Delete the Object
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       my_init
//
// Purpose-
//       _OS_BSD DLL initialization function.
//
//----------------------------------------------------------------------------
#if defined(_OS_BSD) && defined(BRINGUP) && defined(HCDM)
__attribute__((constructor))
static void my_init()
{
   IFHCDM( traceh("Inside my_init()\n"); )
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
#if defined(_OS_BSD) && defined(BRINGUP) && defined(HCDM)
__attribute__((destructor))
static void my_fini()
{
   IFHCDM( traceh("Inside my_fini()\n"); )
}
#endif

