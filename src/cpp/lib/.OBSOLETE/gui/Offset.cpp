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
//       Offset.cpp
//
// Purpose-
//       Graphical User Interface: Offset implementation
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

#include "gui/Offset.h"
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
static const XYOffset  zeroOffset= {0,0}; // Zero offset

//----------------------------------------------------------------------------
//
// Method-
//       Offset::~Offset
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Offset::~Offset( void )          // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Offset(%p)::~Offset() %s\n", __LINE__, this, name);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Offset::Offset
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Offset::Offset(                  // Constructor
     Object*           parent)      // -> Parent Object
:  Object(parent)
,  offset(zeroOffset)
{
   #ifdef HCDM
     Logger::log("%4d: Offset(%p)::Offset(%p)\n", __LINE__, this, parent);
   #endif
}

   Offset::Offset(                  // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset)      // Bounds offset
:  Object(parent)
,  offset(offset)
{
   #ifdef HCDM
     Logger::log("%4d: Offset(%p)::Offset(%p,O{%d,%d})\n",
                 __LINE__, this, parent, offset.x, offset.y);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Offset::change
//
// Purpose-
//       Change the Offset, reflecting the change upwards in the tree
//
//----------------------------------------------------------------------------
void
   Offset::change(                  // Change the Offset
     const XYOffset&   inpoff,      // Offset (in Object)
     const XYLength&   inplen) const// Length (in Object)
{
   #ifdef HCDM
     Logger::log("%4d: Offset(%p)::change({%d,%d},{%d,%d}) %s\n", __LINE__,
                 this, inpoff.x, inpoff.y, inplen.x, inplen.y, name);
   #endif

   //-------------------------------------------------------------------------
   // Since the Offset object has unbounded length, we do not need to check
   // or limit the length. We simply modify the offset and pass the request
   // up the tree.
   XYOffset offset= inpoff;
   offset.x += this->offset.x;
   offset.y += this->offset.y;

   Object* parent= getParent();
   if( parent != NULL )
     parent->change(offset, inplen);
}

void
   Offset::change( void ) const     // Change the Offset
{
   #ifdef HCDM
     Logger::log("%4d: Offset(%p)::change() %s\n", __LINE__, this, name);
   #endif

   //-------------------------------------------------------------------------
   // Since the Offset object has no length, we take the length of our Buffer
   Buffer* buffer= getBuffer();
   if( buffer != NULL )
     change(zeroOffset, buffer->getLength());
}

//----------------------------------------------------------------------------
//
// Method-
//       Offset::redraw
//
// Purpose-
//       Redraw the Offset, reflecting the redraw upwards in the tree
//
//----------------------------------------------------------------------------
void
   Offset::redraw(                  // Redraw the Offset
     const XYOffset&   inpoff,      // Offset (in Object)
     const XYLength&   inplen)      // Length (in Object)
{
   #ifdef HCDM
     Logger::log("%4d: Offset(%p)::redraw({%d,%d},{%d,%d}) %s\n", __LINE__,
                 this, inpoff.x, inpoff.y, inplen.x, inplen.y, name);
   #endif

   //-------------------------------------------------------------------------
   // Since the Offset object has unbounded length, we do not need to check
   // or limit the length. We simply modify the offset and pass the request
   // up the tree.
   XYOffset offset= inpoff;
   offset.x += this->offset.x;
   offset.y += this->offset.y;

   Object* parent= getParent();
   if( parent != NULL )
     parent->redraw(offset, inplen);
}

void
   Offset::redraw( void )           // Redraw the Offset
{
   #ifdef HCDM
     Logger::log("%4d: Offset(%p)::redraw() %s\n", __LINE__, this, name);
   #endif

   //-------------------------------------------------------------------------
   // Since the Offset object has no length, we take the length of our Buffer
   Buffer* buffer= getBuffer();
   if( buffer != NULL )
     redraw(zeroOffset, buffer->getLength());
}

//----------------------------------------------------------------------------
//
// Method-
//       Offset::visit
//
// Purpose-
//       Visit this and all child Objects
//
//----------------------------------------------------------------------------
void
   Offset::visit(                   // Visit the Object tree using
     ObjectVisitor&    visitor)     // This Visitor
{
   Object::visit(visitor);
}

Object*                             // The last Object visited
   Offset::visit(                   // Visit the Object tree using
     ObjectVisitor&    visitor,     // This Visitor within
     const XYOffset&   offset,      // This offset and
     const XYLength&   length)      // This length
{
   Object* result= NULL;            // No winner yet
   if( length.x > 0 && length.y > 0
     && (offset.x + length.x) > this->offset.x
     && (offset.y + length.y) > this->offset.y )
   {
     // This Offset is within range. Visit it
     if( visitor.visit(this) != NULL ) // If this Window is acceptable
     {
       result= this;                // We have a winner!

       Object* object= getChild();  // Visit all child Objects
       if( object != NULL )
       {
         // Compute the remaining visible offset and length
         XYOffset remoff= offset;
         XYLength remlen= length;
         if( offset.x < this->offset.x )
         {
           remoff.x= 0;
           remlen.x -= (this->offset.x - offset.x);
         }
         else
           remoff.x -= this->offset.x;
         if( offset.y < this->offset.y )
         {
           remoff.y= 0;
           remlen.y -= (this->offset.y - offset.y);
         }
         else
           remoff.y -= this->offset.y;

         while( object != NULL )
         {
           Object* found= object->visit(visitor, remoff, remlen);
           if( found != NULL )
             result= found;
           object= object->getPeer();
         }
       }
     }
   }

   #if defined(HCDM) && TRUE
     Logger::log("%4d: (%p=%s)= Offset(%p)::visit({%d,%d},{%d,%d}) %s\n",
                 __LINE__, result, result == NULL ? "NONE" : result->getName(),
                 this, offset.x, offset.y, length.x, length.y, name);
   #endif

   return result;
}

