//----------------------------------------------------------------------------
//
//       Copyright (C) 2024 Frank Eskesen.
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
//       2024/08/30
//
//----------------------------------------------------------------------------
#define _XOPEN_SOURCE_EXTENDED 1

#include <string>                   // For std::string
#include <stdio.h>                  // For sprintf
#include <sys/types.h>              // For system types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For pub::fileman::Name
#include <pub/List.h>               // For pub::List
#include <pub/Trace.h>              // For pub::Trace
#include <pub/Utf.h>                // For pub::Utf classes
#include <pub/utility.h>            // For pub::utility::visify

#include "Active.h"                 // For Active
#include "Config.h"                 // For Config, namespace config
#include "EdData.h"                 // For EdData
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile
#include "EdHist.h"                 // For EdHist
#include "EdInps.h"                 // For EdInps - base class
#include "EdMark.h"                 // For EdMark
#include "EdOuts.h"                 // For EdOuts - implemented

using namespace config;             // For config::opt_*, ...
using namespace pub::debugging;     // For debugging
using pub::Trace;                   // For pub::Trace
using pub::utility::dump;           // For pub::utility::dump (when debugging)
using pub::utility::visify;         // For pub::utility::visify

typedef pub::Utf::utf8_t   utf8_t;  // Import utf8_t
typedef pub::Utf::utf16_t  utf16_t; // Import utf16_t
typedef pub::Utf::utf32_t  utf32_t; // Import utf32_t

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose

// Controls
,  IO_TRACE= true                   // I/O trace mode?
}; // Compilation controls

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static const char*     blank= " ";  // A blank string

static pub::signals::Connector<EdMark::ChangeEvent>
                       changeEvent_connector;

//----------------------------------------------------------------------------
//
// Struct-
//       putcr_long
//       putcr_short
//
// Purpose-
//       Internal trace records for putcr operation.
//
//----------------------------------------------------------------------------
struct putcr_long {
enum { DATA_SIZE= 40 };             // The output data length
char                   ident[4];    // The trace type identifier     ".PUT"
char                   unit[4];     // The trace data sub-identifier "data"
uint64_t               clock;       // The UTC epoch clock, in nanoseconds
uint16_t               col;         // The screen (X) column
uint16_t               row;         // The screen (Y) row
uint16_t               GC;          // Graphic context
uint16_t               length;      // The output data length
char                   data[DATA_SIZE]; // The output data
}; // struct putcr_long

struct putcr_short {
enum { DATA_SIZE= 8 };              // The output data length
char                   ident[4];    // The trace type identifier     ".PUT"
char                   unit[4];     // The trace data sub-identifier "data"
uint64_t               clock;       // The UTC epoch clock, in nanoseconds
uint16_t               col;         // The screen (X) column
uint16_t               row;         // The screen (Y) row
uint16_t               GC;          // Graphic context
uint16_t               length;      // The output data length
char                   data[DATA_SIZE]; // The output data
}; // struct putcr_short

//----------------------------------------------------------------------------
//
// Subroutine-
//       io_trace
//
// Purpose-
//       (Debugging) I/O trace
//
//----------------------------------------------------------------------------
static inline void
   io_trace(                        // I/O trace
     int               line,        // Caller's line number
     const char*       addr,        // Data address
     size_t            size)        // Data length
{
   char buffer[24];
   if( size < 17 ) {
     memcpy(buffer, addr, size);
     buffer[size]= '\0';
   } else {
     memcpy(buffer, addr, 16);
     strcpy(buffer+16, "...");
   }
   traceh("%4d trace(%3zd.'%s')\n", line, size, visify(buffer).c_str());
}

static inline void
   io_trace(                        // I/O trace
     int               line,        // Caller's line number
     const char*       addr)        // Data address
{  io_trace(line, addr, strlen(addr)); }

