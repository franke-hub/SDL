//----------------------------------------------------------------------------
//
//       Copyright (c) 2010-2020 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Font.cpp
//
// Purpose-
//       Graphical User Interface: Font implementation
//
// Last change date-
//       2020/10/03
//
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <com/Logger.h>

#include <gui/Types.h>
#include <gui/Buffer.h>
#include <gui/Text.h>
#include <gui/UniCode.h>
#include <gui/Window.h>

#include "gui/Font.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#include "gui/namespace.gui"
//----------------------------------------------------------------------------
// Internal constants
//----------------------------------------------------------------------------
static const XYLength  zeroLength= {0, 0};

//----------------------------------------------------------------------------
//
// Class-
//       _SystemFont
//
// Purpose-
//       _SystemFont descriptor.
//
// Usage notes (defaults)-
//       color:    RGB::Black       // Foreground Color (BG in Text)
//       font:     8x15             // (Color RGB::Black)
//
//----------------------------------------------------------------------------
class _SystemFont {                 // _SystemFont descriptor
//----------------------------------------------------------------------------
// _SystemFont::Attributes
//----------------------------------------------------------------------------
protected:
Color_t                color;       // The (foregound) color
char*                  desc;        // The descriptive name of the Font
XYLength               length;      // Nominal character length (pixels)

//----------------------------------------------------------------------------
// _SystemFont::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~_SystemFont( void );            // Destructor

protected:
   _SystemFont( void );             // Constructor

public:
static _SystemFont*                 // -> _SystemFont
   make(                            // Factory method (OS Dependent)
     const char*       desc);       // Font descriptor

//----------------------------------------------------------------------------
//
// Protected method-
//       _SystemFont::getDevice
//
// Public method-
//       _SystemFont::getColor
//       _SystemFont::getDescriptor
//       _SystemFont::getLength
//       _SystemFont::getValidDescriptor
//       _SystemFont::isValidDescriptor
//       _SystemFont::extent
//       _SystemFont::render
//       _SystemFont::setColor
//
// Purpose-
//       Font transfer methods.
//
//----------------------------------------------------------------------------
protected:
Device*                             // The Device
   getDevice(                       // Get Device
     Object*           object) const; // Associated with this Object

public:
inline Color_t                      // The (foreground) Color
   getColor( void ) const           // Get (foreground) Color
{  return color;
}

inline const char*                  // The Font descriptor
   getDescriptor( void ) const      // Get Font descriptor
{  return desc;
}

inline const XYLength&              // The nominal character length (pixels)
   getLength( void ) const          // Get nominal character length (pixels)
{  return length;
}

static const char*                  // Exception message (NULL OK)
   getValidDescriptor(              // Get first valid font descriptor
     const char*       desc,        // For this Font descriptor (wildcards OK)
     unsigned          length,      // Result string length
     char*             result);     // OUTPUT: Result string (NUL terminated)

static const char*                  // Exception message (NULL OK)
   isValidDescriptor(               // Is Font descriptor valid?
     const char*       desc);       // For this Font descriptor (wildcards OK)

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
     int               justify= 0); // This Justification mode

inline void
   setColor(                        // Set foreground Color
     Color_t           color)       // To this Color
{  this->color= color;
}
}; // class _SystemFont

