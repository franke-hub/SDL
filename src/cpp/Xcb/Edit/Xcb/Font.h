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
//       2020/09/06
//
//----------------------------------------------------------------------------
#ifndef XCB_FONT_H_INCLUDED
#define XCB_FONT_H_INCLUDED

#include "Bringup.h"                // TODO: REMOVE

#include <exception>                // For std::runtime_error

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
//            (That's when the
//         3) When done creating graphics contexts, you can (optionally)
//            close the Font. The (needed) offset remains.
//       Note that when using putxy the drawable and graphics context must
//       match. You can use putxy with multiple drawable/graphic pairs.
//
//       The underlying xcb operation is xcb_image_text_8.
//       UTF8 is (currently) NOT supported.
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
                       font_info;   // The Font information

//----------------------------------------------------------------------------
// xcb::Font::Constructor
//----------------------------------------------------------------------------
public:
   Font(                            // Constructor
     Window*           window)      // Window descriptor
:  window(window)
{
   if( opt_hcdm )
     debugh("Font(%p)::Font(%p)\n", this, window);
}

//----------------------------------------------------------------------------
// xcb::Font::Destructor
//----------------------------------------------------------------------------
public:
   ~Font( void )                    // Destructor
{
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
{
   if( opt_hcdm )
     debugh("Font(%p)::close\n", this);

   xcb_connection_t* const conn= window->connection;
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
{
   debugf("Font(%p)::debug(%s)\n", this, info ? info : "");

   debugf("..window(%p,%s) fontGC(%u) fontID(%u) offset[%d,%d] length[%u,%u]\n"
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
{
   if( opt_hcdm && opt_verbose > 1 )
     debugh("Font(%p)::makeGC(%.6x,%.6x)\n", this, uint32_t(fg), uint32_t(bg));

   if( fontID == 0 || font_info == nullptr ) {
     user_debug("Font(%p)::makeGC, Font not open\n", this);
     return 0;
   }

   // Create the Graphic Context
   xcb_connection_t* const conn= window->connection;
   xcb_drawable_t    const draw= window->window_id;

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
{
   if( opt_hcdm )
     debugh("Font(%p)::open(%s)\n", this, name ? name : "<default>");

   if( fontID || font_info )        // Reset all font information, if needed
     close();

   if( name == nullptr )            // If using system-wide default font
     name= "7x13";

   xcb_connection_t* const conn= window->connection;

   fontID= xcb_generate_id(conn);
   xcb_void_cookie_t void_cookie=
   xcb_open_font_checked(conn, fontID, strlen(name), name);
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
   length.height= font_info->max_bounds.ascent + font_info->max_bounds.descent;

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
//----------------------------------------------------------------------------
public:
void
   putxy(                           // Draw text
     xcb_gcontext_t    fontGC,      // The target graphic context
     unsigned          left,        // Left (X) offset
     unsigned          top,         // Top  (Y) offset
     const char*       text)        // Using this text
{
   if( opt_hcdm && opt_verbose > 1 )
     debugh("Font(%p)::putxy(%u,[%d,%d],'%s')\n", this
           , fontGC, left, top, text);

   uint8_t length= strlen(text);
   if( length > 255 ) length= 255;
   window->NOQUEUE("xcb_image_text_8", xcb_image_text_8
                  ( window->connection, length, window->window_id, fontGC
                  , left, top + offset.y, text) );
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
