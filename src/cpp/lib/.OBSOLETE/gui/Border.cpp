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
//       Border.cpp
//
// Purpose-
//       Graphical User Interface: Border implementation
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <com/Logger.h>

#include "gui/Types.h"
#include "gui/Buffer.h"

#include "gui/Border.h"
using namespace GUI;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const XYLength  zeroLength= {0, 0};

//----------------------------------------------------------------------------
//
// Method-
//       Border::~Border
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Border::~Border( void )          // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Border(%p)::~Border() %s\n", __LINE__, this, name);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Border::Border
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Border::Border(                  // Constructor
     Object*           parent)      // -> Parent Object
:  Object(parent), length(zeroLength)
{
   #ifdef HCDM
     Logger::log("%4d: Border(%p)::Border(%p)\n", __LINE__, this, parent);
   #endif
}

   Border::Border(                  // Constructor
     Object*           parent,      // -> Parent Object
     const XYLength&   length)      // Border length
:  Object(parent), length(length)
{
   #ifdef HCDM
     Logger::log("%4d: Border(%p)::Border(%p,L{%d,%d})\n",
                 __LINE__, this, parent, length.x, length.y);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Border::render
//
// Purpose-
//       Render the Border
//
//----------------------------------------------------------------------------
void
   Border::render( void )           // Render the Border
{
   #ifdef HCDM
     Logger::log("%4d: Border(%p)::render() %s\n", __LINE__, this, name);
   #endif

   // TODO: NOT CODED YET
}

