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
//       Xcb/Font.cpp
//
// Purpose-
//       Implement Xcb/Font.h
//
// Last change date-
//       2020/12/26
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For Debug object

#include "Xcb/Font.h"               // Implementation class

using pub::Debug;                   // For Debug object
using namespace pub::debugging;     // For debugging

namespace xcb {
//----------------------------------------------------------------------------
//
// Subroutine-
//       xcb::int2char2b
//
// Purpose-
//       Convert a 16-bit integer to xcb_char2b_t
//
//----------------------------------------------------------------------------
static inline xcb_char2b_t          // The output character
   int2char2b(                      // Convert integer to xcb_char2b_t
     int               inp)         // This (16 bit) integer
{  return { (uint8_t)(inp>>8), (uint8_t)inp }; }

//----------------------------------------------------------------------------
//
// Method-
//       xcb::Font::Font
//       xcb::Font::~Font
//
// Purpose-
//       Constructor/Destructor
//
//----------------------------------------------------------------------------
   Font::Font(                      // Constructor
     Window*           window)      // Window descriptor
:  window(window)
{  if( opt_hcdm )
     debugh("Font(%p)::Font(%p)\n", this, window);
}

   Font::~Font( void )              // Destructor
{  if( opt_hcdm )
     debugh("Font(%p)::~Font\n", this);

   close();
}

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
   Font::debug(const char* info) const // Debugging display
{  debugf("Font(%p)::debug(%s)\n", this, info ? info : "");

   debugf("..window(%p,%s) fontGC(%u)\n"
          "..fontID(%u) offset[%d,%d] length[%u,%u]\n"
         , window, window->get_name().c_str(), fontGC, fontID
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
//       xcb::Font::close
//
// Purpose-
//       Close the point
//
//----------------------------------------------------------------------------
void
   Font::close( void )              // Clear the Font attributes
{  if( opt_hcdm )
     debugh("Font(%p)::close\n", this);

   xcb_connection_t* const conn= window->c;
   if( fontID) {                    // If font exists
     window->ENQUEUE("xcb_close_font", xcb_close_font_checked(conn, fontID) );
     fontID= 0;
   }

   if( font_info ) {
     free(font_info);
     font_info= nullptr;
   }
}

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
   Font::makeGC(                    // Create a Font Graphic Contexts
     Pixel_t           fg,          // Foreground pixel
     Pixel_t           bg)          // Background pixel
{  if( opt_hcdm && opt_verbose > 1 )
     debugh("Font(%p)::makeGC(%.6x,%.6x)\n", this, uint32_t(fg), uint32_t(bg));

   if( fontID == 0 ) {
     user_debug("Font(%p)::makeGC, Font not open\n", this);
     return 0;
   }

   // Create the Graphic Context
   xcb_connection_t* const conn= window->c;
   xcb_drawable_t    const draw= window->widget_id;

   xcb_gcontext_t fontGC= xcb_generate_id(conn);
   uint32_t mask= XCB_GC_FOREGROUND | XCB_GC_BACKGROUND | XCB_GC_FONT;
   uint32_t parm[3];
   parm[0]= fg;
   parm[1]= bg;
   parm[2]= fontID;
   window->ENQUEUE("xcb_create_gc",
                   xcb_create_gc(conn, fontGC, draw, mask, parm) );

   // Initialize the default Graphic Context
   if( this->fontGC == 0 )
     this->fontGC= fontGC;

   if( opt_hcdm )
     debugh("%u= Font(%p)::makeGC(%.6x,%.6x)\n", fontGC, this
           , uint32_t(fg), uint32_t(bg));

   return fontGC;
}

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
   Font::open(                      // Open the Font
     const char*       name)        // The font name
{  if( opt_hcdm )
     debugh("Font(%p)::open(%s)\n", this, name ? name : "<default>");

   if( fontID || font_info )        // Reset all font information, if needed
     close();

   if( name == nullptr )            // If using system-wide default font
     name= "7x13";

   xcb_connection_t* const conn= window->c;

   fontID= xcb_generate_id(conn);
   xcb_void_cookie_t void_cookie=
   xcb_open_font_checked(conn, fontID, uint16_t(strlen(name)), name);
   xcb_generic_error_t* error=
   xcb_request_check(conn, void_cookie);
   if( error ) {
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
   Font::putxy(                     // Draw text
     xcb_gcontext_t    fontGC,      // The target graphic context
     unsigned          left,        // Left (X) offset
     unsigned          top,         // Top  (Y) offset
     const char*       text)        // Using this text
{  if( opt_hcdm && opt_verbose > 1 )
     debugh("Font(%p)::putxy(%u,[%d,%d],'%s')\n", this
           , fontGC, left, top, text);

   pub::UTF8::Decoder decoder(text); // UTF8 input buffer
   xcb_char2b_t out[256];           // UTF16 output buffer
   unsigned outlen;                 // UTF16 output buffer length
   unsigned outpix= left;           // Current output pixel
   for(outlen= 0; outlen<256; outlen++) {
     outpix += length.width;        // Ending pixel (+1)
     if( outpix > window->rect.width ) // If past end of screen
       break;

     int code= decoder.decode();    // Next input character
     if( code < 0 )                 // If none left
       break;

     if( code >= 0x00010000 ) {     // If two characters required
       if( outlen >= 255 )          // If there's only room for one character
         break;
       code -= 0x00010000;          // Subtract extended origin
//     code &= 0x000fffff;          // 20 bit remainder (operation not needed)
       out[outlen++]= int2char2b(0x0000d800 | (code >> 10)); // High order code
       code &= 0x000003ff;          // Low order 10 bits
       code |= 0x0000dc00;          // Low order code word
     }

     out[outlen]= int2char2b(code); // Set (possibly low order) code
   }
   if( outlen == 0 ) return;        // Zero length easy to render
   if( outlen >= 256 ) outlen= 255; // Only 8-bit length allowed

   window->NOQUEUE("xcb_image_text_16", xcb_image_text_16
                  ( window->c, uint8_t(outlen), window->widget_id, fontGC
                  , uint16_t(left), uint16_t(top + offset.y), out) );
}
}  // namespace xcb
