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
//       WormBuffer.cpp
//
// Purpose-
//       Graphical User Interface: WormBuffer implementation
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <com/Logger.h>
#include <com/Random.h>

#include "gui/Types.h"
#include "gui/Event.h"
#include "gui/Window.h"
#include "gui/Types.h"

#include "WormBuffer.h"
using namespace GUI;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// External references
//----------------------------------------------------------------------------
static Random&         RNG= Random::standard; // Our random number generator

//----------------------------------------------------------------------------
//
// Struct-
//       XYDelta
//
// Purpose-
//       Define X and Y change values.
//
//----------------------------------------------------------------------------
struct XYDelta
{
   int                 x;           // X change value
   int                 y;           // Y change value
}; // struct XYDelta

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const XYOffset  defaultOffset= {0,0};   // Default offset
static const XYLength  defaultLength= {512,512}; // Default length
static const Color_t   defaultColor= 0x00000000; // Default Color
static const XYLength  unitLength= {1,1}; // Length of one Pixel
static int             wormIdent= 0;// Worm identifier

//----------------------------------------------------------------------------
// Update control tables
//----------------------------------------------------------------------------
static XYDelta         pattmm[4]=
   {{+0, -1}, {-1, -1}, {-1, -1}, {-1, +0}};

static XYDelta         pattmz[4]=
   {{-1, -1}, {-1, +0}, {-1, +0}, {-1, +1}};

static XYDelta         pattmp[4]=
   {{-1, +0}, {-1, +1}, {-1, +1}, {+0, +1}};

static XYDelta         pattzm[4]=
   {{+1, -1}, {+0, -1}, {+0, -1}, {-1, -1}};

static XYDelta         pattzz[4]=
   {{-1, +0}, {+0, +1}, {+0, +1}, {+0, -1}};

static XYDelta         pattzp[4]=
   {{-1, +1}, {+0, +1}, {+0, +1}, {+1, +1}};

static XYDelta         pattpm[4]=
   {{+1, +0}, {+1, -1}, {+1, -1}, {+0, -1}};

static XYDelta         pattpz[4]=
   {{+1, +1}, {+1, +0}, {+1, +0}, {+1, -1}};

static XYDelta         pattpp[4]=
   {{+0, +1}, {+1, +1}, {+1, +1}, {+1, +0}};

static XYDelta*        metapatt[3][3]= // The pattern selector pattern
   { {pattmm, pattmz, pattmp}
   , {pattzm, pattzz, pattzp}
   , {pattpm, pattpz, pattpp}
   };

#define DIM_COLOR 48
static Color_t         colorPatt[DIM_COLOR]=
   { 0x00ff0000                     //  0
   , 0x00ff0000                     //  1
   , 0x00ff0000                     //  2
   , 0x00ff0000                     //  3
   , 0x00ff0000                     //  4
   , 0x00ff0000                     //  5
   , 0x00ff0000                     //  6
   , 0x00ff0000                     //  7

   , 0x0000ff00                     //  8
   , 0x0000ff00                     //  9
   , 0x0000ff00                     // 10
   , 0x0000ff00                     // 11
   , 0x0000ff00                     // 12
   , 0x0000ff00                     // 13
   , 0x0000ff00                     // 14
   , 0x0000ff00                     // 15

   , 0x000000ff                     // 16
   , 0x000000ff                     // 17
   , 0x000000ff                     // 18
   , 0x000000ff                     // 19
   , 0x000000ff                     // 20
   , 0x000000ff                     // 21
   , 0x000000ff                     // 22
   , 0x000000ff                     // 23

   , 0x0000ffff                     // 24
   , 0x0000ffff                     // 25
   , 0x0000ffff                     // 26
   , 0x0000ffff                     // 27
   , 0x0000ffff                     // 28
   , 0x0000ffff                     // 29
   , 0x0000ffff                     // 30
   , 0x0000ffff                     // 31

   , 0x00ff00ff                     // 32
   , 0x00ff00ff                     // 33
   , 0x00ff00ff                     // 34
   , 0x00ff00ff                     // 35
   , 0x00ff00ff                     // 36
   , 0x00ff00ff                     // 37
   , 0x00ff00ff                     // 38
   , 0x00ff00ff                     // 39

   , 0x00ffff00                     // 40
   , 0x00ffff00                     // 41
   , 0x00ffff00                     // 42
   , 0x00ffff00                     // 43
   , 0x00ffff00                     // 44
   , 0x00ffff00                     // 45
   , 0x00ffff00                     // 46
   , 0x00ffff00                     // 47
   };

