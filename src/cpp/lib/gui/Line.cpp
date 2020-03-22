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
//       Line.cpp
//
// Purpose-
//       Graphical User Interface: Line implementation
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
#include "gui/Bounds.h"

#include "gui/Line.h"
using namespace GUI;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define USE_IMMEDIATE_CHANGE 0      // If defined, change during render

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#if( USE_IMMEDIATE_CHANGE )
  #define SETPIXEL(_x,_y) {setPixel(_x,_y,color); \
              outoff.x= _x; outoff.y= _y; change(outoff, unitLength); }
#else
  #define SETPIXEL(_x,_y)  setPixel(_x,_y,color)
#endif

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static const XYOffset  zeroOffset= {0, 0};

#if( USE_IMMEDIATE_CHANGE )
static const XYLength  unitLength= {1, 1};
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       describe
//
// Purpose-
//       Describe a line
//
// Region-
//       Even though the region is inverted when displayed
//       we use the standard notation:
//
//               |
//               |
//            II |  I
//               |
//       --------0--------
//               |
//           III | IV
//               |
//               |
//
//----------------------------------------------------------------------------
static inline int                   // The region
   describe(                        // Describe a line
     const XYOffset&   origin,      // Line origin
     const XYOffset&   ending,      // Line endpoint
     XYOffset&         offset,      // OUTPUT: Offset
     XYLength&         length)      // OUTPUT: Length
{
   int region= 1;
   offset.x= origin.x;
   length.x= ending.x - origin.x;
   if( ending.x < origin.x )
   {
     region= 2;
     if( ending.y < origin.y )
       region= 3;
     offset.x= ending.x;
     length.x= origin.x - ending.x;
   }

   offset.y= origin.y;
   length.y= ending.y - origin.y;
   if( ending.y < origin.y )
   {
     if( ending.x >= origin.x )
       region= 4;
     offset.y= ending.y;
     length.y= origin.y - ending.y;
   }

   return region;
}

//----------------------------------------------------------------------------
//
// Method-
//       Line::~Line
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Line::~Line( void )              // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Line(%p)::~Line() %s\n", __LINE__, this, name);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Line::Line
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Line::Line(                      // Constructor
     Object*           parent)      // -> Parent Object
:  Object(parent)
,  origin(zeroOffset)
,  ending(zeroOffset)
{
   #ifdef HCDM
     Logger::log("%4d: Line(%p)::Line(%p)\n", __LINE__, this, parent);
   #endif

   while( parent != NULL )
   {
     Bounds* bounds= dynamic_cast<Bounds*>(parent);
     if( bounds != NULL )
     {
       ending.x= bounds->getLength().x;
       ending.y= bounds->getLength().y;
       break;
     }

     parent= parent->getParent();
   }
}

   Line::Line(                      // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   origin,      // Line origin
     const XYOffset&   ending)      // Line endpoint
:  Object(parent)
,  origin(origin)
,  ending(ending)
{
   #ifdef HCDM
     Logger::log("%4d: Line(%p)::Line(%p,{%d,%d}::{%d,%d})\n", __LINE__,
                 this, parent, origin.x, origin.y, ending.x, ending.y);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Line::draw
//
// Purpose-
//       Draw the line, returning the change offset and length
//
//----------------------------------------------------------------------------
void
   Line::draw(                      // Draw the Line
     XYOffset&         offset,      // (OUTPUT) Change offset
     XYLength&         length)      // (OUTPUT) Change length
{
   double              dx, dy;      // dx/dy, dy/dx
   int                 region;      // The region of the line
                                    // 1 (origin is at bottom, left)
                                    // 2 (origin is at bottom, right)
                                    // 3 (origin is at top, right)
                                    // 4 (origin is at top, left)

   Length_t            xL, yL;      // Lengths
   Offset_t            x, y;        // Row and Column

   #if( USE_IMMEDIATE_CHANGE )
     XYOffset outoff;               // Used in SETPIXEL
   #endif

   // Render this Object
   region= describe(origin, ending, offset, length);
   #if defined(HCDM) && TRUE
     Logger::log("%4d: Line(%p).describe({%d,%d},{%d,%d},=>%d,{%d,%d},{%d,%d})"
                 " %s\n"
                 , __LINE__, this , origin.x, origin.y , ending.x, ending.y
                 , region , offset.x, offset.y , length.x, length.y
                 , name
                 );
   #endif

   xL= length.x;
   yL= length.y;
   dx= (double)(xL);
   dy= (double)(yL);

   if( xL >= yL )
   {
     if( dx != 0.0 )
       dy= dy / dx;

     switch(region)
     {
       case 1:
         for(x= offset.x; x<xL; x++)
         {
           y= (int)(x * dy);
           SETPIXEL(x,      offset.y + y);
         }
         break;

       case 2:
         for(x= offset.x; x<xL; x++)
         {
           y= (int)(x * dy);
           SETPIXEL(xL-x-1, offset.y + y);
         }
         break;

       case 3:
         for(x= offset.x; x<xL; x++)
         {
           y= (int)(x * dy);
           SETPIXEL(xL-x-1, offset.y + yL-y-1);
         }
         break;

       case 4:
         for(x= offset.x; x<xL; x++)
         {
           y= (int)(x * dy);
           SETPIXEL(x,      offset.y + yL-y-1);
         }
         break;
     }
   }
   else
   {
     if( dy != 0.0 )
       dx= dx / dy;

     switch(region)
     {
       case 1:
         for(y= offset.y; y<yL; y++)
         {
           x= (int)(y * dx);
           SETPIXEL(offset.x + x,      y);
         }
         break;

       case 2:
         for(y= offset.y; y<yL; y++)
         {
           x= (int)(y * dx);
           SETPIXEL(offset.x + xL-x-1, y);
         }
         break;

       case 3:
         for(y= offset.y; y<yL; y++)
         {
           x= (int)(y * dx);
           SETPIXEL(offset.x + xL-x-1, yL-y-1);
         }
         break;

       case 4:
         for(y= offset.y; y<yL; y++)
         {
           x= (int)(y * dx);
           SETPIXEL(offset.x + x,      yL-y-1);
         }
         break;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Line::line
//
// Purpose-
//       Update the origin and ending points, render, and change the Line
//
//----------------------------------------------------------------------------
void
   Line::line(                      // Reset, render, and change the Line
     const XYOffset&   origin,      // Line origin
     const XYOffset&   ending)      // Line endpoint
{
   #ifdef HCDM
     Logger::log("%4d: Line(%p)::line({%d,%d}::{%d,%d}) %s\n", __LINE__,
                 this, origin.x, origin.y, ending.x, ending.y, name);
   #endif

   this->origin= origin;
   this->ending= ending;
   XYOffset offset;
   XYLength length;
   draw(offset, length);

   #if( USE_IIMMEDIATE_CHANGE == FALSE )
     if( length.x == 0 )
       length.x= 1;
     if( length.y == 0 )
       length.y= 1;
     change(offset, length);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Line::render
//
// Purpose-
//       Render this Object
//
//----------------------------------------------------------------------------
void
   Line::render( void )             // Render the Line
{
   #ifdef HCDM
     Logger::log("%4d: Line(%p)::render() %s\n", __LINE__, this, name);
   #endif

   XYOffset            offset;      // The line offset
   XYLength            length;      // The line length

   draw(offset, length);
}

