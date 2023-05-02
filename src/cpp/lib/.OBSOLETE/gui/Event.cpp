//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Event.cpp
//
// Purpose-
//       Graphical User Interface: Event implementation
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <stdio.h>

#include "gui/Object.h"
using namespace GUI;

#include "gui/Event.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Event::debug()
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   Event::debug( void ) const       // Debugging display
{
   Logger::log("%4d: Event(%p)::debug() EC(%d) ED(%d) O(%d,%d) L(%d,%d)\n",
               __LINE__, this, code, data,
               offset.x, offset.y, length.x, length.y);
}

