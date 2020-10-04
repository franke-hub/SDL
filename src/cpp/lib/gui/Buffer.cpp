//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Buffer.cpp
//
// Purpose-
//       Graphical User Interface: Buffer implementation
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "gui/Types.h"
using namespace GUI;

#include "gui/Buffer.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const XYLength  unitLength= {1,1}; // Unit length
static const XYOffset  zeroOffset= {0,0}; // Zero offset

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::~Buffer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Buffer::~Buffer( void )          // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Buffer(%p)::~Buffer() %s\n", __LINE__, this, name);
   #endif

   if( pixel != NULL )
   {
     free(pixel);
     pixel= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::Buffer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
void
   Buffer::buildObject( void )      // Initialize Buffer
{
   #ifdef HCDM
     Logger::log("%4d: Buffer(%p)::buildObject()\n", __LINE__, this);
   #endif

   this->pixel= NULL;

   this->name= "Buffer";
   if( length.x == 0 )
     length.x= 1;
   if( length.y == 0 )
     length.y= 1;
   resize(length);
}

   Buffer::Buffer(                  // Constructor
     Object*           parent)      // -> Parent Object
:  Bounds(parent, unitLength)
{
   #ifdef HCDM
     Logger::log("%4d: Buffer(%p)::Buffer()\n", __LINE__, this);
   #endif

   buildObject();
}

   Buffer::Buffer(                  // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset)      // Object offset
:  Bounds(parent, offset, unitLength)
{
   #ifdef HCDM
     Logger::log("%4d: Buffer(%p)::Buffer(O{%d,%d})\n",
                 __LINE__, this, offset.x, offset.y);
   #endif

   buildObject();
}

   Buffer::Buffer(                  // Constructor
     Object*           parent,      // -> Parent Object
     const XYLength&   length)      // Object length
:  Bounds(parent, length)
{
   #ifdef HCDM
     Logger::log("%4d: Buffer(%p)::Buffer(L{%d,%d})\n",
                 __LINE__, this, length.x, length.y);
   #endif

   buildObject();
}

   Buffer::Buffer(                  // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset,      // Object offset
     const XYLength&   length)      // Object length
:  Bounds(parent, offset, length)
{
   #ifdef HCDM
      Logger::log("%4d: Buffer(%p)::Buffer({%d,%d},{%d,%d})\n", __LINE__,
                  this, offset.x, offset.y, length.x, length.y);
   #endif

   buildObject();
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::getPixel
//
// Purpose-
//       Accessor: Get Pixel*
//
// Implementation note-
//       It is not necessary to override setPixel because setPixel uses
//       getPixel to address the Pixel.
//
//----------------------------------------------------------------------------
Pixel*                              // -> Pixel
   Buffer::getPixel(                // Get Pixel*
     XOffset_t         x,           // X (horizontal) offset
     YOffset_t         y) const     // Y (vertical) offset
{
   #if defined(HCDM) && FALSE
     Logger::log("%4d: Buffer(%p)::getPixel(%d,%d) %s\n", __LINE__,
                 this, x, y, name);
   #endif

   if( x >= length.x || y >= length.y )
   {
     #ifdef HCDM
       Logger::log("%4d: ERROR: Buffer(%p)::getPixel(%d,%d) %s "
                   "length(%d,%d)\n",
                   __LINE__, this, x, y, name, length.x, length.y);
     #endif
     return NULL;
   }

   return (Pixel*)(pixel + (y * length.x) + x);
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::render()
//
// Purpose-
//       Render this Buffer
//
//----------------------------------------------------------------------------
void
   Buffer::render( void )           // Render the Buffer
{
   #ifdef HCDM
     Logger::log("%4d: Buffer(%p)::render() %s\n", __LINE__, this, name);
   #endif

   // Set default color
   Color_t color= this->color;
   if( getAttribute(TRANSPARENT) )
     color |= 0xff000000;

   Pixel* targetP= getPixel(0,0);
   for(Offset_t y= 0; y<length.y; y++)
   {
     for(Offset_t x= 0; x<length.x; x++)
     {
       targetP->setColor(color);
       targetP++;
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::resize
//
// Purpose-
//       Resize the Buffer.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   Buffer::resize(                  // Resize the Buffer
     const XYLength&   length)      // Resize length
{
   Pixel*              pixel;       // Replacement buffer
   unsigned            size;        // Replacement size

   #if defined(HCDM)
     Logger::log("%4d: Buffer(%p)::resize(%d,%d) %s\n", __LINE__,
                 this, length.x, length.y, name);
   #endif

   // Allocate replacement buffer
   size= length.x*length.y*sizeof(Pixel);
   if( size == 0 )
     return "BufferSizeException";
   pixel= (Pixel*)malloc(size);
   if( pixel == NULL )
     return "NoStorageException";

   if( this->pixel != NULL )
     free(this->pixel);

   this->pixel= pixel;
   this->length= length;

   // Redraw the buffer
   memset((char*)pixel, 0, size);
   RenderVisitor visitor;
   visit(visitor);

   // Expose the change
   change();

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Buffer::upload
//
// Purpose-
//       Upload the Buffer.
//
//----------------------------------------------------------------------------
void
   Buffer::upload( void )           // Upload the Buffer
{
   #ifdef HCDM
     Logger::log("%4d: Buffer(%p)::upload() %s\n", __LINE__, this, getName());
   #endif

   if( getAttribute(VISIBLE) )
   {
     int transparent= getAttribute(TRANSPARENT);

     XYOffset offset;
     XYLength length;
     Buffer* target= this->range(offset, length); // Get visible range
     if( target != NULL )
     {
       for(Offset_t y= 0; y< length.y; y++)
       {
         Pixel* sPixel= this->getPixel(0, y);
         Pixel* tPixel= target->getPixel(offset.x, offset.y + y);
         for(Offset_t x= 0; x< length.x; x++)
         {
           if( transparent )
           {
             Color_t color= sPixel->getColor();
             if( (color & 0xff000000) != 0 )
               tPixel->setColor(color);
           }
           else
             tPixel->setColor(sPixel->getColor());
           sPixel++;
           tPixel++;
         }
       }

////// upload(target); // (NOT recursively)
     }
   }
}