//----------------------------------------------------------------------------
//
// Method-
//       _SystemFont::~_SystemFont
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   _SystemFont::~_SystemFont( void )// Destructor
{
   #ifdef HCDM
     Logger::log("%4d: _SystemFont(%p)::~_SystemFont() %s\n", __LINE__,
                 this, desc);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemFont::_SystemFont
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   _SystemFont::_SystemFont( void ) // Constructor
:  color(RGB::Black), desc(NULL), length(zeroLength)
{
   #ifdef HCDM
     Logger::log("%4d: X11Font(%p)::X11Font(%s)\n", __LINE__,
                 this, desc);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemFont::getDevice
//
// Purpose-
//       Device accessor
//
//----------------------------------------------------------------------------
Device*                             // The Device
   _SystemFont::getDevice(          // Get Device
     Object*           object) const // Associated with this Object
{
   return object->getWindow()->device;
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemFont::extent
//
// Purpose-
//       Determine text extent
//
//----------------------------------------------------------------------------
void
   _SystemFont::extent(             // Get text extent for this Font for
     const std::string&text,        // This text
     XYLength&         length)      // (OUTPUT) Pixel length
{  (void)text; (void)length;        // Unused here
}

//----------------------------------------------------------------------------
//
// Method-
//       _SystemFont::render
//
// Purpose-
//       Render text using this Font
//
//----------------------------------------------------------------------------
void
   _SystemFont::render(             // Render text using this Font into
     Buffer*           buffer,      // This Buffer at
     const XYOffset&   offset,      // This pixel offset for
     const XYLength&   length,      // This pixel length using
     const std::string&text,        // This text and
     int               justify)     // This Justification mode
{  // Parameters unused here
   (void)buffer; (void)offset; (void)length; (void)text; (void)justify;
}

//----------------------------------------------------------------------------
//
// Method-
//       Font::~Font
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Font::~Font( void )              // Destructor
{
   #ifdef HCDM
     Logger::log("%4d: Font(%p)::~Font() %s\n", __LINE__,
                 this, getDescriptor());
   #endif

   if( font != NULL )
   {
     delete font;
     font= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Font::Font
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Font::Font(                      // Constructor
     const char*       desc)        // Associated Font
:  font(NULL)
{
   #ifdef HCDM
     Logger::log("%4d: Font(%p)::Font(%s)\n", __LINE__, this, desc);
   #endif

   font= _SystemFont::make(desc);
   if( font == NULL )
   {
     fprintf(stderr, "Font::Font(%s) invalid font\n", desc);
     throw "InvalidFontException";
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Font::isValidDescriptor
//       Font::getColor
//       Font::getDescriptor
//       Font::getLength
//       Font::getValidDescriptor
//       Font::setColor
//       Font::extent
//       Font::render
//
// Purpose-
//       Transfer functions.
//
//----------------------------------------------------------------------------
const char*                         // Exception message (NULL OK)
   Font::isValidDescriptor(         // Test Font descriptor validity
     const char*       desc)        // For this descriptor (wildcards OK)
{
   return _SystemFont::isValidDescriptor(desc);
}

Color_t                             // The (foreground) Color
   Font::getColor( void ) const     // Get (foreground) Color
{
   return font->getColor();
}

const char*                         // The Font descriptor
   Font::getDescriptor( void ) const// Get Font descriptor
{
   return font->getDescriptor();
}

const XYLength&                     // The nominal character length (pixels)
   Font::getLength( void ) const    // Get nominal character length (pixels)
{
   return font->getLength();
}

const char*                         // Exception message (NULL OK)
   Font::getValidDescriptor(        // Get first valid Font descriptor
     const char*       desc,        // For this descriptor (wildcards OK)
     unsigned          length,      // Result string length
     char*             result)      // OUTPUT: Result string
{
   return _SystemFont::getValidDescriptor(desc, length, result);
}

void
   Font::setColor(                  // Set (foreground) Color
     Color_t           color)       // To this Color
{
   font->setColor(color);
}

void
   Font::extent(                    // Determine text extent for
     const std::string&text,        // This text
     XYLength&         length)      // (OUTPUT) Length (in pixels)
{
   font->extent(text, length);
}

void
   Font::render(                    // Render text using this Font into
     Buffer*           buffer,      // This Buffer at
     const XYOffset&   offset,      // This pixel offset for
     const XYLength&   length,      // This pixel length using
     const std::string&text,        // This text and
     int               justify)     // This Justification mode
{
   font->render(buffer, offset, length, text, justify);
}
#include "gui/namespace.end"

//----------------------------------------------------------------------------
//
// Section-
//       System Dependent Include
//
// Purpose-
//       Operating System dependent Device driver.
//
//----------------------------------------------------------------------------
#if   defined(_OS_WIN)
  #include "OS/WIN/WinFont.cpp"
#elif defined(_OS_BSD)
  #include "OS/BSD/X11Font.cpp"
#else
  #error "Invalid OS"
#endif

