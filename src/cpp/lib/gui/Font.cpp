//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       gui/Font.cpp
//
// Purpose-
//       Implement gui/Font.h
//
// Last change date-
//       2024/04/22
//
//----------------------------------------------------------------------------
#include <exception>                // For std::runtime_error

#include <pub/Debug.h>              // For namespace pub::debugging

#include "gui/Global.h"             // For opt_* definitions, ...
#include "gui/Device.h"             // For gui::Device
#include "gui/Font.h"               // Implementation class
#include "gui/Types.h"              // For type definitions

using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging

namespace gui {
//----------------------------------------------------------------------------
//
// Method-
//       gui::Font::Font
//       gui::Font::~Font
//
// Purpose-
//       Constructor/Destructor
//
//----------------------------------------------------------------------------
   Font::Font(                      // Constructor
     Device*           device)      // Device descriptor
:  device(device)
{  if( opt_hcdm )
     debugh("Font(%p)::Font(%p)\n", this, device);
}

   Font::~Font( void )              // Destructor
{  if( opt_hcdm )
     debugh("Font(%p)::~Font\n", this);

   close();
}

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
   Font::debug(const char* info) const // Debugging display
{  debugf("Font(%p)::debug(%s)\n", this, info ? info : "");

   debugf("..device(%p,%s) fontID(%u) offset[%d,%d] length[%u,%u]\n"
         , device, device->get_name().c_str(), fontID
         , offset.x, offset.y, length.width, length.height);
   debugf("..info(%p.0x%zx):\n", font_info, sizeof(*font_info));
   if( font_info == nullptr )
     return;

   xcb_query_font_reply_t& i= *this->font_info; // The Font information
   debugf("...min_bounds[%d,%d,%d,%d,%d,%u]\n", i.min_bounds.left_side_bearing
         , i.min_bounds.right_side_bearing, i.min_bounds.character_width
         , i.min_bounds.ascent, i.min_bounds.descent, i.min_bounds.attributes);
   debugf("...max_bounds[%d,%d,%d,%d,%d,%u]\n", i.max_bounds.left_side_bearing
         , i.max_bounds.right_side_bearing, i.max_bounds.character_width
         , i.max_bounds.ascent, i.max_bounds.descent, i.max_bounds.attributes);
   debugf("...info.min_/max_/default_char[%u,%u,%u]\n", i.min_char_or_byte2
         , i.max_char_or_byte2, i.default_char);
   debugf("...properties_len(%u), draw_direction(%u)\n", i.properties_len
         , i.draw_direction);
   debugf("...min/max_byte1[%u,%u]\n", i.min_byte1, i.max_byte1);
   debugf("...all_chars_exist(%u)\n", i.all_chars_exist);
   debugf("...font_ascent/descent[%d,%d]\n", i.font_ascent, i.font_descent);
   debugf("...char_infos_len(%u)\n", i.char_infos_len);
}

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
   Font::close( void )              // Clear the Font attributes
{  if( opt_hcdm )
     debugh("Font(%p)::close\n", this);

   xcb_connection_t* const conn= device->c;
   if( fontID) {                    // If font exists
     device->ENQUEUE("xcb_close_font", xcb_close_font_checked(conn, fontID) );
     fontID= 0;
   }

   if( font_info ) {
     free(font_info);
     font_info= nullptr;
   }

   device->flush();
}

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
   gui::Font::makeGC(               // Create a Font Graphic Contexts
     Pixel_t           fg,          // Foreground pixel
     Pixel_t           bg)          // Background pixel
{  if( opt_hcdm && opt_verbose > 1 )
     debugh("Font(%p)::makeGC(%.6x,%.6x)\n", this, uint32_t(fg), uint32_t(bg));

   if( fontID == 0 ) {
     fprintf(stderr, "Font(%p)::makeGC, Font not open\n", this);
     return 0;
   }

   // Create the Graphic Context
   xcb_connection_t* const conn= device->c;
   xcb_drawable_t    const draw= device->widget_id;

   xcb_gcontext_t fontGC= xcb_generate_id(conn);
   uint32_t mask= XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
   uint32_t parm[3];
   parm[0]= fg;
   parm[1]= bg;
   parm[2]= fontID;
   device->ENQUEUE("xcb_create_gc"
                  , xcb_create_gc_checked(conn, fontGC, draw, mask, parm) );
   device->flush();

   if( opt_hcdm && opt_verbose > 0 )
     debugh("%u= Font(%p)::makeGC(%.6x,%.6x)\n", fontGC, this
           , uint32_t(fg), uint32_t(bg));

   return fontGC;
}

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
   Font::open(                      // Open the Font
     const char*       name)        // The font name
{  if( opt_hcdm )
     debugh("Font(%p)::open(%s)\n", this, name ? name : "<default>");

   if( fontID || font_info )        // Reset all font information, if needed
     close();

   if( name == nullptr )            // If using system-wide default font
     name= "7x13";

   xcb_connection_t* const conn= device->c;

   fontID= xcb_generate_id(conn);
   xcb_void_cookie_t void_cookie=
   xcb_open_font_checked(conn, fontID, uint16_t(strlen(name)), name);
   xcb_generic_error_t* error=
   xcb_request_check(conn, void_cookie);
   if( error ) {
     fprintf(stderr, "Font::open(%s) failure\n", name);
     fontID= 0;
     xcberror(error);
     int rc= error->error_code;
     free(error);
     return rc;
   }

   xcb_query_font_cookie_t font_cookie= xcb_query_font(conn, fontID);
   font_info= xcb_query_font_reply(conn, font_cookie, &error);
   if( font_info == nullptr || error ) {
     fprintf(stderr, "Font::open(%s) failure\n", name);
     xcberror(error);
     int rc= error->error_code;
     free(error);
     return rc;
   }

   offset.x= 0;
   offset.y= font_info->max_bounds.ascent;
   length.width= font_info->max_bounds.character_width;
   length.height= uint16_t( font_info->max_bounds.ascent
                          + font_info->max_bounds.descent);

   return 0;
}
}  // namespace gui
