//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Terminal.cpp
//
// Purpose-
//       Terminal control.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <com/Debug.h>
#include <com/syslib.h>

#include "com/Terminal.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       Terminal::Terminal
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Terminal::Terminal( void )       // Constructor
:  Keyboard(), TextScreen()
{
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       Terminal::~Terminal
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Terminal::~Terminal( void )      // Constructor
{
}

