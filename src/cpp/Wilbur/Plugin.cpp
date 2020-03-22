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
//       Plugin.cpp
//
// Purpose-
//       Plugin object methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <com/Debug.h>

#include "Plugin.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef logf
#define logf traceh                 // Alias for trace w/header
#endif

#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Method-
//       Plugin::~Plugin
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Plugin::~Plugin( void )          // Destructor
{
   IFHCDM( logf("Plugin(%p)::~Plugin()\n", this); )

   take(interface);                 // Delete the Library object
   interface= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Plugin::Plugin
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Plugin::Plugin(                  // Constructor
     const char*       name)        // Name of library
:  Loader(name)
,  interface(NULL)
{
   IFHCDM( logf("Plugin(%p)::Plugin(%s)\n", this, name); )

   interface= make();               // Make the Library object
}

