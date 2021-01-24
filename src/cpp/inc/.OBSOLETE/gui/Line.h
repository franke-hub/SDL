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
//       Line.h
//
// Purpose-
//       Graphical User Interface: Line
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_LINE_H_INCLUDED
#define GUI_LINE_H_INCLUDED

#ifndef GUI_OBJECT_H_INCLUDED
#include "Object.h"
#endif

#include "namespace.gui"
//----------------------------------------------------------------------------
//
// Class-
//       Line
//
// Purpose-
//       Line descriptor.
//
//----------------------------------------------------------------------------
class Line : public Object {        // Line descriptor
//----------------------------------------------------------------------------
// Line::Attributes
//----------------------------------------------------------------------------
protected:
XYOffset               origin;      // Origin offset
XYOffset               ending;      // Ending offset

//----------------------------------------------------------------------------
// Line::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Line( void );                   // Destructor
   Line(                            // Constructor
     Object*           parent = NULL); // -> Parent Object

   Line(                            // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   origin,      // Line origin
     const XYOffset&   ending);     // Line endpoint

//----------------------------------------------------------------------------
//
// Public method-
//       Line::line
//
// Purpose-
//       Reset the starting and ending positions of the line,
//       render, and change it.
//
//----------------------------------------------------------------------------
public:
virtual void
   line(                            // Describe the Line
     const XYOffset&   origin,      // Line origin
     const XYOffset&   ending);     // Line endpoint

//----------------------------------------------------------------------------
//
// Public method-
//       Line::render
//
// Purpose-
//       Render the associated Line.
//
//----------------------------------------------------------------------------
public:
virtual void
   render( void );                  // Render the Line

//----------------------------------------------------------------------------
//
// Protected method-
//       Line::draw
//
// Purpose-
//       Draw the line, returning the change offset and length
//
//----------------------------------------------------------------------------
public:
virtual void
   draw(                            // Draw the Line
     XYOffset&         offset,      // (OUTPUT) Change offset
     XYLength&         length);     // (OUTPUT) Change length
}; // class Line
#include "namespace.end"

#endif // GUI_LINE_H_INCLUDED
