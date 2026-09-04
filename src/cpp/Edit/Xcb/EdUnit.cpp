//----------------------------------------------------------------------------
//
//       Copyright (C) 2024-2026 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
// SPDX-License-Identifier: GPL-3.0-only
//----------------------------------------------------------------------------
//
// Title-
//       EdUnit.cpp
//
// Purpose-
//       Editor: Input/output interface; Handle editor operations.
//
// Last change date-
//       2026/08/28
//
//----------------------------------------------------------------------------
#include <cstdio>                   // For sprintf
#include <string>                   // For std::string

#include <sys/types.h>              // For system types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/List.h>               // For pub::List
#include "pub/Trace.h"              // For pub::Trace
#include <pub/Utf.h>                // For pub::utf8_decoder

#include "Active.h"                 // For Active
#include "Config.h"                 // For Config, namespace config
#include "EdData.h"                 // For EdData
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile
#include "EdHist.h"                 // For EdHist
#include "EdMark.h"                 // For EdMark
#include "EdOpts.h"                 // For EdOpts
#include "EdUnit.h"                 // For EdUnit - implemented

using namespace config;             // For config::opt_*, ...
using namespace editor;             // For convenience
using namespace pub::debugging;     // For debugging
using pub::Trace;                   // For pub::Trace

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // Compilation controls

//----------------------------------------------------------------------------
// External data areas (Initialized by subclass)
//----------------------------------------------------------------------------
// const char*            EdUnit::EDITOR;
// const char*            EdUnit::DEFAULT_CONFIG;

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::op_*
//
// Purpose-
//       Handle editor operation
//
// Implementation notes-
//       These methods indirectly modify the screen, therefore they should
//       not be declared const.
//
//----------------------------------------------------------------------------
void
   EdUnit::op_debug( void )         // Enter/exit debug mode
{
   if( diagnostic ) {
     diagnostic= false;
     Config::errorf("Diagnostic mode exit\n");
     pub::Trace* trace= pub::Trace::table;
     if( trace )
       trace->flag[pub::Trace::X_HALT]= false;
   } else {
     Editor::alertf("*DEBUG*");     // (Sets editor::diagnostic, stops trace)
   }
}

void
   EdUnit::op_copy_cursor_to_hist( void ) // Copy cursor line to history line
{
   // (This sequence DOES NOT change the cursor line)
   Active& active= data->active;    // The current command line
   const char* command= active.truncate(); // Truncate it
   hist->activate(command);         // Activate the history view
}

void
   EdUnit::op_copy_file_name_to_hist( void ) // Copy file name to history line
{  hist->activate(file->name.c_str()); }

void
   EdUnit::op_copy_hist_to_file( void ) // Insert history line into file
{  put_message( do_insert(hist->get_buffer()) ); } // NOT TESTED

void
   EdUnit::op_exit_safely( void )   // Exit if no files changed
{  put_message( do_quit() ); }

void
   EdUnit::op_goto_changed( void )  // Activate changed file
{
   if( key_state & KS_NFC ) {       // If NFC message active
     draw_history();
     key_state &= ~(KS_NFC);
   } else {
     if( un_changed() ) {
       put_message("No files changed");
       key_state |= (KS_NFC);
     }
   }
   key_state &= ~(KS_ESC);          // Handle 'ALT-\', op-if-changes key
}

void
   EdUnit::op_goto_next_file( void ) // Activate next file
{
   data->commit();
   EdFile* next= file->get_next();
   if( next == nullptr )
     next= file_list.get_head();
   if( next != file )
     activate(next);
}

void
   EdUnit::op_goto_prev_file( void ) // Activate  prior file
{
   data->commit();
   EdFile* prev= file->get_prev();
   if( prev == nullptr )
     prev= file_list.get_tail();
   if( prev != file )
     activate(prev);
}

void
   EdUnit::op_help( void )          // Display help information
{  command_help(); }

void
   EdUnit::op_insert_line( void )   // Insert a new, empty line
{  put_message( do_insert() ); }    // Insert line after cursor

void
   EdUnit::op_join_line( void )     // Join cursor line with next line
{  put_message( do_join() ); }      // Join current/next lines

void
   EdUnit::op_key_arrow_down( void ) // Handle down arrow key
{  view->move_cursor_V(1); }

void
   EdUnit::op_key_arrow_left( void ) // Handle left arrow key
{
   size_t column= view->get_column(); // The cursor column
   if( column > 0 )
     move_cursor_H(column - 1);
}

void
   EdUnit::op_key_arrow_right( void ) // Handle right arrow key
{  move_cursor_H(view->get_column() + 1); }

