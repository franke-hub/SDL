//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdTerm.cpp
//
// Purpose-
//       Editor: Implement EdTerm.h screen handler.
//
// Last change date-
//       2023/01/02
//
// Implementation notes-
//       EdInps.cpp implements keyboard and mouse event handlers.
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
#include <pub/Utf.h>                // For pub::Utf classes

#include "Active.h"                 // For Active
#include "Config.h"                 // For Config, namespace config
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile
#include "EdHist.h"                 // For EdHist
#include "EdMark.h"                 // For EdMark

using namespace config;             // For config::opt_*, ...
using namespace pub::debugging;     // For debugging
using pub::Trace;                   // For pub::Trace

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Use bringup debugging?

,  HM_ROW= 1                        // History/Message line row
}; // Compilation controls

//----------------------------------------------------------------------------
// Typedefs and enumerations
//----------------------------------------------------------------------------
typedef pub::Utf::utf8_t   utf8_t;  // Import utf8_t
typedef pub::Utf::utf16_t  utf16_t; // Import utf16_t
typedef pub::Utf::utf32_t  utf32_t; // Import utf32_t

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static pub::signals::Connector<EdMark::ChangeEvent>
                       changeEvent_connector;

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
// EdTerm::Constructor
//----------------------------------------------------------------------------
   EdTerm::EdTerm(                  // Constructor
     Widget*           parent,      // Parent Widget
     const char*       name)        // Widget name
:  Window(parent, name ? name : "EdTerm")
,  active(*editor::active), font(*config::font)
{
   if( opt_hcdm )
     debugh("EdTerm(%p)::EdTerm\n", this);

   // Handle EdMark::ChangeEvent (lambda function)
   // Purpose: Repair EdTerm::head (if it changed)
   using Event= EdMark::ChangeEvent;
   changeEvent_connector= EdMark::change_signal.connect([this](Event& event) {
     EdFile* file= event.file;
     const EdRedo* redo= event.redo;

     /// If the head line was removed, we need to adjust it so that we point
     /// to a head line that's actually in the file.
     if( head->is_within(redo->head_remove, redo->tail_remove) ) {
       for(EdLine* L= head->get_prev(); L; L= L->get_prev() ) {
         if( !L->is_within(redo->head_remove, redo->tail_remove) ) {
           head= L->get_next();
           if( file == editor::file )
             editor::data->row_zero= file->get_row(head);
           return;
         }
       }

       // This should not occur. The top line, the only one with a nullptr
       // get_prev(), should never be within a redo_remove list. This indicates
       // that something has gone very wrong and can't be auto-corrected.
       Editor::alertf("%4d EdTerm: internal error\n", __LINE__);
     }

     /// If the removal occurs in the current file prior to the head line,
     /// row_zero needs to be adjusted as well. We'll just update it since
     /// it's more work to see if it needs updating than to just do it.
     if( file == editor::file )
       editor::data->row_zero= file->get_row(head);
   });

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
        | XCB_EVENT_MASK_FOCUS_CHANGE
//      | XCB_EVENT_MASK_PROPERTY_CHANGE
        ;
}

