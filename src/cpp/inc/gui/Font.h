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
//       gui/Font.h
//
// Purpose-
//       XCB Font descriptor
//
// Last change date-
//       2021/01/22
//
//----------------------------------------------------------------------------
#ifndef GUI_FONT_H_INCLUDED
#define GUI_FONT_H_INCLUDED

#include "gui/Types.h"              // For type definitions
#include "gui/Window.h"             // For Window

namespace gui {
//----------------------------------------------------------------------------
//
// Class-
//       gui::Font
//
// Purpose-
//       XCB Font descriptor.
//
//----------------------------------------------------------------------------
class Font {                        // Font descriptor
//----------------------------------------------------------------------------
// gui::Font::Attributes
//----------------------------------------------------------------------------
public:
Device*                device= nullptr; // The associated Device

xcb_font_t             fontID= 0;   // The associated font
xcb_point_t            offset= {0, 0}; // Font offset
WH_size_t              length= {0, 0}; // Font length

xcb_query_font_reply_t*
                       font_info= nullptr; // The Font information

//----------------------------------------------------------------------------
// gui::Font::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   Font(                            // Constructor
     Device*           device);     // Device descriptor

   ~Font( void );                   // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       gui::Font::debug
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
//       gui::Font::close
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
//       gui::Font::makeGC
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
//       gui::Font::open
//
// Purpose-
//       Open the Font (only)
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   open(                            // Open the Font
     const char*       name= nullptr); // The font name
}; // class Font
}  // namespace gui
#endif // GUI_FONT_H_INCLUDED
