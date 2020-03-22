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
//       Offset.h
//
// Purpose-
//       Graphical User Interface: Offset (layout control)
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_OFFSET_H_INCLUDED
#define GUI_OFFSET_H_INCLUDED

#ifndef GUI_OBJECT_H_INCLUDED
#include "Object.h"                 // This include is guaranteed
#endif

#include "namespace.gui"            // Graphical User Interface
//----------------------------------------------------------------------------
//
// Class-
//       Offset
//
// Purpose-
//       Offset (layout control.)
//
// Usage-
//       The Offset object is used for layout control. Unlike Bounds,
//       it has no associated length.
//
//----------------------------------------------------------------------------
class Offset : public Object {      // Base GUI Offset
//----------------------------------------------------------------------------
// Offset::Attributes
//----------------------------------------------------------------------------
protected:                          // Explicity available to derived Objects
XYOffset               offset;      // Offset of Object (from parent)

//----------------------------------------------------------------------------
// Offset::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Offset( void );                 // Destructor
   Offset(                          // Constructor
     Object*           parent = NULL); // -> Parent Object

   Offset(                          // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset);     // Offset

//----------------------------------------------------------------------------
//
// Public method-
//       Offset::getOffset
//       Offset::setOffset
//
// Purpose-
//       Accessor methods.
//
//----------------------------------------------------------------------------
public:
inline const XYOffset&              // Associated XYOffset
   getOffset( void ) const          // Access the offset
{  return offset;
}

inline void
   setOffset(                       // Change the offset
     XYOffset&         offset)      // To this offset
{  this->offset= offset;
}

//----------------------------------------------------------------------------
//
// Public method-
//       Offset::change (public Object::change)
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
//       Offset::redraw (public Object::redraw)
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
//       Offset::visit(             // (public Object::visit)
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
//       Offset::visit(             // (public Object::visit)
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
}; // class Offset
#include "namespace.end"

#endif // GUI_OFFSET_H_INCLUDED
