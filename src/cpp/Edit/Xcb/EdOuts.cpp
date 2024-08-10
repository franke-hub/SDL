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
//       EdOuts.cpp
//
// Purpose-
//       Editor: Implement EdOuts.h: Terminal output services
//
// Last change date-
//       2024/07/27
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <stdio.h>                  // For sprintf
#include <sys/types.h>              // For system types
#include <xcb/xproto.h>             // For XCB types
#include <xcb/xfixes.h>             // For XCB xfixes extension

#include <gui/Font.h>               // For gui::Font
#include <gui/Types.h>              // For gui::DEV_EVENT_MASK
#include <gui/Window.h>             // For gui::Window
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For pub::fileman::Name
#include <pub/List.h>               // For pub::List
#include <pub/Trace.h>              // For pub::Trace
#include <pub/Utf.h>                // For pub::Utf classes
#include <pub/utility.h>            // For pub::utility methods

#include "Active.h"                 // For Active
#include "Config.h"                 // For Config, namespace config
#include "Editor.h"                 // For namespace editor
#include "EdData.h"                 // For EdData
#include "EdFile.h"                 // For EdFile
#include "EdHist.h"                 // For EdHist
#include "EdInps.h"                 // For EdInps, super class
#include "EdMark.h"                 // For EdMark
#include "EdOuts.h"                 // For EdOuts, implemented
#include "EdType.h"                 // For Editor types

#define PUB _LIBPUB_NAMESPACE       // The PUB library namespacce
using namespace config;             // For config::opt_*, ...
using namespace PUB;                // For PUB library objects
using namespace PUB::debugging;     // For debugging methods
using PUB::Trace;                   // For pub::Trace
using PUB::utility::dump;           // For pub::utility::dump

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // Compilation controls

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static pub::signals::Connector<EdMark::ChangeEvent>
                       changeEvent_connector;