//----------------------------------------------------------------------------
//
// Subroutine-
//       set_main_name
//
// Purpose-
//       Set the windows title (using an escape sequence)
//
//----------------------------------------------------------------------------
static inline void
   set_main_name(                   // Set the window title decoraion
     const char*       title)       // The new title
{  if( IO_TRACE && opt_hcdm )
     traceh("EdOuts::set_main_name(%s)\n", title);

   fprintf(stdout, "\x1b]2;%s\x07", title); // Uses escape sequence
   fflush(stdout);
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
   EdOuts::EdOuts( void )           // Constructor
:  EdInps()
{  if( opt_hcdm )
     traceh("EdOuts(%p)::EdOuts\n", this);

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
{  if( opt_hcdm ) traceh("EdOuts(%p)::~EdOuts\n", this); }

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
     traceh("EdOuts(%p)::activate(%s)\n", this
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
     traceh("EdOuts(%p)::draw\n", this);

   Trace::trace(".DRW", " all", head, tail);
   erase();                         // Clear the screen

   // Display the text (if any)
   tail= this->head;
   if( tail ) {
     EdLine* line= tail;
     row_used= USER_TOP;

     unsigned max_used= row_size - USER_BOT;
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
       traceh("%4d %s row_used(%d)\n", __LINE__, __FILE__, row_used);
   }

   draw_top();                      // Draw top (status, hist/message) lines
   if( editor::view == editor::data )
     show_cursor();
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
   ssize_t col_zero= editor::data->col_zero;
   const char* text= get_text(line); // Get associated text
   utf8_decoder decoder(text);
   if( col_zero ) {                 // If column offset
     if( decoder.set_column_index(col_zero) ) // If past end of text
       text= "";                    // Use empty text
     else
       text += decoder.get_offset();
   }

   if( line->flags & EdLine::F_MARK ) {
     ssize_t col_last= col_zero + col_size; // Last file screen column
     ssize_t lh_mark= 0;            // Default, mark line
     ssize_t rh_mark= col_size;
     EdMark& mark= *editor::mark;
     if( mark.mark_col >= 0 ) {     // If column mark active
       if( mark.mark_lh > col_last || mark.mark_rh < col_zero ) {
         lh_mark= rh_mark= col_size + 1;
       } else if( mark.mark_lh < col_zero ) {
         lh_mark= 0;
         rh_mark= mark.mark_rh - col_zero + 1;
       } else {
         lh_mark= mark.mark_lh - col_zero;
         rh_mark= lh_mark + (mark.mark_rh - mark.mark_lh) + 1;
       }
     }

     // Marked lines are written in three sections:
     //  L) The unmarked Left section at the beginning (may be null)
     //  M) The marked Middle section (may be the entire line)
     //  R) The unmarked Right section at the end (may be null)
     active.reset(text);            // Load the line (with col_zero origin)
     active.get_column(col_size+1); // Fill buffer to screen length
     const char* buffer= active.get_buffer();
     decoder.reset(buffer, strlen(buffer));

     decoder.set_column_index(lh_mark);
     Offset lh_off= decoder.get_offset();
     decoder.set_column_index(rh_mark);
     Offset rh_off= decoder.get_offset();
     Offset off_last= decoder.get_length();

     // Left section
     if( lh_off )
       putcr(gc_font, 0, row, buffer, lh_off);

     // Middle section
     if( rh_off > lh_off )
       putcr(gc_mark, int(lh_mark), row, buffer+lh_off, rh_off - lh_off);

     // Right section
     if( off_last > rh_off )
       putcr(gc_font, int(rh_mark), row, buffer+rh_off, off_last - rh_off);
   } else {
     putcr(gc_font, 0, row, text);
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
     traceh("EdOuts(%p)::draw_history view(%s)\n", this
           , editor::view == editor::hist ? "hist" : "data");

   EdHist* const hist= editor::hist;
   EdView* const view= editor::view;

   if( view != hist ) {             // If history not active
     hist->active.reset();
     hist->active.index(col_size + 1);
     const char* buffer= hist->active.get_buffer();
     putcr(hist->get_gc(), 0, HIST_MESS_ROW, buffer);
     return;
   }

   if( HCDM )
     Trace::trace(".DRW", "hist", hist->cursor);
   const char* buffer= hist->get_buffer();
   putcr(hist->get_gc(), 0, HIST_MESS_ROW, buffer);
   show_cursor();
}

bool                                // Return code, TRUE if handled
   EdOuts::draw_message( void )     // Message line
{  if( opt_hcdm )
     traceh("EdOuts(%p)::draw_message view(%s)\n", this
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
     traceh("EdOuts(%p)::draw_status view(%s)\n", this
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
   putcr(editor::hist->get_gc(), 0, 0, buffer);
}

void
   EdOuts::draw_text(               // Draw a screen line
     GC_t              GC,          // The target graphic context
     uint32_t          row,         // The row number (absolute)
     const char*       text)        // The text to draw
{  if( IO_TRACE && opt_hcdm && opt_verbose > 0 )
     traceh("EdOuts(%p)::draw_text(%u,%d,...:%zd) \n", this, GC, row
           , strlen(text));

   putcr(GC, 0, row, text);
}

void
   EdOuts::draw_top( void )         // Redraw the top lines
{
   draw_status();
   if( !draw_message() )
     draw_history();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::flush
//
// Purpose-
//       Complete an operation
//
// Implementation notes-
//       Not normally required: Next poll automatically flushes.
//
//----------------------------------------------------------------------------
void
   EdOuts::flush( void )            // Complete an operation
{  if( opt_hcdm )
     traceh("EdOuts(%p)::flush()\n", this);

   wrefresh(win);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdOuts::hide_cursor
//       EdOuts::show_cursor
//
// Purpose-
//       Hide the screen cursor character
//       Show the screen cursor character
//
//----------------------------------------------------------------------------
void
   EdOuts::hide_cursor( void )      // Hide the character cursor
{  if( opt_hcdm && opt_verbose > 0 )
     traceh("EdOuts(%p)::hide_cursor cr[%u,%u]\n", this
           , editor::view->col, editor::view->row);

   EdView* const view= editor::view;
   Active& active= view->active;
   utf8_decoder decoder;

   Index column= view->get_column();
   const char* buffer= active.get_column(column);
   decoder.reset(buffer, strlen(buffer));

   utf8_decoder copy= decoder.copy_column();
   Length length= copy.get_length();
   if( length == 0 ) {
     buffer= blank;
     length= 1;
   }
   putcr(view->get_gc(), view->col, view->row, buffer, length);
}

void
   EdOuts::show_cursor( void )      // Show the character cursor
{  if( opt_hcdm && opt_verbose > 0 )
     traceh("EdOuts(%p)::show_cursor cr[%u,%u]\n", this
           , editor::view->col, editor::view->row);

   EdView* const view= editor::view;
   Active& active= view->active;
   utf8_decoder decoder;

   Index column= view->get_column();
   const char* buffer= active.get_column(column);
   decoder.reset(buffer, strlen(buffer));

   utf8_decoder copy= decoder.copy_column();
   Length length= copy.get_length();
   if( length == 0 ) {
     buffer= blank;
     length= 1;
   }
   putcr(gc_flip, view->col, view->row, buffer, length);
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
   size_t current= view->get_column(); // Get current column
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
//       EdOuts::putcr
//
// Purpose-
//       Draw text at column, row, length in bytes
//       Draw text at column, row, accounting for column
//
//----------------------------------------------------------------------------
void
   EdOuts::putcr(                   // Draw text
     GC_t              GC,          // The graphic context
     unsigned          col,         // The (X) column
     unsigned          row,         // The (Y) row
     const char*       text,        // Using this text
     size_t            size)        // For this byte length
{  if( IO_TRACE && opt_hcdm && opt_verbose > 0 ) {
     char buffer[24];
     if( size < 17 ) {
       memcpy(buffer, text, size);
       buffer[size]= '\0';
     } else {
       memcpy(buffer, text, 16);
       strcpy(buffer+16, "...");
     }
     traceh("EdOuts.putcr(%u,[%d,%d],%3zd.'%s')\n", GC, col, row
           , size, visify(buffer).c_str());
   }

   // If nothing to write or col's past the end of the screen, do nothing
   if( size == 0 || col >= col_size )
     return;

   // Compute output length in bytes
   size_t COL= col_size - col;      // Number of columns left on line
   size_t OUT= size;                // Default: length == size

   utf8_decoder decoder(text, size);
   // Shrink output size if past end of screen
   Count count= decoder.get_column_count(); // Output length in columns
   if( count > COL ) {
     decoder.set_column_index(COL);
     OUT= decoder.get_offset();
   }

   // The curses addstr methods provide '\b' and '\t' special handling.
   // This botches our screen handling, so we prevent that by
   // replacing '\b' and '\t' with '~'. Since text is const,
   // we use an Active's buffer for this replacement.
   Active* active= editor::altact;
   char* output= const_cast<char*>(active->resize(OUT));
   memcpy(output, text, OUT);

   char*
   C= (char*)strchr(output, '\b');  // Remove '\b' characters
   while( C ) {
     *C= '~';
     C= (char*)strchr(C+1, '\b');
   }

   C= (char*)strchr(output, '\t');  // Remove '\t' characters
   while( C ) {
     *C= '~';
     C= (char*)strchr(C+1, '\t');
   }

   // Write the text
   color_set(short(GC), nullptr);   // Set the color
   mvwaddstr(win, row, col, output); // Write the string

   // Internal trace
   if( opt_verbose || opt_iodm ) {
     unsigned cr_size= (unsigned)sizeof(putcr_short);
     if( OUT > putcr_short::DATA_SIZE )
       cr_size= (unsigned)sizeof(putcr_long);

     putcr_long* R= (putcr_long*)Trace::storage_if(cr_size);
     if( R ) {
       R->col= htons(uint16_t(col));
       R->row= htons(uint16_t(row));
       R->GC= htons(uint16_t(GC));
       R->length= htons(uint16_t(OUT));
       if( OUT > putcr_short::DATA_SIZE ) {
         Trace::Buffer<putcr_long::DATA_SIZE> buff(output);
         memcpy(R->data, buff.temp, putcr_long::DATA_SIZE);
       } else {
         Trace::Buffer<putcr_short::DATA_SIZE> buff(output);
         memcpy(R->data, buff.temp, putcr_short::DATA_SIZE);
       }
       memcpy(R->unit, "data", 4);
       ((Trace::Record*)R)->trace(".OUT"); // Trace::trace(".OUT", "data")
     }
   }
}

void
   EdOuts::putcr(                   // Draw text
     GC_t              GC,          // The graphic context
     unsigned          col,         // The (X) column
     unsigned          row,         // The (Y) row
     const char*       text)        // Using this text
{  putcr(GC, col, row, text, strlen(text)); }

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
     uint32_t          width,       // New width  (In columns)
     uint32_t          height)      // New height (In rows)
{  if( opt_hcdm )
     traceh("EdOuts(%p)::resized(%u,%u)\n", this, width, height);

   col_size= width;
   row_size= height;
   if( operational )
     draw();
}
