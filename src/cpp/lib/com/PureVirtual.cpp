//----------------------------------------------------------------------------
//
//       Copyright (c) 2014 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       PureVirtual.cpp
//
// Purpose-
//       Implements pure virtual methods not implemented elsewhere.
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#include <com/Debug.h>

#include "com/NamedLock.h"          // Implementation class

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#ifndef SCDM
#undef  SCDM                        // If defined, Soft Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Dependent macros
//----------------------------------------------------------------------------
#include <com/ifmacro.h>

//----------------------------------------------------------------------------
//
// Method-
//       NamedLock::~NamedLock
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   NamedLock::~NamedLock( void )    // Destructor
{
   IFSCDM( traceh("%4d NamedLock(%p)::~NamedLock()\n", __LINE__, this); )
}
