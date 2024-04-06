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
//       Editor: Input/output server
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
#include "EdInps.h"                 // For EdInps, super class
#include "EdMark.h"                 // For EdMark
#include "EdOuts.h"                 // For EdOuts, implemented

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

,  HM_ROW= 1                        // History/Message line row
}; // Compilation controls

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static pub::signals::Connector<EdMark::ChangeEvent>
                       changeEvent_connector;

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
{
   if( opt_hcdm )
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
// EdOuts::Destructor
//----------------------------------------------------------------------------
   EdOuts::~EdOuts( void )          // Destructor
{
   if( opt_hcdm )
     debugh("EdOuts(%p)::~EdOuts\n", this);
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
//       EdOuts::activate(EdFile*)
//
// Purpose-
//       Activate, then draw a file at its current position.
//
//----------------------------------------------------------------------------
void
   EdOuts::activate(                // Activate
     EdFile*           act_file)    // This file
{
   if( opt_hcdm )
     debugh("EdOuts(%p)::activate(%s)\n", this
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
{
   if( opt_hcdm )
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
     draw_cursor();
   flush();
}

//----------------------------------------------------------------------------
//
// Methods-
//       EdOuts::draw_cursor (set == true)
//       EdOuts::undo_cursor (set == false)
//
// Purpose-
//       Set or clear the screen cursor character
//
//----------------------------------------------------------------------------
void
   EdOuts::draw_cursor(bool set)    // Set/clear the character cursor
{
   EdView* const view= editor::view;

   if( opt_hcdm && opt_verbose > 0 )
     debugh("EdOuts(%p)::%s_cursor cr[%u,%u]\n", this
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

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::draw_history
//       EdOuts::draw_message
//       EdOuts::draw_status
//       EdOuts::draw_top
//
// Purpose-
//       Draw the history line
//       Draw the message line
//       Draw the status line
//       Draw the top lines
//
//----------------------------------------------------------------------------
void
   EdOuts::draw_history( void )     // Redraw the history line
{
   EdHist* const hist= editor::hist;
   EdView* const view= editor::view;

   if( opt_hcdm )
     debugh("EdOuts(%p)::draw_history view(%s)\n", this
           , view == hist ? "hist" : "data");

   if( view != hist ) {             // If history not active
     hist->active.reset();
     hist->active.index(col_size + 1);
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
   EdOuts::draw_message( void )     // Message line
{
   EdMess* mess= editor::file->mess_list.get_head();
   if( mess == nullptr )
     return false;

   if( opt_hcdm )
     debugh("EdOuts(%p)::draw_message view(%s)\n", this
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
   EdOuts::draw_status( void )      // Redraw the status line
{
   EdView* const data= editor::data;
   EdFile* const file= editor::file;

   if( opt_hcdm )
     debugh("EdOuts(%p)::draw_status view(%s)\n", this
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
   putxy(editor::hist->get_gc(), 1, 1, buffer);
   flush();
}

void
   EdOuts::draw_top( void )         // Redraw the top lines
{
   // Draw the background
   xcb_rectangle_t fill= {};
   fill.width=  (decltype(fill.height))(rect.width+1);
   fill.height= (decltype(fill.height))(2*font.length.height + 1);
   xcb_gcontext_t gc= editor::file->is_changed() ? bg_chg : bg_sts;
   NOQUEUE("xcb_poly_fill_rectangle", xcb_poly_fill_rectangle
          ( c, widget_id, gc, 1, &fill) );

   // Draw the status and message/history lines
   draw_status();
   if( draw_message() )
     return;
   draw_history();
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
//       EdOuts::move_screen_V
//
// Purpose-
//       Move screen vertically
//
//----------------------------------------------------------------------------
void
   EdOuts::move_screen_V(           // Move screen vertically
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
//       EdOuts::resized
//
// Purpose-
//       Handle Window resized event
//
//----------------------------------------------------------------------------
void
   EdOuts::resized(                 // Handle Window resized event
     unsigned          x,           // New width
     unsigned          y)           // New height
{
   if( opt_hcdm )
     debugh("EdOuts(%p)::resize(%u,%u)\n", this, x, y);

   if( opt_hcdm ) {
     gui::WH_size_t size= get_size();
     debugf("%4d [%d x %d]= get_size\n",  __LINE__, size.width, size.height);
   }

   // Accept whatever size the window manager gives us.
   rect.width= (decltype(rect.width))x;
   rect.height= (decltype(rect.height))y;

   // We adjust the column and row count so we only draw complete characters.
   unsigned prior_col= col_size;
   unsigned prior_row= row_size;
   col_size= (x - 2) / font.length.width;
   row_size= (y - 2) / font.length.height;

   // Some window managers don't an expose event when the window shrinks,
   // we redraw to removes partial characters.
   if( col_size > prior_col || row_size > prior_row ) // If bigger
     return;                        // (An expose event will be generated)
   if( col_size < prior_col || row_size < prior_row ) { // If smaller
     EdView* data= editor::data;
     if( row_size < prior_row ) {
       while( (data->row + 1)*font.length.height >= unsigned(rect.height-2) )
         --data->row;
       synch_active();
     }

     if( col_size <= data->col ) {
       while( (data->col + 1)*font.length.width >= unsigned(rect.width-2) )
         --data->col;
       move_cursor_H(data->col);
     }

     draw();                        // Redraw, removing partial characters
   }
   return;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::synch_active
//
// Purpose-
//       Set the Active (cursor) line from the current row.
//
//----------------------------------------------------------------------------
void
   EdOuts::synch_active( void )     // Set the Active (cursor) line
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
//       EdOuts::synch_file
//
// Purpose-
//       Save the current state in the active file
//
//----------------------------------------------------------------------------
void
   EdOuts::synch_file(              // Synchronize the active file
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
