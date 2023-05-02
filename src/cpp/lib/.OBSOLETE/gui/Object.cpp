//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Object.cpp
//
// Purpose-
//       Graphical User Interface: Object implementation
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <com/Logger.h>

#include "gui/Types.h"
#include "gui/Action.h"
#include "gui/Bounds.h"
#include "gui/Buffer.h"
#include "gui/Offset.h"
#include "gui/Window.h"

#include "gui/Object.h"
using namespace GUI;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Internal constants
//----------------------------------------------------------------------------
static const XYLength  unitLength= {1,1}; // Unit length

static const char*     CORRUPT_TREE=   "Corrupt tree";
static const char*     PARENT_NOTNULL= "Parent not NULL";
static const char*     PARENT_IS_NULL= "Parent is NULL";
static const char*     PARENTS_DIFFER= "Parents differ";
static const char*     THIS_ARGUMENT=  "this==argument";

//----------------------------------------------------------------------------
//
// Method-
//       Attributes::~Attributes
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Attributes::~Attributes( void )  // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Attributes::Attributes
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Attributes::Attributes(          // Constructor
     unsigned long     attributes)  // Initial attributes
:  attributes(attributes)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Attributes::getAttribute
//
// Purpose-
//       Get (Boolean) attribute.
//
//----------------------------------------------------------------------------
int                                 // The attribute
   Attributes::getAttribute(        // Get Attribute
     int               attribute) const // Attribute identifier
{
   int index= 1 << attribute;
   int result= (this->attributes&index) != 0;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Attributes::setAttribute
//
// Purpose-
//       Set attribute.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   Attributes::setAttribute(        // Set Attribute
     int               attribute,   // Attribute identifier
     int               value)       // Attribute value
{
   int index= 1 << attribute;
   if( value )
     this->attributes |= index;
   else
     this->attributes &= (~index);

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::~Object
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Object::~Object( void )          // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::~Object() %s\n", __LINE__, this, name);
   #endif

   // Remove this Object from its parent chain
   if( parent != NULL )
   {
     parent->remove(this);
     parent= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::Object
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Object::Object(                  // Constructor
     Object*           parent)      // -> Parent Object
:  Attributes(1 << VISIBLE)
,  parent(parent)
,  peer(NULL)
,  child(NULL)
,  action(NULL)
,  color(0)
,  name("Object")
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::Object(%p)\n", __LINE__, this, parent);
   #endif

   if( parent != NULL )
   {
     peer= parent->child;
     parent->child= this;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::getBuffer
//
// Purpose-
//       Get containing Buffer*
//
//----------------------------------------------------------------------------
Buffer*                             // The containing Buffer
   Object::getBuffer( void ) const  // Get containing Buffer
{
   Buffer* buffer= NULL;
   Object* object= getParent();
   while( object != NULL )
   {
     buffer= dynamic_cast<Buffer*>(object);
     if( buffer != NULL )
       break;

     object= object->getParent();
   }

   return buffer;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::getPixel
//
// Purpose-
//       Get Pixel* (for x/y offset)
//
//----------------------------------------------------------------------------
Pixel*                              // The Pixel*
   Object::getPixel(                // Get Pixel at
     XOffset_t         x,           // X (horizontal) offset
     YOffset_t         y) const     // Y (vertical) offset
{
   Pixel* pixel= NULL;              // Resultant
   Object* object= getParent();
   while( object != NULL )
   {
     Buffer* buffer= dynamic_cast<Buffer*>(object);
     if( buffer != NULL )
     {
       pixel= buffer->getPixel(x, y);
       break;
     }

     Bounds* bounds= dynamic_cast<Bounds*>(object);
     if( bounds != NULL )
     {
       if( x >= bounds->getLength().x
           || y >= bounds->getLength().y )
         return NULL;

       x += bounds->getOffset().x;
       y += bounds->getOffset().y;
     }

     object= object->getParent();
   }

   return pixel;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::getWindow
//
// Purpose-
//       Get containing Window*
//
//----------------------------------------------------------------------------
Window*                             // The containing Window
   Object::getWindow( void ) const  // Get containing Window
{
   const Window* window= NULL;
   const Object* object= this;
   while( object != NULL )
   {
     window= dynamic_cast<const Window*>(object);
     if( window != NULL )
       break;

     object= object->getParent();
   }

   return (Window*)window;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::setColor
//
// Purpose-
//       Set (background) Color.
//
//----------------------------------------------------------------------------
void
   Object::setColor(                // Set the Object's Color
     Color_t           color)       // To this Color
{
   this->color= color;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::setPixel
//
// Purpose-
//       Get Pixel*, set it's Color.
//
//----------------------------------------------------------------------------
Pixel*                              // The Pixel*
   Object::setPixel(                // Set Pixel at
     XOffset_t         x,           // X (horizontal) offset
     YOffset_t         y,           // Y (vertical) offset
     Color_t           color)       // To this Color
{
   Pixel* pixel= (Pixel*)getPixel(x, y);
   if( pixel != NULL )
     pixel->setColor(color);

   return pixel;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::addAction
//
// Purpose-
//       Add an Action item to the list.
//
//----------------------------------------------------------------------------
void
   Object::addAction(               // Add Action item
     Action*           action)      // The Action item
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::addAction(%p) %s\n", __LINE__,
                 this, action, name);
   #endif

   if( action->parent == NULL )
   {
     action->parent= this;
     action->next= this->action;
     this->action= action;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::delAction
//
// Purpose-
//       Remove an Action item from the list.
//
//----------------------------------------------------------------------------
void
   Object::delAction(               // Delete Action item
     Action*           action)      // The Action item
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::delAction(%p) %s\n", __LINE__,
                 this, action, name);
   #endif

   if( action->parent == this )
   {
     if( this->action == action )
       this->action= action->next;
     else
     {
       Action* prior= this->action;
       while( prior != NULL )
       {
         if( prior->next == action )
         {
           prior->next= action->next;
           break;
         }

         prior= prior->next;
       }
     }

     action->parent= NULL;
     action->next= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::change
//
// Purpose-
//       Reflect a change in this Object
//
//----------------------------------------------------------------------------
void
   Object::change(                  // Change the buffer
     const XYOffset&   offset,      // Offset (in Object)
     const XYLength&   length) const// Length (in Object)
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::change({%d,%d},{%d,%d}) %s\n", __LINE__,
                 this, offset.x, offset.y, length.x, length.y, name);
   #endif

   if( parent != NULL )
     parent->change(offset, length);
}

void
   Object::change( void ) const     // Reflect change upward
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::change() %s\n", __LINE__, this, name);
   #endif

   if( parent != NULL )
     parent->change();
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::range
//
// Purpose-
//       Determine the visible range of this Object
//
//----------------------------------------------------------------------------
Buffer*                             // The target Buffer
   Object::range(                   // Update the range attribute
     XYOffset&         offset,      // Visible offset in Buffer
     XYLength&         length) const// Visible length in Buffer
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::range() %s\n", __LINE__, this, name);
   #endif

   offset.x= offset.y= 0;             // Default resultant
   length.x= length.y= 0;
// Buffer* buffer= buffer= getBuffer(); // Get associated Buffer (Weird code)
   Buffer* buffer= getBuffer(); // Get associated Buffer
   if( buffer != NULL )             // If a Buffer exists
   {
     Offset_t xOffset= 0;
     Offset_t yOffset= 0;
     Length_t xLength= buffer->getLength().x;
     Length_t yLength= buffer->getLength().y;

     //-----------------------------------------------------------------------
     // Find first Bounds
     const Object* object= this;
     while( object != buffer )
     {
       const Bounds* bounds= dynamic_cast<const Bounds*>(object);
       if( bounds != NULL )
       {
         xLength= bounds->getLength().x;
         yLength= bounds->getLength().y;
         break;
       }
       else
       {
         const Offset* offset= dynamic_cast<const Offset*>(object);
         if( offset != NULL )
         {
           xOffset += offset->getOffset().x;
           yOffset += offset->getOffset().y;
         }
       }

       object= object->getParent();
     }

     while( object != buffer )
     {
       const Bounds* bounds= dynamic_cast<const Bounds*>(object);
       if( bounds != NULL )
       {
         if( xOffset >= bounds->getLength().x
             || yOffset >= bounds->getLength().y )
           return NULL;

         if( (xOffset + xLength) > bounds->getLength().x )
           xLength= bounds->getLength().x - xOffset;
         if( (yOffset + yLength) > bounds->getLength().y )
           yLength= bounds->getLength().y - xOffset;

         xOffset += bounds->getOffset().x;
         yOffset += bounds->getOffset().y;
       }
       else
       {
         const Offset* offset= dynamic_cast<const Offset*>(object);
         if( offset != NULL )
         {
           xOffset += offset->getOffset().x;
           yOffset += offset->getOffset().y;
         }
       }

       object= object->getParent();
     }

     //-----------------------------------------------------------------------
     // Now the offset is relative to the buffer origin
     if( xOffset >= buffer->getLength().x
         || yOffset >= buffer->getLength().y ) // If out of range
       return NULL;

     if( (xOffset + xLength) > buffer->getLength().x )
       xLength= buffer->getLength().x - xOffset;
     if( (yOffset + yLength) > buffer->getLength().y )
       yLength= buffer->getLength().y - xOffset;

     // Set return values
     offset.x= xOffset;
     offset.y= yOffset;
     length.x= xLength;
     length.y= yLength;
   }

   return buffer;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::redraw
//
// Purpose-
//       Reflect a redraw of this Object
//
//----------------------------------------------------------------------------
void
   Object::redraw(                  // Redraw the Object
     const XYOffset&   offset,      // Offset (in Object)
     const XYLength&   length)      // Length (in Object)
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::redraw({%d,%d},{%d,%d}) %s\n", __LINE__,
                 this, offset.x, offset.y, length.x, length.y, name);
   #endif

   if( parent != NULL )
     parent->redraw(offset, length);
}

void
   Object::redraw( void )           // Redraw the Object
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::redraw() %s\n", __LINE__, this, name);
   #endif

   if( parent != NULL )
     parent->redraw();
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::visit
//
// Purpose-
//       Visit this and all child Objects
//
//----------------------------------------------------------------------------
void
   Object::visit(                   // Visit the Object tree using
     ObjectVisitor&    visitor)     // This Visitor
{
   Object* object= visitor.visit(this); // Visit this Object
   if( object != NULL )
   {
     Object* object= getChild();    // Visit all child Objects
     if( object != NULL )           // If children exist
     {
       object->visit(visitor);      // Visit the child, pushing the stack
       object= object->getPeer();   // Visit all the child's peers
       while( object != NULL )
       {
         object->visit(visitor);
         object= object->getPeer();
       }
     }

     //-----------------------------------------------------------------------
     // As a special case for Buffer objects and RenderVisitor visitors,
     // we upload the buffer after the subtree is rendered
     Buffer* buffer= dynamic_cast<Buffer*>(this);
     if( buffer != NULL && dynamic_cast<RenderVisitor*>(&visitor) != NULL )
       buffer->upload();
   }

   #if defined(HCDM) && TRUE
     Logger::log("%4d: Object(%p)::visit(%p)\n", __LINE__, this, &visitor);
   #endif
}

Object*                             // The last Object visited
   Object::visit(                   // Visit the Object tree using
     ObjectVisitor&    visitor,     // This Visitor for
     const XYOffset&   offset,      // This offset and
     const XYLength&   length)      // This length
{
   Object* result= NULL;            // No winner yet
   Object* object= visitor.visit(this); // Visit this Object
   if( object != NULL )             // If this Object is acceptable
     object= child;                 // Start with the child Object

   while( object != NULL )
   {
     Object* found= object->visit(visitor, offset, length);
     if( found != NULL )
       result= found;
     object= object->peer;
   }

   #if defined(HCDM) && TRUE
     Logger::log("%4d: (%p=%s)= Object(%p)::visit({%d,%d},{%d,%d}) %s\n",
                 __LINE__, result, result == NULL ? "NONE" : result->getName(),
                 this, offset.x, offset.y, length.x, length.y, name);
   #endif

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::render()
//
// Purpose-
//       Render this Object
//
//----------------------------------------------------------------------------
void
   Object::render( void )           // Object (redraw) this Object
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::render() %s\n", __LINE__, this, name);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::prior
//
// Purpose-
//       Address the prior Object in the parent's child list.
//
// Implementation notes-
//       This is a private method. Error checking is not required.
//       Note that this method finds the prior pointer when this element
//       is at the head of the chain by completely scanning the chain.
//
//----------------------------------------------------------------------------
Object*                             // -> Prior Object
   Object::prior( void ) const      // Address prior Object
{
   Object* result= parent->child;
   while( result != NULL )
   {
     if( result->peer == this )
       break;

     result= result->peer;
   }

   #ifdef HCDM
     Logger::log("%4d: %p(%s)= Object(%p)::prior() %s\n", __LINE__,
                 result, (result==NULL) ? "HEAD" : result->name, this, name);
   #endif

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::insert
//
// Purpose-
//       Insert an Object onto our child list
//
// Usage notes-
//       DOES NOT MODIFY or REFERENCE object->child.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   Object::insert(                  // Insert a child Object
     Object*           object)      // The Object to insert
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::insert(%p(%s)) %s\n", __LINE__,
                 this, object, (object==NULL) ? "NULL" : object->name, name);
   #endif

   if( object->parent != NULL )     // If it's already on some list
     return PARENT_NOTNULL;

   object->parent= this;
   object->peer= child;
   child= object;

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::lower
//
// Purpose-
//       Change the priority of this Object
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   Object::lower(                   // Lower Object priority
     Object*           object)      // Below this Object
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::lower(%p(%s)) %s\n", __LINE__,
                 this, object, (object==NULL) ? "NULL" : object->name, name);
   #endif

   if( parent == NULL )
     return PARENT_IS_NULL;

   if( parent != object->parent )
     return PARENTS_DIFFER;

   if( this == object )
     return THIS_ARGUMENT;

   // Checking complete
   Object* ptrThis= prior();        // Remove this from parent chain
   if( ptrThis == NULL )
     parent->child= peer;
   else
     ptrThis->peer= peer;

   Object* ptrThat= object->prior(); // Insert this onto parent chain
   peer= object;                    // We are BEFORE the object
   if( ptrThat == NULL )
     parent->child= this;
   else
     ptrThat->peer= this;

   return NULL;
}

const char*                         // Exception message (NULL OK)
   Object::lower( void )            // Lower Object priority (to lowest)
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::lower() %s\n", __LINE__, this, name);
   #endif

   if( parent == NULL )
     return PARENT_IS_NULL;

   // Checking complete
   Object* ptrThis= prior();
   if( ptrThis != NULL )            // If not already at head of chain
   {
     ptrThis->peer= peer;           // Remove this from the chain
     peer= parent->child;           // Insert this at head of chain
     parent->child= this;
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::raise
//
// Purpose-
//       Change the priority of this Object
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   Object::raise(                   // Raise Object priority
     Object*           object)      // Above this Object
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::raise(%p(%s)) %s\n", __LINE__,
                 this, object, (object==NULL) ? "NULL" : object->name, name);
   #endif

   if( parent == NULL )
     return PARENT_IS_NULL;

   if( parent != object->parent )
     return PARENTS_DIFFER;

   if( this == object )
     return THIS_ARGUMENT;

   // Checking complete
   Object* ptrThis= prior();        // Remove this from parent chain
   if( ptrThis == NULL )
     parent->child= peer;
   else
     ptrThis->peer= peer;

   peer= object->peer;              // Insert into parent chain
   object->peer= this;              // We are AFTER the object

   return NULL;
}

const char*                         // Exception message (NULL OK)
   Object::raise( void )            // Raise Object priority (to highest)
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::raise() %s\n", __LINE__, this, name);
   #endif

   if( parent == NULL )
     return PARENT_IS_NULL;

   // Checking complete
   if( peer != NULL  )              // If not already at tail of parent chain
   {
     Object* ptrThis= prior();      // Remove this from parent chain
     if( ptrThis == NULL )
       parent->child= peer;
     else
       ptrThis->peer= peer;

     // Insert this at tail of parent chain
     Object* object= parent->child; // (Cannot be NULL in this sequence)
     for(;;)
     {
       if( object->peer == NULL )
         break;

       object= object->peer;
     }

     peer= NULL;                    // Insert at tail of parent chain
     object->peer= this;
   }

   return NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Object::remove
//
// Purpose-
//       Remove an Object from this Object's child list
//
// Usage notes-
//       DOES NOT MODIFY or REFERENCE object->child.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   Object::remove(                  // Remove a child Object
     Object*           object)      // The Object to remove
{
   #ifdef HCDM
     Logger::log("%4d: Object(%p)::remove(%p(%s)) %s\n", __LINE__,
                 this, object, (object==NULL) ? "NULL" : object->name, name);
   #endif

   if( object->parent != this )
     return PARENTS_DIFFER;

   Object* ptrThat= object->prior();
   if( ptrThat == NULL && child != object )
     return CORRUPT_TREE;

   if( ptrThat == NULL )
     child= object->peer;
   else
     ptrThat->peer= object->peer;

   object->parent= NULL;
   object->peer= NULL;

   return NULL;
}