//----------------------------------------------------------------------------
//
// Subroutine-
//       unexpected
//
// Purpose-
//       An unexpected event occurred. (Conditionally) write debugging message.
//
//----------------------------------------------------------------------------
static inline void
   unexpected(int line)             // Handle unexpected event @ __LINE__
{
   if( true )
     debugh("\n%4d %s HCDM **UNEXPECTED**\n\n", line, __FILE__);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::EdOuts
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   EdOuts::EdOuts(                  // Constructor
     Widget*           parent,      // Parent Widget
     const char*       name)        // Widget name
:  EdInps(parent, name ? name : "EdOuts")
{  if( opt_hcdm )
     debugh("EdOuts(%p)::EdOuts\n", this);

   // Handle EdMark::ChangeEvent (lambda function)
   // Purpose: Repair EdOuts::head (if it changed)
   using Event= EdMark::ChangeEvent;
   changeEvent_connector= EdMark::change_signal.connect([this](Event& event) {
     EdFile* file= event.file;
     const EdRedo* redo= event.redo;

     // If the head line was removed, we need to adjust it so that we point
     // to a head line that's actually in the file.
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
       Editor::alertf("%4d EdOuts: internal error\n", __LINE__);
     }

     // If the removal occurs in the current file prior to the head line,
     // row_zero needs to be adjusted as well. We'll just update it since
     // it's more work to see if it needs updating than to just do it.
     if( file == editor::file )
       editor::data->row_zero= file->get_row(head);
   });
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::~EdOuts
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   EdOuts::~EdOuts( void )          // Destructor
{  if( opt_hcdm ) debugh("EdOuts(%p)::~EdOuts\n", this); }

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::configure
//
// Purpose-
//       Configure the Window
//
//----------------------------------------------------------------------------
void
   EdOuts::configure( void )        // Configure the Window
{  if( opt_hcdm )
     debugh("EdOuts(%p)::configure\n", this);

   // Configure the Window
   bg= config::text_bg;             // (Basic Window colors, for clear)
   fg= config::text_fg;

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
   Window::configure();

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
//       EdOuts::get_text
//
// Purpose-
//       Return the line text, which differs for the cursor line
//
//----------------------------------------------------------------------------
const char*                         // The text
   EdOuts::get_text(                // Get text
     const EdLine*     line) const  // For this EdLine
{
   EdData* data= editor::data;
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
//       EdOuts::activate(EdFile*)
//
// Purpose-
//       Activate, then draw a file at its current position.
//
//----------------------------------------------------------------------------
void
   EdOuts::activate(                // Activate
     EdFile*           act_file)    // This file
{  if( opt_hcdm )
     debugh("EdOuts(%p)::activate(%s)\n", this
           , act_file ? act_file->get_name().c_str() : "nullptr");

   EdData* const data= editor::data;
   EdFile* const file= editor::file;

   // Trace file activation
   Trace::trace(".ACT", "file", file, act_file); // (Old, new)

   // Out with the old
   if( file )
     synch_file();

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
//       EdOuts::activate(EdLine*)
//
// Purpose-
//       Move the cursor to the specified line, redrawing as required
//
//----------------------------------------------------------------------------
void
   EdOuts::activate(                // Activate
     EdLine*           act_line)    // This line
{
   EdData* const data= editor::data;
   EdFile* const file= editor::file;

   // Trace line activation
   Trace::trace(".ACT", "line", data->cursor, act_line); // (Old, new)

   // Activate
   hide_cursor();                   // Clear the current cursor
   data->commit();                  // Commit any active line
   data->active.reset(act_line->text); // Activate the new line
   data->cursor= act_line;          // "
   data->activate();                // Insure data view

   // Locate line on-screen
   EdLine* line= (EdLine*)this->head;
   for(unsigned r= USER_TOP; (r+1) < row_size; r++) { // Set the Active line
     if( line == act_line ) {
       data->row= r;
       show_cursor();
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
   Editor::alertf("%4d EdOuts file(%p) line(%p)", __LINE__
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
//       EdOuts::draw
//
// Purpose-
//       Draw the entire Window
//
//----------------------------------------------------------------------------
void
   EdOuts::draw( void )             // Draw the entire Window
{  if( opt_hcdm )
     debugh("EdOuts(%p)::draw\n", this);

   Trace::trace(".DRW", " all", head, tail);
   // Clear the drawable window
   NOQUEUE("xcb_clear_area", xcb_clear_area
          ( c, 0, widget_id, 0, 0, rect.width, rect.height) );

   // Display the text (if any)
   tail= this->head;
   if( tail ) {
     EdLine* line= tail;
     row_used= USER_TOP;

     unsigned max_used= row_size - USER_BOT;
     if( get_y(max_used-1) > rect.height )
       --max_used;
     while( row_used < max_used ) {
       if( line == nullptr )
         break;

       draw_line(row_used, line);
       row_used++;
       tail= line;
       line= line->get_next();
     }

     row_used -= USER_TOP;
     if( opt_hcdm && opt_verbose > 1 )
       debugh("%4d %s row_used(%d)\n", __LINE__, __FILE__, row_used);
   }

   draw_top();                      // Draw top (status, hist/message) lines
   if( editor::view == editor::data )
     show_cursor();
   flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::draw_line
//
// Purpose-
//       Draw one line
//
//----------------------------------------------------------------------------
void
   EdOuts::draw_line(               // Draw one data line
     unsigned          row,         // The (absolute) row number
     const EdLine*     line)        // The line to draw
{
   int y= get_y(int(row));          // Convert row to pixel offset
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
       putxy(gc_font, x, y, R);
       *R= '\0';                    // (Terminate right section)
     }

     // Middle section
     if( lh_mark < 0 ) lh_mark= 0;
     char* M= L + pub::Utf8::index(L, lh_mark);
     int x= get_x(lh_mark);
     putxy(gc_mark, x, y, M);
     *M= '\0';                      // (Terminate middle section)

     // Left section
     if( lh_mark > 0 )
       putxy(gc_font, 1, y, L);
   } else {
     putxy(gc_font, 1, y, text);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::draw_history
//       EdOuts::draw_message
//       EdOuts::draw_status
//       EdOuts::draw_text
//       EdOuts::draw_top
//
// Purpose-
//       Draw the history line
//       Draw the message line
//       Draw the status line
//       Draw a text line
//       Draw the top lines
//
//----------------------------------------------------------------------------
void
   EdOuts::draw_history( void )     // Redraw the history line
{  if( opt_hcdm )
     debugh("EdOuts(%p)::draw_history view(%s)\n", this
           , editor::view == editor::hist ? "hist" : "data");

   EdHist* const hist= editor::hist;
   EdView* const view= editor::view;

   if( view != hist ) {             // If history not active
     hist->active.reset();
     hist->active.index(col_size + 1);
     const char* buffer= hist->active.get_buffer();
     putcr(hist->get_gc(), 0, HIST_MESS_ROW, buffer);
     flush();
     return;
   }

   if( HCDM )
     Trace::trace(".DRW", "hist", hist->cursor);
   const char* buffer= hist->get_buffer();
   putcr(hist->get_gc(), 0, HIST_MESS_ROW, buffer);
   show_cursor();
   flush();
}

bool                                // Return code, TRUE if handled
   EdOuts::draw_message( void )     // Message line
{  if( opt_hcdm )
     debugh("EdOuts(%p)::draw_message view(%s)\n", this
           , editor::view == editor::hist ? "hist" : "data");

   EdMess* mess= editor::file->mess_list.get_head();
   if( mess == nullptr )
     return false;

   key_state |= KS_MSG;             // Message present
   if( editor::view == editor::hist )
     hide_cursor();

   char buffer[1024];               // Message buffer
   memset(buffer, ' ', sizeof(buffer));
   buffer[sizeof(buffer) - 1]= '\0';
   strcpy(buffer, mess->mess.c_str());
   buffer[strlen(buffer)]= ' ';

   if( HCDM )
     Trace::trace(".DRW", " msg");
   putcr(gc_msg, 0, HIST_MESS_ROW, buffer);
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
   EdOuts::draw_status( void )      // Redraw the status line
{  if( opt_hcdm )
     debugh("EdOuts(%p)::draw_status view(%s)\n", this
           , editor::view == editor::hist ? "hist" : "data");

   EdView* const data= editor::data;
   EdFile* const file= editor::file;

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

   if( key_state & KS_INS )         // If inserting state (not REP)
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
   putxy(editor::hist->get_gc(), 1, 1, buffer);
   flush();
}

void
   EdOuts::draw_text(               // Draw a screen line
     GC_t              GC,          // The target graphic context
     uint32_t          row,         // The row number (absolute)
     const char*       text)        // The text to draw
{  if( opt_hcdm && opt_verbose > 0 )
     debugh("draw_text(%d, %d, %s)\n", GC, row, text);

   putcr(GC, 0, row, text);
}

void
   EdOuts::draw_top( void )         // Redraw the top lines
{
   // Draw the background
   xcb_rectangle_t fill= {};
   fill.width=  (decltype(fill.height))(rect.width+1);
   fill.height= (decltype(fill.height))(2*font->length.height + 1);
   xcb_gcontext_t gc= editor::file->is_changed() ? bg_chg : bg_sts;
   NOQUEUE("xcb_poly_fill_rectangle", xcb_poly_fill_rectangle
          ( c, widget_id, gc, 1, &fill) );

   // Draw the status and message/history lines
   draw_status();
   if( !draw_message() )
     draw_history();
}

//----------------------------------------------------------------------------
//
// Methods-
//       EdOuts::hide_cursor
//       EdOuts::show_cursor
//
// Purpose-
//       Hide the screen cursor character
//       Show the screen cursor character
//
// Implementation notes-
//       The XCB editor does not recognize combining characters.
//
//----------------------------------------------------------------------------
void
   EdOuts::hide_cursor( void )      // Hide the character cursor
{  if( opt_hcdm && opt_verbose > 0 )
     debugh("EdOuts(%p)::hide_cursor cr[%u,%u]\n", this
           , editor::view->col, editor::view->row);

#if 0
   EdView* const view= editor::view;
   Active& active= view->active;
   active.get_points();
   utf8_decoder decoder(active.get_buffer(), active.get_used());

   decoder.set_offset(view->get_column());
   utf32_t code= decoder.decode();
   if( code == 0 || code == UTF_EOF )
     code= ' ';

   char buffer[8];
   utf8_encoder encoder(buffer, sizeof(buffer)); ;
encoder.debug("hide constructed");
   encoder.encode(code);

   putcr(view->get_gc(), view->col, view->row
        , buffer, encoder.get_offset());
debugf("hide: %d %zd\n", code, encoder.get_offset());
dump(buffer, encoder.get_offset());
encoder.debug("hide");
#else
   EdView* const view= editor::view;

   char buffer[8];                  // The cursor encoding buffer
   size_t column= view->get_column(); // The current column
   const utf8_t* data= (const utf8_t*)view->active.get_buffer(column);
   utf32_t code= pub::Utf8::decode(data);
   if( code == 0 )
     code= ' ';
   int L= pub::Utf8::encode(code, (utf8_t*)buffer);
   buffer[L]= '\0';

   putcr(view->get_gc(), view->col, view->row, buffer);
//debugf("hide: 0x%.3x %p.%d\n", code, buffer, L);
//dump(buffer, L+1);
#endif
}

void
   EdOuts::show_cursor( void )      // Show the character cursor
{  if( opt_hcdm && opt_verbose > 0 )
     debugh("EdOuts(%p)::show_cursor cr[%u,%u]\n", this
           , editor::view->col, editor::view->row);

#if 0
debugf("%4d Outs HCDM\n", __LINE__);
   EdView* const view= editor::view;
   Active& active= view->active;
   active.get_points();
   utf8_decoder decoder(active.get_buffer(), active.get_used());
decoder.debug("show constructed");

   decoder.set_offset(view->get_column());
decoder.debug("show offset");
   utf32_t code= decoder.decode();
   if( code == 0 || code == UTF_EOF )
     code= ' ';

   char buffer[8];
   utf8_encoder encoder(buffer, sizeof(buffer)); ;
encoder.debug("show constructed");
   encoder.encode(code);

   putcr(gc_flip, view->col, view->row
        , buffer, encoder.get_offset());
debugf("show: %d %zd\n", code, encoder.get_offset());
dump(buffer, encoder.get_offset());
encoder.debug("show");
#else
   EdView* const view= editor::view;

   char buffer[8];                  // The cursor encoding buffer
   size_t column= view->get_column(); // The current column
   const utf8_t* data= (const utf8_t*)view->active.get_buffer(column);
   utf32_t code= pub::Utf8::decode(data);
   if( code == 0 )
     code= ' ';
   int L= pub::Utf8::encode(code, (utf8_t*)buffer);
   buffer[L]= '\0';

   putcr(gc_flip, view->col, view->row, buffer, L);
//debugf("show: 0x%.3x %p.%d\n", code, buffer, L);
//dump(buffer, L+1);
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::grab_mouse
//       EdOuts::hide_mouse
//       EdOuts::show_mouse
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
   EdOuts::grab_mouse( void )       // Grab the mouse cursor
{
   using gui::WH_t;

   uint32_t x_origin= config::geom.x + rect.width/2;
   uint32_t y_origin= config::geom.y + rect.height/2;

   NOQUEUE("xcb_warp_pointer", xcb_warp_pointer
          (c, XCB_NONE, widget_id, 0,0,0,0, WH_t(x_origin), WH_t(y_origin)) );
   flush();
}

void
   EdOuts::hide_mouse( void )       // Hide the mouse cursor
{
   if( motion.state != CS_HIDDEN ) { // If not already hidden
     NOQUEUE("xcb_hide_cursor", xcb_xfixes_hide_cursor(c, widget_id) );
     motion.state= CS_HIDDEN;
     flush();
   }
}

void
   EdOuts::show_mouse( void )       // Show the mouse cursor
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
//       EdOuts::move_cursor_H
//
// Purpose-
//       Move cursor horizontally
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 if draw performed
   EdOuts::move_cursor_H(           // Move cursor horizontally
     size_t            column)      // The (absolute) LINE column number
{
   int rc= 1;                       // Default, draw not performed

   hide_cursor();                   // Clear the current cursor

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
     show_cursor();                 // Show the cursor and
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
//       EdOuts::move_screen_V
//
// Purpose-
//       Move screen vertically
//
//----------------------------------------------------------------------------
void
   EdOuts::move_screen_V(           // Move screen vertically
     int               rows)        // The SCREEN row count (down is positive)
{
   EdView* data= editor::data;
   data->commit();

   if( rows > 0 ) {                 // Move down
     while( rows-- ) {
       EdLine* line= (EdLine*)head->get_next();
       if( line == nullptr )
         break;

       data->row_zero++;
       head= line;
     }
   } else if( rows < 0 ) {          // Move up
     while( rows++ ) {
       EdLine* line= (EdLine*)head->get_prev();
       if( line == nullptr )
         break;

       data->row_zero--;
       head= line;
     }
   }

   synch_active();
   draw();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::move_window
//
// Purpose-
//       Position the window (absolute position)
//
// Implementation notes-
//       xcb_configure_window has no effect before the first window::draw().
//       Does not flush
//
//----------------------------------------------------------------------------
void
   EdOuts::move_window(             // Position the window
     int32_t           x_origin,    // (Absolute) X origin
     int32_t           y_origin)    // (Absolute) Y origin
{
   uint16_t mask= XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y;
   uint32_t parm[2];
   parm[0]= x_origin;
   parm[1]= y_origin;
   ENQUEUE("xcb_configure_window", xcb_configure_window_checked
          (c, widget_id, mask, parm) );
   // flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::putxy
//
// Purpose-
//       Draw text at [left,top] point
//
//----------------------------------------------------------------------------
void
   EdOuts::putxy(                   // Draw text
     xcb_gcontext_t    gc,          // The target graphic context
     unsigned          left,        // Left (X) pixel offset
     unsigned          top,         // Top  (Y) pixel offset
     const char*       text,        // Using this text
     size_t            length)      // For this length
{  if( opt_hcdm && opt_verbose > 0 ) {
     char buffer[24];
     if( length < 17 )
       strcpy(buffer, text);
     else {
       memcpy(buffer, text, 16);
       strcpy(buffer+16, "...");
     }
     debugh("EdOuts(%p)::putxy(%u,[%d,%d],'%s')\n", this
           , gc, left, top, buffer);
   }

#if true
   enum{ DIM= 256 };                // xcb_image_text_16 maximum UNIT length
   xcb_char2b_t out[DIM];           // UTF16 big endian output buffer

   utf8_decoder  decoder((const utf8_t*)text, length);
   utf16_encoder encoder((utf16BE_t*)out, DIM, MODE_BE);

   unsigned outlen= 0;              // UTF16 output length, in units
   unsigned outorg= left;           // Current output origin pixel index
   unsigned outpix= left;           // Current output pixel index
   for(utf32_t code= decoder.decode(); code; code= decoder.decode()) {
     if( code == UTF_EOF )          // If end of file
       break;

#if 0 // EXPERIMENT: FAILED (This just overwrites the original character.)
     // *** EXPERIMENTAL *** VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
     if( Utf::is_combining(code) ) {
       if( outlen ) {
         ENQUEUE("xcb_image_text_16", xcb_image_text_16
                ( c, uint8_t(outlen), widget_id, gc
                , uint16_t(outorg), uint16_t(top + font->offset.y), out) );
         outpix -= font->length.width; // (Backspace)
         outorg= outpix;
         outlen= 0;
         encoder.reset((pub::Utf::utf16BE_t*)out, DIM);
       } else if( left > font->length.width ) { // else (outlen == 0) && ...
         // Column logic error:
         // Output started with combining code, not at left edge of screen
         unexpected(__LINE__);
         outorg= left - font->length.width;
         outpix= outorg;
       }
     }
     // *** EXPERIMENTAL *** ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#endif

     if( outlen >= (DIM - 4) ) {    // If near the end
       NOQUEUE("xcb_image_text_16", xcb_image_text_16
              ( c, uint8_t(outlen), widget_id, gc
              , uint16_t(outorg), uint16_t(top + font->offset.y), out) );
       outorg= outpix;
       outlen= 0;
       encoder.reset();
     }

     outpix += font->length.width;  // Ending pixel (+1)
     if( outpix >= rect.width )     // If at or past end of screen
       break;

     outlen += encoder.encode(code); // Encode the codepoint
   }

   if( outlen )                     // If there's something left to render
     NOQUEUE("xcb_image_text_16", xcb_image_text_16
            ( c, uint8_t(outlen), widget_id, gc
            , uint16_t(outorg), uint16_t(top + font->offset.y), out) );
#else
   enum{ DIM= 256 };                // xcb_image_text_16 maximum UNIT length
   xcb_char2b_t out[DIM];           // UTF16 big endian output buffer

   unsigned outlen= 0;              // UTF16 output buffer length
   unsigned outorg= left;           // Current output origin index
   unsigned outpix= left;           // Current output pixel index
   for(auto it= pub::Utf8::const_iterator((const utf8_t*)text); *it; ++it) {
     if( outlen > (DIM-4) ) {       // If time for a partial write
       NOQUEUE("xcb_image_text_16", xcb_image_text_16
              ( c, uint8_t(outlen), widget_id, gc
              , uint16_t(outorg), uint16_t(top + font->offset.y), out) );
       outorg= outpix;
       outlen= 0;
     }

     utf32_t code= *it;             // Next encoding
     outpix += font->length.width;  // Ending pixel (+1)
     if( outpix >= rect.width || code == 0 ) // If at end of encoding
       break;

     pub::Utf16::encode(code, (utf16_t*)out + outlen);
     outlen += pub::Utf16::length(code);
   }

   if( outlen )                     // If there's something left to render
     NOQUEUE("xcb_image_text_16", xcb_image_text_16
            ( c, uint8_t(outlen), widget_id, gc
            , uint16_t(outorg), uint16_t(top + font->offset.y), out) );
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::resized
//
// Purpose-
//       Handle Window resized event
//
//----------------------------------------------------------------------------
void
   EdOuts::resized(                 // Handle Window resized event
     uint32_t          width,       // New width  (In Pixels)
     uint32_t          height)      // New height (In Pixels)
{  if( opt_hcdm ) {
     debugh("EdOuts(%p)::resized(%u,%u)\n", this, width, height);

     if( opt_verbose > 1 ) {
       gui::WH_size_t size= get_size();
       debugh("%4d [%d x %d]= get_size\n",  __LINE__, size.width, size.height);
     }
   }

   // Accept whatever size the window manager gives us.
   rect.width= (decltype(rect.width))width;
   rect.height= (decltype(rect.height))height;

   // We adjust the column and row count so we only draw complete characters.
   unsigned prior_col= col_size;
   unsigned prior_row= row_size;
   col_size= (width - 2) / font->length.width;
   row_size= (height - 2) / font->length.height;

   // Some window managers don't an expose event when the window shrinks,
   // we redraw to remove partial characters.
   if( col_size > prior_col || row_size > prior_row ) // If bigger
     return;                        // (An expose event will be generated)
   if( col_size < prior_col || row_size < prior_row ) { // If smaller
     EdView* data= editor::data;
     if( row_size < prior_row ) {
       while( (data->row + 1)*font->length.height >= unsigned(rect.height-2) )
         --data->row;
       synch_active();
     }

     if( col_size <= data->col ) {
       while( (data->col + 1)*font->length.width >= unsigned(rect.width-2) )
         --data->col;
       move_cursor_H(data->col);
     }

     draw();                        // Redraw, removing partial characters
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::set_font
//
// Purpose-
//       Set the Font
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   EdOuts::set_font(                // Set the Font
     const char*          name)     // To this name
{  if( opt_hcdm )
     debugh("EdOuts(%p)::set_font(%s) geom(%d,%d,%u,%u)\n", this, name
           , config::geom.x, config::geom.y
           , config::geom.width, config::geom.height);

   int rc= font->open(name);        // Set the font

   // Update layout (resize required)
   if( rc == 0 ) {
     set_geom(config::geom);        // Set the Geometry
   }

   return rc;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::set_geom
//
// Purpose-
//       Set the Geometry
//
//----------------------------------------------------------------------------
void
   EdOuts::set_geom(                // Set the Geometry
     const geometry_t& geom)        // From this Geometry
{  if( opt_hcdm && opt_verbose > 0 )
     debugh("EdOuts(%p)::set_geom(%d,%d,%u,%u)\n", this
           , geom.x, geom.y, geom.width, geom.height);

   col_size= geom.width;
   row_size= geom.height;
   min_size= { gui::WH_t(MINI_C   * font->length.width  + 2)
             , gui::WH_t(MINI_R   * font->length.height + 2) };
   use_size= { gui::WH_t(col_size * font->length.width  + 2)
             , gui::WH_t(row_size * font->length.height + 2) };
   use_unit= { gui::WH_t(font->length.width), gui::WH_t(font->length.height) };
}
