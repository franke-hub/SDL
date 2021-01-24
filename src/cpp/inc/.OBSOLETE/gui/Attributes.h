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
//       Attributes.h
//
// Purpose-
//       Graphical User Interface: Attributes
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_ATTRIBUTES_H_INCLUDED
#define GUI_ATTRIBUTES_H_INCLUDED

#ifndef GUI_TYPES_H_INCLUDED
#include "Types.h"                  // This include is guaranteed
#endif

#include "namespace.gui"
//----------------------------------------------------------------------------
//
// Class--
//       Attributes
//
// Purpose-
//       Attributes container.
//
// Usage notes-
//       Attribute are defined by subclasses, and have no intrinsic meaning
//       to the Attribute object.
//
//----------------------------------------------------------------------------
class Attributes {                  // Attributes
//----------------------------------------------------------------------------
// Attributes::Attributes
//----------------------------------------------------------------------------
protected:
unsigned long          attributes;  // The attributes

//----------------------------------------------------------------------------
// Attributes::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Attributes( void );             // Destructor
   Attributes(                      // Constructor
     unsigned long     attributes= 0); // Initial attributes

//----------------------------------------------------------------------------
// Attributes::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Attribute value (FALSE/TRUE)
   getAttribute(                    // Get attribute
     int               attribute) const; // Attribute identifier

virtual const char*                 // Exception message (NULL OK)
   setAttribute(                    // Set attribute
     int               attribute,   // Attribute identifier
     int               value);      // Attribute value
}; // class Attributes
#include "namespace.end"

#endif // GUI_ATTRIBUTES_H_INCLUDED
