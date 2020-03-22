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
//       Window.cpp
//
// Purpose-
//       Graphical User Interface: Window implementation
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "gui/Types.h"
#include "gui/Action.h"
#include "gui/Event.h"
#include "gui/Buffer.h"

#include "gui/Device.h"
#include "gui/Window.h"
using namespace GUI;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifdef _OS_WIN
  #include <windows.h>
  #define sleep(x) ::Sleep(x*1000)
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const XYOffset  zeroOffset= {0,0}; // Zero offset
static const XYLength  unitLength= {1,1}; // Unit length

//----------------------------------------------------------------------------
//
// Class-
//       ActionVisitor
//
// Purpose-
//       Find an Object to handle an Action event.
//
//----------------------------------------------------------------------------
class ActionVisitor : public ObjectVisitor { // ActionVisior
public:
Object*                result;      // Resultant

   ~ActionVisitor( void ) {}
   ActionVisitor( void )
:  ObjectVisitor(), result(NULL) {}

virtual Object*                     // The visited Object
   visit(                           // Visit an Object
     Object*           object)      // The Object to visit
{
   if( object->getAction() == NULL )
   {
     if( dynamic_cast<Bounds*>(object) != NULL )
       result= NULL;
   }
   else
     result= object;

   return object;
}
}; // class ActionVisitor

//----------------------------------------------------------------------------
//
// Method-
//       Window::~Window
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Window::~Window( void )          // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Window(%p)::~Window() %s\n", __LINE__, this, name);
   #endif

   // Delete our Device
   delete device;
   device= NULL;
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::Window
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
void
   Window::buildObject( void )      // Initialize Window
{
   #ifdef HCDM
     Logger::log("%4d: Window(%p)::buildObject()\n", __LINE__, this);
   #endif

   currentFocus= currentHover= currentMover= NULL;
   name= "Window";
   attributes= 0;
   device= Device::make(this);
}

   Window::Window( void )           // Constructor
:  Buffer(NULL)
{
   #ifdef HCDM
     Logger::log("%4d: Window(%p)::Window()\n", __LINE__, this);
   #endif

   buildObject();
}

   Window::Window(                  // Constructor
     const XYOffset&   offset)      // Object offset
:  Buffer(NULL, offset)
{
   #ifdef HCDM
     Logger::log("%4d: Window(%p)::Window(O{%d,%d})\n",
                 __LINE__, this, offset.x, offset.y);
   #endif

   buildObject();
}

   Window::Window(                  // Constructor
     const XYLength&   length)      // Object length
:  Buffer(NULL, length)
{
   #ifdef HCDM
     Logger::log("%4d: Window(%p)::Window(L{%d,%d})\n",
                 __LINE__, this, length.x, length.y);
   #endif

   buildObject();
}

   Window::Window(                  // Constructor
     const XYOffset&   offset,      // Object offset
     const XYLength&   length)      // Object length
