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
//       Text.cpp
//
// Purpose-
//       Graphical User Interface: Text implementation
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

#include "gui/Types.h"
#include "gui/Buffer.h"
#include "gui/Font.h"

#include "gui/Text.h"
using namespace GUI;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Text::~Text
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Text::~Text( void )              // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Text(%p)::~Text() %s\n", __LINE__, this, name);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Text::Text
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
void
   Text::setDefaults( void )
{
   color= RGB::White;
}

   Text::Text(                      // Constructor
     Object*           parent)      // -> Parent Object
:  Bounds(parent)
,  font(NULL), text(""), mode()
{
   #ifdef HCDM
     Logger::log("%4d: Text(%p)::Text(%p)\n", __LINE__, this, parent);
   #endif

   setDefaults();
}

   Text::Text(                      // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset)      // Text offset
:  Bounds(parent, offset)
,  font(NULL), text(""), mode()
{
   #ifdef HCDM
     Logger::log("%4d: Text(%p)::Text(%p,O{%d,%d})\n",
                 __LINE__, this, parent, offset.x, offset.y);
   #endif

   setDefaults();
}

   Text::Text(                      // Constructor
     Object*           parent,      // -> Parent Object
     const XYLength&   length)      // Text length
:  Bounds(parent, length)
,  font(NULL), text(""), mode()
{
   #ifdef HCDM
     Logger::log("%4d: Text(%p)::Text(%p,L{%d,%d})\n",
                 __LINE__, this, parent, length.x, length.y);
   #endif

   setDefaults();
}

   Text::Text(                      // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset,      // Text offset
     const XYLength&   length)      // Text length
:  Bounds(parent, offset, length)
,  font(NULL), text(""), mode()
{
   #ifdef HCDM
     Logger::log("%4d: Text(%p)::Text(%p,{%d,%d},{%d,%d})\n", __LINE__,
                 this, parent, offset.x, offset.y, length.x, length.y);
   #endif

   setDefaults();
}

//----------------------------------------------------------------------------
//
// Method-
//       Text::setFont
//
// Purpose-
//       Change the Font
//
//----------------------------------------------------------------------------
Font*                               // The prior Font
   Text::setFont(                   // Set associated Font
     Font*             font)        // To this Font
{
   #ifdef HCDM
     Logger::log("%4d: Text(%p)::setFont(%p) %s\n", __LINE__, this, font, name);
   #endif

   Font* result= this->font;
   this->font= font;
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Text::setText
//
// Purpose-
//       Change the Text
//
//----------------------------------------------------------------------------
void
   Text::setText(                   // Set associated text
     const char*       text)        // To this text
{
   #ifdef HCDM
     Logger::log("%4d: Text(%p)::setText(%s) %s\n", __LINE__, this,
                 text, name);
   #endif

   this->text= text;
}

void
   Text::setText(                   // Set associated text
     const std::string&text)        // To this text
{
   #ifdef HCDM
     Logger::log("%4d: Text(%p)::setText(%s) %s\n", __LINE__, this,
                 text.c_str(), name);
   #endif

   this->text= text;
}

//----------------------------------------------------------------------------
//
// Method-
//       Text::render
//
// Purpose-
//       Render the Text
//
//----------------------------------------------------------------------------
void
   Text::render( void )             // Render the Text
{
   #ifdef HCDM
     Logger::log("%4d: Text(%p)::render() %s\n", __LINE__, this, name);
   #endif

   if( getAttribute(VISIBLE) )
   {
     XYOffset offset;               // Offset (in target)
     XYLength length;               // Length (in target) that's visible
     Buffer* buffer= range(offset, length); // Get visible range
     if( !getAttribute(TRANSPARENT) )
     {
       for(Offset_t y= 0; y < length.y; y++)
       {
         Pixel* pixel= buffer->getPixel(offset.x, offset.y + y);
         for(Offset_t x= 0; x < length.x; x++)
         {
           pixel->setColor(color);
           pixel++;
         }
       }
     }

     font->render(buffer, offset, length, text, mode.getMode());
   }
}

