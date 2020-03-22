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
//       Factory.cpp
//
// Purpose-
//       Factory object methods.
//
// Last change date-
//       2012/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <com/define.h>
#include <com/Debug.h>
#include "Factory.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Factory::~Factory
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Factory::~Factory( void )        // Destructor
{
   #ifdef HCDM
     debugf("Factory(%p)::~Factory()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Factory::Factory
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
   Factory::Factory( void )         // Default constructor
:  Interface()
{
   #ifdef HCDM
     debugf("Factory(%p)::Factory()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Factory::make
//
// Purpose-
//       Default make method.
//
//----------------------------------------------------------------------------
Interface*                          // Resultant Interface Object
   Factory::make( void )            // Create an Interface Object
{
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Factory::take
//
// Purpose-
//       Default take method.
//
//----------------------------------------------------------------------------
void
   Factory::take(                   // Recycle
     Interface*        object)      // This Interface Object
{
   delete object;
}

