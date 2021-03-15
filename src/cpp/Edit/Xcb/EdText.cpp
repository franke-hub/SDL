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
//       EdText.cpp
//
// Purpose-
//       Editor: Implement EdText.h
//
// Last change date-
//       2021/03/13
//
// Implementation notes-
//       EdInps.cpp provides keyboard and mouse event handlers.
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <stdio.h>                  // For sprintf
#include <sys/types.h>              // For system types
#include <xcb/xproto.h>             // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes extension

#include <gui/Device.h>             // For gui::Device
#include <gui/Font.h>               // For gui::Font
#include <gui/Types.h>              // For gui::DEV_EVENT_MASK
#include <gui/Window.h>             // For gui::Window
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For pub::fileman::Name
#include <pub/List.h>               // For pub::List
#include <pub/Trace.h>              // For pub::Trace
#include <pub/UTF8.h>               // For pub::UTF8

#include "Active.h"                 // For Active
#include "Config.h"                 // For Config, namespace config
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile
#include "EdHist.h"                 // For EdHist
#include "EdMark.h"                 // For EdMark

using namespace config;             // For config::opt_*, ...
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Use bringup debugging?
}; // Compilation controls

//----------------------------------------------------------------------------
//
// Subroutine-
//       int2char2b
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
// Subroutine-
//       trunc
//
// Purpose-
//       Truncate down.
//
//----------------------------------------------------------------------------
static inline unsigned              // The truncated value
   trunc(                           // Get truncated value
     unsigned          v,           // The value to truncate
     unsigned          unit)        // The unit length
{
   v /= unit;                       // Number of units (truncated)
   v *= unit;                       // Number of units * unit length
   return v;
}

//----------------------------------------------------------------------------
// EdText::Constructor
//----------------------------------------------------------------------------
   EdText::EdText(                  // Constructor
     Widget*           parent,      // Parent Widget
     const char*       name)        // Widget name
:  Window(parent, name ? name : "EdText")
,  active(*editor::active), font(*config::font)
{
   if( opt_hcdm )
     debugh("EdText(%p)::EdText\n", this);

   // Basic window colors
   bg= config::text_bg;
   fg= config::text_fg;

   // Layout
   col_size= config::geom.width;   row_size= config::geom.height;   unsigned mini_c= MINI_C;
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
//      | XCB_EVENT_MASK_PROPERTY_CHANGE
        ;
}

