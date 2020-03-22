//----------------------------------------------------------------------------
//
//       Copyright (c) 2019-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Install.cpp
//
// Purpose-
//       Install object methods
//
// Last change date-
//       2020/01/10
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pub/Debug.h>              // For debugging

#include "ConsoleCommand.h"
#include "ConsoleService.h"
#include "Install.h"

using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include <pub/ifmacro.h>

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static int             initialized= false; // One-time initialization control

//----------------------------------------------------------------------------
//
// Method-
//       Install::~Install
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Install::~Install( void )        // Destructor
{  traceh("Install(%p)::~Install\n", this); }

//----------------------------------------------------------------------------
//
// Method-
//       Install::Install
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Install::Install( void )         // Constructor
{  traceh("Install(%p)::Install\n", this);

   if( !initialized )               // Called once during static initialization
   {
     initialized= true;

     // Implementation note:
     // Commands must be defined before services
     CommandMap[new ConsoleCommand()]; // Create the ConsoleCommand handler
     ServiceMap[new ConsoleService()]; // Create the ConsoleService
   }
}
