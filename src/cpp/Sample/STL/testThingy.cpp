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
//       testThingy.cpp
//
// Purpose-
//       Test Thingy.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#include "Main.h"

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
//
// Subroutine-
//       test00
//
// Purpose-
//       Bringup test.
//
//----------------------------------------------------------------------------
extern void
   test00( void )
{
   wtlc(LevelInfo, "testThingy::test00()\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testThingy
//
// Purpose-
//       Test Thingy.
//
//----------------------------------------------------------------------------
extern void
   testThingy( void )
{
   wtlc(LevelStd, "testThingy()\n");
}

