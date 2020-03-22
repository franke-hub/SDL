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
//       Interface.cpp
//
// Purpose-
//       Interface object methods.
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
#include "Interface.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Interface::~Interface
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Interface::~Interface( void )    // Destructor
{
   #ifdef HCDM
     debugf("Interface(%p)::~Interface()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Interface::Interface
//
// Purpose-
//       Default constructor.
//
//----------------------------------------------------------------------------
#if( 0 ) // NOT DEFINED
   Interface::Interface( void )     // Default constructor
{
   #ifdef HCDM
     debugf("Interface(%p)::Interface()\n", this);
   #endif
}
#endif

