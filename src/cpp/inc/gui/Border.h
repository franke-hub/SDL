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
//       Border.h
//
// Purpose-
//       Graphical User Interface: Border
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_BORDER_H_INCLUDED
#define GUI_BORDER_H_INCLUDED

#ifndef GUI_OBJECT_H_INCLUDED
#include "Object.h"
#endif

#include "namespace.gui"            // Graphical User Interface
//----------------------------------------------------------------------------
//
// Class-
//       Border
//
// Purpose-
//       Border.
//
// Usage notes-
//       A Border has a length but no offset. The length is the same on the
//       (length.x) left and right and on the (length.y) top and bottom.
//
//----------------------------------------------------------------------------
class Border : public Object {      // Border
//----------------------------------------------------------------------------
// Border::Attributes
//----------------------------------------------------------------------------
protected:                          // Explicity available to derived Objects
XYLength               length;      // Length

//----------------------------------------------------------------------------
// Border::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Border( void );                 // Destructor
   Border(                          // Constructor
     Object*           parent = NULL); // -> Parent Object

   Border(                          // Constructor
     Object*           parent,      // -> Parent Object
     const XYLength&   length);     // Border outside length

//----------------------------------------------------------------------------
//
// Public method-
//       Border::getLength
//       Border::setLength
//
// Purpose-
//       Accessor methods.
//
//----------------------------------------------------------------------------
public:
inline const XYLength&              // Associated XYLength
   getLength( void ) const          // Get assocated XYLength
{  return length;
}

inline void
   setLength(                       // Change the XYLength
     const XYLength&   length)      // To this Length
{  this->length= length;
}

//----------------------------------------------------------------------------
//
// Public method-
//       Border::render (public Object::render)
//
// Purpose-
//       Render this Object.
//
//----------------------------------------------------------------------------
public:
virtual void
   render( void );                  // Render the Object
}; // class Border
#include "namespace.end"

#endif // GUI_BORDER_H_INCLUDED