//----------------------------------------------------------------------------
//
// Subroutine-
//       sign
//
// Purpose-
//       AKA SGN function
//
//----------------------------------------------------------------------------
static inline int                   // -1, 0, +1
   sign(                            // The SGN function
     int               x)           // Argument
{
   int                 result;      // Resultant

   if( x > 0 )
     result= +1;
   else if( x < 0 )
     result= -1;
   else
     result= 0;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       WormBuffer::~WormBuffer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   WormBuffer::~WormBuffer( void )  // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: WormBuffer(%p)::~WormBuffer() %s\n",
                 __LINE__, this, name);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       WormBuffer::WormBuffer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   WormBuffer::WormBuffer(          // Constructor
     Bounds*           parent)      // -> Parent Bounds
:  Bounds(parent, defaultOffset, parent->getLength())
{
   #ifdef HCDM
     Logger::log("%4d: WormBuffer(%p)::WormBuffer(%p)\n",
                 __LINE__, this, parent);
   #endif
}

   WormBuffer::WormBuffer(          // Constructor
     Bounds*           parent,      // -> Parent Bounds
     XYOffset&         offset)      // Bounds offset
:  Bounds(parent, offset, parent->getLength())
{
   #ifdef HCDM
     Logger::log("%4d: WormBuffer(%p)::WormBuffer(%p,O{%d,%d})\n",
                 __LINE__, this, parent, offset.x, offset.y);
   #endif
}

   WormBuffer::WormBuffer(          // Constructor
     Bounds*           parent,      // -> Parent Bounds
     XYLength&         length)      // Bounds length
:  Bounds(parent, defaultOffset, length)
{
   #ifdef HCDM
     Logger::log("%4d: WormBuffer(%p)::WormBuffer(%p,L{%d,%d})\n",
                 __LINE__, this, parent, length.x, length.y);
   #endif
}

   WormBuffer::WormBuffer(          // Constructor
     Bounds*           parent,      // -> Parent Bounds
     XYOffset&         offset,      // Bounds offset
     XYLength&         length)      // Bounds length
