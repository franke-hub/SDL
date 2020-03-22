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
//       Buffer.h
//
// Purpose-
//       Graphical User Interface: Buffer
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_BUFFER_H_INCLUDED
#define GUI_BUFFER_H_INCLUDED

#ifndef GUI_TYPES_H_INCLUDED
#include "Types.h"                  // This include is guaranteed
#endif

#ifndef GUI_BOUNDS_H_INCLUDED
#include "Bounds.h"
#endif

#include "namespace.gui"            // Graphical User Interface
//----------------------------------------------------------------------------
//
// Class-
//       Buffer
//
// Purpose-
//       Pixel container Object.
//
//----------------------------------------------------------------------------
class Buffer : public Bounds {      // Pixel container object
//----------------------------------------------------------------------------
// Buffer::Attributes
//----------------------------------------------------------------------------
protected:
Pixel*                 pixel;       // The physical buffer

//----------------------------------------------------------------------------
// Buffer::Constructors
//----------------------------------------------------------------------------
private:
void
   buildObject( void );             // Constructor helper

public:
virtual
   ~Buffer( void );                 // Destructor
   Buffer(                          // Constructor
     Object*           parent = NULL); // -> Parent Object

   Buffer(                          // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset);     // Object offset

   Buffer(                          // Constructor
     Object*           parent,      // -> Parent Object
     const XYLength&   length);     // Object length

   Buffer(                          // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset,      // Object offset
     const XYLength&   length);     // Object length

//----------------------------------------------------------------------------
//
// Public method-
//       Buffer::getPixel
//
// Purpose-
//       Address a picture element.
//
//----------------------------------------------------------------------------
public:
virtual Pixel*                      // -> Pixel
   getPixel(                        // Get Pixel* for
     XOffset_t         x,           // X (horizontal) offset
     YOffset_t         y) const;    // Y (vertical) offset

//----------------------------------------------------------------------------
//
// Public method-
//       Buffer::render
//
// Purpose-
//       Render the Buffer (Set the default color)
//
//----------------------------------------------------------------------------
public:
virtual void
   render( void );                  // Render the Buffer

//----------------------------------------------------------------------------
//
// Public method-
//       Buffer::resize
//
// Purpose-
//       Resize the Buffer.
//
//----------------------------------------------------------------------------
public:
virtual const char*                 // Exception message (NULL OK)
   resize(                          // Resize the Buffer
     const XYLength&   length);     // Resize length

//----------------------------------------------------------------------------
//
// Public method-
//       Buffer::upload
//
// Purpose-
//       Upload this buffer into its parent Buffer.
//
// Usage notes-
//       This method is not expected to be used by client programs.
//       It is used by the Object::visit method as a special case, to
//       complete Buffer::render after the child tree is rendered.
//
//----------------------------------------------------------------------------
public:
virtual void
   upload( void );                  // Upload the Buffer
}; // class Buffer
#include "namespace.end"

#endif // GUI_BUFFER_H_INCLUDED
