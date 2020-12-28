//----------------------------------------------------------------------------
//
//       Copyright (C) 2020 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Xcb/Font.h
//
// Purpose-
//       XCB Font descriptor
//
// Last change date-
//       2020/12/26
//
//----------------------------------------------------------------------------
#ifndef XCB_FONT_H_INCLUDED
#define XCB_FONT_H_INCLUDED

#include <exception>                // For std::runtime_error

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/UTF8.h>               // For pub::UTF8

#include "Xcb/Global.h"             // For opt_* definitions, ...
#include "Xcb/Types.h"              // For type definitions
#include "Xcb/Window.h"             // For Window

namespace xcb {
//----------------------------------------------------------------------------
//
// Class-
//       xcb::Font
//
// Purpose-
//       XCB Font descriptor.
//
// Implementation note-
//       In order to use putxy, you need a xcb_drawable_t, a xcb_gcontect_t,
//       and the starting offset of the font.
//       To create the xcb_gcontext_t, you need the same xcb_drawable_t,
//       and an xcb_font_t. You also need to select FG and BG pixels.
//       To create the xcb_font_t, you need the name of the Font. The
//       xcb_font_t stands alone, not requiring the drawable or gcontext.
//
//       So, in order to use Font::putxy, use this sequence:
//         When constructed, a Font is associated with a Window.
//         This Window's connection and drawable are used as needed.
//         (Use different Font objects for different Window objects.)
//
//         1) Font::open(name), initializing fontID, offset, and length
//         2) Use Font::makeGC to create a graphics context.
//            The first one created will be the inital default.
//            Note: Font::makeGC is not useable until Window::configure
//            (That's when the dimensions are known.)
//         3) When done creating graphics contexts, you can (optionally)
//            close the Font. The (needed) offset remains.
//       Note that when using putxy the drawable and graphics context must
//       match. You can use putxy with multiple drawable/graphic pairs.
//
//       The underlying xcb operation is xcb_image_text_8.
//
//----------------------------------------------------------------------------
class Font {                        // Font descriptor
//----------------------------------------------------------------------------
// xcb::Font::Attributes
//----------------------------------------------------------------------------
public:
Window*                window= nullptr; // The associated Window

xcb_gcontext_t         fontGC= 0;   // The default graphic context
xcb_font_t             fontID= 0;   // The associated font
xcb_point_t            offset= {0, 0}; // Font offset
WH_size_t              length= {0, 0}; // Font length

xcb_query_font_reply_t*
                       font_info= nullptr; // The Font information

//----------------------------------------------------------------------------
// xcb::Font::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   Font(                            // Constructor
     Window*           window);     // Window descriptor

   ~Font( void );                   // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Font::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   debug(const char* info= nullptr) const; // Debugging display

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Font::close
//
// Purpose-
//       Close the point
//
//----------------------------------------------------------------------------
void
   close( void );                   // Clear the Font attributes

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Font::makeGC
//
// Purpose-
//       Create a Font Graphic Context. (Font must be initialized)
//
// Implementation note-
//       The first Graphic Context is set as the default.
//
//----------------------------------------------------------------------------
xcb_gcontext_t                      // The created graphic context
   makeGC(                          // Create a Font Graphic Contexts
     Pixel_t           fg,          // Foreground pixel
     Pixel_t           bg);         // Background pixel

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Font::open
//
// Purpose-
//       Open the Font (only)
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   open(                            // Open the Font
     const char*       name= nullptr); // The font name

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Font::putxy
//
// Purpose-
//       Draw text at [left,top] point
//
// Implementation note-
//       TODO: Handle text length > 255
//
//----------------------------------------------------------------------------
void
   putxy(                           // Draw text
     xcb_gcontext_t    fontGC,      // The target graphic context
     unsigned          left,        // Left (X) offset
     unsigned          top,         // Top  (Y) offset
     const char*       text);       // Using this text

void
   putxy(                           // Draw text (using default draw and GC)
     unsigned          left,        // Left (X) offset
     unsigned          top,         // Top  (Y) offset
     const char*       text)        // Using this text
{  putxy(fontGC, left, top, text); }
}; // class Font
}  // namespace xcb
#endif // XCB_FONT_H_INCLUDED
