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
//       WormBuffer.hpp
//
// Purpose-
//       Graphical User Interface: WormBuffer
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_WORMBUFFER_HPP_INCLUDED
#define GUI_WORMBUFFER_HPP_INCLUDED

#ifndef LIST_H_INCLUDED
#include <com/List.h>
#endif

#ifndef GUI_BUFFER_H_INCLUDED
#include "gui/Buffer.h"
#endif

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Worm;

//----------------------------------------------------------------------------
//
// Class-
//       WormBuffer
//
// Purpose-
//       Buffer for Worm Objects
//
//----------------------------------------------------------------------------
class WormBuffer : public GUI::Bounds { // Buffer for Worm Objects
//----------------------------------------------------------------------------
// WormBuffer::Attributes
//----------------------------------------------------------------------------
protected:
DHSL_List<Worm>        list;        // The list of Worm Objects

//----------------------------------------------------------------------------
// WormBuffer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~WormBuffer( void );             // Destructor

   WormBuffer(                      // Constructor
     GUI::Bounds*      parent);     // -> Parent Bounds

   WormBuffer(                      // Constructor
     GUI::Bounds*      parent,      // -> Parent Bounds
     GUI::XYOffset&    offset);     // Bounds offset

   WormBuffer(                      // Constructor
     GUI::Bounds*      parent,      // -> Parent Bounds
     GUI::XYLength&    length);     // Bounds length

   WormBuffer(                      // Constructor
     GUI::Bounds*      parent,      // -> Parent Bounds
     GUI::XYOffset&    offset,      // Bounds offset
     GUI::XYLength&    length);     // Bounds length

//----------------------------------------------------------------------------
//
// Public method-
//       WormBuffer::append
//
// Purpose-
//       Add a Worm to the WormBuffer
//
//----------------------------------------------------------------------------
public:
void
   append(                          // Append a Worm
     Worm&             worm);       // The Worm to append

//----------------------------------------------------------------------------
//
// Public method-
//       WormBuffer::reset
//
// Purpose-
//       Reset (randomize) the WormBuffer's Worms
//
//----------------------------------------------------------------------------
public:
void
   reset( void );                   // Reset the Worm

//----------------------------------------------------------------------------
//
// Method-
//       WormBuffer::setPixel
//
// Purpose-
//       Set a Pixel in the Buffer.
//
//----------------------------------------------------------------------------
virtual GUI::Pixel*                 // Resultant Pixel*
   setPixel(                        // Set a Pixel
     GUI::XOffset_t    x,           // X offset
     GUI::YOffset_t    y,           // Y offset
     GUI::Color_t      color);      // Color

//----------------------------------------------------------------------------
//
// Public method-
//       WormBuffer::toggle
//
// Purpose-
//       Toggle the WormBuffer
//
//----------------------------------------------------------------------------
public:
virtual void
   toggle( void );                  // Toggle the WormBuffer
}; // class WormBuffer

//----------------------------------------------------------------------------
//
// Class-
//       Worm
//
// Purpose-
//       Worm descriptor.
//
//----------------------------------------------------------------------------
class Worm : public DHSL_List<Worm>::Link { // Worm Object
//----------------------------------------------------------------------------
// Worm::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  WORM_SIZE= 128                   // Length of Worm
}; // enum

//----------------------------------------------------------------------------
// Worm::Attributes
//----------------------------------------------------------------------------
protected:
GUI::XYOffset          offset[WORM_SIZE]; // The Worm position array
int                    color;       // Current color (index)
int                    ident;       // Worm identifier
int                    dX;          // X direction delta (-1, 0, +1)
int                    dY;          // Y direction delta (-1, 0, +1)

//----------------------------------------------------------------------------
// Worm::Constructors
//----------------------------------------------------------------------------
public:
   ~Worm( void );                   // Destructor
   Worm( void );                    // Constructor

//----------------------------------------------------------------------------
//
// Public method-
//       Worm::toggle
//
// Purpose-
//       Toggle the Worm
//
//----------------------------------------------------------------------------
public:
void
   toggle(                          // Toggle the Worm
     WormBuffer*       buffer);     // In this Buffer

//----------------------------------------------------------------------------
//
// Public method-
//       Worm::reset
//
// Purpose-
//       Reset (randomize) the Worm
//
//----------------------------------------------------------------------------
public:
void
   reset(                           // Reset the Worm
     GUI::Bounds*      bounds);     // In this Bounds
}; // class Worm

#endif // GUI_WORMBUFFER_HPP_INCLUDED
