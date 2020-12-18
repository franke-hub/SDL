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
//       2020/12/16
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
// xcb::Font::Constructor
//----------------------------------------------------------------------------
public:
   Font(                            // Constructor
     Window*           window)      // Window descriptor
:  window(window)
{  using namespace pub::debugging;

   if( opt_hcdm )
     debugh("Font(%p)::Font(%p)\n", this, window);
}

//----------------------------------------------------------------------------
// xcb::Font::Destructor
//----------------------------------------------------------------------------
   ~Font( void )                    // Destructor
{  using namespace pub::debugging;

   if( opt_hcdm )
     debugh("Font(%p)::~Font\n", this);

   close();
}

//----------------------------------------------------------------------------
//
// Method-
//       Font::close
//
// Purpose-
//       Close the point
//
//----------------------------------------------------------------------------
public:
void
   close( void )                    // Clear the Font attributes
{  using namespace pub::debugging;

   if( opt_hcdm )
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
//       Font::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   debug(const char* info= nullptr) const // Debugging display
{  using namespace pub::debugging;

   debugf("Font(%p)::debug(%s)\n", this, info ? info : "");

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
//       xcb::Font::int2char2b
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
     Pixel_t           bg)          // Background pixel
{  using namespace pub::debugging;

   if( opt_hcdm && opt_verbose > 1 )
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
   open(                            // Open the Font
     const char*       name= nullptr) // The font name
{  using namespace pub::debugging;

   if( opt_hcdm )
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
   putxy(                           // Draw text
     xcb_gcontext_t    fontGC,      // The target graphic context
     unsigned          left,        // Left (X) offset
     unsigned          top,         // Top  (Y) offset
     const char*       text)        // Using this text
{  using namespace pub::debugging;

   if( opt_hcdm && opt_verbose > 1 )
     debugh("Font(%p)::putxy(%u,[%d,%d],'%s')\n", this
           , fontGC, left, top, text);

   pub::UTF8::Decoder decoder(text); // UTF8 input buffer
   xcb_char2b_t out[256];           // UTF16 output buffer
   unsigned outlen;                 // UTF16 output buffer length
   unsigned outpix= left;           // Current output pixel
   for(outlen= 0; outlen<256; outlen++) {
     outpix += length.width;         // Ending pixel (+1)
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

void
   putxy(                           // Draw text (using default draw and GC)
     unsigned          left,        // Left (X) offset
     unsigned          top,         // Top  (Y) offset
     const char*       text)        // Using this text
{  putxy(fontGC, left, top, text); }
}; // class Font
}  // namespace xcb
#endif // XCB_FONT_H_INCLUDED