:  Buffer(NULL, offset, length)
{
   #ifdef HCDM
     Logger::log("%4d: Window(%p)::Window({%d,%d},{%d,%d})\n", __LINE__,
                 this, offset.x, offset.y, length.x, length.y);
   #endif

   buildObject();
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::setAttribute
//
// Purpose-
//       Change attribute.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   Window::setAttribute(            // Set Attribute
     Attribute         attribute,   // Attribute identifier
     int               value)       // Attribute value
{
   #if defined(HCDM)
     Logger::log("%4d: Window(%p)::setAttribute(%d) %s\n", __LINE__,
                 this, attribute, name);
   #endif

   Attributes::setAttribute(attribute, value);
   return device->setAttribute(attribute, value);
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::getFocus
//       Window::getHover
//       Window::getMover
//
//       Window::setFocus
//       Window::setHover
//       Window::setMover
//
// Purpose-
//       Accessor methods.
//
//----------------------------------------------------------------------------
Object*                             // The current Focus Object
   Window::getFocus( void ) const   // Get current Focus Object
{  return currentFocus;
}

Object*                             // The current Hover Object
   Window::getHover( void ) const   // Get current Hover Object
{  return currentHover;
}

Object*                             // The current Mover Object
   Window::getMover( void ) const   // Get current Mover Object
{  return currentMover;
}

void
   Window::setFocus(                // Set current Focus Object
     Object*           object)      // To this Object
{  currentFocus= object;
}

void
   Window::setHover(                // Set current Hover Object
     Object*           object)      // To this Object
{  currentHover= object;
}

void
   Window::setMover(                // Set current Mover Object
     Object*           object)      // To this Object
{  currentMover= object;
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::change
//
// Purpose-
//       Reflect a change in this Window
//
//----------------------------------------------------------------------------
void
   Window::change(                  // Reflect change
     const XYOffset&   inpoff,      // Offset (in Window)
     const XYLength&   inplen) const// Length (in Window)
{
   #ifdef HCDM
     Logger::log("%4d: Window(%p)::change({%d,%d},{%d,%d}) %s\n", __LINE__,
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
       length.x= this->length.x - inpoff.x;
     if( (inpoff.y + inplen.y) > this->length.y )
       length.y= this->length.y - inpoff.y;

     device->change(inpoff, length);
   }
}

void
   Window::change( void ) const     // Reflect change
{
   XYOffset offset= {0,0};
   change(offset, length);
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::move
//
// Purpose-
//       Move the Window
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   Window::move(                    // Reposition the Window
     const XYOffset&   offset)      // Offset (from parent)
{
   #ifdef HCDM
     Logger::log("%4d: Window(%p)::move(%d,%d) %s\n",
                 __LINE__, this, offset.x, offset.y, name);
   #endif

   this->offset= offset;
   return device->move(offset);
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::redraw
//
// Purpose-
//       Redraw this Window.
//
//----------------------------------------------------------------------------
void
   Window::redraw(                  // Redraw the Window for
     const XYOffset&   inpoff,      // Offset (in Window) and
     const XYLength&   inplen)      // Length (in Window)
{
   #ifdef HCDM
     Logger::log("%4d: Window(%p)::redraw({%d,%d},{%d,%d}) %s\n", __LINE__,
                 this, inpoff.x, inpoff.y, inplen.x, inplen.y, name);
   #endif

   // Render this Window
   RenderVisitor visitor;
   visit(visitor, inpoff, inplen);

   // Expose the change
   change(inpoff, inplen);
}

void
   Window::redraw( void )           // Redraw the Window
{
   redraw(zeroOffset, length);
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::resize
//
// Purpose-
//       Resize the Window
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   Window::resize(                  // Resize the Window
     const XYLength&   length)      // Length
{
   #ifdef HCDM
     Logger::log("%4d: Window(%p)::resize(%d,%d) %s\n",
                 __LINE__, this, length.x, length.y, name);
   #endif

   const char* result= Buffer::resize(length);
   if( result == NULL )
     result= device->resize(length);

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::visit
//
// Purpose-
//       Visit this and all child Objects
//
//----------------------------------------------------------------------------
void
   Window::visit(                   // Visit the object tree using
     ObjectVisitor&    visitor)     // This Visitor
{
   Bounds::visit(visitor);          // Visit this Object
}

Object*                             // The last Object visited
   Window::visit(                   // Visit the Object tree using
     ObjectVisitor&    visitor,     // This Visitor for
     const XYOffset&   offset,      // This offset and
     const XYLength&   length)      // This length
{
   Object* result= NULL;            // No winner yet
   if( length.x > 0 && length.y > 0
     && offset.x < this->length.x && offset.y < this->length.y )
   {
     // This Window is within range. Visit it
     if( visitor.visit(this) != NULL ) // If this Window is acceptable
     {
       result= this;                // We have a winner!

       Object* object= getChild();  // Visit all child Objects
       if( object != NULL )
       {
         // Compute the remaining visible offset and length
         XYOffset remoff= offset;
         XYLength remlen= length;
         if( (remoff.x + remlen.x) > this->length.x )
           remlen.x= this->length.x - remoff.x;
         if( (remoff.y + remlen.y) > this->length.y )
           remlen.y= this->length.y - remoff.y;

         Object* found= object->visit(visitor, remoff, remlen);
         if( found != NULL )
           result= found;
         object= object->getPeer();
         while( object != NULL )
         {
           found= object->visit(visitor, remoff, remlen);
           if( found != NULL )
             result= found;
           object= object->getPeer();
         }
       }
     }
   }

   #if defined(HCDM) && TRUE
     Logger::log("%4d: (%p=%s)= Window(%p)::visit({%d,%d},{%d,%d}) %s\n",
                 __LINE__, result, result == NULL ? "NONE" : result->getName(),
                 this, offset.x, offset.y, length.x, length.y, name);
   #endif

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::wait
//
// Purpose-
//       Wait for Window events.
//
//----------------------------------------------------------------------------
void
   Window::wait( void )             // Wait for Window termination
{
   #ifdef HCDM
     Logger::log("%4d: Window(%p)::wait()\n", __LINE__, this);
   #endif

   device->wait();
}

//----------------------------------------------------------------------------
//
// Method-
//       Window::callback
//
// Purpose-
//       Window callback
//
// Implementaton notes-
//       Locates the object that should handle the event. If it is an
//       Action object, the object handles it. Otherwise, the default does.
//
//----------------------------------------------------------------------------
void
   Window::callback(                // Window callback
     const Event&      e)           // For this Event
{

   #if defined(HCDM) && FALSE
     Logger::log("%4d: Window(%p)::callback() %s\n",
                 __LINE__, this, name);
     e.debug();
   #endif

   // Locate the Object and Action associated with the Event
   ActionVisitor visitor;
   Object* object= NULL;            // The Action Object
   Action* action= NULL;            // The Action item
   int code= e.getCode();
   switch(code)
   {
     case Event::EC_KEYDOWN:
     case Event::EC_KEYUP:
       object= this;
       if( currentFocus != NULL )
         object= currentFocus;
       break;

     case Event::EC_MOUSEOVER:
       // Locate associated Object
       if( e.getData() != Event::MO_EXIT )
       {
         visit(visitor, e.getOffset(), e.getLength());
         if( visitor.result != NULL )
           object= visitor.result;
       }

       // Generate Enter/Exit events
       if( currentHover != object )
       {
         if( currentHover != NULL )
         {
           action= currentHover->getAction();
           while( action != NULL )
           {
             Event inner(Event::EC_MOUSEOVER, Event::MO_EXIT,
                         zeroOffset, unitLength);
             action->callback(inner);
             action= action->getNext();
           }
         }

         currentHover= object;
         if( currentHover != NULL )
         {
           action= currentHover->getAction();
           while( action != NULL )
           {
             Event inner(Event::EC_MOUSEOVER, Event::MO_ENTER,
                         zeroOffset, unitLength);
             action->callback(inner);
             action= action->getNext();
           }
         }
       }
       break;

     case Event::EC_MOUSEDOWN:
     case Event::EC_MOUSEUP:
       visit(visitor, e.getOffset(), e.getLength());
       if( visitor.result != NULL )
         object= visitor.result;
       break;

     default:                       // Global event
       action= getAction();
       break;
   }

   //-------------------------------------------------------------------------
   // Handle the actual Event
   if( object != NULL )
   {
     action= object->getAction();
     while( action != NULL )
     {
       action->callback(e);
       action= action->getNext();
     }
   }
}

