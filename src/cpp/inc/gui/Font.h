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
//       Font.h
//
// Purpose-
//       Graphical User Interface: Font
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_FONT_H_INCLUDED
#define GUI_FONT_H_INCLUDED

#ifndef GUI_TYPES_H_INCLUDED
#include "Types.h"
#endif

#include "namespace.gui"
//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class _SystemFont;
class Buffer;

//----------------------------------------------------------------------------
//
// Class-
//       Font
//
// Purpose-
//       Font descriptor.
//
// Usage notes (defaults)-
//       color:    RGB::Black       // Foreground Color (BG in Text)
//       font:     8x15             // (Color RGB::Black)
//
// Font name-
//       -foundry  The name of the developer
//       -family   The font family name
//       -weight   The font weight {bold,medium}
//       -slant    The slant {o(blique),i(talic),r(oman)} Roman is unslanted
//       -setWidth The width {normal,condensed,narrow,double}
//      --pixels   (Calculated as in example)
//       -points   (In tenths of a point) (one point is 1/72 inch)
//       -hRes     Horizontal resolution in dots per inch
//       -vRes     Vertical resolution in dots per inch
//       -spacing  Spacing {m(onospace),p(roportional)}
//       -avgWidth Averge width (In tenths of a pixel)
//       -format   Character set
//
//       Examples:
//       -adobe-courier-bold-o-normal--10-100-75-75-m-60-iso8859-1
//           10.0 points, 10/72 inches *  75 dots/inch, 10.42 pixels (10)
//
//       -adobe-courier-bold-o-normal--17-120-100-100-m-60-iso8859-1
//           12.0 points, 12/72 inches * 100 dots/inch, 16.67 pixels (17)
//
//----------------------------------------------------------------------------
class Font {                        // Font descriptor
//----------------------------------------------------------------------------
// Font::Attributes
//----------------------------------------------------------------------------
protected:
_SystemFont*           font;        // The _SystemFont object

//----------------------------------------------------------------------------
// Font::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Font( void );                   // Destructor
   Font(                            // Constructor
     const char*       desc= NULL); // The Font descriptor

//----------------------------------------------------------------------------
//
// Public method-
//       Font::isValidDescriptor
//
//       Font::getColor
//       Font::getDescriptor
//       Font::getLength
//       Font::getValidDescriptor
//
//       Font::setColor
//
// Purpose-
//       Accessor methods.
//
//----------------------------------------------------------------------------
public:
static const char*                  // Exception message (NULL OK)
   isValidDescriptor(               // Is Font descriptor valid?
     const char*       desc);       // For this Font descriptor (wildcards OK)

Color_t                             // The (foreground) Color
   getColor( void ) const;          // Get (foreground) Color

const char*                         // The Font descriptor
   getDescriptor( void ) const;     // Get Font descriptor

const XYLength&                     // The nominal character length (pixels)
   getLength( void ) const;         // Get nominal character length (pixels)

static const char*                  // Exception message (NULL OK)
   getValidDescriptor(              // Get first valid font descriptor
     const char*       desc,        // For this Font descriptor (wildcards OK)
     unsigned          length,      // Result string length
     char*             result);     // OUTPUT: Result string ('\0' terminated)

void
   setColor(                        // Set (foreground) Color
     Color_t           color);      // To this Color

//----------------------------------------------------------------------------
//
// Public method-
//       Font::extent
//
// Purpose-
//       Determine text extent.
//
//----------------------------------------------------------------------------
public:
virtual void
   extent(                          // Determine text extent for
     const std::string&text,        // This text
     XYLength&         length);     // (OUTPUT) Length (in pixels)

//----------------------------------------------------------------------------
//
// Public method-
//       Font::render
//
// Purpose-
//       Render text.
//
//----------------------------------------------------------------------------
public:
virtual void
   render(                          // Render text into
     Buffer*           buffer,      // This buffer at
     const XYOffset&   offset,      // This pixel offset for
     const XYLength&   length,      // This pixel length using
     const std::string&text,        // This text and
     int               justify= 0); // This Justification mode
}; // class Font
#include "namespace.end"

#endif // GUI_FONT_H_INCLUDED
