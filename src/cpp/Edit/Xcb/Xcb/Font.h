//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
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
//       2021/01/10
//
//----------------------------------------------------------------------------
#ifndef XCB_FONT_H_INCLUDED
#define XCB_FONT_H_INCLUDED

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
//----------------------------------------------------------------------------
class Font {                        // Font descriptor
//----------------------------------------------------------------------------
// xcb::Font::Attributes
//----------------------------------------------------------------------------
public:
Device*                device= nullptr; // The associated Device

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
     Device*           device);     // Device descriptor

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
}; // class Font
}  // namespace xcb
#endif // XCB_FONT_H_INCLUDED
