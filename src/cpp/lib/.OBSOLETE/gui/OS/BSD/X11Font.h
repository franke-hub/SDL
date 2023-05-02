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
//       OS/BSD/X11Font.h
//
// Purpose-
//       Graphical User Interface: _SystemFont implementation
//
// Last change date-
//       2010/01/01
//
//----------------------------------------------------------------------------
// Included only from OS/BSD/Font.cpp

//----------------------------------------------------------------------------
//
// Class-
//       X11Font
//
// Purpose-
//       X11Font descriptor.
//
// Usage notes (defaults)-
//       color:    RGB::Black       // Foreground Color (BG in Text)
//       font:     8x15             // (Color RGB::Black)
//
//----------------------------------------------------------------------------
class X11Font : public _SystemFont {// X11Font descriptor
//----------------------------------------------------------------------------
// Internal data areas (protected by Barrier)
//----------------------------------------------------------------------------
protected:
static Barrier         barrier;     // Attribute Mutex (Barrier)
static unsigned        count;       // Reference count

static Display*        disp;        // Display handle
static int             xscr;        // Display screen
static XID             xwin;        // Font window
static GC              xgco;        // Graphic context
static XImage*         ximg;        // XImage (from Pixmap)
static Pixmap          xmap;        // Pixmap handle
static XYLength        xmapLength;  // Pixmap length

//----------------------------------------------------------------------------
// X11Font::Attributes
//----------------------------------------------------------------------------
protected:
XFontStruct*           font;        // The X11 font

//----------------------------------------------------------------------------
// X11Font::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~X11Font( void );                // Destructor

protected:
   X11Font(                         // Constructor
     char*             name,        // The font name
     XFontStruct*      font);       // The font descriptor

public:
static _SystemFont*                 // -> _SystemFont
   make(                            // Factory method
     const char*       desc);       // Font descriptor

//----------------------------------------------------------------------------
//
// Public method-
//       X11Font::getValidDescriptor
//       X11Font::isValidDescriptor
//       X11Font::setColor
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
// Private method-
//       X11Font::init
//       X11Font::term
//       X11Font::reconfig
//
// Purpose-
//       Static data controls.
//
//----------------------------------------------------------------------------
private:
static void
   init(                            // Initialize static data for
     const XYLength&   length);     // This pixel area length

static void
   term( void );                    // Terminate static data

void
   reconfig(                        // Reconfigure for
     const XYLength&   length);     // This pixel area length

//----------------------------------------------------------------------------
//
// Public method-
//       X11Font::extent
//       X11Font::render
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
}; // class X11Font