void
   EdUnit::op_key_arrow_up( void )  // Handle up arrow key
{  view->move_cursor_V(-1); }

void
   EdUnit::op_key_backspace( void ) // Handle backspace key
{
   if( view == data && file_protected() )
     return;

   size_t column= view->get_column(); // The cursor column
   if( column > 0 )
     column--;
   view->active.remove_char(column);
   move_cursor_H(column);
   view->draw_active();
   draw_top();
}

void
   EdUnit::op_key_dead( void )      // Handle dead key
{  put_message("Invalid key"); }

void
   EdUnit::op_key_delete( void )    // Handle delete key
{
   if( view == data && file_protected() )
     return;

   view->active.remove_char(view->get_column());
   view->active.append_text(" ");
   view->draw_active();
   draw_top();
}

void
   EdUnit::op_key_end( void )       // Handle end key
{
   // Handle end key. Processing depends on EdOpts::has_unicode_combining().
   const char* buffer= view->active.truncate();
   utf8_decoder decoder(buffer);

   if( EdOpts::has_unicode_combining() )
     move_cursor_H(decoder.get_column_count() - 1);
   else
     move_cursor_H(decoder.get_symbol_count() - 1);
}

void
   EdUnit::op_key_enter( void )     // Handle enter key
{
   move_cursor_H(0);
   view->enter_key();
}

void
   EdUnit::op_key_home( void )      // Handle home key
{
   hide_cursor();
   view->col= 0;
   if( view->col_zero ) {
     view->col_zero= 0;
     draw();
   } else
     draw_top();

   show_cursor();
   flush();
}

void
   EdUnit::op_key_idle( void )      // Handle NOP key
{  }

void
   EdUnit::op_key_insert( void )    // Handle insert key
{
   key_state ^= KS_INS;             // Invert the insert state
   draw_top();
}

void
   EdUnit::op_key_page_down( void ) // Handle page down key
{
   int rows= row_size - (USER_TOP + USER_BOT + 1);
   move_screen_V(+rows);
}

void
   EdUnit::op_key_page_up( void )   // Handle page up key
{
   int rows= row_size - (USER_TOP + USER_BOT + 1);
   move_screen_V(-rows);
}

void
   EdUnit::op_key_tab_forward( void ) // Handle forward tab operation
{  move_cursor_H(tab_forward(view->get_column())); }

void
   EdUnit::op_key_tab_reverse( void ) // Handle reverse tab operation
{  move_cursor_H(tab_reverse(view->get_column())); }

void
   EdUnit::op_line_to_bot( void )   // Move cursor line to end of screen
{
   while( data->row < (row_size - 1) ) {
     if( head->get_prev() == nullptr )
       break;

     ++data->row;
     head= head->get_prev();
     --data->row_zero;

     if( row_used < (row_size - 1) )
       ++row_used;
   }

   draw();
}

void
   EdUnit::op_line_to_top( void )   // Move cursor line to top of screen
{
   head= data->cursor;
   data->row_zero += (data->row - USER_TOP);
   data->row= USER_TOP;
   draw();
}

void
   EdUnit::op_mark_block( void )    // Create/modify a block mark
{
   put_message(mark->mark(file, data->cursor, data->get_column()));
   draw();
}

void
   EdUnit::op_mark_copy( void )     // Copy marked lines
{
   const char* error= mark->verify_copy(data->cursor);
   if( error ) {
     put_message(error);
     return;
   }

   mark->copy();
   mark->paste(file, data->cursor, data->get_column());
   draw();
}

void
   EdUnit::op_mark_cut( void )      // Cut the mark, creating a stash
{
   put_message( mark->cut() );
   draw();
}

void
   EdUnit::op_mark_delete( void )   // Delete marked lines
{
   put_message( mark->cut() );
   draw();
}

void
   EdUnit::op_mark_format( void )   // Format a mark using word tokens
{
   data->commit();
   put_message( mark->format() );   // Format the paragraph
}

void
   EdUnit::op_mark_line( void )     // Create/modify a line mark
{
   put_message( mark->mark(file, data->cursor) );
   draw();
}

void
   EdUnit::op_mark_move( void )     // Move marked lines
{
   const char* error= mark->verify_move(data->cursor);
   if( error ) {
     put_message(error);
     return;
   }

   put_message( mark->cut() );
   mark->paste(file, data->cursor, data->get_column());
   draw();
}

void
   EdUnit::op_mark_paste( void )    // Paste the current stash
{
   data->commit();
   const char* error= mark->paste(file, data->cursor, data->get_column());
   if( error )
     put_message(error);
   else
     draw();
}

void
   EdUnit::op_mark_stash( void )    // Stash the current mark
{  put_message( mark->copy() ); }

