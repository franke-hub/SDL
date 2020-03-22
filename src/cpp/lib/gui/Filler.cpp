//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Filler.cpp
//
// Purpose-
//       Graphical User Interface: Filler implementation
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gui/Types.h"
#include "gui/Buffer.h"

#include "gui/Filler.h"
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
static const XYOffset  defaultOffset= {0,0};   // Default offset
static const XYLength  defaultLength= {32,32}; // Default length

//----------------------------------------------------------------------------
//
// Method-
//       Filler::~Filler
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Filler::~Filler( void )          // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Filler(%p)::~Filler() %s\n",
                 __LINE__, this, name);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Filler::Filler
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Filler::Filler(                  // Constructor
     Object*           parent)      // -> Parent Object
:  Bounds(parent, defaultOffset, defaultLength)
{
   #ifdef HCDM
     Logger::log("%4d: Filler(%p)::Filler(%p)\n",
                 __LINE__, this, parent);
   #endif
}

   Filler::Filler(                  // Constructor
     Object*           parent,      // -> Parent Object
     XYOffset&         offset)      // Object offset
:  Bounds(parent, offset)
{
   #ifdef HCDM
     Logger::log("%4d: Filler(%p)::Filler(%p,O{%d,%d})\n",
                 __LINE__, this, parent, offset.x, offset.y);
   #endif
}

   Filler::Filler(                  // Constructor
     Object*           parent,      // -> Parent Object
     XYLength&         length)      // Object length
:  Bounds(parent, length)
{
   #ifdef HCDM
     Logger::log("%4d: Filler(%p)::Filler(%p,L{%d,%d})\n",
                 __LINE__, this, parent, length.x, length.y);
   #endif
}

   Filler::Filler(                  // Constructor
     Object*           parent,      // -> Parent Object
     XYOffset&         offset,      // Object offset
     XYLength&         length)      // Object length
:  Bounds(parent, offset, length)
{
   #ifdef HCDM
     Logger::log("%4d: Filler(%p)::Filler(%p,{%d,%d},{%d,%d})\n",
                 __LINE__, this, parent,
                 offset.x, offset.y, length.x, length.y);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Filler::render
//
// Purpose-
//       Fill using the background Color
//
// Usage notes-
//       Obeys VISIBLE and TRANSPARENT attributes.
//
//----------------------------------------------------------------------------
void
   Filler::render( void )           // Fill using the background Color
{
   #ifdef HCDM
     Logger::log("%4d: Filler(%p)::render() %s\n", __LINE__, this, name);
   #endif

   // Render this Object
   if( getAttribute(VISIBLE) && !getAttribute(TRANSPARENT) )
   {
     XYOffset offset;               // Offset (in target)
     XYLength length;               // Length (in target) that's visible
     Buffer* buffer= range(offset, length); // Get visible range
     for(Offset_t y= 0; y < length.y; y++) // (If buffer == NULL, length.y == 0)
     {
       Pixel* pixel= buffer->getPixel(offset.x, offset.y + y);
       for(Offset_t x= 0; x < length.x; x++)
       {
         pixel->setColor(color);
         pixel++;
       }
     }
   }
}

