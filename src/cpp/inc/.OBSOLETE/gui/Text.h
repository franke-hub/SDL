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
//       Text.h
//
// Purpose-
//       Graphical User Interface: Text
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_TEXT_H_INCLUDED
#define GUI_TEXT_H_INCLUDED

#ifndef GUI_BOUNDS_H_INCLUDED
#include "Bounds.h"                 // (Includes Types.h)
#endif

#ifndef GUI_JUSTIFICATION_H_INCLUDED
#include "Justification.h"
#endif

#include "namespace.gui"
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Font;

//----------------------------------------------------------------------------
//
// Class-
//       Text
//
// Purpose-
//       Text descriptor.
//
// Usage notes (defaults)-
//       color:    RGB::White       // Background Color (FG in Font)
//       font:     8x15             // (Color RGB::Black)
//       text:     NULL (blank)
//       mode:     TB_TOP | LR_LEFT
//
//----------------------------------------------------------------------------
class Text : public Bounds {        // Text descriptor
//----------------------------------------------------------------------------
// Text::Attributes
//----------------------------------------------------------------------------
protected:
Font*                  font;        // The associated Font
std::string            text;        // The actual text
Justification          mode;        // Justification mode

//----------------------------------------------------------------------------
// Text::Constructors
//----------------------------------------------------------------------------
private:
void
   setDefaults( void );             // Set defaults

public:
virtual
   ~Text( void );                   // Destructor
   Text(                            // Constructor
     Object*           parent = NULL); // -> Parent Object

   Text(                            // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset);     // Bounds offset (pixels)

   Text(                            // Constructor
     Object*           parent,      // -> Parent Object
     const XYLength&   length);     // Bounds length (pixels)

   Text(                            // Constructor
     Object*           parent,      // -> Parent Object
     const XYOffset&   offset,      // Bounds offset (pixels)
     const XYLength&   length);     // Bounds length (pixels)

//----------------------------------------------------------------------------
//
// Public method-
//       Text::getFont
//       Text::getJustification
//       Text::getText
//
//       Text::setBG
//       Text::setFG
//       Text::setFont
//       Text::setJustification
//       Text::setText
//
// Purpose-
//       Accessor methods.
//
// Usage notes-
//       The set methods have no side effects and do not invoke redraw.
//
//----------------------------------------------------------------------------
public:                             // GET METHODS ---------------------------
inline Font*                        // The associated Font
   getFont( void ) const            // Get associated Font
{  return font;
}

inline int                          // The associated justification mode
   getJustification( void ) const   // Get associated justification mode
{  return mode.getMode();
}

inline const char*                  // The associated text
   getText( void ) const            // Get associated text
{  return text.c_str();
}

public:                             // SET METHODS ---------------------------
Font*                               // The previous Font
   setFont(                         // Set associated Font
     Font*             font);       // To this Font

inline void
   setJustification(                // Set justification mode
     int               mode)        // To this mode
{  this->mode.setMode(mode);
}

void
   setText(                         // Set associated text
     const char*       text);       // To this text

void
   setText(                         // Set associated text
     const std::string&text);       // To this text

//----------------------------------------------------------------------------
//
// Public method-
//       Text::render
//
// Purpose-
//       Render the Text, drawing its content.
//
//----------------------------------------------------------------------------
public:
virtual void
   render( void );                  // Draw Object content
}; // class Text
#include "namespace.end"

#endif // GUI_TEXT_H_INCLUDED
