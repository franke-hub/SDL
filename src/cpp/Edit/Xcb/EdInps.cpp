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
//       EdInps.cpp
//
// Purpose-
//       Editor: Input/output server (See EdOuts.h)
//
// Last change date-
//       2024/04/06
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <stdio.h>                  // For sprintf
#include <sys/types.h>              // For system types
#include <xcb/xproto.h>             // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes extension

#include <gui/Device.h>             // For gui::Device
#include <gui/Font.h>               // For gui::Font
#include <gui/Keysym.h>             // For X11/keysymdef
#include <gui/Types.h>              // For gui::DEV_EVENT_MASK
#include <gui/Window.h>             // For gui::Window
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For pub::fileman::Name
#include <pub/List.h>               // For pub::List
#include <pub/Trace.h>              // For pub::Trace
#include <pub/Utf.h>                // For pub::Utf classes

#include "Active.h"                 // For Active
#include "Config.h"                 // For Config, namespace config
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile
#include "EdHist.h"                 // For EdHist
#include "EdInps.h"                 // For EdInps, implemented
#include "EdMark.h"                 // For EdMark

using namespace config;             // For config::opt_*, ...
using namespace pub::debugging;     // For debugging
using pub::Trace;                   // For pub::Trace

typedef pub::Utf::utf8_t   utf8_t;  // Import utf8_t
typedef pub::Utf::utf16_t  utf16_t; // Import utf16_t
typedef pub::Utf::utf32_t  utf32_t; // Import utf32_t

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

,  KP_MAX= 0xffbf                   // Keypad maximum key value
,  KP_MIN= 0xff80                   // Keypad minimum key value
}; // Compilation controls

//----------------------------------------------------------------------------
// Keypad conversion table (Dependent upon /usr/include/X11/keysymdef.h)
//----------------------------------------------------------------------------
static unsigned short  kp_num[64]=  // Keypad conversion table, numlock on
{     ' ', 0xff81, 0xff82, 0xff83, 0xff84, 0xff85, 0xff86, 0xff87 // 0xff80 ..
,  0xff88, 0xff89, 0xff8a, 0xff8b, 0xff8c, 0xff0d, 0xff8e, 0xff8f
,  0xff90, 0xff91, 0xff92, 0xff93, 0xff94,    '7',    '4',    '8'
,     '6',    '2',    '9',    '3',    '1',    '5',    '0',    '.' // 0xff98 ..
,  0xffa0, 0xffa1, 0xffa2, 0xffa3, 0xffa4, 0xffa5, 0xffa6, 0xffa7
,  0xffa8, 0xffa9,    '*',    '+', 0xffac,    '-', 0xffae,    '/'
,     '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7' // 0xffb0 ..
,     '8',    '9', 0xffba, 0xffbb, 0xffbc,    '=', 0xffbe, 0xffbf // 0xffb8 ..
}; // kp_num

static unsigned short  kp_off[64]=  // Keypad conversion table, numlock off
{  0xff80, 0xff81, 0xff82, 0xff83, 0xff84, 0xff85, 0xff86, 0xff87 // 0xff80 ..
,  0xff88, 0xff89, 0xff8a, 0xff8b, 0xff8c, 0xff0d, 0xff8e, 0xff8f
,  0xff90, 0xff91, 0xff92, 0xff93, 0xff94, 0xff50, 0xff51, 0xff52
,  0xff53, 0xff54, 0xff55, 0xff56, 0xff57, 0xff58, 0xff63, 0xffff // 0xff98 ..
,  0xffa0, 0xffa1, 0xffa2, 0xffa3, 0xffa4, 0xffa5, 0xffa6, 0xffa7
,  0xffa8, 0xffa9,    '*',    '+', 0xffac,    '-', 0xffae,    '/'
,     '0',    '1',    '2',    '3',    '4',    '5',    '6',    '7' // 0xffb0 ..
,     '8',    '9', 0xffba, 0xffbb, 0xffbc,    '=', 0xffbe, 0xffbf // 0xffb8 ..
}; // kp_off

