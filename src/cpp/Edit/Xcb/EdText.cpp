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
//       EdText.cpp
//
// Purpose-
//       Editor: Implement EdText.h
//
// Last change date-
//       2021/01/10
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <stdio.h>                  // For fprintf TODO: REMOVE
#include <sys/types.h>              // For system types
#include <pub/List.h>               // For pub::List
#include <pub/UTF8.h>               // For pub::UTF8

#include <xcb/xproto.h>             // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes extension

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For pub::fileman::Name
#include <pub/Trace.h>              // For pub::Trace

#include "Xcb/Active.h"             // For xcb::Active
#include "Xcb/Device.h"             // For xcb::Device
#include "Xcb/Keysym.h"             // For X11/keysymdef
#include "Xcb/Types.h"              // For xcb::DEV_EVENT_MASK
#include "Xcb/Window.h"             // For xcb::Window

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
,  USE_BRINGUP= true                // Use bringup debugging? TODO: REMOVE
,  USE_HIDDEN= true                 // Use mouse hide/show? TODO: REMOVE
// USE_HIDDEN= false                // Use mouse hide/show? TODO: REMOVE
}; // Compilation controls

static struct EdText_initializer {  // TODO: REMOVE
   EdText_initializer( void )       // (DO NOT USE DEBUGF)
{  fprintf(stderr, "%4d EdText: Cursor hiding(%s)\n", __LINE__
                 , USE_HIDDEN ? "ENABLED" : "DISABLED");
}
}  static_initializer;

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
:  Window(parent, name ? name : "EdText"), font(*config::font)
{
   if( opt_hcdm )
     debugh("EdText(%p)::EdText\n", this);

   // Basic window colors
   bg= config::text_bg;
   fg= config::text_fg;

   // Layout
   if( col_size == 0 ) col_size= COLS_W;
   if( row_size == 0 ) row_size= ROWS_H;
   unsigned mini_c= MINI_C;
   unsigned mini_r= MINI_R;
   if( mini_c > col_size ) mini_c= col_size;
   if( mini_r > row_size ) mini_r= row_size;
   min_size= { xcb::WH_t(mini_c   * font.length.width  + 1)
             , xcb::WH_t(mini_r   * font.length.height + 2) };
   use_size= { xcb::WH_t(col_size * font.length.width  + 1)
             , xcb::WH_t(row_size * font.length.height + 2) };
   use_unit= { xcb::WH_t(font.length.width), xcb::WH_t(font.length.height) };

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
   if( gc_cmd ) ENQUEUE("xcb_free_gc", xcb_free_gc_checked(c, gc_cmd) );
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
   debugf("..gc_chg(%u) gc_cmd(%u) gc_msg(%u) gc_sts(%u)\n"
         , gc_chg, gc_cmd, gc_msg, gc_sts);
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
unsigned                            // The column
   EdText::get_col(                 // Get column
     unsigned          x)           // For this x pixel position
{  return x/font.length.width; }

unsigned                            // The row
   EdText::get_row(                 // Get row
     unsigned          y)           // For this y pixel position
{  return y/font.length.height; }

unsigned                            // The offset in Pixels
   EdText::get_x(                   // Get offset in Pixels
     unsigned          col)         // For this column
{  return col * font.length.width + 1; }

unsigned                            // The offset in Pixels
   EdText::get_y(                   // Get offset in Pixels
     unsigned          row)         // For this row
{  return row * font.length.height + 1; }

xcb_point_t                         // The offset in Pixels
   EdText::get_xy(                  // Get offset in Pixels
     unsigned          col,         // And this column
     unsigned          row)         // For this row
{  return {xcb::PT_t(get_x(col)), xcb::PT_t(get_y(row))}; }

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
     xcb::Line*        line)        // For this Line
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
   unsigned outpix= left;           // Current output pixel
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
//     printf("%4d HCDM\n", __LINE__); // TODO: REMOVE
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
//     printf(" %4drow_zero(%zd) rows(%zd) row_size(%u) USER_TOP(%u)\n", __LINE__
//           , data->row_zero, file->rows, row_size, USER_TOP);

       // If near top of file
       if( data->row_zero < (row_size - USER_TOP) ) {
//       printf("%4d HCDM\n", __LINE__); // TODO: REMOVE
         this->head= file->line_list.get_head();
         data->row= (unsigned)data->row_zero + USER_TOP;
         data->row_zero= 0;
         draw();
         return;
       }

       // If near end of file
       if( data->row_zero > (file->rows + 1 + USER_TOP - row_size ) ) {
//       printf("%4d HCDM\n", __LINE__); // TODO: REMOVE
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

       // Not near top nor end of file
//     printf("%4d HCDM\n", __LINE__); // TODO: REMOVE
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
   Editor::alertf("%4d HCDM EdText file(%p) line(%p)", __LINE__
                 , file, act_line); // TODO: REMOVE
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
   gc_cmd= font.makeGC(command_fg, command_bg);
   gc_msg= font.makeGC(message_fg, message_bg);
   gc_sts= font.makeGC(status_fg,  status_bg);

   // Configure views
   EdView* const data= editor::data;
   data->gc_flip= flipGC;
   data->gc_font= fontGC;
   data->gc_mark= markGC;
   EdHist* const hist= editor::hist;
   hist->gc_flip= flipGC;
   hist->gc_font= gc_cmd;
   hist->gc_mark= gc_cmd;

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
   putxy(gc_cmd, 1, 1, buffer);
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

   char buffer[256];                // Status line buffer
   memset(buffer, ' ', sizeof(buffer)); // (Blank fill)
   buffer[sizeof(buffer)-1]= '\0';
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

   if( xcb::keystate & xcb::KS_INS ) // Set insert mode (if not REP)
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
   xcb::WH_size_t size= get_size(__LINE__); // TODO: ??? Why is this needed ???
   rect.width=  size.width;
   rect.height= size.height;
   NOQUEUE("xcb_clear_area", xcb_clear_area
          ( c, 0, widget_id, 0, 0, rect.width, rect.height) );

   // Display the text (if any)
   size_t col_zero= editor::data->col_zero;
   tail= nullptr;
   if( this->head ) {
     EdLine* line= this->head;
     tail= line;

     row_used= 0;
     unsigned y= get_y(USER_TOP);
     unsigned font_height= font.length.height;
     unsigned last_height= rect.height - USER_BOT * font_height;
     while( (y + font_height) <= last_height ) {
       if( line == nullptr )
         break;
       const char* text= get_text(line); // Get associated text
       if( col_zero )               // If offset
         text += pub::UTF8::index((const unsigned char*)text, col_zero);
       xcb_gcontext_t gc= fontGC;
       if( line->flags & EdLine::F_MARK ) {
         gc= markGC;
         active.reset(text);        // Fill line
         active.fetch(strlen(text) + 256);
         text= active.get_buffer();
       }
       putxy(gc, 1, y, text);
       y += font_height;
       tail= line;
       row_used++;
       line= line->get_next();
     }
     if( opt_hcdm ) debugf("%4d LAST xy(%d,%d)\n", __LINE__, 0, y);

     if( USE_BRINGUP ) {
       // BRINGUP: Draw diagonal line (to see where boundaries are)
       if( opt_hcdm && opt_verbose > 2 ) { // (This is still optional)
//       debug(pub::utility::to_string("%4d EdText diagonal", __LINE__).c_str());
         xcb_point_t points[2]= { {0,                0}
                                , {xcb::PT_t(rect.width), xcb::PT_t(rect.height)}
                                };
         NOQUEUE("xcb_poly_line", xcb_poly_line(c
                , XCB_COORD_MODE_ORIGIN, widget_id, fontGC, 2, points));
         if( opt_verbose > 2 )
           debugf("%4d POLY {0,{%d,%d}}\n", __LINE__, rect.width, rect.height);
       }
     }
   }

   draw_info();                     // Draw the information line
   draw_cursor();
   show();
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
   using xcb::WH_t;

   uint32_t x_origin= 1030;
   uint32_t y_origin= 0;
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
   xcb::WH_size_t size= get_size(__LINE__);
   if( size.width == x && size.height == y ) // If unchanged
     return;                        // Nothing to do

   // Reconfigure the window
   set_size(x, y, __LINE__);
   col_size= x / font.length.width;
   row_size= y / font.length.height;

   // Diagnostics
   if( opt_hcdm ) {
     xcb::WH_size_t size= get_size();
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
//       Set the Active (cursor) line to the current row.
//
// Inputs-
//       this->head= top screen line
//       data->row   screen row
//
//----------------------------------------------------------------------------
void
   EdText::synch_active( void )     // Set the Active (cursor) line
{
   EdView* const data= editor::data;

   const char* code= " row";        // Default, row match
   EdLine* active= nullptr;         // Default, no active line
   EdLine* line= (EdLine*)this->head; // Get the first line
   if( line == nullptr ) {          // If Empty      // TODO: SHOULD NOT OCCUR
     Editor::alertf("%4d HCDM EdText\n", __LINE__); // TODO: REMOVE
     return;
   }

   if( data->row < USER_TOP )
     data->row= USER_TOP;

   for(unsigned r= USER_TOP; ; r++) { // Set the Active line
     if( r == data->row ) {
//     code= " row";                // Row match
       active= line;
       break;
     }

     EdLine* next= line->get_next();
     if( next == nullptr ) {        // (Can occur if window grows)
       code= "next";                // Next line null
       data->row= r;
       active= line;
       break;
     }

     if( (r + 1) >= row_size ) {    // (Can occur if window shrinks)
       code= "size";                // Window shrink
       data->row= r;
       active= line;
       break;
     }

     line= next;
   }

   // Set the new active line (with trace)
   Config::trace(".CSR", code, active, data->cursor); // (New, old)
   data->cursor= active;
   data->active.reset(active->text);
   draw_cursor();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdText::key_input
//
// Purpose-
//       Handle keypress event
//
//----------------------------------------------------------------------------
void
   EdText::key_alt(                 // Handle this
     xcb_keysym_t      key)         // Alt_Key input event
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdMark* const mark= editor::mark;

   switch(key) {                    // ALT-
     case 'B':                      // Mark block
       editor::put_message(mark->mark(file, data->cursor, data->get_column()));
       draw();
       break;

     case 'C': {                    // Copy
       const char* error= mark->copy();
       if( error ) {
         editor::put_message(error);
         break;
       }

       mark->paste(file, data->cursor);
       draw();
       break;
     }
     case 'D': {                    // Delete the mark
       editor::put_message( mark->cut() );
       draw();
       break;
     }
     case 'I': {                    // Insert
       data->commit();
       EdLine* after= data->cursor; // Insert afer the current cursor line
       if( after->get_next() == nullptr ) // If it's the last line
         after= after->get_prev();  // Use the prior line instead

       EdLine* head= file->new_line(); // Get new, empty insert line
       EdLine* tail= head;

       // Handle insert after no delimiter line
       EdRedo* redo= new EdRedo();
       if( after->delim[0] == '\0' &&  after->delim[1] == '\0' ) {
         head= file->new_line(after->text); // Get "after" line replacement

         pub::List<EdLine> list;    // Connect head and tail
         list.fifo(head);
         list.fifo(tail);

         // Remove the current "after" from the file, updating the REDO
         file->remove(after);       // (Does not modify edLine->get_prev())
         redo->head_remove= redo->tail_remove= after;
         after= after->get_prev();
       }

       // Insert the line(s) (with redo)
       data->col_zero= data->col= 0;
       file->insert(after, head, tail);
       redo->head_insert= head;
       redo->tail_insert= tail;
       file->redo_insert(redo);
       file->activate(tail);        // (Activate the newly inserted line)
       draw();                      // And redraw
       break;
     }
     case 'L':                      // Mark line
       editor::put_message( mark->mark(file, data->cursor) );
       draw();
       break;

     case 'M': {                    // Move (Uses cut/paste)
       const char* error= mark->cut();
       if( error ) {
         editor::put_message(error);
         break;
       }

       mark->paste(file, data->cursor);
       draw();
       break;
     }
     case 'Q':                      // (Safe) quit
       if( editor::un_changed() )
         editor::exit();
       break;

     case 'U': {                    // Undo mark
       EdFile* mark_file= mark->mark_file;
       mark->undo();
       if( file == mark_file )
         draw();
       break;
     }
     default:
       editor::put_message("Invalid key");
       draw_info();
   }
}

void
   EdText::key_ctl(                 // Handle this
     xcb_keysym_t      key)         // Ctrl_Key input event
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdMark* const mark= editor::mark;

   switch(key) {                    // CTRL-
     case 'C': {                    // Copy
       editor::put_message( mark->copy() );
       break;
     }
     case 'V': {                    // Paste
       const char* error= mark->paste(file, data->cursor);
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
     default:
       editor::put_message("Invalid key");
       draw_info();
   }
}

// Handle protected line. Disallow keys which modify text.
int                                 // Return code, TRUE if error message
   EdText::key_protected(           // Handle this protected line
     xcb_keysym_t      key,         // Input key
     int               state)       // Alt/Ctl/Shift state mask
{
   if( key >= 0x0020 && key < 0x007F ) { // If text key
     int mask= state & (xcb::KS_ALT | xcb::KS_CTRL);

     if( mask ) {
       if( mask == xcb::KS_ALT ) {
         key= toupper(key);
         switch(key) {
           case 'I':                  // INSERT allowed
           case 'Q':                  // QUIT allowed
           case 'U':                  // EdMark::reset allowed
             return false;
             break;

           default:
             break;
         }
       }
     }
   } else {
     switch( key ) {                // Check for disallowed key
       case 0x007F:                 // (DEL KEY, should not occur)
       case XK_BackSpace:
       case XK_Delete:
         break;

       default:                     // All others allowed
         return false;
         break;
     }
   }

   editor::put_message("Protected line");
   return true;
}

void
   EdText::key_input(               // Handle this
     xcb_keysym_t      key,         // Key input event
     int               state)       // Alt/Ctl/Shift state mask
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdView* const view= editor::view;

   if( opt_hcdm ) {
     char B[2]; B[0]= '\0'; B[1]= '\0'; const char* K= B;
     if( key >= 0x0020 && key < 0x007F ) B[0]= char(key);
     else K= editor::key_to_name(key);
     debugh("EdText(%p)::key_input(0x%.4x,%.4x) '%s'\n", this
           , key, state, K);
   }

   const char* name= editor::key_to_name(key);
   Config::trace(".KEY", (state<<16) | (key & 0x0000ffff), name);

   // Handle protected line
   if( view == data ) {             // Only applies to data view
     if( data->cursor->flags & EdLine::F_PROT // If protected line
         && key_protected(key, state) ) // And modification key
       return;                      // (Disallowed)
   }

   // Handle input key
   file->rem_message_type();        // Remove informational message
   if( file->mess_list.get_head() ) { // If any message remains
     draw_info();
     return;
   }

   size_t column= view->get_column(); // The cursor column
   if( key >= 0x0020 && key < 0x007F ) { // If text key
     int mask= state & (xcb::KS_ALT | xcb::KS_CTRL);
     if( mask ) {
       key= toupper(key);
       switch(mask) {
         case xcb::KS_ALT:
           key_alt(key);
           break;

         case xcb::KS_CTRL:
           key_ctl(key);
           break;

         default:                   // (KS_ALT *AND* KS_CTRL)
           editor::put_message("Invalid key");
       }
       return;
     }

     if( xcb::keystate & xcb::KS_INS ) { // If Insert state
       view->active.insert_char(column, key);
       if( move_cursor_H(column + 1) ) {
         const char* buffer= view->active.get_buffer();
         putxy(view->get_gc(), get_xy(view->col - 1, view->row), buffer + view->active.index(column));
       }
     } else {
       view->active.replace_char(column, key);
       move_cursor_H(column + 1);
     }
     draw_info();
     draw_cursor();
     flush();

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
       ;;                           // (Silently Ignore)
       break;

     case XK_BackSpace: {
       undo_cursor();               // Clear the cursor
       if( column > 0 )
         column--;
       view->active.remove_char(column);
       int rc= move_cursor_H(column);
       if( rc ) {                   // If only line redraw
         view->active.append_text(" "); // (Clear removed character)
         const char* buffer= view->active.get_buffer(column);
         putxy(view->get_gc(), get_xy(view->col, view->row), buffer);
         draw_cursor();
         flush();
       }
       break;
     }
     case 0x007F:                  // (Should not occur)
     case XK_Delete: {
       view->active.remove_char(column);
       view->active.append_text(" ");
       const char* buffer= view->active.get_buffer(column);
       putxy(view->get_gc(), get_xy(view->col, view->row), buffer);
       draw_info();
       draw_cursor();
       flush();
       break;
     }
     case XK_Escape:                // Escape: Invert history view
       editor::do_history();
       break;

     case XK_Insert: {              // Insert key
       xcb::keystate ^= xcb::KS_INS; // Invert the insert state
       draw_info();
       break;
     }
     case XK_Return: {
       move_cursor_H(0);
       view->enter_key();
       break;
     }
     case XK_Tab: {                 // TODO: PRELIMINARY
       enum { TAB= 8 };             // TAB Column BRINGUP (( 2 ** N ))
       column += TAB;
       column &= ~(TAB - 1);
       move_cursor_H(column);
       break;
     }
     case XK_ISO_Left_Tab:          // TODO: PRELIMINARY
       if( column ) {               // If not already at column[0]
         enum { TAB= 8 };           // TAB Column BRINGUP (( 2 ** N ))
         if( column <= TAB )
           column= 0;
         else {
           if( (column % TAB) == 0 )
             column--;
           column &= ~(TAB - 1);
         }
         move_cursor_H(column);
       }
       break;

     //-----------------------------------------------------------------------
     // Function keys
     case XK_F1: {
       printf(" F1: This help message\n"
              " F2: Bringup test\n"
              " F3: Quit File\n"
              " F4: Test changed\n"
              " F5: Locate\n"
              " F6: Change\n"
              " F7: Previous File\n"
              " F8: Next File\n"
              " F9: Quick debug\n"
              "F10: Line to top\n"
              "F11: Undo\n"
              "F12: Redo\n"
              "A-I: Insert\n"
              "A-Q: Quit\n"
       );
       break;
     }
     case XK_F2: {                  // Bringup test
       editor::do_test();
       break;
     }
     case XK_F3: {                  // (Safe) quit
       data->commit();
       editor::do_exit();
       break;
     }
     case XK_F4: {                  // Test changed
       if( editor::un_changed() )
         editor::put_message("No files changed");
       break;
     }
     case XK_F5: {
       editor::put_message(editor::do_locate());
       break;
     }
     case XK_F6: {
       editor::put_message(editor::do_change());
       break;
     }
     case XK_F7: {                  // Prior file
       data->commit();
       EdFile* file= editor::file->get_prev();
       if( file == nullptr )
         file= editor::file_list.get_tail();
       if( file != editor::file ) {
         activate(file);
         draw();
       }
       break;
     }
     case XK_F8: {                  // Next file
       data->commit();
       EdFile* file= editor::file->get_next();
       if( file == nullptr )
         file= editor::file_list.get_head();
       if( file != editor::file ) {
         activate(file);
         draw();
       }
       break;
     }
     case XK_F9: {                  // Quick debug. TODO: REMOVE/REPLACE
       pub::Trace* trace= pub::Trace::trace;
       if( trace ) {
         if( trace->flag[pub::Trace::X_HALT] ) {
           Config::errorf("Tracing resumed\n");
           trace->flag[pub::Trace::X_HALT]= false;
           return;
         }

         trace->flag[pub::Trace::X_HALT]= true;
       } // else Config::errorf("Tracing inactive\n");

       Editor::alertf("F9");
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
       if( view->active.undo() ) {
         view->active.index(view->col_zero+col_size); // Blank fill
         putxy(view->get_gc(), get_xy(0, view->row), view->active.get_buffer(view->col_zero));
         draw_info();
         draw_cursor();
       } else
         file->undo();
       break;
     }
     case XK_F12: {                 // Redo
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
         draw_info();

       draw_cursor();
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
       undo_cursor();
       data->commit();
       unsigned count= row_size - (USER_TOP + USER_BOT);
       if( head->get_prev() && count ) {
         while( --count ) {
           EdLine* up= (EdLine*)head->get_prev();
           if( up == nullptr )
             break;

           data->row_zero--;
           head= up;
         }
         synch_active();
         draw();
       }
       draw_cursor();
       break;
     }
     case XK_Page_Down: {           // Page_Down key
       undo_cursor();
       data->commit();
       unsigned count= row_size - (USER_TOP + USER_BOT);
       if( head->get_next() && count ) {
         while( --count ) {
           EdLine* up= (EdLine*)head->get_next();
           if( up == nullptr )
             break;

           data->row_zero++;
           head= up;
         }
         synch_active();
         draw();
       }
       draw_cursor();
       break;
     }
     case XK_End: {                 // End key
       move_cursor_H(view->active.get_cols());
       break;
     }

     default:
       editor::put_message("Invalid key");
   }
}

//============================================================================
// EdText::Event handlers
//============================================================================
void
   EdText::button_press(            // Handle this
     xcb_button_press_event_t* event) // Button press event
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;
   EdHist* const hist= editor::hist;
   EdView* const view= editor::view;

   // Use E.detail and xcb::Types::BUTTON_TYPE to determine button
   // E.root_x/y is position on root window; E.event_x/y is position on window
   xcb_button_release_event_t& E= *event;
   if( opt_hcdm )
     debugh("button:   %.2x root[%d,%d] event[%d,%d] state(0x%.4x)"
           " ss(%u) rec(%u,%u,%u)\n"
           , E.detail, E.root_x, E.root_y, E.event_x, E.event_y, E.state
           , E.same_screen, E.root, E.event, E.child);

   size_t current_col= view->get_column(); // The current column number
   unsigned button_row= get_row(E.event_y); // (Absolute) button row
   switch( E.detail ) {
     case xcb::BT_LEFT: {           // Left button
       if( button_row < USER_TOP ) { // If on command/history line
         if( file->rem_message() ) { // If message removed
           draw_info();
           break;
         }

         if( view == hist )         // If history active
           move_cursor_H(hist->col_zero + get_col(E.event_x)); // Update column
         else
           hist->activate();
         draw_info();
         break;
       }

       if( view == hist ) {         // If history active
         data->activate();
         draw_info();
       }

       if( button_row != view->row ) // If row changed
         data->move_cursor_V(button_row - view->row); // Set new row
       move_cursor_H(view->col_zero + get_col(E.event_x)); // Set new column
       break;
     }
     case xcb::BT_RIGHT: {          // Right button
       if( button_row < USER_TOP ) {
         if( file->rem_message() ) { // If message removed
           draw_info();
           break;
         }

         // Invert history command line mode
         editor::do_history();
       }
       break;
     }
     case xcb::WT_PUSH:             // Mouse wheel push (away)
       view->move_cursor_V(-3);
       break;

     case xcb::WT_PULL:             // Mouse wheel pull (toward)
       view->move_cursor_V(+3);
       break;

     case xcb::WT_LEFT:             // Mouse wheel left
       move_cursor_H(current_col > 3 ? current_col - 3 : 0);
       break;

     case xcb::WT_RIGHT:            // Mouse wheel right
       move_cursor_H(current_col + 3);
       break;

     case xcb::BT_CNTR:             // Middle button (ignored)
     default:                       // (Buttons 6 and 7 undefined)
       break;
   }
}

