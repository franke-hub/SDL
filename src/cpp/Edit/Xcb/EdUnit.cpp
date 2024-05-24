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
//       EdUnit.cpp
//
// Purpose-
//       Editor: Input/output interface; Handle editor operations.
//
// Last change date-
//       2024/05/15
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <stdio.h>                  // For sprintf
#include <sys/types.h>              // For system types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/List.h>               // For pub::List
#include <pub/Trace.h>              // For pub::Trace

#include "Active.h"                 // For Active
#include "Config.h"                 // For Config, namespace config
#include "EdData.h"                 // For EdData
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile
#include "EdHist.h"                 // For EdHist
#include "EdMark.h"                 // For EdMark
#include "EdUnit.h"                 // For EdUnit, implemented

using namespace config;             // For config::opt_*, ...
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
{  using namespace editor;

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
{  using namespace editor;

   // (This sequence DOES NOT change the cursor line)
   Active& active= data->active; // The current command line
   const char* command= active.truncate(); // Truncate it
   hist->activate(command); // Activate the history view
}

void
   EdUnit::op_copy_file_name_to_hist( void ) // Copy file name to history line
{  using namespace editor;

   hist->activate(file->name.c_str());
}

void
   EdUnit::op_copy_hist_to_file( void ) // Insert history line into file
{  using namespace editor;

   // NOT TESTED
   put_message( do_insert(hist->get_buffer()) );
}

void
   EdUnit::op_exit_safely( void )   // Exit if no files changed
{  using namespace editor;

   put_message( do_quit() );
}

void
   EdUnit::op_goto_changed( void )  // Activate changed file
{  using namespace editor;

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
{  using namespace editor;

   data->commit();
   EdFile* next= file->get_next();
   if( next == nullptr )
     next= file_list.get_head();
   if( next != file )
     activate(next);
}

void
   EdUnit::op_goto_prev_file( void ) // Activate  prior file
{  using namespace editor;

   data->commit();
   EdFile* prev= file->get_prev();
   if( prev == nullptr )
     prev= file_list.get_tail();
   if( prev != file )
     activate(prev);
}

void
   EdUnit::op_help( void )          // Display help information
{  using namespace editor;

   command_help();
}

void
   EdUnit::op_insert_line( void )   // Insert a new, empty line
{  using namespace editor;

   put_message( do_insert() );      // Insert line after cursor
}

void
   EdUnit::op_join_line( void )     // Join cursor line with next line
{  using namespace editor;

   put_message( do_join() );        // Join current/next lines
}

void
   EdUnit::op_key_arrow_down( void ) // Handle down arrow key
{  using namespace editor;

   view->move_cursor_V(1);
}

void
   EdUnit::op_key_arrow_left( void ) // Handle left arrow key
{  using namespace editor;

   size_t column= view->get_column(); // The cursor column
   if( column > 0 )
     move_cursor_H(column - 1);
}

void
   EdUnit::op_key_arrow_right( void ) // Handle right arrow key
{  using namespace editor;

   move_cursor_H(view->get_column() + 1);
}

void
   EdUnit::op_key_arrow_up( void )  // Handle up arrow key
{  using namespace editor;

   view->move_cursor_V(-1);
}

void
   EdUnit::op_key_backspace( void ) // Handle backspace key
{  using namespace editor;

   if( data_protected() )
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
{  using namespace editor;

   put_message("Invalid key");
}

void
   EdUnit::op_key_delete( void )    // Handle delete key
{  using namespace editor;

   if( data_protected() )
     return;

   view->active.remove_char(view->get_column());
   view->active.append_text(" ");
   view->draw_active();
   draw_top();
}

void
   EdUnit::op_key_end( void )       // Handle end key
{  using namespace editor;

   move_cursor_H(view->active.get_cols());
}

void
   EdUnit::op_key_enter( void )     // Handle enter key
{  using namespace editor;

   move_cursor_H(0);
   view->enter_key();
}

void
   EdUnit::op_key_home( void )      // Handle home key
{  using namespace editor;

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
{  using namespace editor;

}

void
   EdUnit::op_key_insert( void )    // Handle insert key
{  using namespace editor;

   key_state ^= KS_INS;             // Invert the insert state
   draw_top();
}

void
   EdUnit::op_key_page_down( void ) // Handle page down key
{  using namespace editor;

   int rows= row_size - (USER_TOP + USER_BOT + 1);
   move_screen_V(+rows);
}

void
   EdUnit::op_key_page_up( void )   // Handle page up key
{  using namespace editor;

   int rows= row_size - (USER_TOP + USER_BOT + 1);
   move_screen_V(-rows);
}

void
   EdUnit::op_key_tab_forward( void ) // Handle forward tab operation
{  using namespace editor;

   move_cursor_H(tab_forward(view->get_column()));
}

void
   EdUnit::op_key_tab_reverse( void ) // Handle reverse tab operation
{  using namespace editor;

   move_cursor_H(tab_reverse(view->get_column()));
}

void
   EdUnit::op_line_to_bot( void )   // Move cursor line to end of screen
{  using namespace editor;

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
{  using namespace editor;

   head= data->cursor;
   data->row_zero += (data->row - USER_TOP);
   data->row= USER_TOP;
   draw();
}

void
   EdUnit::op_mark_block( void )    // Create/modify a block mark
{  using namespace editor;

   put_message(mark->mark(file, data->cursor, data->get_column()));
   draw();
}

void
   EdUnit::op_mark_copy( void )     // Copy marked lines
{  using namespace editor;

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
{  using namespace editor;

   put_message( mark->cut() );
   draw();
}

void
   EdUnit::op_mark_delete( void )   // Delete marked lines
{  using namespace editor;

   put_message( mark->cut() );
   draw();
}

void
   EdUnit::op_mark_format( void )   // Format a mark using word tokens
{  using namespace editor;

   data->commit();
   put_message( mark->format() ); // Format the paragraph
}

void
   EdUnit::op_mark_line( void )     // Create/modify a line mark
{  using namespace editor;

   put_message( mark->mark(file, data->cursor) );
   draw();
}

void
   EdUnit::op_mark_move( void )     // Move marked lines
{  using namespace editor;

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
{  using namespace editor;

   data->commit();
   const char* error= mark->paste(file, data->cursor, data->get_column());
   if( error )
     put_message(error);
   else
     draw();
}

void
   EdUnit::op_mark_stash( void )    // Stash the current mark
{  using namespace editor;

   put_message( mark->copy() );
}

void
   EdUnit::op_mark_undo( void )     // Undo the mark
{  using namespace editor;

   EdFile* mark_file= mark->mark_file;
   mark->undo();
   if( file == mark_file )
     draw();
   else
     draw_top();                    // (Remove "No Mark" message)
}

void
   EdUnit::op_quit( void )          // Unconditionally quit current file
{  using namespace editor;

   remove_file();
}

void
   EdUnit::op_redo( void )          // Redo the previous (file) undo
{  using namespace editor;

   data->commit();
   file->redo();
}

void
   EdUnit::op_repeat_change( void ) // Repeat the prior change operation
{  using namespace editor;

   put_message( do_change() );
}

void
   EdUnit::op_repeat_locate( void ) // Repeat the prior locate operation
{  using namespace editor;

   put_message( do_locate() );
}

void
   EdUnit::op_safe_exit( void )     // Exit (editor) if no files changed
{  using namespace editor;

   put_message( do_quit() );
}

void
   EdUnit::op_safe_quit( void )     // Quit (file) if no files changed
{  using namespace editor;

   put_message( do_quit() );
}

void
   EdUnit::op_save( void )          // Save the current file
{  using namespace editor;

   data->commit();
   const char* error= write_file(nullptr);
   if( error )
     put_message(error);
   else
     draw_top();
}

void
   EdUnit::op_split_line( void )    // Split the cursor line into two lines
{  using namespace editor;

   put_message( do_split() );       // Split the current line
}

void
   EdUnit::op_swap_view( void )     // Handle swap view
{  using namespace editor;

   do_view();
}

void
   EdUnit::op_undo( void )          // Undo the previous (file) redo
{  using namespace editor;

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
//       EdUnit::synch_active
//
// Purpose-
//       Set the Active (cursor) line, usually from the current row.
//
//----------------------------------------------------------------------------
void
   EdUnit::synch_active( void )     // Set the Active (cursor) line
{  using namespace editor;

   if( data->row < USER_TOP )       // (File initial row == 0)
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
     show_cursor();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdUnit::synch_file
//
// Purpose-
//       Save the current state in the active file
//
//----------------------------------------------------------------------------
void
   EdUnit::synch_file( void ) const // Synchronize the active file
{  using namespace editor;

   data->commit();

   file->csr_line= data->cursor;
   file->top_line= this->head;
   file->col_zero= data->col_zero;
   file->row_zero= data->row_zero;
   file->col= data->col;
   file->row= data->row;
}
