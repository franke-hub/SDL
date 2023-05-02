//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2014 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       OS/WIN/WinFont.h
//
// Purpose-
//       Graphical User Interface: _SystemFont implementation
//
// Last change date-
//       2014/01/01
//
//----------------------------------------------------------------------------
#ifndef GUI_OSWIN_WINFONT_H_INCLUDED
#define GUI_OSWIN_WINFONT_H_INCLUDED

#include "gui/namespace.gui"
//----------------------------------------------------------------------------
//
// Class-
//       WinFont
//
// Purpose-
//       WinFont descriptor.
//
// Usage notes (defaults)-
//       color:    RGB::Black       // Foreground Color (BG in Text)
//       font:     8x15             // (Color RGB::Black)
//
//----------------------------------------------------------------------------
class WinFont : public _SystemFont {// WinFont descriptor
//----------------------------------------------------------------------------
// WinFont::Attributes
//----------------------------------------------------------------------------
protected:
HFONT                  hFont;       // FONT handle
HDC                    hDC;         // Memory Device Context
LOGFONT                lFont;       // FONT data

//----------------------------------------------------------------------------
// WinFont::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~WinFont( void );                // Destructor

protected:
   WinFont(                         // Constructor
     LOGFONT&          lf);         // The associated LOGFONT

public:
static _SystemFont*                 // -> _SystemFont
   make(                            // Factory method
     const char*       desc);       // Font descriptor

//----------------------------------------------------------------------------
//
// Public method-
//       WinFont::getValidDescriptor
//       WinFont::isValidDescriptor
//       WinFont::setColor
//
// Purpose-
//       Font transfer methods.
//
//----------------------------------------------------------------------------
public:
static const char*                  // Exception message (NULL OK)
   getValidDescriptor(              // Get first valid font descriptor
     const char*       desc,        // For this Font descriptor (wildcards OK)
     unsigned          length,      // Result string length
     char*             result);     // OUTPUT: Result string (NUL terminated)

static const char*                  // Exception message (NULL OK)
   isValidDescriptor(               // Is Font descriptor valid?
     const char*       desc);       // For this Font descriptor (wildcards OK)

//----------------------------------------------------------------------------
//
// Public method-
//       WinFont::extent
//       WinFont::render
//
// Purpose-
//       Font transfer methods.
//
//----------------------------------------------------------------------------
public:
virtual void
   extent(                          // Get text extent for
     const std::string&text,        // This text
     XYLength&         length);     // (OUTPUT) Pixel length

virtual void
   render(                          // Render text into
     Buffer*           buffer,      // This buffer at
     const XYOffset&   offset,      // This pixel offset for
     const XYLength&   length,      // This pixel length using
     const std::string&text,        // This text and
     int               justify);    // This Justification mode
}; // class WinFont
#include "gui/namespace.end"
#endif // GUI_OSWIN_WINFONT_H_INCLUDED

