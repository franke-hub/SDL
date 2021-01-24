//----------------------------------------------------------------------------
//
//       Copyright (c) 2010 Frank Eskesen.
//
//       This file is free content, distributed under the MIT license.
//       (See accompanying file LICENSE.MIT or the original contained
//       within https://opensource.org/licenses/MIT)
//
//----------------------------------------------------------------------------
//
// Title-
//       Window.h
//
// Purpose-
//       Graphical User Interface: Window
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_WINDOW_H_INCLUDED
#define GUI_WINDOW_H_INCLUDED

#ifndef GUI_BUFFER_H_INCLUDED
#include "Buffer.h"
#endif

#include "namespace.gui"
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class _SystemFont;
class Device;
class Event;

//----------------------------------------------------------------------------
//
// Class-
//       Window
//
// Purpose-
//       Window descriptor.
//
// Usage notes-
//       The TRANSPARENT attribute has no effect.
//
//----------------------------------------------------------------------------
class Window : public Buffer {      // Window descriptor
//----------------------------------------------------------------------------
// Window::Attributes
//----------------------------------------------------------------------------
friend class _SystemFont;           // For device access
friend class Device;                // For callback

protected:
Device*                device;      // The associated Device
Object*                currentFocus; // The current Focus Object
Object*                currentHover; // The current Hover Object
Object*                currentMover; // The current Mover Object (drag/drop)

//----------------------------------------------------------------------------
// Window::Constructors
//----------------------------------------------------------------------------
private:
void
   buildObject( void );             // Constructor helper

public:
virtual
   ~Window( void );                 // Destructor
   Window( void );                  // Constructor

   Window(                          // Constructor
     const XYOffset&   offset);     // Object offset

   Window(                          // Constructor
     const XYLength&   length);     // Object length

   Window(                          // Constructor
     const XYOffset&   offset,      // Object offset
     const XYLength&   length);     // Object length

//----------------------------------------------------------------------------
//
// Public method-
//       Window::setAttribute
//
// Purpose-
//       Change an Attribute.
//
//----------------------------------------------------------------------------
public:
virtual const char*                 // Exception message (NULL OK)
   setAttribute(                    // Device set attribute
     Attribute         attribute,   // Attribute identifier
     int               value);      // Attribute value

//----------------------------------------------------------------------------
//
// Public method-
//       Window::getFocus           // (Keyboard handler)
//       Window::getMover           // (Drag/drop object)
//
//       Window::setFocus           // (Keyboard handler)
//       Window::setMover           // (Drag/drop object)
//
// Purpose-
//       Accessor methods.
//
//----------------------------------------------------------------------------
public:
virtual Object*                     // The current keyboard Focus Object
   getFocus( void ) const;          // Get current keyboard Focus Object

virtual Object*                     // The current mouseover Object
   getHover( void ) const;          // Get current mouseover Object

virtual Object*                     // The current keyboard Mover Object
   getMover( void ) const;          // Get current keyboard Mover Object

virtual void
   setFocus(                        // Set current keyboard Focus Object
     Object*           object);     // To this Object

virtual void
   setHover(                        // Set current mouseover Object
     Object*           object);     // To this Object

virtual void
   setMover(                        // Set current drag/drop Object
     Object*           object);     // To this Object

//----------------------------------------------------------------------------
//
// Public method-
//       Window::change
//
// Purpose-
//       Update the buffer change coordinates.
//
// Implementation notes-
//       When a Window change method is invoked, it reflects the change out
//       to its associated device. The Window itself does not change, so the
//       const attribute is appropriate for these methods.
//
//----------------------------------------------------------------------------
public:
virtual void
   change(                          // Reflect change
     const XYOffset&   offset,      // Offset
     const XYLength&   length) const; // Length

virtual void
   change( void ) const;            // Reflect change

//----------------------------------------------------------------------------
//
// Public method-
//       Window::move
//
// Purpose-
//       Reposition the Window itself.
//
//----------------------------------------------------------------------------
public:
virtual const char*                 // Exception message (NULL OK)
   move(                            // Reposition the Object
     const XYOffset&   offset);     // Offset (from parent)

//----------------------------------------------------------------------------
//
// Public method-
//       Window::redraw
//
// Purpose-
//       Redraw the Window.
//
//----------------------------------------------------------------------------
public:
virtual void
   redraw(                          // Redraw part of the Window
     const XYOffset&   offset,      // Offset
     const XYLength&   length);     // Length

virtual void
   redraw( void );                  // Redraw the Window

//----------------------------------------------------------------------------
//
// Public method-
//       Window::resize
//
// Purpose-
//       Resize the Object.
//
//----------------------------------------------------------------------------
public:
virtual const char*                 // Exception message (NULL OK)
   resize(                          // Resize the Object
     const XYLength&   length);     // Resize length

//----------------------------------------------------------------------------
//
// Public method-
//       Window::visit(ObjectVisitor&)
//
// Purpose-
//       Visit this Object and its children.
//
//----------------------------------------------------------------------------
public:
virtual void
   visit(                           // Visit the Object tree
     ObjectVisitor&    visitor);    // The ObjectVisitor

//----------------------------------------------------------------------------
//
// Public method-
//       Window::visit(ObjectVisitor&, const XYOffset&, const XYLength&)
//
// Purpose-
//       Visit this Object and all its children that are within the specified
//       boundary criteria. The resultant is the LAST Object instance on the
//       tree that satisfies the criteria.
//
// Usage note-
//       Normally, the resultant would be the visible object that completely
//       satisfied the boundary conditions. However, this method does NOT
//       examine the VISIBLE attribute, so an invisible resultant is possible.
//
//----------------------------------------------------------------------------
public:
virtual Object*                     // Resultant, or NULL if none
   visit(                           // Visit the Object tree using
     ObjectVisitor&    visitor,     // This ObjectVisitor for
     const XYOffset&   offset,      // This offset and
     const XYLength&   length);     // This length

//----------------------------------------------------------------------------
//
// Public method-
//       Window::wait
//
// Purpose-
//       Wait for Window termination.
//
//----------------------------------------------------------------------------
public:
virtual void
   wait( void );                    // Wait for Window termination

//----------------------------------------------------------------------------
//
// Protected method-
//       Window::callback
//
// Purpose-
//       Device event notification callback.
//
//----------------------------------------------------------------------------
protected:
virtual void
   callback(                        // Device callback
     const Event&      event);      // For this Event
}; // class Window
#include "namespace.end"

#endif // GUI_WINDOW_H_INCLUDED