:  Bounds(parent, offset, length)
{
   #ifdef HCDM
     Logger::log("%4d: WormBuffer(%p)::WormBuffer(%p,{%d,%d},{%d,%d})\n",
                 __LINE__, this, parent,
                offset.x, offset.y, length.x, length.y);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       WormBuffer::setPixel
//
// Purpose-
//       Set a Pixel in the Buffer.
//
//----------------------------------------------------------------------------
Pixel*                              // Resultant Pixel*
   WormBuffer::setPixel(            // Set a Pixel
     XOffset_t         x,           // X offset
     YOffset_t         y,           // Y offset
     Color_t           color)       // Color
{
   Pixel*              pixel;       // -> Pixel
   XYOffset            offset;      // XYoffset

   // Set the new pixel
   pixel= getPixel(x, y);
   if( pixel != NULL )
   {
     pixel->setColor(color);
     offset.x= x;
     offset.y= y;
     change(offset, unitLength);
   }

   return pixel;
}

//----------------------------------------------------------------------------
//
// Method-
//       WormBuffer::append
//
// Purpose-
//       Append a Worm
//
//----------------------------------------------------------------------------
void
   WormBuffer::append(              // Append a Worm
     Worm&             worm)        // The Worm
{
   list.fifo(&worm);
}

//----------------------------------------------------------------------------
//
// Method-
//       WormBuffer::reset
//
// Purpose-
//       Reset the Worm list
//
//----------------------------------------------------------------------------
void
   WormBuffer::reset( void )        // Reset the Worm List
{
   Worm*               worm;        // Worming Worm*

   worm= (Worm*)list.getHead();
   while( worm != NULL )
   {
     worm->reset(this);

     worm= (Worm*)worm->getNext();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       WormBuffer::toggle
//
// Purpose-
//       Toggle the WormBuffer
//
//----------------------------------------------------------------------------
void
   WormBuffer::toggle( void )       // Toggle the WormBuffer
{
   Worm*               worm;        // -> Worm

   #ifdef HCDM
     Logger::log("%4d: WormBuffer(%p)::toggle() %s\n", __LINE__, this, name);
   #endif

   // Toggle this Object
   worm= (Worm*)list.getHead();     // Get the first Worm
   while( worm != NULL )
   {
     worm->toggle(this);
     worm= (Worm*)worm->getNext();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Worm::~Worm
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Worm::~Worm( void )              // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Worm(%p)::~Worm()\n",
                 __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Worm::Worm
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Worm::Worm( void )               // Constructor
:  DHSL_List<Worm>::Link()
,  color(0)
,  ident(wormIdent++)
,  dX(0)
,  dY(0)
{
   int                 i;

   #ifdef HCDM
      Logger::log("%4d: Worm(%p)::Worm()\n",
                  __LINE__, this);
   #endif

   for(i= 0; i<WORM_SIZE; i++)
     offset[i].x= offset[i].y= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       Worm::toggle
//
// Purpose-
//       Toggle the Worm.
//
//----------------------------------------------------------------------------
void
   Worm::toggle(                    // Toggle the Worm
     WormBuffer*       buffer)      // In this Buffer
{
   XYDelta*            patt;        // The selected pattern
   int                 R;           // Random index
   int                 x;           // (Signed) X offset
   int                 y;           // (Signed) Y offset

   int                 i;

   // Clear the current tail pixel
   buffer->setPixel(offset[WORM_SIZE-1].x, offset[WORM_SIZE-1].y, defaultColor);

   // Make room for new element
   for(i= WORM_SIZE-1; i>0; i--)
     offset[i]= offset[i-1];

   // Select next head position
   R= 2;
   if( (RNG.get() % 6) == 0 )
     R= RNG.get() % 4;

   patt= metapatt[sign(dX)+1][sign(dY)+1];
   dX= patt[R].x;
   dY= patt[R].y;

   x= offset[0].x + dX;
   if( x < 0 )
     x= buffer->getLength().x - 1;
   else if( x >= buffer->getLength().x )
     x= 0;

   y= offset[0].y + dY;
   if( y < 0 )
     y= buffer->getLength().y - 1;
   else if( y >= buffer->getLength().y )
     y= 0;

   offset[0].x= x;
   offset[0].y= y;

   // Set the new head pixel
   color++;
   if( color >= DIM_COLOR )
     color= 0;
   buffer->setPixel(x, y, colorPatt[color]);

   #ifdef HCDM
//// if( ident == 0 )
       Logger::log("[%2d] Worm(%p) Color(%3d) {%3d,%3d} {%3d,%3d}=>{%3d,%3d}\n",
                   ident, this, color, dX, dY,
                   offset[1].x, offset[1].y,
                   offset[0].x, offset[0].y);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Worm::reset
//
// Purpose-
//       Reset (randomize) the Worm.
//
//----------------------------------------------------------------------------
void
   Worm::reset(                     // Reset the Worm
     Bounds*           bounds)      // In this Bounds
{
   XOffset_t           x;
   YOffset_t           y;

   int                 i;

   color= RNG.get() % DIM_COLOR;
   dX= 0;
   dY= 0;
   x= RNG.get() % bounds->getLength().x;
   y= RNG.get() % bounds->getLength().y;

   for(i= 0; i<WORM_SIZE; i++)
   {
     offset[i].x= x;
     offset[i].y= y;
   }
}