// (Statically) verify kp_num and kp_off definitions
static_assert(0xff80 == XK_KP_Space     && 0xffbf == XK_F2
           && 0xff8d == XK_KP_Enter     && 0xff0d == XK_Return
           && 0xff95 == XK_KP_Home      && 0xff50 == XK_Home
           && 0xff96 == XK_KP_Left      && 0xff51 == XK_Left
           && 0xff97 == XK_KP_Up        && 0xff52 == XK_Up
           && 0xff98 == XK_KP_Right     && 0xff53 == XK_Right
           && 0xff99 == XK_KP_Down      && 0xff54 == XK_Down
           && 0xff9a == XK_KP_Page_Up   && 0xff55 == XK_Page_Up
           && 0xff9b == XK_KP_Page_Down && 0xff56 == XK_Page_Down
           && 0xff9c == XK_KP_End       && 0xff57 == XK_End
           && 0xff9d == XK_KP_Begin     && 0xff58 == XK_Begin
           && 0xff9e == XK_KP_Insert    && 0xff63 == XK_Insert
           && 0xff9f == XK_KP_Delete    && 0xffff == XK_Delete
             , "/usr/include/X11/keysymdefs mismatch");

//----------------------------------------------------------------------------
//
// Subroutine-
//       key_to_name
//
// Purpose-
//       Convert xcb_keysym_t to its name.
//
//----------------------------------------------------------------------------
static const char*                  // The name of the key
   key_to_name(xcb_keysym_t key)    // Convert xcb_keysym_t to name
{
   static char buffer[8];           // (Static) return buffer
   static const char* F_KEY= "123456789ABCDEF";

   if( key >= 0x0020 && key <= 0x007f ) { // If text key (but not TAB or ESC)
     buffer[0]= char(key);
     buffer[1]= '\0';
     return buffer;
   }

   if( key >= XK_F1 && key <= XK_F12 ) { // If function key
     buffer[0]= 'F';
     buffer[1]= F_KEY[key - XK_F1];
     buffer[2]= '\0';
     return buffer;
   }

   switch( key ) {                  // Handle control key
     case XK_ISO_Left_Tab:
       return "Left tab";

     case XK_BackSpace:
       return "BackSpace";

     case XK_Tab:
       return "Tab";

     case XK_Return:
       return "Return";

     case XK_Pause:
       return "Pause";

     case XK_Scroll_Lock:
       return "Scroll lock";

     case XK_Escape:
       return "Escape";

     case XK_Delete:
       return "Delete";

     case XK_Insert:
       return "Insert";

     case XK_Num_Lock:
       return "Num lock";

     case XK_Home:
       return "Home";

     case XK_End:
       return "End";

     case XK_Menu:
       return "Menu";

     case XK_Break:
       return "Break";

     case XK_Left:
       return "Left arrow";

     case XK_Up:
       return "Up arrow";

     case XK_Right:
       return "Right arrow";

     case XK_Down:
       return "Down arrow";

     case XK_Page_Up:
       return "Page up";

     case XK_Page_Down:
       return "Page down";

     case XK_Shift_L:
     case XK_Shift_R:
       return "Shift";

     case XK_Alt_L:
     case XK_Alt_R:
       return "Alt";

     case XK_Control_L:
     case XK_Control_R:
       return "Ctrl";

     default:
       break;
   }

   sprintf(buffer, "0x%.2x", key);
   return buffer;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_text_key
//
// Purpose-
//       Is the key a text key?
//
//----------------------------------------------------------------------------
static bool
   is_text_key(                     // Is key a text key?
     xcb_keysym_t      key)         // Input key
{
   if( (key >= 0x0020 && key < 0x007F) // If standard text key
       || key == '\b' || key == '\t' || key == '\x1B' ) // or escaped key
     return true;

   return false;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       is_protected_key
//
// Purpose-
//       Determine whether a keypress is allowed for a protected line.
//
//----------------------------------------------------------------------------
static int                          // Return code, TRUE if error message
   is_protected_key(                // Is keypress protected
     xcb_keysym_t      key,         // Input key
     int               state)       // Alt/Ctl/Shift state mask
{
   if( is_text_key(key) ) {         // If text key
     int mask= state & (gui::KS_ALT | gui::KS_CTRL);

     if( mask ) {
       key= toupper(key);
       if( mask == gui::KS_ALT ) {
         switch(key) {                // Allowed keys:
           case 'C':                  // COPY MARK
           case 'D':                  // DELETE MARK
           case 'I':                  // INSERT
           case 'M':                  // MOVE MARK
           case 'Q':                  // QUIT
           case 'U':                  // UNDO MARK
             return false;

           default:
             break;
         }
       } else if( mask == gui::KS_CTRL ) {
         switch(key) {                // Allowed keys:
           case 'C':                  // COPY MARK
           case 'Q':                  // QUIT (safe)
           case 'S':                  // SAVE
           case 'V':                  // PASTE COPY
           case 'X':                  // CUT MARK
           case 'Y':                  // REDO
           case 'Z':                  // UNDO
             return false;

           default:
             break;
         }
       }
     }
   } else {
     switch( key ) {                // Check for disallowed keys
       case 0x007F:                 // (DEL KEY, should not occur)
       case XK_BackSpace:
       case XK_Delete:
         break;

       default:                     // All others allowed
         return false;
     }
   }

   editor::put_message("Protected");
   return true;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::EdInps
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   EdInps::EdInps(                  // Constructor
     Widget*           parent,      // Parent Widget
     const char*       name)        // Widget name
:  Window(parent, name ? name : "EdInps")
,  font(*editor::font)
{
   if( opt_hcdm )
     debugh("EdInps(%p)::EdInps\n", this);

   // Handle EdMark::ChangeEvent (lambda function)
   // (Implement in EdOuts.cpp)

   // Basic window colors
   bg= config::text_bg;
   fg= config::text_fg;

   // Layout
   col_size= config::geom.width;
   row_size= config::geom.height;
   unsigned mini_c= MINI_C;
   unsigned mini_r= MINI_R;
   if( mini_c > col_size ) mini_c= col_size;
   if( mini_r > row_size ) mini_r= row_size;
   min_size= { gui::WH_t(mini_c   * font.length.width  + 2)
             , gui::WH_t(mini_r   * font.length.height + 2) };
   use_size= { gui::WH_t(col_size * font.length.width  + 2)
             , gui::WH_t(row_size * font.length.height + 2) };
   use_unit= { gui::WH_t(font.length.width), gui::WH_t(font.length.height) };

   // Set window event mask
   emask= XCB_EVENT_MASK_KEY_PRESS
//      | XCB_EVENT_MASK_KEY_RELEASE
        | XCB_EVENT_MASK_BUTTON_PRESS
        | XCB_EVENT_MASK_POINTER_MOTION
        | XCB_EVENT_MASK_BUTTON_MOTION
        | XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_STRUCTURE_NOTIFY
//      | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
        | XCB_EVENT_MASK_FOCUS_CHANGE
//      | XCB_EVENT_MASK_PROPERTY_CHANGE
        ;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::~EdInps
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   EdInps::~EdInps( void )          // Destructor
{
   if( opt_hcdm )
     debugh("EdInps(%p)::~EdInps\n", this);

   if( flipGC ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, flipGC) );
   if( fontGC ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, fontGC) );
   if( markGC ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, markGC) );
   if( bg_chg ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, bg_chg) );
   if( bg_sts ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, bg_sts) );
   if( gc_chg ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_chg) );
   if( gc_msg ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_msg) );
   if( gc_sts ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_sts) );

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   EdInps::debug(                   // Debugging display
     const char*       info) const  // Associated info
{
   debugf("EdOuts(%p)::debug(%s) Named(%s)\n", this, info ? info : ""
         , get_name().c_str());

   debugf("..head(%p) tail(%p) col_size(%u) row_size(%u) row_used(%u)\n"
         , head, tail, col_size, row_size, row_used);
   debugf("..motion(%d,%d,%d,%d)\n", motion.state, motion.time
         , motion.x, motion.y);
   debugf("..fontGC(%u) flipGC(%u) markGC(%u)\n", fontGC, flipGC, markGC);
   debugf("..gc_chg(%u) gc_msg(%u) gc_sts(%u)\n"
         , gc_chg, gc_msg, gc_sts);
   debugf("..protocol(%u) wm_close(%u)\n", protocol, wm_close);
   Window::debug(info);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
void
   EdInps::configure( void )        // Configure the Window
{
   if( opt_hcdm )
     debugh("EdInps(%p)::configure\n", this);

   Window::configure();             // Create the Window
   flush();

   // Create the graphic contexts
   fontGC= font.makeGC(fg, bg);          // (The default)
   flipGC= font.makeGC(bg, fg);          // (Inverted)
   markGC= font.makeGC(mark_fg,    mark_bg);
   bg_chg= font.makeGC(change_bg,  change_bg);
   bg_sts= font.makeGC(status_bg,  status_bg);
   gc_chg= font.makeGC(change_fg,  change_bg);
   gc_msg= font.makeGC(message_fg, message_bg);
   gc_sts= font.makeGC(status_fg,  status_bg);

   // Configure views
   EdView* const data= editor::data;
   data->gc_flip= flipGC;
   data->gc_font= fontGC;
   data->gc_mark= markGC;
   EdHist* const hist= editor::hist;
   hist->gc_flip= flipGC;

   // Set up WM_DELETE_WINDOW protocol handler
   protocol= name_to_atom("WM_PROTOCOLS", true);
   wm_close= name_to_atom("WM_DELETE_WINDOW");
   ENQUEUE("xcb_change_property", xcb_change_property_checked
          ( c, XCB_PROP_MODE_REPLACE, widget_id
          , protocol, 4, 32, 1, &wm_close) );
   if( opt_hcdm )
     debugh("%4d %s PROTOCOL(%d), atom WM_CLOSE(%d)\n", __LINE__, __FILE__
           , protocol, wm_close);

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::grab_mouse
//       EdInps::hide_mouse
//       EdInps::show_mouse
//
// Purpose-
//       Grab the mouse cursor
//       Hide the mouse cursor
//       Show the mouse cursor
//
// Implementation notes-
//       xcb_configure_window has no effect before the first window::draw().
//
//----------------------------------------------------------------------------
void
   EdInps::grab_mouse( void )       // Grab the mouse cursor
{
   using gui::WH_t;

   uint32_t x_origin= config::geom.x;
   uint32_t y_origin= config::geom.y;
   if( x_origin || y_origin ) {     // If position specified
     uint16_t mask= XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
     uint32_t parm[2];
     parm[0]= x_origin;
     parm[1]= y_origin;
     ENQUEUE("xcb_configure_window", xcb_configure_window_checked
            (c, widget_id, mask, parm) );
   } else {                         // If position assigned
     flush();                       // (Yes, this is needed)
     xcb_get_geometry_cookie_t cookie= xcb_get_geometry(c, widget_id);
     xcb_get_geometry_reply_t* r= xcb_get_geometry_reply(c, cookie, nullptr);
     if( r ) {
       x_origin= r->x;
       y_origin= r->y;
       free(r);
     } else {
       debugf("%4d EdInps xcb_get_geometry error\n", __LINE__);
     }
   }

   x_origin += rect.width/2;
   y_origin += rect.height/2;
   NOQUEUE("xcb_warp_pointer", xcb_warp_pointer
          (c, XCB_NONE, widget_id, 0,0,0,0
          , WH_t(x_origin), WH_t(y_origin)) );
   flush();
}

void
   EdInps::hide_mouse( void )       // Hide the mouse cursor
{
   if( motion.state != CS_HIDDEN ) { // If not already hidden
     NOQUEUE("xcb_hide_cursor", xcb_xfixes_hide_cursor(c, widget_id) );
     motion.state= CS_HIDDEN;
     flush();
   }
}

void
   EdInps::show_mouse( void )       // Show the mouse cursor
{
   if( motion.state != CS_VISIBLE ) { // If not already visible
     NOQUEUE("xcb_show_cursor", xcb_xfixes_show_cursor(c, widget_id) );
     motion.state= CS_VISIBLE;
     flush();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::key_alt
//
// Purpose-
//       Handle alt-key event
//
//----------------------------------------------------------------------------
void
   EdInps::key_alt(                 // Handle this
     xcb_keysym_t      key)         // Alt_Key input event
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdMark* const mark= editor::mark;

   switch(key) {                    // ALT-
     case 'B': {                    // Mark block
       editor::put_message(mark->mark(file, data->cursor, data->get_column()));
       draw();
       break;
     }
     case 'C': {                    // Copy the mark
       const char* error= mark->verify_copy(data->cursor);
       if( error ) {
         editor::put_message(error);
         break;
       }

       mark->copy();
       mark->paste(file, data->cursor, data->get_column());
       draw();
       break;
     }
     case 'D': {                    // Delete the mark
       editor::put_message( mark->cut() );
       draw();
       break;
     }
     case 'J': {                    // Join lines
       editor::put_message( editor::do_join() ); // Join current/next lines
       break;
     }
     case 'I': {                    // Insert line
       editor::put_message( editor::do_insert() ); // Insert line after cursor
       break;
     }
     case 'L': {                    // Mark line
       editor::put_message( mark->mark(file, data->cursor) );
       draw();
       break;
     }
     case 'M': {                    // Move mark (Uses cut/paste)
       const char* error= mark->verify_move(data->cursor);
       if( error ) {
         editor::put_message(error);
         break;
       }

       editor::put_message( mark->cut() );
       mark->paste(file, data->cursor, data->get_column());
       draw();
       break;
     }
     case 'P': {                    // Format paragraph
       data->commit();
       editor::put_message( mark->format() ); // Format the paragraph
       break;
     }
     case 'S': {                    // Split line
       editor::put_message( editor::do_split() ); // Split the current line
       break;
     }
     case 'U': {                    // Undo mark
       EdFile* mark_file= mark->mark_file;
       mark->undo();
       if( file == mark_file )
         draw();
       else
         draw_top();                // (Remove "No Mark" message)
       break;
     }
     case '\\': {                   // Escape
       keystate |= KS_ESC;
       break;
     }
     default:
       editor::put_message("Invalid key");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::key_ctl
//
// Purpose-
//       Handle ctl-key event
//
//----------------------------------------------------------------------------
void
   EdInps::key_ctl(                 // Handle this
     xcb_keysym_t      key)         // Ctrl_Key input event
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdMark* const mark= editor::mark;

   switch(key) {                    // CTRL-
     case 'C': {                    // Copy the mark
       editor::put_message( mark->copy() );
       break;
     }
     case 'Q': {                    // Quit
       editor::put_message( editor::do_quit() );
       break;
     }
     case 'S': {                    // Save
       data->commit();
       const char* error= editor::write_file(nullptr);
       if( error )
         editor::put_message(error);
       else
         draw_top();
       break;
     }
     case 'V': {                    // Paste (from copy/cut)
       data->commit();
       const char* error= mark->paste(file, data->cursor, data->get_column());
       if( error )
         editor::put_message(error);
       else
         draw();
       break;
     }
     case 'X': {                    // Cut the mark
       editor::put_message( mark->cut() );
       draw();
       break;
     }
     case 'Y': {                    // REDO
       data->commit();
       file->redo();
       break;
     }
     case 'Z': {                    // UNDO
       if( data->active.undo() ) {
         data->draw_active();
         draw_top();
       } else
         file->undo();
       break;
     }
     default:
       editor::put_message("Invalid key");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdInps::putxy
//
// Purpose-
//       Draw text at [left,top] point
//
//----------------------------------------------------------------------------
void
   EdInps::putxy(                   // Draw text
     xcb_gcontext_t    fontGC,      // The target graphic context
     unsigned          left,        // Left (X) offset
     unsigned          top,         // Top  (Y) offset
     const char*       text)        // Using this text
{  if( opt_hcdm && opt_verbose > 0 ) {
     char buffer[24];
     if( strlen(text) < 17 )
       strcpy(buffer, text);
     else {
       memcpy(buffer, text, 16);
       strcpy(buffer+16, "...");
     }
     debugh("EdOuts(%p)::putxy(%u,[%d,%d],'%s')\n", this
           , fontGC, left, top, buffer);
   }

   enum{ DIM= 256 };                // xcb_image_text_16 maximum length
   xcb_char2b_t out[DIM];           // UTF16 output buffer

   unsigned outlen= 0;              // UTF16 output buffer length
   unsigned outorg= left;           // Current output origin index
   unsigned outpix= left;           // Current output pixel index
   for(auto it= pub::Utf8::const_iterator((const utf8_t*)text); *it; ++it) {
     if( outlen > (DIM-4) ) {       // If time for a partial write
       NOQUEUE("xcb_image_text_16", xcb_image_text_16
              ( c, uint8_t(outlen), widget_id, fontGC
              , uint16_t(outorg), uint16_t(top + font.offset.y), out) );
       outorg= outpix;
       outlen= 0;
     }

     utf32_t code= *it;             // Next encoding
     outpix += font.length.width;   // Ending pixel (+1)
     if( outpix >= rect.width || code == 0 ) // If at end of encoding
       break;

     pub::Utf16::encode(code, (utf16_t*)out + outlen);
     outlen += pub::Utf16::length(code);
   }

   if( outlen )                     // If there's something left to render
     NOQUEUE("xcb_image_text_16", xcb_image_text_16
            ( c, uint8_t(outlen), widget_id, fontGC
            , uint16_t(outorg), uint16_t(top + font.offset.y), out) );
}

//----------------------------------------------------------------------------
//
// Method-
//       Window event handler methods
//
// Purpose-
//       Window *_event_t handlers
//
//----------------------------------------------------------------------------
void
   EdInps::button_press(            // Handle this
     xcb_button_press_event_t* event) // Button press event
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdHist* const hist= editor::hist;
   EdView* const view= editor::view;

   // Use E.detail and gui::Types::BUTTON_TYPE to determine button
   // E.root_x/y is position on root window; E.event_x/y is position on window
   xcb_button_release_event_t& E= *event;
   if( opt_hcdm && opt_verbose > 0 )
     debugh("button:   %.2x root[%d,%d] event[%d,%d] state(0x%.4x)"
           " ss(%u) rec(%u,%u,%u)\n"
           , E.detail, E.root_x, E.root_y, E.event_x, E.event_y, E.state
           , E.same_screen, E.root, E.event, E.child);

   size_t current_col= view->get_column(); // The current column number
   unsigned button_row= get_row(E.event_y); // (Absolute) button row

   switch( E.detail ) {
     case gui::BT_LEFT: {           // Left button
       if( button_row < USER_TOP ) { // If on top of screen
         if( !file->rem_message() ) { // If no message removed or remains
           if( view == hist )       // If history active
             move_cursor_H(hist->col_zero + get_col(E.event_x)); // Update column
           else
             hist->activate();
         }
         draw_top();
         break;
       }

       // Button press is on data screen
       if( view == hist ) {         // If history active
         data->activate();
         draw_top();
       }

       if( button_row != data->row ) { // If row changed
         if( button_row > row_used ) // (Button should not cause scroll up)
           button_row= row_used;
         data->move_cursor_V(button_row - data->row); // Set new row
       }
       move_cursor_H(data->col_zero + get_col(E.event_x)); // Set new column
       break;
     }
     case gui::BT_RIGHT: {          // Right button
       if( button_row < USER_TOP ) { // If on top of screen
         if( file->rem_message() ) { // If message removed
           draw_top();
           break;
         }

         // Invert the view
         editor::do_view();
       }
       break;
     }
     case gui::WT_PUSH:             // Mouse wheel push (away)
       move_screen_V(-3);
       break;

     case gui::WT_PULL:             // Mouse wheel pull (toward)
       move_screen_V(+3);
       break;

     case gui::WT_LEFT:             // Mouse wheel left
       move_cursor_H(current_col > 3 ? current_col - 3 : 0);
       break;

     case gui::WT_RIGHT:            // Mouse wheel right
       move_cursor_H(current_col + 3);
       break;

     case gui::BT_CNTR:             // Middle button (ignored)
     default:                       // (Buttons 6 and 7 undefined)
       break;
   }
}

void
   EdInps::client_message(          // Handle this
     xcb_client_message_event_t* E) // Client message event
{
   if( opt_hcdm )
     debugh("message: type(%u) data(%u)\n", E->type, E->data.data32[0]);

   if( E->type == protocol && E->data.data32[0] == wm_close )
     device->operational= false;
}

void
   EdInps::configure_notify(        // Handle this
     xcb_configure_notify_event_t* E) // Configure notify event
{
   if( opt_hcdm )
     debugh("configure_notify(%d,%d) window(%x)\n"
           , E->width, E->height, E->window);

   // (Ignore anything other than a window size change, e.g. window movement)
   if( rect.width != E->width || rect.height != E->height )
     resized(E->width, E->height);
}

void
   EdInps::expose(                  // Handle this
     xcb_expose_event_t* E)         // Expose event
{
   if( opt_hcdm )
     debugh("expose(%x) %d [%d,%d,%d,%d]\n"
           , E->window, E->count, E->x, E->y, E->width, E->height);

   draw();
}

void
   EdInps::focus_in(                // Handle this
     xcb_focus_in_event_t* E)       // Focus-in event
{  using namespace editor;

   if( opt_hcdm && opt_verbose > 0 )
     debugh("gain focus: detail(%d) event(%d) mode(%d)\n"
           , E->detail, E->event, E->mode);

   if( !(view == hist && file->mess_list.get_head()) ) {
     draw_cursor();
     flush();
   }
   status |= SF_FOCUS;

   show_mouse();
}

void
   EdInps::focus_out(               // Handle this
     xcb_focus_out_event_t* E)      // Focus-out event
{  using namespace editor;

   if( opt_hcdm && opt_verbose > 0 )
     debugh("lost focus: detail(%d) event(%d) mode(%d)\n"
           , E->detail, E->event, E->mode);

   if( !(view == hist && file->mess_list.get_head()) ) {
     undo_cursor();
     flush();
   }
   status &= ~(SF_FOCUS);

   hide_mouse();
}

void
   EdInps::key_input(               // Handle this
     xcb_keysym_t      key,         // Key input event
     int               state)       // Alt/Ctl/Shift state mask
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdView* const view= editor::view;

   // Diagnostics
   const char* key_name= key_to_name(key);
   Trace::trace(".KEY", (state<<16) | (key & 0x0000ffff), key_name);
   if( opt_hcdm && opt_verbose > 0 )
     debugh("EdInps(%p)::key_input(0x%.4x,%.4x) '%s'\n", this
           , key, state, key_name);

   // Convert Keypad keys to standard keys
   if( key >= KP_MIN && key <= KP_MAX ) {
     unsigned short* kp_tab= kp_off;
     if( state & gui::KS_NUML )
       kp_tab= kp_num;
     key= kp_tab[key - KP_MIN];
   }

   // Handle character escape state
   if( keystate & KS_ESC ) {
     if( key == XK_BackSpace || key == XK_Tab || key == XK_Escape
         || is_text_key(key) ) {
       key &= 0x00FF;               // Keys "cleverly chosen to map to ASCII"
       state= 0;
     }
     keystate ^= KS_ESC;
   }

   // Handle protected line
   if( view == data ) {             // Only applies to data view
     if( data->cursor->flags & EdLine::F_PROT // If protected line
         && is_protected_key(key, state) ) // And modification key
       return;                      // (Disallowed)
   }

   // Handle message completion, removing informational messages.
   file->rem_message_type();        // Remove current informational message
   if( draw_message() )             // If another message is present
     return;                        // (Return, ignoring the current key)

   if( status & (SF_MESSAGE | SF_NFC_MESSAGE) ) { // If a message completed
     status &= ~SF_MESSAGE;         // (SF_NFC_MESSAGE status removed later)
     draw_history();
   }

   // Handle input key
   size_t column= view->get_column(); // The cursor column
   if( is_text_key(key) ) {         // If text key
     int mask= state & (gui::KS_ALT | gui::KS_CTRL);
     if( mask ) {
       key= toupper(key);
       switch(mask) {
         case gui::KS_ALT:
           key_alt(key);
           break;

         case gui::KS_CTRL:
           key_ctl(key);
           break;

         default:                   // (KS_ALT *AND* KS_CTRL)
           editor::put_message("Invalid key");
       }
       return;
     }

     if( editor::data_protected() )
       return;

     if( keystate & KS_INS ) {      // If Insert state
       view->active.insert_char(column, key);
       if( move_cursor_H(column + 1) )
         view->draw_active();
     } else {
       view->active.replace_char(column, key);
       move_cursor_H(column + 1);
     }
     draw_top();
     draw_cursor();
     flush();

     status &= ~(SF_NFC_MESSAGE);   // "No File Changed" message not active
     return;
   }

   switch( key ) {                  // Handle data key
     case XK_Shift_L:               // Left Shift key
     case XK_Shift_R:               // Right Shift key
     case XK_Control_L:             // Left Control key
     case XK_Control_R:             // Right Control key
     case XK_Caps_Lock:             // Caps Lock key
     case XK_Shift_Lock:            // Shift Lock key
     case XK_Meta_L:                // Left Meta key
     case XK_Meta_R:                // Right Meta key
     case XK_Alt_L:                 // Left Alt key
     case XK_Alt_R:                 // Right Alt key
     case XK_Super_L:               // Left Super key
     case XK_Super_R:               // Right Super key
     case XK_Hyper_L:               // Left Hyper key
     case XK_Hyper_R:               // Right Hyper key
     case XK_Num_Lock:              // Num Lock key
       ;;                           // (Silently Ignore)
       break;

     case XK_BackSpace: {
       if( editor::data_protected() )
         return;

       if( column > 0 )
         column--;
       view->active.remove_char(column);
       move_cursor_H(column);
       view->active.append_text(" ");
       view->draw_active();
       draw_top();
       break;
     }
     case XK_Break:
     case XK_Pause: {
       if( state & gui::KS_ALT ) { // Alt-Pause, diagnostic dump
         if( editor::diagnostic ) {
           editor::diagnostic= false;
           Config::errorf("Diagnostic mode exit\n");
           pub::Trace* trace= pub::Trace::table;
           if( trace )
             trace->flag[pub::Trace::X_HALT]= false;
         } else {
           Editor::alertf("*DEBUG*"); // (Sets editor::diagnostic, stops trace)
         }
       }
       break;
     }
     case 0x007F:                  // (This encoding should not occur)
     case XK_Delete: {             // (This encoding is expected instead)
       if( editor::data_protected() )
         return;

       view->active.remove_char(column);
       view->active.append_text(" ");
       view->draw_active();
       draw_top();
       break;
     }
     case XK_Escape: {              // Escape: Invert the view
       editor::do_view();
       break;
     }
     case XK_Insert: {              // Insert key
       keystate ^= KS_INS;          // Invert the insert state
       draw_top();
       break;
     }
     case XK_Return: {
       if( state & gui::KS_CTRL) {  // If Ctrl-Enter
         editor::put_message( editor::do_insert() ); // Insert line
       } else {
         move_cursor_H(0);
         view->enter_key();
       }
       break;
     }
     case XK_Tab: {
       move_cursor_H(editor::tab_forward(column));
       break;
     }
     case XK_ISO_Left_Tab: {
       move_cursor_H(editor::tab_reverse(column));
       break;
     }

     //-----------------------------------------------------------------------
     // Function keys
     case XK_F1: {
       editor::command_help();
       break;
     }
     case XK_F2: {                  // NOT ASSIGNED
       break;
     }
     case XK_F3: {                  // (Safe) quit
       editor::put_message( editor::do_quit() );
       break;
     }
     case XK_F4: {                  // Test changed
       if( status & SF_NFC_MESSAGE ) { // (Duplicate F4 removes message)
         draw_history();
       } else {
         if( editor::un_changed() ) {
           editor::put_message("No files changed");
           status |= SF_NFC_MESSAGE;
           return;
         }
       }
       break;
     }
     case XK_F5: {
       editor::put_message( editor::do_locate() );
       break;
     }
     case XK_F6: {
       editor::put_message( editor::do_change() );
       break;
     }
     case XK_F7: {                  // Prior file
       data->commit();
       EdFile* file= editor::file->get_prev();
       if( file == nullptr )
         file= editor::file_list.get_tail();
       if( file != editor::file )
         activate(file);
       break;
     }
     case XK_F8: {                  // Next file
       data->commit();
       EdFile* file= editor::file->get_next();
       if( file == nullptr )
         file= editor::file_list.get_head();
       if( file != editor::file )
         activate(file);
       break;
     }
     case XK_F9: {                  // Copy filename/cursor line to history
       if( state & gui::KS_CTRL ) { // (^-F9) Copy cursor line to command line
         // (This sequence DOES NOT change the cursor line)
         Active& active= data->active; // The current command line
         const char* command= active.truncate(); // Truncate it
         editor::hist->activate(command); // Activate the history view
       } else {                     // (F9) Copy file name to command line
         editor::hist->activate(editor::file->name.c_str());
       }
       break;
     }
     case XK_F10: {                 // Line to top
       head= data->cursor;
       data->row_zero += (data->row - USER_TOP);
       data->row= USER_TOP;
       draw();
       break;
     }
     case XK_F11: {                 // Undo
       if( data->active.undo() ) {
         data->draw_active();
         draw_top();
       } else
         file->undo();
       break;
     }
     case XK_F12: {                 // Redo
       data->commit();
       file->redo();
       break;
     }

     //-----------------------------------------------------------------------
     // Cursor motion keys
     case XK_Home: {                // Home key
       undo_cursor();
       view->col= 0;
       if( view->col_zero ) {
         view->col_zero= 0;
         draw();
       } else
         draw_top();

       draw_cursor();
       flush();
       break;
     }
     case XK_Left: {                // Left arrow
       if( column > 0 )
         move_cursor_H(column - 1);
       break;
     }
     case XK_Up: {                  // Up arrow
       view->move_cursor_V(-1);
       break;
     }
     case XK_Right: {               // Right arrow
       move_cursor_H(column + 1);
       break;
     }
     case XK_Down: {                // Down arrow
       view->move_cursor_V(1);
       break;
     }
     case XK_Page_Up: {             // Page_Up key
       int rows= row_size - (USER_TOP + USER_BOT + 1);
       move_screen_V(-rows);
       break;
     }
     case XK_Page_Down: {           // Page_Down key
       int rows= row_size - (USER_TOP + USER_BOT + 1);
       move_screen_V(+rows);
       break;
     }
     case XK_End: {                 // End key
       move_cursor_H(view->active.get_cols());
       break;
     }
     default:
       editor::put_message("Invalid key");
       break;
   }

   status &= ~(SF_NFC_MESSAGE);     // "No File Changed" message not active
}

void
   EdInps::motion_notify(           // Handle this
     xcb_motion_notify_event_t* E)  // Motion notify event
{
   if( opt_hcdm && opt_verbose > 1 )
     debugh("motion: time(%u) detail(%d) event(%d) xy(%d,%d)\n"
           , E->time, E->detail, E->event, E->event_x, E->event_y);

   if( E->event_x != motion.x || E->event_y != motion.y ) {
     show_mouse();
   } else {
     if( (E->time - motion.time) < 1000 ) // If less than 1 second idle
       return;                      // Ignore
     if( config::USE_MOUSE_HIDE )
       hide_mouse();
   }

   motion.time= E->time;
   motion.x= E->event_x;
   motion.y= E->event_y;
}

// EdInps::property_notify does nothing except write a debugging message.
void
   EdInps::property_notify(         // Handle this
     xcb_property_notify_event_t* E) // Property notify event
{
   if( opt_hcdm )
     debugh("property_notify: window(%x) atom(%x,%s) state(%d)\n"
           , E->window, E->atom, atom_to_name(E->atom).c_str(), state);
}