//----------------------------------------------------------------------------
// EdText::Destructor
//----------------------------------------------------------------------------
   EdText::~EdText( void )          // Destructor
{
   if( opt_hcdm )
     debugh("EdText(%p)::~EdText\n", this);

   if( flipGC ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, flipGC) );
   if( fontGC ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, fontGC) );
   if( markGC ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, markGC) );
   if( gc_chg ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_chg) );
   if( gc_msg ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_msg) );
   if( gc_sts ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_sts) );

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   EdText::debug(                   // Debugging display
     const char*       info) const  // Associated info
{
   debugf("EdText(%p)::debug(%s) Named(%s)\n", this, info ? info : ""
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
//       EdText::get_col
//       EdText::get_row
//       EdText::get_x
//       EdText::get_y
//       EdText::get_xy
//
// Purpose-
//       Convert pixel x position to (screen) column
//       Convert pixel y position to (screen) row
//       Get pixel position for column.
//       Get pixel position for row.
//       Get [col,row] pixel position.
//
//----------------------------------------------------------------------------
int                                 // The column
   EdText::get_col(                 // Get column
     int               x)           // For this x pixel position
{  return x/font.length.width; }

int                                 // The row
   EdText::get_row(                 // Get row
     int               y)           // For this y pixel position
{  return y/font.length.height; }

int                                 // The offset in Pixels
   EdText::get_x(                   // Get offset in Pixels
     int               col)         // For this column
{  return col * font.length.width + 1; }

int                                 // The offset in Pixels
   EdText::get_y(                   // Get offset in Pixels
     int               row)         // For this row
{  return row * font.length.height + 1; }

xcb_point_t                         // The offset in Pixels
   EdText::get_xy(                  // Get offset in Pixels
     int               col,         // And this column
     int               row)         // For this row
{  return {gui::PT_t(get_x(col)), gui::PT_t(get_y(row))}; }

//----------------------------------------------------------------------------
//
// Method-
//       EdText::get_text
//
// Purpose-
//       Return the line text, which differs for the cursor line
//
//----------------------------------------------------------------------------
const char*                         // The text
   EdText::get_text(                // Get text
     EdLine*           line)        // For this EdLine
{
   const char* text= line->text;    // Default, use line text
   if( line == editor::data->cursor ) // If this is the cursor line
     text= editor::data->active.get_buffer(); // Return the active buffer
   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::putxy
//
// Purpose-
//       Draw text at [left,top] point
//
// Implementation note-
//       TODO: Handle text length > 255
//
//----------------------------------------------------------------------------
void
   EdText::putxy(                   // Draw text
     xcb_gcontext_t    fontGC,      // The target graphic context
     unsigned          left,        // Left (X) offset
     unsigned          top,         // Top  (Y) offset
     const char*       text)        // Using this text
{  if( opt_hcdm && opt_verbose > 1 )
     debugh("EdText(%p)::putxy(%u,[%d,%d],'%s')\n", this
           , fontGC, left, top, text);

   pub::UTF8::Decoder decoder(text); // UTF8 input buffer
   xcb_char2b_t out[256];           // UTF16 output buffer
   unsigned outlen;                 // UTF16 output buffer length
   unsigned outpix= left;           // Current output pixel index
   for(outlen= 0; outlen<256; outlen++) {
     outpix += font.length.width;   // Ending pixel (+1)
     if( outpix > rect.width )      // If past end of screen
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

   NOQUEUE("xcb_image_text_16", xcb_image_text_16
          ( c, uint8_t(outlen), widget_id, fontGC
          , uint16_t(left), uint16_t(top + font.offset.y), out) );
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::activate
//
// Purpose-
//       Set the current file
//       Set the current line
//
//----------------------------------------------------------------------------
void
   EdText::activate(                // Activate
     EdFile*           act_file)    // This file
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;

   if( opt_hcdm )
     debugh("EdText(%p)::activate(%s)\n", this, act_file->get_name().c_str());

   // Trace file activation
   Config::trace(".ACT", "file", act_file, file); // (New, Old)

   // Out with the old
   if( file ) {
     data->commit();

     file->csr_line= data->cursor;
     file->top_line= this->head;
     file->col_zero= data->col_zero;
     file->row_zero= data->row_zero;
     file->col= data->col;
     file->row= data->row;
   }

   // In with the new
   editor::file= act_file;
   this->head= nullptr;
   if( act_file ) {
     this->head= this->tail= act_file->top_line;
     data->col_zero= act_file->col_zero;
     data->row_zero= act_file->row_zero;
     data->col=  act_file->col;
     data->row=  act_file->row;
     if( data->row < USER_TOP )
       data->row= USER_TOP;

     // Update window title, omitting middle of file name if necessary
     char buffer[64];
     const char* C= act_file->name.c_str();
     size_t      L= strlen(C);

     strcpy(buffer, "Edit: ");
     if( L > 57 ) {
       memcpy(buffer + 6, C, 27);
       memcpy(buffer + 33, "...", 3);
       strcpy(buffer + 36, C + L - 27);
     } else {
       memcpy(buffer + 6, C, L+1);
     }
     set_main_name(buffer);

     // Synchronize
     synch_active();
   }
}

void
   EdText::activate(                // Activate
     EdLine*           act_line)    // This line
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;

   // Trace line activation
   Config::trace(".ACT", "line", act_line, data->cursor); // (New, Old)

   // Activate
   undo_cursor();                   // Clear the character cursor
   data->commit();                  // Commit any active line
   data->active.reset(act_line->text); // Activate the new line
   data->cursor= act_line;          // "
   data->activate();                // Insure data view

   // Locate line on-screen
   EdLine* line= (EdLine*)this->head;
   for(unsigned r= USER_TOP; (r+1) < row_size; r++) { // Set the Active line
     if( line == act_line ) {
       data->row= r;
       draw_cursor();
       draw_info();
       return;
     }

     EdLine* next= line->get_next();
     if( next == nullptr )
       break;

     line= next;
   }

   // Line off-screen. Locate line in file
   data->row_zero= 0;
   for( line= file->line_list.get_head(); line; line= line->get_next() ) {
     if( line == act_line ) {       // If line found
       // If near top of file
       if( data->row_zero < (row_size - USER_TOP) ) {
         this->head= file->line_list.get_head();
         data->row= (unsigned)data->row_zero + USER_TOP;
         data->row_zero= 0;
         draw();
         return;
       }

       // If near end of file
       if( data->row_zero > (file->rows + 1 + USER_TOP - row_size ) ) {
         data->row_zero= file->rows + 2 + USER_TOP - row_size;
         data->row= USER_TOP;
         unsigned r= row_size - 1;
         line= file->line_list.get_tail(); // "** END OF FILE **", rows + 1
         while( r > USER_TOP ) {
           if( line == act_line )
             data->row= r;
           line= line->get_prev();
           r--;
         }
         this->head= line;
         draw();
         return;
       }

       // Not near top or end of file
       unsigned r= row_size / 2;
       data->row= r;
       data->row_zero -= r - USER_TOP;
       while( r > USER_TOP ) {
         line= line->get_prev();
         r--;
       }
       this->head= line;
       draw();
       return;
     }

     data->row_zero++;
   }

   // Line is not in file (SHOULD NOT OCCUR)
   Editor::alertf("%4d EdText file(%p) line(%p)", __LINE__
                 , file, act_line);
   data->cursor= line= file->line_list.get_head();
   data->col_zero= data->col= 0;
   data->row_zero= 0;
   data->row= USER_TOP;
   draw();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
void
   EdText::configure( void )        // Configure the Window
{
   if( opt_hcdm )
     debugh("EdText(%p)::configure\n", this);

   Window::configure();             // Create the Window
   flush();

   // Create the graphic contexts
   fontGC= font.makeGC(fg, bg);          // (The default)
   flipGC= font.makeGC(bg, fg);          // (Inverted)
   markGC= font.makeGC(mark_fg,    mark_bg);
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
   enqueue(__LINE__, "xcb_change_property"
          , xcb_change_property_checked
          ( c, XCB_PROP_MODE_REPLACE, widget_id
          , protocol, 4, 32, 1, &wm_close) );
   if( opt_hcdm )
     debugf("atom PROTOCOL(%d)\natom WM_CLOSE(%d)\n", protocol, wm_close);

   flush();
}

//----------------------------------------------------------------------------
//
// Methods-
//       EdText::draw_cursor
//       EdText::undo_cursor
//
// Purpose-
//       Set or clear the screen cursor character
//
//----------------------------------------------------------------------------
void
   EdText::draw_cursor(bool set)    // Set the character cursor
{
   EdView* const view= editor::view;

   if( opt_hcdm && opt_verbose > 1 )
     debugh("EdText(%p)::cursor_%s cursor[%u,%u]\n", this
           , set ? "S" : "C", view->col, view->row);

   if( editor::file->mess_list.get_head() ) // If message line present
     return;                        // (Do nothing)

   size_t column= view->get_column(); // The current column
   char buffer[8];                  // Encoder buffer
   pub::UTF8::Encoder encoder(buffer, sizeof(buffer));
   pub::UTF8::Decoder decoder(view->active.get_buffer(column));
   int code= decoder.decode();
   if( code <= 0 )
     code= ' ';
   encoder.encode(code);
   buffer[encoder.get_used()]= '\0';

   xcb_gcontext_t gc= set ? view->gc_flip : view->get_gc();
   putxy(gc, get_xy(view->col, view->row), buffer);

   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::draw_info
//       EdText::draw_message
//       EdText::draw_history
//       EdText::draw_status
//
// Purpose-
//       Draw the information line: draw_message, draw_history, or draw_status
//         Draw the message line
//         Draw the history line
//         Draw the status line
//
//----------------------------------------------------------------------------
static void
   format6(
     size_t            value,
     char*             buffer)
{
   if( value > 10000000 )
     sprintf(buffer, "*%.6u", unsigned(value % 1000000));
   else
     sprintf(buffer, "%7zu", value);
}

static void
   format8(
     size_t            value,
     char*             buffer)
{
   if( value > 1000000000 )
     sprintf(buffer, "*%.8u", unsigned(value % 100000000));
   else
     sprintf(buffer, "%9zu", value);
}

bool                                // Return code, TRUE if handled
   EdText::draw_history( void )     // Redraw the history line
{
   EdHist* const hist= editor::hist;
   EdView* const view= editor::view;

   if( view != hist )               // If history not active
     return false;

   if( HCDM )
     Config::trace(".DRW", "hist", hist->cursor);
   const char* buffer= hist->get_buffer();
   putxy(hist->get_gc(), 1, 1, buffer);
   draw_cursor();
   flush();
   return true;
}

void
   EdText::draw_info( void )        // Redraw the information line
{
   if( draw_message() ) return;
   if( draw_history() ) return;
   draw_status();
}

bool                                // Return code, TRUE if handled
   EdText::draw_message( void )     // Message line
{
   EdMess* mess= editor::file->mess_list.get_head();
   if( mess == nullptr ) return false;

   char buffer[256];                // Message buffer
   memset(buffer, ' ', sizeof(buffer));
   buffer[sizeof(buffer) - 1]= '\0';
   strcpy(buffer, mess->mess.c_str());
   buffer[strlen(buffer)]= ' ';

   if( HCDM )
     Config::trace(".DRW", " msg");
   putxy(gc_msg, 1, 1, buffer);
   flush();
   return true;
}

void
   EdText::draw_status( void )      // Redraw the status line
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;

   char buffer[256];                // Status line buffer
   memset(buffer, ' ', sizeof(buffer)); // (Blank fill)
   buffer[sizeof(buffer)-1]= '\0';
   // Offset:      012345678901234567890123456789012345678901234567890123456
   strcpy(buffer, "C[*******] L[*********,*********] [REP] [UNIX] EDIT V3.0");
   buffer[56]= ' ';                 // Full size (255) length buffer
   char number[16];                 // Numeric buffer
   size_t draw_col= data->get_column() + 1;
   format6(draw_col, number);
   memcpy(buffer+2, number, 7);
   size_t draw_row= data->get_row() - USER_TOP;
   format8(draw_row, number);
   memcpy(buffer+13, number, 9);
   format8(file->rows,     number);
   memcpy(buffer+23, number, 9);
   std::string S= pub::fileman::Name::get_file_name(file->name);
   size_t L= S.length();
   if( L > 192 )
     L= 192;
   memcpy(buffer+57, S.c_str(), L);

   if( keystate & KS_INS )          // Set insert mode (if not REP)
     memcpy(buffer+35, "INS", 3);
   if( file->mode != EdFile::M_UNIX ) {
     if( file->mode == EdFile::M_DOS )
       memcpy(buffer+41, "=DOS", 4);
     else if( file->mode == EdFile::M_MIX )
       memcpy(buffer+41, "=MIX", 4);
     else if( file->mode == EdFile::M_BIN ) // Set file mode (if not UNIX)
       memcpy(buffer+41, "=BIN", 4);
   }

   if( HCDM )
     Config::trace(".DRW", " sts", (void*)draw_col, (void*)draw_row);
   xcb_gcontext_t gc= gc_sts;       // Set background/foreground GC
   if( file->changed || file->damaged || data->active.get_changed() )
     gc= gc_chg;
   putxy(gc, 1, 1, buffer);
   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::draw
//
// Purpose-
//       Draw the entire screen, data and info
//
//----------------------------------------------------------------------------
void
   EdText::draw( void )             // Redraw the Window
{
   if( opt_hcdm )
     debugh("EdText(%p)::draw\n", this);

   Config::trace(".DRW", " all", head, tail);
   // Clear the drawable window
   NOQUEUE("xcb_clear_area", xcb_clear_area
          ( c, 0, widget_id, 0, 0, rect.width, rect.height) );

   // Display the text (if any)
   ssize_t col_zero= editor::data->col_zero;
   ssize_t col_last= col_zero + col_size; // Last file screen column
   tail= this->head;
   if( tail ) {
     EdMark& mark= *editor::mark;
     int is_mark= bool(mark.mark_file); // Is mark active?
     int lh_mark= 0;                // Default, mark line
     int rh_mark= col_size;
     if( mark.mark_col >= 0 ) {     // If column mark active
       if( mark.mark_lh > col_last || mark.mark_rh < col_zero ) {
         is_mark= false;            // Inactive if out of range
       } else {                     // Otherwise compute screen offsets
         lh_mark= int( mark.mark_lh - col_zero );
         rh_mark= lh_mark + int( mark.mark_rh - mark.mark_lh ) + 1;
       }
     }

     EdLine* line= tail;
     row_used= 0;
     unsigned y= get_y(USER_TOP);
     unsigned font_height= font.length.height;
     unsigned last_height= rect.height - USER_BOT * font_height;
     while( (y + font_height) <= last_height ) {
       if( line == nullptr )
         break;
       const char* text= get_text(line); // Get associated text
       if( col_zero )               // If offset
         text += pub::UTF8::index(text, col_zero);
       if( is_mark && (line->flags & EdLine::F_MARK) ) {
         // Marked lines are written in three sections:
         //  R) The unmarked Right section at the end (may be null)
         //  M) The marked Middle section (may be the entire line)
         //  L) The unmarked Left section at the beginning (may be null)
         active.reset(text);        // Load, then blank fill the line
         active.fetch(strlen(text) + col_last + 1);
         char* L= (char*)active.get_buffer(); // Text, length >= col_last + 1
         if( unsigned(rh_mark) < col_size ) { // Right section
           char* R= L + pub::UTF8::index(L, rh_mark);
           unsigned x= get_x(rh_mark);
           putxy(fontGC, x, y, R);
           *R= '\0';                // (Terminate right section)
         }
         // Middle section
         if( lh_mark < 0 ) lh_mark= 0;
         char* M= L + pub::UTF8::index(L, lh_mark);
         int x= get_x(lh_mark);
         putxy(markGC, x, y, M);
         *M= '\0';                  // (Terminate middle section)
         // Left section
         if( lh_mark > 0 )
           putxy(fontGC, 1, y, L);
       } else {
         putxy(fontGC, 1, y, text);
       }
       y += font_height;
       tail= line;
       row_used++;
       line= line->get_next();
     }
     if( opt_hcdm )
       debugf("%4d LAST xy(%d,%d)\n", __LINE__, 0, y);
   }

   draw_info();                     // Draw the information line
   draw_cursor();
   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::grab_mouse
//       EdText::hide_mouse
//       EdText::show_mouse
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
   EdText::grab_mouse( void )       // Grab the mouse cursor
{
   using gui::WH_t;

   uint32_t x_origin= config::geom.x;
   uint32_t y_origin= config::geom.y;
   uint16_t mask= XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
   uint32_t parm[2];
   parm[0]= x_origin;
   parm[1]= y_origin;
   ENQUEUE("xcb_configure_window", xcb_configure_window_checked
          (c, widget_id, mask, parm) );

   x_origin += rect.width/2;
   y_origin += rect.height/2;
   NOQUEUE("xcb_warp_pointer", xcb_warp_pointer
          (c, XCB_NONE, widget_id, 0,0,0,0
          , WH_t(x_origin), WH_t(y_origin)) );
   flush();
}

void
   EdText::hide_mouse( void )       // Hide the mouse cursor
{
   if( motion.state != CS_HIDDEN ) { // If not already hidden
     NOQUEUE("xcb_hide_cursor", xcb_xfixes_hide_cursor(c, widget_id) );
     motion.state= CS_HIDDEN;
     flush();
   }
}

void
   EdText::show_mouse( void )       // Show the mouse cursor
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
//       EdText::move_cursor_H
//
// Purpose-
//       Move cursor horizontally
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 if draw performed
   EdText::move_cursor_H(           // Move cursor horizontally
     size_t            column)      // The (absolute) column number
{
   int rc= 1;                       // Default, draw not performed

   undo_cursor();                   // Clear the current cursor

   EdView* const view= editor::view;
   size_t current= view->get_column(); // Set current column
   unsigned col_move= col_size / 8; // MINIMUM shift size
   if( col_move == 0 ) col_move= 1;
   if( column < current ) {         // If moving cursor left
     if( column < view->col_zero ) { // If shifting left
       rc= 0;                       // Redraw required
       if( column <= (col_size - col_move) )
         view->col_zero= 0;
       else
         view->col_zero= column - col_move;
     }
   } else if( column > current ) {  // If moving right
     if( column >= (view->col_zero + col_size ) ) { // If shifting right
       rc= 0;                       // Redraw required
       view->col_zero= column - col_size + col_move;
     }
   }
   view->col= unsigned(column - view->col_zero);

   if( rc ) {                       // If full redraw not needed
     draw_cursor();                 // Just set cursor
     draw_info();                   // Update status line
   } else {                         // If full redraw needed
     if( view == editor::data )     // If data view, draw_info included
       draw();
     else
       draw_info();
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::resize
//
// Purpose-
//       Resize the window
//
//----------------------------------------------------------------------------
void
   EdText::resize(                  // Resize the Window
     unsigned          x,           // New width
     unsigned          y)           // New height
{
   if( opt_hcdm )
     debugh("EdText(%p)::resize(%d,%d)\n", this, x, y);

   if( x < min_size.width )  x= min_size.width;
   if( y < min_size.height ) y= min_size.height;
   if( true  ) {                    // Truncate  size (+ 1 pixel border)
     x= trunc(x, font.length.width) + 2;
     y= trunc(y, font.length.height) + 2;
   }

   // If size unchanged, do nothing
   gui::WH_size_t size= get_size();
   if( size.width == x && size.height == y ) // If unchanged
     return;                        // Nothing to do

   // Reconfigure the window
   set_size(x, y);
   rect.width= (decltype(rect.width))x;
   rect.height= (decltype(rect.height))y;
   col_size= x / font.length.width;
   row_size= y / font.length.height;


   // Diagnostics
   if( opt_hcdm ) {
     gui::WH_size_t size= get_size();
     debugf("%4d [%d x %d]= chg_size <= [%d x %d]\n",  __LINE__
           , size.width, size.height, rect.width, rect.height);
     rect.width=  size.width;
     rect.height= size.height;
   }

// draw(); // Not required: Expose events generated by set_size()
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::synch_active
//
// Purpose-
//       Set the Active (cursor) line from the current row.
//
//----------------------------------------------------------------------------
void
   EdText::synch_active( void )     // Set the Active (cursor) line
{
   EdView* const data= editor::data;
   if( data->row < USER_TOP )       // (File initial value == 0)
     data->row= USER_TOP;

   EdLine* line= head;              // Get the top line
   const char* match_type= " row";  // Default, row match
   for(unsigned r= USER_TOP; ; r++) { // Set the Active line
     if( r == data->row ) {
//     match_type= " row";          // Row match
       break;
     }

     EdLine* next= line->get_next();
     if( next == nullptr ) {        // (Can occur if window grows)
       match_type= "next";          // Next line null
       data->row= r;
       break;
     }

     if( (r + 1) >= row_size ) {    // (Can occur if window shrinks)
       match_type= "size";          // Window shrink
       data->row= r;
       break;
     }

     line= next;
   }

   // Set the new active line (with trace)
   Config::trace(".CSR", match_type, line, data->cursor); // (New, old)
   data->cursor= line;
   data->active.reset(line->text);
   draw_cursor();
}
