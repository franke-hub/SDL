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
//       Bounds.h
//
// Purpose-
//       Graphical User Interface: Base Bounds
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_BOUNDS_H_INCLUDED
#define GUI_BOUNDS_H_INCLUDED

#ifndef GUI_OBJECT_H_INCLUDED
#include "Object.h"                 // This include is guaranteed
#endif

#include "namespace.gui"            // Graphical User Interface
//----------------------------------------------------------------------------
//
// Class-
//       Bounds
//
// Purpose-
//       Base GUI Bounds.
//
//----------------------------------------------------------------------------
class Bounds : public Object {      // Base GUI Bounds
//----------------------------------------------------------------------------
// Bounds::Attributes
//----------------------------------------------------------------------------
protected:                          // Explicity available to derived Objects
XYOffset               offset;      // Offset of Bounds (from parent)
XYLength               length;      // Length of Bounds

//----------------------------------------------------------------------------
// Bounds::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Bounds( void );                 // Destructor
   Bounds(                          // Constructor
     Object*           parent = NULL); // -> Parent Object

   Bounds(                          // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset);     // Bounds offset

   Bounds(                          // Constructor
     Object*           parent,      // -> Parent Object
     const XYLength&   length);     // Bounds length

   Bounds(                          // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset,      // Bounds offset
     const XYLength&   length);     // Bounds length

//----------------------------------------------------------------------------
//
// Public method-
//       Bounds::getLength
//       Bounds::getOffset
//       Bounds::setLength
//       Bounds::setOffset
//
// Purpose-
//       Accessor methods.
//
// Usage notes-
//       Methods setLength and setOffset are simple accessor methods.
//       Unlike resize and move, they have no side effects.
//
//----------------------------------------------------------------------------
public:
inline const XYLength&              // Associated XYLength
   getLength( void ) const          // Access the length
{  return length;
}

inline const XYOffset&              // Associated XYOffset
   getOffset( void ) const          // Access the offset
{  return offset;
}

inline void
   setLength(                       // Change the length
     XYLength&         length)      // To this length
{  this->length= length;
}

inline void
   setOffset(                       // Change the offset
     XYOffset&         offset)      // To this offset
{  this->offset= offset;
}

//----------------------------------------------------------------------------
//
// Public method-
//       Bounds::move
//
// Purpose-
//       Reposition the Bounds within the parent Bounds.
//
// Usage notes-
//       In a Bounds object, this is a simple accessor method
//       Derived classes may have side effects.
//
//----------------------------------------------------------------------------
public:
virtual const char*                 // Exception message (NULL OK)
   move(                            // Reposition the Bounds
     const XYOffset&   offset);     // Offset (from parent)

//----------------------------------------------------------------------------
//
// Public method-
//       Bounds::resize
//
// Purpose-
//       Resize the Bounds.
//
// Usage notes-
//       In a Bounds object, this is a simple accessor method
//       Derived classes may have side effects.
//
//----------------------------------------------------------------------------
public:
virtual const char*                 // Exception message (NULL OK)
   resize(                          // Resize the Bounds
     const XYLength&   length);     // Resize length

//----------------------------------------------------------------------------
//
// Public method-
//       Bounds::change (public Object::change)
//
// Purpose-
//       Reflect a change upward to the Window object.
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
//       Bounds::redraw (public Object::redraw)
//
// Purpose-
//       See Object::redraw.
//
//----------------------------------------------------------------------------
public:
virtual void
   redraw(                          // Redraw part of the Object
     const XYOffset&   offset,      // Offset
     const XYLength&   length);     // Length

virtual void
   redraw( void );                  // Redraw the Object

//----------------------------------------------------------------------------
//
// Public method-
//       Bounds::visit(             // (public Object::visit)
//         ObjectVisitor&)
//
// Purpose-
//       See Object::visit
//
//----------------------------------------------------------------------------
public:
virtual void
   visit(                           // Visit the Object tree
     ObjectVisitor&    visitor);    // The ObjectVisitor

//----------------------------------------------------------------------------
//
// Public method-
//       Bounds::visit(             // (public Object::visit)
//         ObjectVisitor&, const XYOffset&, const XYLength&)
//
// Purpose-
//       See Object::visit
//
//----------------------------------------------------------------------------
public:
virtual Object*                     // Resultant, or NULL if none
   visit(                           // Visit the Object tree using
     ObjectVisitor&    visitor,     // This ObjectVisitor for
     const XYOffset&   offset,      // This offset and
     const XYLength&   length);     // This length
}; // class Bounds
#include "namespace.end"

#endif // GUI_BOUNDS_H_INCLUDED
