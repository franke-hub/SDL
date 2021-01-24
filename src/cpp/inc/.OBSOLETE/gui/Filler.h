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
//       Filler.h
//
// Purpose-
//       Graphical User Interface: Filler
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_FILLER_H_INCLUDED
#define GUI_FILLER_H_INCLUDED

#ifndef GUI_BOUNDS_H_INCLUDED
#include "gui/Bounds.h"
#endif

#include "namespace.gui"            // Graphical User Interface
//----------------------------------------------------------------------------
//
// Class-
//       Filler
//
// Purpose-
//       The render method sets the entire Object to the background Color.
//
//----------------------------------------------------------------------------
class Filler : public Bounds {      // Filler Object (Fills the Bounds)
//----------------------------------------------------------------------------
// Filler::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Filler( void );                 // Destructor
   Filler(                          // Constructor
     Object*           parent = NULL); // -> Parent Object

   Filler(                          // Constructor
     Object*           parent,      // -> Parent Object
     XYOffset&         offset);     // Object offset

   Filler(                          // Constructor
     Object*           parent,      // -> Parent Object
     XYLength&         length);     // Object length

   Filler(                          // Constructor
     Object*           parent,      // -> Parent Object
     XYOffset&         offset,      // Object offset
     XYLength&         length);     // Object length

//----------------------------------------------------------------------------
//
// Public method-
//       Filler::render
//
// Purpose-
//       Render the Filler in the associated Window.
//
// Usage notes-
//       Honors Attribute::VISIBLE and Attribute::TRANSPARENT.
//       Unless (VISIBLE && !TRANSPARENT), this method does nothing.
//
//----------------------------------------------------------------------------
public:
virtual void
   render( void );                  // Render the Filler
}; // class Filler
#include "namespace.end"

#endif // GUI_FILLER_H_INCLUDED
