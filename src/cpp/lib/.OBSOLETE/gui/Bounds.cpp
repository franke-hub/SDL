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
//       Bounds.cpp
//
// Purpose-
//       Graphical User Interface: Bounds implementation
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

#include "gui/Bounds.h"
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
static const XYLength  unitLength= {1,1}; // Unit length
static const XYLength  zeroLength= {0,0}; // Zero length

//----------------------------------------------------------------------------
//
// Method-
//       Bounds::~Bounds
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Bounds::~Bounds( void )          // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Bounds(%p)::~Bounds() %s\n", __LINE__, this, name);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Bounds::Bounds
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Bounds::Bounds(                  // Constructor
     Object*           parent)      // -> Parent Object
:  Object(parent)
,  offset(zeroOffset)
,  length(zeroLength)
{
   #ifdef HCDM
     Logger::log("%4d: Bounds(%p)::Bounds(%p)\n", __LINE__, this, parent);
   #endif
}

   Bounds::Bounds(                  // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset)      // Bounds offset
:  Object(parent)
,  offset(offset)
,  length(zeroLength)
{
   #ifdef HCDM
     Logger::log("%4d: Bounds(%p)::Bounds(%p,O{%d,%d})\n",
                 __LINE__, this, parent, offset.x, offset.y);
   #endif
}

   Bounds::Bounds(                  // Constructor
     Object*           parent,      // -> Parent Object
     const XYLength&   length)      // Bounds length
:  Object(parent)
,  offset(zeroOffset)
,  length(length)
{
   #ifdef HCDM
     Logger::log("%4d: Bounds(%p)::Bounds(%p,L{%d,%d})\n",
                 __LINE__, this, parent, length.x, length.y);
   #endif
}

   Bounds::Bounds(                  // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset,      // Bounds offset
     const XYLength&   length)      // Bounds length
:  Object(parent)
,  offset(offset)
,  length(length)
{
   #ifdef HCDM
     Logger::log("%4d: Bounds(%p)::Bounds(%p,{%d,%d},{%d,%d})\n", __LINE__,
                 this, parent, offset.x, offset.y, length.x, length.y);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Bounds::move
//
// Purpose-
//       Move (relative to parent)
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   Bounds::move(                    // Move this Bounds
     const XYOffset&   offset)      // Offset (from parent)
{
   #ifdef HCDM
     Logger::log("%4d: Bounds(%p)::move(%d,%d) %s\n", __LINE__,
                 this, offset.x, offset.y, name);
   #endif

   this->offset= offset;            // No checking is required!
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Bounds::resize
//
// Purpose-
//       Resize the Bounds
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   Bounds::resize(                  // Resize the Bounds
     const XYLength&   length)      // Resize length
{
   #ifdef HCDM
     Logger::log("%4d: Bounds(%p)::resize(%d,%d) %s\n", __LINE__, this,
                 length.x, length.y, name);
   #endif

   this->length= length;            // No checking is required!
   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Bounds::change
//
// Purpose-
//       Change the Bounds, reflecting the change upwards in the tree
//
//----------------------------------------------------------------------------
void
   Bounds::change(                  // Change the Bounds
     const XYOffset&   inpoff,      // Offset (in Object)
     const XYLength&   inplen) const// Length (in Object)
{
   #ifdef HCDM
     Logger::log("%4d: Bounds(%p)::change({%d,%d},{%d,%d}) %s\n", __LINE__,
                 this, inpoff.x, inpoff.y, inplen.x, inplen.y, name);
   #endif

   //-------------------------------------------------------------------------
   // First we have to insure that the specified change is within our
   // boundaries. If either offset axis is greater than our corresponding
   // length, the change is outside our boundaries and has no effect.
   if( inpoff.x < this->length.x
       && inpoff.y < this->length.y )
   {
     //-----------------------------------------------------------------------
     // We limit the length of the change to remain within our boundaries
     XYLength length= inplen;
     if( (inpoff.x + inplen.x) > this->length.x )
       length.x= this->length.x - offset.x;
     if( (inpoff.y + inplen.y) > this->length.y )
       length.y= this->length.y - offset.y;

     XYOffset offset= inpoff;
     offset.x += this->offset.x;
     offset.y += this->offset.y;

     Object* parent= getParent();
     if( parent != NULL )
       parent->change(offset, length);
   }
}

void
   Bounds::change( void ) const     // Change the Bounds
{
   #ifdef HCDM
     Logger::log("%4d: Bounds(%p)::change() %s\n", __LINE__, this, name);
   #endif

   change(zeroOffset, length);
}

//----------------------------------------------------------------------------
//
// Method-
//       Bounds::redraw
//
// Purpose-
//       Redraw the Bounds, reflecting the redraw upwards in the tree
//
//----------------------------------------------------------------------------
void
   Bounds::redraw(                  // Redraw the Bounds
     const XYOffset&   inpoff,      // Offset (in Object)
     const XYLength&   inplen)      // Length (in Object)
{
   #ifdef HCDM
     Logger::log("%4d: Bounds(%p)::redraw({%d,%d},{%d,%d}) %s\n", __LINE__,
                 this, inpoff.x, inpoff.y, inplen.x, inplen.y, name);
   #endif

   //-------------------------------------------------------------------------
   // First we have to insure that the specified redraw is within our
   // boundaries. If either offset axis is greater than our corresponding
   // length, the redraw is outside our boundaries and has no effect.
   if( inpoff.x < this->length.x
       && inpoff.y < this->length.y )
   {
     //-----------------------------------------------------------------------
     // We limit the length of the redraw to remain within our boundaries
     XYLength length= inplen;
     if( (inpoff.x + inplen.x) > this->length.x )
       length.x= this->length.x - offset.x;
     if( (inpoff.y + inplen.y) > this->length.y )
       length.y= this->length.y - offset.y;

     XYOffset offset= inpoff;
     offset.x += this->offset.x;
     offset.y += this->offset.y;

     Object* parent= getParent();
     if( parent != NULL )
       parent->redraw(offset, length);
   }
}

void
   Bounds::redraw( void )           // Redraw the Bounds
{
   #ifdef HCDM
     Logger::log("%4d: Bounds(%p)::redraw() %s\n", __LINE__, this, name);
   #endif

   redraw(zeroOffset, length);
}

//----------------------------------------------------------------------------
//
// Method-
//       Bounds::visit
//
// Purpose-
//       Visit this and all child Objects
//
//----------------------------------------------------------------------------
void
   Bounds::visit(                   // Visit the Object tree using
     ObjectVisitor&    visitor)     // This Visitor
{
   Object::visit(visitor);
}

Object*                             // The last Object visited
   Bounds::visit(                   // Visit the Object tree using
     ObjectVisitor&    visitor,     // This Visitor within
     const XYOffset&   offset,      // This offset and
     const XYLength&   length)      // This length
{
   Object* result= NULL;            // No winner yet
   if( length.x > 0 && length.y > 0
     && (offset.x + length.x) > this->offset.x
     && (offset.y + length.y) > this->offset.y
     && offset.x < (this->offset.x + this->length.x)
     && offset.y < (this->offset.y + this->length.y) )
   {
     // This Bounds is within range. Visit it
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

         if( (remoff.x + remlen.x) > this->length.x )
           remlen.x= this->length.x - remoff.x;
         if( (remoff.y + remlen.y) > this->length.y )
           remlen.y= this->length.y - remoff.y;

         while( object != NULL )
         {
           Object* found= object->visit(visitor, remoff, remlen);
           if( found != NULL )
             result= found;
           object= object->getPeer();
         }
       }

       //---------------------------------------------------------------------
       // As a special case for Buffer objects and RenderVisitor visitors,
       // we upload the buffer after the subtree is rendered
       Buffer* buffer= dynamic_cast<Buffer*>(this);
       if( buffer != NULL && dynamic_cast<RenderVisitor*>(&visitor) != NULL )
         buffer->upload();
     }
   }

   #if defined(HCDM) && TRUE
     Logger::log("%4d: (%p=%s)= Bounds(%p)::visit({%d,%d},{%d,%d}) %s\n",
                 __LINE__, result, result == NULL ? "NONE" : result->getName(),
                 this, offset.x, offset.y, length.x, length.y, name);
   #endif

   return result;
}