//----------------------------------------------------------------------------
// EdTerm::Destructor
//----------------------------------------------------------------------------
   EdTerm::~EdTerm( void )          // Destructor
{
   if( opt_hcdm )
     debugh("EdTerm(%p)::~EdTerm\n", this);

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
//       EdTerm::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   EdTerm::debug(                   // Debugging display
     const char*       info) const  // Associated info
{
   debugf("EdTerm(%p)::debug(%s) Named(%s)\n", this, info ? info : ""
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
//       EdTerm::get_col
//       EdTerm::get_row
//       EdTerm::get_x
//       EdTerm::get_y
//       EdTerm::get_xy
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
   EdTerm::get_col(                 // Get column
     int               x) const     // For this x pixel position
{  return x/font.length.width; }

int                                 // The row
   EdTerm::get_row(                 // Get row
     int               y) const     // For this y pixel position
{  return y/font.length.height; }

int                                 // The offset in Pixels
   EdTerm::get_x(                   // Get offset in Pixels
     int               col) const   // For this column
{  return col * font.length.width + 1; }

int                                 // The offset in Pixels
   EdTerm::get_y(                   // Get offset in Pixels
     int               row) const   // For this row
{  return row * font.length.height + 1; }

xcb_point_t                         // The offset in Pixels
   EdTerm::get_xy(                  // Get offset in Pixels
     int               col,         // And this column
     int               row) const   // For this row
{  return {gui::PT_t(get_x(col)), gui::PT_t(get_y(row))}; }

//----------------------------------------------------------------------------
//
// Method-
//       EdTerm::get_text
//
// Purpose-
//       Return the line text, which differs for the cursor line
//
//----------------------------------------------------------------------------
const char*                         // The text
   EdTerm::get_text(                // Get text
     const EdLine*     line) const  // For this EdLine
{
   EdView* data= editor::data;
   const char* text= line->text;    // Default, use line text
   if( line == data->cursor ) {     // If this is the cursor line
     data->active.fetch(data->col_zero + col_size); // Add blank fill
     text= data->active.get_buffer(); // Return the active buffer
   }
   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdTerm::putxy
//
// Purpose-
//       Draw text at [left,top] point
//
//----------------------------------------------------------------------------
void
   EdTerm::putxy(                   // Draw text
     xcb_gcontext_t    fontGC,      // The target graphic context
     unsigned          left,        // Left (X) offset
     unsigned          top,         // Top  (Y) offset
     const char*       text)        // Using this text
{  if( opt_hcdm && opt_verbose > 0 )
     debugh("EdTerm(%p)::putxy(%u,[%d,%d],'%s')\n", this
           , fontGC, left, top, text);

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
     if( outpix > rect.width || code == 0 ) // If at end of encoding
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
//       EdTerm::activate(EdFile*)
//
// Purpose-
//       Activate, then draw a file at its current position.
//
//----------------------------------------------------------------------------
void
   EdTerm::activate(                // Activate
     EdFile*           act_file)    // This file
{
   if( opt_hcdm )
     debugh("EdTerm(%p)::activate(%s)\n", this
           , act_file ? act_file->get_name().c_str() : "nullptr");

   EdView* const data= editor::data;
   EdFile* const file= editor::file;

   // Trace file activation
   Trace::trace(".ACT", "file", file, act_file); // (Old, new)

   // Out with the old
   if( file )
     synch_file(file);

   // In with the new
   editor::file= act_file;
   this->head= this->tail= nullptr;
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

     // Synchronize, then draw the screen
     synch_active();
     draw();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdTerm::activate(EdLine*)
//
// Purpose-
//       Move the cursor to the specified line, redrawing as required
//
//----------------------------------------------------------------------------
void
   EdTerm::activate(                // Activate
     EdLine*           act_line)    // This line
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;

   // Trace line activation
   Trace::trace(".ACT", "line", data->cursor, act_line); // (Old, new)

   // Activate
   undo_cursor();                   // Clear the current cursor
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
       draw_top();
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
   Editor::alertf("%4d EdTerm file(%p) line(%p)", __LINE__
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
//       EdTerm::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
void
   EdTerm::configure( void )        // Configure the Window
{
   if( opt_hcdm )
     debugh("EdTerm(%p)::configure\n", this);

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
   ENQUEUE("xcb_change_property", xcb_change_property_checked
          ( c, XCB_PROP_MODE_REPLACE, widget_id
          , protocol, 4, 32, 1, &wm_close) );
   if( opt_hcdm )
     debugf("atom PROTOCOL(%d)\natom WM_CLOSE(%d)\n", protocol, wm_close);

   flush();
}

//----------------------------------------------------------------------------
//
// Methods-
//       EdTerm::draw_cursor (set == true)
//       EdTerm::undo_cursor (set == false)
//
// Purpose-
//       Set or clear the screen cursor character
//
//----------------------------------------------------------------------------
void
   EdTerm::draw_cursor(bool set)    // Set/clear the character cursor
{
   EdView* const view= editor::view;

   if( opt_hcdm && opt_verbose > 0 )
     debugh("EdTerm(%p)::%s_cursor cr[%u,%u]\n", this
           , set ? "draw" : "undo", view->col, view->row);

   char buffer[8];                  // The cursor encoding buffer
   size_t column= view->get_column(); // The current column
   const utf8_t* data= (const utf8_t*)view->active.get_buffer(column);
   utf32_t code= pub::Utf8::decode(data);
   if( code == 0 )
     code= ' ';
   pub::Utf8::encode(code, (utf8_t*)buffer);
   buffer[pub::Utf8::length(code)]= '\0';

   xcb_gcontext_t gc= set ? view->gc_flip : view->get_gc();
   putcr(gc, view->col, view->row, buffer);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdTerm::draw_top
//       EdTerm::draw_history
//       EdTerm::draw_message
//       EdTerm::draw_status
//
// Purpose-
//       Draw the top lines: draw_status, then draw_message or draw_history
//         Draw the history line
//         Draw the message line
//         Draw the status line
//
//----------------------------------------------------------------------------
void
   EdTerm::draw_top( void )         // Redraw the top lines
{
   draw_status();
   if( draw_message() )
     return;
   draw_history();
}

void
   EdTerm::draw_history( void )     // Redraw the history line
{
   EdHist* const hist= editor::hist;
   EdView* const view= editor::view;

   if( opt_hcdm )
     debugh("EdTerm(%p)::draw_history view(%s)\n", this
           , view == hist ? "hist" : "data");

   if( view != hist ) {             // If history not active
     hist->active.reset();
     hist->active.index(1024);
     const char* buffer= hist->active.get_buffer();
     putcr(hist->get_gc(), 0, HM_ROW, buffer);
     flush();
     return;
   }

   if( HCDM )
     Trace::trace(".DRW", "hist", hist->cursor);
   const char* buffer= hist->get_buffer();
   putcr(hist->get_gc(), 0, HM_ROW, buffer);
   draw_cursor();
   flush();
}

bool                                // Return code, TRUE if handled
   EdTerm::draw_message( void )     // Message line
{
   EdMess* mess= editor::file->mess_list.get_head();
   if( mess == nullptr )
     return false;

   if( opt_hcdm )
     debugh("EdTerm(%p)::draw_message view(%s)\n", this
           , editor::view == editor::hist ? "hist" : "data");

   status |= SF_MESSAGE;            // Message present
   if( editor::view == editor::hist )
     undo_cursor();

   char buffer[1024];               // Message buffer
   memset(buffer, ' ', sizeof(buffer));
   buffer[sizeof(buffer) - 1]= '\0';
   strcpy(buffer, mess->mess.c_str());
   buffer[strlen(buffer)]= ' ';

   if( HCDM )
     Trace::trace(".DRW", " msg");
   putcr(gc_msg, 0, HM_ROW, buffer);
   flush();
   return true;
}

// format6: format 6 character field (draw_status helper)
// format8: format 8 character field (draw_status helper)
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

void
   EdTerm::draw_status( void )      // Redraw the status line
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;

   if( opt_hcdm )
     debugh("EdTerm(%p)::draw_status view(%s)\n", this
           , editor::view == editor::hist ? "hist" : "data");

   char buffer[1024];               // Status line buffer
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
     Trace::trace(".DRW", " sts", (void*)draw_col, (void*)draw_row);
   xcb_gcontext_t gc= gc_sts;       // Set background/foreground GC
   if( file->changed || file->damaged || data->active.get_changed() )
     gc= gc_chg;
   putxy(gc, 1, 1, buffer);
   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdTerm::draw_line
//       EdTerm::draw
//
// Purpose-
//       Draw one line
//       Draw the entire screen, both data and info
//
//----------------------------------------------------------------------------
void
   EdTerm::draw_line(               // Draw one data line
     unsigned          row,         // The (absolute) row number
     const EdLine*     line)        // The line to draw
{
   int y= get_y(int(row));            // Convert row to pixel offset
   ssize_t col_zero= editor::data->col_zero;
   const char* text= get_text(line); // Get associated text
   if( col_zero )                   // If offset
     text += pub::Utf8::index(text, col_zero);
   if( line->flags & EdLine::F_MARK ) {
     ssize_t col_last= col_zero + col_size; // Last file screen column
     int lh_mark= 0;                // Default, mark line
     int rh_mark= col_size;
     EdMark& mark= *editor::mark;
     if( mark.mark_col >= 0 ) {     // If column mark active
       if( mark.mark_lh > col_last || mark.mark_rh < col_zero ) {
         lh_mark= rh_mark= col_size + 1;
       } else {                     // Otherwise compute screen offsets
         lh_mark= int( mark.mark_lh - col_zero );
         rh_mark= lh_mark + int( mark.mark_rh - mark.mark_lh ) + 1;
       }
     }

     // Marked lines are written in three sections:
     //  R) The unmarked Right section at the end (may be null)
     //  M) The marked Middle section (may be the entire line)
     //  L) The unmarked Left section at the beginning (may be null)
     active.reset(text);            // Load, then blank fill the line
     active.fetch(strlen(text) + col_last + 1);
     char* L= (char*)active.get_buffer(); // Text, length >= col_last + 1
     if( unsigned(rh_mark) < col_size ) { // Right section
       char* R= L + pub::Utf8::index(L, rh_mark);
       unsigned x= get_x(rh_mark);
       putxy(fontGC, x, y, R);
       *R= '\0';                    // (Terminate right section)
     }

     // Middle section
     if( lh_mark < 0 ) lh_mark= 0;
     char* M= L + pub::Utf8::index(L, lh_mark);
     int x= get_x(lh_mark);
     putxy(markGC, x, y, M);
     *M= '\0';                      // (Terminate middle section)

     // Left section
     if( lh_mark > 0 )
       putxy(fontGC, 1, y, L);
   } else {
     putxy(fontGC, 1, y, text);
   }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void
   EdTerm::draw( void )             // Redraw the Window
{
   if( opt_hcdm )
     debugh("EdTerm(%p)::draw\n", this);

   Trace::trace(".DRW", " all", head, tail);
   // Clear the drawable window
   NOQUEUE("xcb_clear_area", xcb_clear_area
          ( c, 0, widget_id, 0, 0, rect.width, rect.height) );

   // Display the text (if any)
   tail= this->head;
   if( tail ) {
     EdLine* line= tail;
     row_used= USER_TOP;

     const unsigned max_used= row_size - USER_BOT;
     while( row_used < max_used ) {
       if( line == nullptr )
         break;

       draw_line(row_used, line);
       row_used++;
       tail= line;
       line= line->get_next();
     }

     row_used -= USER_TOP;
     if( opt_hcdm )
       debugf("%4d LAST xy(%d,%d)\n", __LINE__, 0, row_used);
   }

   draw_top();                      // Draw top (status, hist/message) lines
   if( editor::view == editor::data )
     draw_cursor();
   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdTerm::grab_mouse
//       EdTerm::hide_mouse
//       EdTerm::show_mouse
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
   EdTerm::grab_mouse( void )       // Grab the mouse cursor
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
     } else
       debugf("%4d EdTerm xcb_get_geometry error\n", __LINE__);
   }

   x_origin += rect.width/2;
   y_origin += rect.height/2;
   NOQUEUE("xcb_warp_pointer", xcb_warp_pointer
          (c, XCB_NONE, widget_id, 0,0,0,0
          , WH_t(x_origin), WH_t(y_origin)) );
   flush();
}

void
   EdTerm::hide_mouse( void )       // Hide the mouse cursor
{
   if( motion.state != CS_HIDDEN ) { // If not already hidden
     NOQUEUE("xcb_hide_cursor", xcb_xfixes_hide_cursor(c, widget_id) );
     motion.state= CS_HIDDEN;
     flush();
   }
}

void
   EdTerm::show_mouse( void )       // Show the mouse cursor
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
//       EdTerm::move_cursor_H
//
// Purpose-
//       Move cursor horizontally
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 if draw performed
   EdTerm::move_cursor_H(           // Move cursor horizontally
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
     draw_status();                 // Update status line
   } else {                         // If full redraw needed
     if( view == editor::data )     // If data view, draw_top included
       draw();
     else
       draw_history();
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdTerm::move_screen_V
//
// Purpose-
//       Move screen vertically
//
//----------------------------------------------------------------------------
void
   EdTerm::move_screen_V(           // Move screen vertically
     int               rows)        // The row count (down is positive)
{
   EdView* data= editor::data;
   data->commit();

   if( rows > 0 ) {                 // Move down
     while( rows-- ) {
       EdLine* up= (EdLine*)head->get_next();
       if( up == nullptr )
         break;

       data->row_zero++;
       head= up;
     }
   } else if( rows < 0 ) {          // Move up
     while( rows++ ) {
       EdLine* up= (EdLine*)head->get_prev();
       if( up == nullptr )
         break;

       data->row_zero--;
       head= up;
     }
   }

   synch_active();
   draw();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdTerm::resize
//
// Purpose-
//       Resize the window
//
//----------------------------------------------------------------------------
void
   EdTerm::resize(                  // Resize the Window
     unsigned          x,           // New width
     unsigned          y)           // New height
{
   if( opt_hcdm )
     debugh("EdTerm(%p)::resize(%d,%d)\n", this, x, y);

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
//       EdTerm::synch_active
//
// Purpose-
//       Set the Active (cursor) line from the current row.
//
//----------------------------------------------------------------------------
void
   EdTerm::synch_active( void )     // Set the Active (cursor) line
{  using namespace editor;

   if( data->row < USER_TOP )       // (File initial value == 0)
     data->row= USER_TOP;

   EdLine* line= head;              // Get the top line
   const char* match_type= " ???";  // Default, NO match
   for(unsigned r= USER_TOP; ; r++) { // Set the Active line
     if( r == data->row ) {
       match_type= " row";          // Row match
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
   Trace::trace(".CSR", match_type, data->cursor, line); // (Old, new)
   data->cursor= line;
   data->active.reset(line->text);
   if( !(view == hist && file->mess_list.get_head()) )
     draw_cursor();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdTerm::synch_file
//
// Purpose-
//       Save the current state in the active file
//
//----------------------------------------------------------------------------
void
   EdTerm::synch_file(              // Synchronize the active file
     EdFile*           file) const  // (Note that file is not const)
{
   EdView* const data= editor::data;

   if( file == editor::file ) {     // This should *always* be true
     data->commit();

     file->csr_line= data->cursor;
     file->top_line= this->head;
     file->col_zero= data->col_zero;
     file->row_zero= data->row_zero;
     file->col= data->col;
     file->row= data->row;
   }
}
