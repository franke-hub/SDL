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
//       SampleDLL.cpp
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

#include "Interface.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

#ifndef IODM
#undef  IODM                        // If defined, Input/Output Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       DLL_make
//
// Purpose-
//       Allocate and initialize an Interface Object.
//
//----------------------------------------------------------------------------
extern "C"
Interface*                          // Our Interface
   DLL_make( void )                 // Get Interface
{
   return NULL; // NOT CODED YET
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