void
   EdUnit::op_mark_undo( void )     // Undo the mark
{
   EdFile* mark_file= mark->mark_file;
   mark->undo();
   if( file == mark_file )
     draw();
   else
     draw_top();                    // (Remove "No Mark" message)
}

void
   EdUnit::op_quit( void )          // Unconditionally quit current file
{  remove_file(); }

void
   EdUnit::op_redo( void )          // Redo the previous (file) undo
{
   data->commit();
   file->redo();
}

void
   EdUnit::op_repeat_change( void ) // Repeat the prior change operation
{  put_message( do_change() ); }

void
   EdUnit::op_repeat_locate( void ) // Repeat the prior locate operation
{  put_message( do_locate() ); }

void
   EdUnit::op_safe_exit( void )     // Exit (editor) if no files changed
{  put_message( do_quit() ); }

void
   EdUnit::op_safe_quit( void )     // Quit (file) if no files changed
{  put_message( do_quit() ); }

void
   EdUnit::op_save( void )          // Save the current file
{
   data->commit();
   const char* error= write_file(nullptr);
   if( error )
     put_message(error);
   else
     draw_top();
}

void
   EdUnit::op_split_line( void )    // Split the cursor line into two lines
{  put_message( do_split() ); }     // Split the current line

void
   EdUnit::op_swap_view( void )     // Handle swap view
{  do_view(); }

void
   EdUnit::op_undo( void )          // Undo the previous (file) redo
{
   if( data->active.undo() ) {
     data->draw_active();
     draw_top();
   } else {
     file->undo();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::fetch_screen_state
//
// Purpose-
//       Load the current screen state
//
//----------------------------------------------------------------------------
void
   EdUnit::fetch_screen_state( void ) // Load the current screen state
{
   this->head= this->tail= file->top_line;
   data->col_zero= file->col_zero;
   data->row_zero= file->row_zero;
   data->col= file->col;
   data->row= file->row;
   if( data->row < USER_TOP )
     data->row= USER_TOP;

   synch_cursor();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::store_screen_state
//
// Purpose-
//       Save the current screen state
//
//----------------------------------------------------------------------------
void
   EdUnit::store_screen_state( void ) const // Save the current screen state
{
   data->commit();

   file->csr_line= data->cursor;
   file->top_line= this->head;
   file->col_zero= data->col_zero;
   file->row_zero= data->row_zero;
   file->col= data->col;
   file->row= data->row;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::synch_cursor
//
// Purpose-
//       Insure the cursor line, row, and column are validly positioned
//       on-screen.
//
// Implementation notes-
//       Row and column are each clamped independently and unconditionally;
//       clamping a value that's already valid is a no-op. Callers that
//       need to know whether the cursor was visible *before* synching
//       (for example, to decide whether to switch to the history view)
//       must check data->row/data->col against row_size/col_size
//       themselves before calling this method.
//
//       It's possible to shrink the screen so that no column and/or no row
//       can be displayed. For the TERM Editor, this could cause a busy loop
//       to occur because mvwgetch would have an invalid (-1) offset.
//       (This condition is now checked in EdInps::poll, where wgetch is used
//       instead of mvwgetch to avoid the busy loop.)
//
//       The XCB Editor uses asynchronous events rather than polling, and does
//       not have a busy loop in this situation.
//
//----------------------------------------------------------------------------
void
   EdUnit::synch_cursor( void )     // Insure the cursor is on-screen
{
   // Row: keep data->row within [top, row_size-1-USER_BOT]
   unsigned top= USER_TOP;
   if( row_size == 0 )              // (Defensive: no screen at all)
     top= 0;
   else if( top >= row_size )       // If no room for the reserved top rows
     top= row_size - 1;             // (Last on-screen row is the best we can do)

   if( data->row < top )            // (File initial row == 0)
     data->row= top;

   EdLine* line= head;              // Get the top line
   const char* match_type= " ???";  // Default, NO match
   for(unsigned r= top; ; r++) {    // Set the Active line
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

     if( row_size == 0 || (r + 1 + USER_BOT) >= row_size ) { // (Window shrinks)
       match_type= "size";          // Window shrink
       data->row= r;
       break;
     }

     line= next;
   }

   // Column: keep data->col within [0, col_size-1]
   if( data->col >= col_size )
     data->col= col_size ? col_size - 1 : 0;

   // Set the cursor/active line (with trace)
   Trace::trace(".CSR", match_type, data->cursor, line); // (Old, new)
   data->cursor= line;
   data->active.reset(line->text);
   if( !(view == hist && file->mess_list.get_head()) )
     show_cursor();
}
