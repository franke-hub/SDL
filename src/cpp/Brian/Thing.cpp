//----------------------------------------------------------------------------
//
//       Copyright (c) 2024 Frank Eskesen.
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
//       Thing.cpp
//
// Purpose-
//       Implement Thing.h
//
// Last change date-
//       2024/10/27
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For namespace pub::debugging

#include "Thing.h"                  // For NoisyThing, implemented

using pub::Debug;
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum
{  HCDM= true                       // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // (generic) enum

//----------------------------------------------------------------------------
//
// Method-
//       NoisyThing::NoisyThing
//       NoisyThing::NoisyThing~
//
// Purpose-
//       Constructor.
//       Destructor.
//
//----------------------------------------------------------------------------
   NoisyThing::NoisyThing( void )   // Default constructor
{  if( HCDM ) debugf("NoisyThing(%p)!\n", this); }

   NoisyThing::~NoisyThing( void )  // Destructor
{  if( HCDM ) debugf("NoisyThing(%p)~\n", this); }
