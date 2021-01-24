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
//       Justification.h
//
// Purpose-
//       Graphical User Interface: Justification mode (for text)
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_JUSTIFICATION_H_INCLUDED
#define GUI_JUSTIFICATION_H_INCLUDED

#ifndef GUI_TYPES_H_INCLUDED
#include "Types.h"
#endif

#include "namespace.gui"
//----------------------------------------------------------------------------
//
// Class-
//       Justification
//
// Purpose-
//       Justification mode descriptor.
//
//----------------------------------------------------------------------------
class Justification {               // Justification mode
//----------------------------------------------------------------------------
// Justification::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum MODE                           // Justification modes
{  LR_LEFT=  0x00000000             // Left justification
,  LR_RIGHT= 0x00000002             // Right justification
,  LR_CENTER=0x00000003             // Center justification
,  LR_TEXT=  0x00000004             // Text justification
,  LR_MASK=  0x0000000F
,  TB_TOP=   0x00000000             // Top justification
,  TB_BOTTOM=0x00000020             // Bottom justification
,  TB_CENTER=0x00000030             // Center justification
,  TB_MASK=  0x000000F0
}; // enum MODE

//----------------------------------------------------------------------------
// Justification::Attributes
//----------------------------------------------------------------------------
protected:
int                    mode;        // Justification mode

//----------------------------------------------------------------------------
// Justification::Constructors
//----------------------------------------------------------------------------
public:
inline
   ~Justification( void )           // Destructor
{
}

inline
   Justification(                   // Constructor
     int               mode= TB_TOP | LR_LEFT) // Justification mode
{  this->mode= mode;
}

//----------------------------------------------------------------------------
//
// Public method-
//       Justification::getMode
//       Justification::setMode
//
// Purpose-
//       Accessor methods.
//
//----------------------------------------------------------------------------
public:
inline int                          // The associated justification mode
   getMode( void ) const            // Get associated justification mode
{  return mode;
}

inline void
   setMode(                         // Set justification mode
     int               mode)        // To this mode
{  this->mode= mode;
}
}; // class Justification
#include "namespace.end"

#endif // GUI_JUSTIFICATION_H_INCLUDED