void
   EdText::client_message(          // Handle this
     xcb_client_message_event_t* E) // Client message event
{
   if( opt_hcdm )
     debugh("message: type(%u) data(%u)\n", E->type, E->data.data32[0]);

   if( E->type == protocol && E->data.data32[0] == wm_close )
     device->operational= false;
}

void
   EdText::configure_notify(        // Handle this
     xcb_configure_notify_event_t* E) // Configure notify event
{
   if( opt_hcdm )
     debugh("EdText(%p)::configure_notify(%d,%d)\n", this
           , E->width, E->height);

   resize(E->width, E->height);
}

void
   EdText::expose(                  // Handle this
     xcb_expose_event_t* E)         // Expose event
{
   if( opt_hcdm )
     debugh("EdText(%p)::expose(%d) %d [%d,%d,%d,%d]\n", this
           , E->window, E->count, E->x, E->y, E->width, E->height);

   draw();
}

void
   EdText::motion_notify(           // Handle this
     xcb_motion_notify_event_t* E)  // Motion notify event
{
   if( opt_hcdm && opt_verbose >= 0 )
     debugh("motion: time(%u) detail(%d) event(%d) xy(%d,%d)\n"
           , E->time, E->detail, E->event, E->event_x, E->event_y);

   // printf("."); fflush(stdout);  // See when called
   if( E->event_x != motion.x || E->event_y != motion.y )
     { if( USE_HIDDEN ) show_mouse(); } // TODO: REMOVE USE_HIDDEN
   else {
     if( (E->time - motion.time) < 1000 ) // If less than 1 second idle
       return;                      // Ignore
     { if( USE_HIDDEN ) hide_mouse(); } // TODO: REMOVE USE_HIDDEN
   }

   motion.time= E->time;
   motion.x= E->event_x;
   motion.y= E->event_y;
}
