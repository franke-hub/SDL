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
//       Editor.cpp
//
// Purpose-
//       Editor: Implement Editor.h
//
// Last change date-
//       2024/03/31
//
//----------------------------------------------------------------------------
#ifndef _GNU_SOURCE
#define _GNU_SOURCE                 // For strcasestr
#endif
#include <assert.h>                 // For assert
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For string functions
#include <unistd.h>                 // For close, ftruncate
#include <sys/stat.h>               // For stat
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include <gui/Device.h>             // For gui::Device
#include <gui/Font.h>               // For gui::Font
#include <gui/Layout.h>             // For gui::Layout
#include <gui/Widget.h>             // For gui::Widget, our base class
#include <gui/Window.h>             // For gui::Window
#include <pub/Debug.h>              // For Debug, namespace pub::debugging
#include <pub/Fileman.h>            // For namespace pub::fileman
#include <pub/Signals.h>            // For pub::signals
#include <pub/Thread.h>             // For pub::Thread::sleep
#include <pub/Trace.h>              // For pub::Trace

#include "Active.h"                 // For Active
#include "Config.h"                 // For namespace config
#include "Editor.h"                 // For Editor (Implementation class)
#include "EdFile.h"                 // For EdFile, EdLine, EdPool, ...
#include "EdHist.h"                 // For EdHist
#include "EdMark.h"                 // For EdMark
#include "EdMisc.h"                 // For EdMisc TODO: REMOVE
#include "EdPool.h"                 // For EdPool
#include "EdTerm.h"                 // For EdTerm
#include "EdView.h"                 // For EdView

using namespace config;             // For namespace config (opt_*)
using namespace editor;             // For namespace editor
using namespace pub::debugging;     // For debugging
using pub::Debug;                   // For pub::Debug
using pub::Trace;                   // For pub::Trace

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra bringup diagnostics?
,  USE_HCDM_FILE_DEBUG= true        // (As opposed to name-only file info)
}; // Compilation controls

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
EdTerm*                editor::term= nullptr; // The terminal controller

pub::List<EdFile>      editor::file_list; // The list of EdFiles
EdFile*                editor::file= nullptr; // The current File
EdFile*                editor::last= nullptr; // The last inserted File

Active*                editor::actalt= nullptr; // Active, for temporary use
Active*                editor::active= nullptr; // Active, for temporary use
EdMark*                editor::mark= nullptr; // The Mark Handler
EdView*                editor::data= nullptr; // The data view
EdHist*                editor::hist= nullptr; // The history view
EdView*                editor::view= nullptr; // The active view

std::string            editor::locate_string; // The locate string
std::string            editor::change_string; // The change string

pub::List<EdPool>      editor::filePool; // File allocation EdPool
pub::List<EdPool>      editor::textPool; // Text allocation EdPool

// Diagnostic controls -------------------------------------------------------
uint32_t               editor::diagnostic= 0; // Non-zero if abnormal state

// Search controls -----------------------------------------------------------
uint32_t               editor::locate_back= false;
uint32_t               editor::locate_case= false;
uint32_t               editor::locate_wrap= false;

// (Format) margins ----------------------------------------------------------
size_t                 editor::margins[2]= {1, 80}; // {Left, right} margin
size_t                 editor::tabs[Editor::TAB_DIM]= {}; // Tab position array

//----------------------------------------------------------------------------
//
// Subroutine-
//       edit_strstr
//
// Purpose-
//       Editor editor::locate_case controlled strstr
//
//----------------------------------------------------------------------------
static const char*                  // Resultant
   edit_strstr(                     // Multi-case strstr
     const char*       lhs,         // Source string
     const char*       rhs)         // Locate string
{
   if( editor::locate_case )        // If case sensitive compare
     return strstr(lhs, rhs);
   else
     return strcasestr(lhs, rhs);
}

//----------------------------------------------------------------------------
//
// Method-
//       last_strstr
//
// Purpose-
//       Locate last occurance of string in text.
//
//----------------------------------------------------------------------------
static const char*                  // Last occurance, nullptr none
   last_strstr(                     // Locate last occurance of string in text
     const char*       text,        // Text origin
     const char*       find)        // Search string
{
   const char* final= edit_strstr(text, find); // Find first occurance
   if( final ) {                    // If existent
     for(;;) {                      // Find last occurrance
       const char* next= edit_strstr(final+1, find);
       if( next == nullptr )
         break;

       final= next;
     }
   }

   return final;
}

//----------------------------------------------------------------------------
//
// Method-
//       prev_locate
//
// Purpose-
//       Locate next occurance of string, searching backwards
//
//----------------------------------------------------------------------------
static const char*                  // Return message, nullptr if OK
   prev_locate(                     // Locate previous
     int               offset)      // (For locate_change, offset= 0, else 1)
{
   const char* S= locate_string.c_str();

   //-------------------------------------------------------------------------
   // Locate in the active line
   EdLine* line= data->cursor;

   if( (line->flags & EdLine::F_PROT) == 0 ) { // If line is not protected
     Active& A= *editor::active;    // Working Active buffer
     A.reset(data->active.get_buffer());
     size_t column= data->get_column() + editor::locate_string.size();
     if( offset && column > 0 ) {
       const char* C= A.resize(column - 1);
       const char* M= last_strstr(C, S);
       if( M != nullptr ) {
         term->move_cursor_H(M - C);
         term->draw_top();
         return nullptr;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Search backward in file
   for(line= line->get_prev(); line; line= line->get_prev() ) {
     if( (line->flags & EdLine::F_PROT) == 0 ) {
       const char* M= last_strstr(line->text, S);
       if( M != nullptr ) {
         term->activate(line);
         term->move_cursor_H(M - line->text);
         return nullptr;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Search wrap
   if( editor::locate_wrap ) {
     line= file->line_list.get_tail(); // (The "bot of file" line, skipped)
     for(line= line->get_prev(); line; line= line->get_prev()) {
       if( (line->flags & EdLine::F_PROT) == 0 ) {
         const char* M= last_strstr(line->text, S);
         if( M != nullptr ) {
           term->activate(line);
           term->move_cursor_H(M - line->text);
           put_message("Wrapped");
           return nullptr;
         }
       }
     }
   }

   return "Not found";
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::Editor
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Editor::Editor(                  // Constructor
     int               argi,        // Argument index
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   using namespace editor;

   if( opt_hcdm )
     debugh("Editor::Editor\n");

   // Allocate initial textPool
   textPool.fifo(new EdPool(EdPool::MIN_SIZE));

   // Allocate editor namespace objects
   actalt= new Active();            // An Active work area
   active= new Active();            // An Active work area
   term= new EdTerm();              // Terminal controller
   data= new EdView();              // Data view
   hist= new EdHist();              // History view
   mark= new EdMark();              // Mark handler
   view= hist;                      // (Initial view)

   //-------------------------------------------------------------------------
   // Load the edit files
   int protect= false;              // Default, read/write files
   if( argv[0][0] == 'v' || argv[0][0] == 'V' ) // If View command (not Edit)
     protect= true;

   for(int i= argi; i<argc; i++)
     file_loader(argv[i], protect);

   if( file_list.get_head() == nullptr ) // Always have something
     file_loader(nullptr);          // Even if it's an empty file

   //-------------------------------------------------------------------------
   // Activate the terminal
   device->insert(term);
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::~Editor
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Editor::~Editor( void )          // Destructor
{
   using namespace editor;

   // Remove and delete Files
   for(EdFile* file= file_list.remq(); file; file= file_list.remq())
     delete file;

   // Remove and delete storage pools
   for(EdPool* pool= textPool.remq(); pool; pool= textPool.remq())
     delete pool;

   for(EdPool* pool= filePool.remq(); pool; pool= filePool.remq())
     delete pool;

   // Delete allocated objects
   delete actalt;
   delete active;
   delete term;
   delete data;
   delete hist;
   delete mark;
   delete window;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Editor::debug(                   // Debugging display
     const char*       info)        // Associated info
{
   debugf("Editor::debug(%s)\n", info ? info : "");
   debugf("..device(%p) window(%p) term(%p)\n", device, window, term);
   debugf("..mark(%p) data(%p) hist(%p) view(%p)\n", mark, data, hist, view);
   debugf("..locate[%s] change[%s]\n"
         , locate_string.c_str(), change_string.c_str());

   debugf("\n..file_list(%p,%p) file(%p)\n"
         , file_list.get_head(), file_list.get_tail(), file);
   for(EdFile* file= file_list.get_head(); file; file= file->get_next()) {
     if( USE_HCDM_FILE_DEBUG )      // If hard core file debugging
       file->debug(info);
     else                           // If name only file debugging
       debugf("..[%p] '%s'\n", file, file->name.c_str());
   }

   size_t size; size_t used;        // Total size, Total used
   size= used= 0;
   debugf("\n..filePool[%p,%p]\n", filePool.get_head(), filePool.get_tail());
   for(EdPool* pool= filePool.get_head(); pool; pool= pool->get_next()) {
     debugf("..[%p] used(%'8zu) size(%'8zu)\n", pool
          , pool->get_used(), pool->get_size());
     size += pool->get_size();
     used += pool->get_used();
   }
   debugf("..****TOTAL**** used(%'8zu) size(%'8zu)\n", used, size);

   size= used= 0;
   debugf("\n..textPool[%p,%p]\n", textPool.get_head(), textPool.get_tail());
   for(EdPool* pool= textPool.get_head(); pool; pool= pool->get_next()) {
     debugf("..[%p] used(%'8zu) size(%'8zu)\n", pool
           , pool->get_used(), pool->get_size());
     size += pool->get_size();
     used += pool->get_used();
   }
   debugf("..****TOTAL**** used(%'8zu) size(%'8zu)\n", used, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::alertf
//
// Purpose-
//       Debugging alert
//
//----------------------------------------------------------------------------
void
   Editor::alertf(                  // Debugging alert
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer
   va_start(argptr, fmt);           // Initialize va_ functions
   std::string S= pub::utility::to_stringv(fmt, argptr);
   va_end(argptr);                  // Close va_ functions

   static int recursion= 0;
   debug_set_mode(Debug::MODE_INTENSIVE); // (Flush after each debugf line)
   debugf("Editor::alertf(%s)%s\n", S.c_str(), recursion ? " recursion" : "");
   if( recursion )
     exit(EXIT_FAILURE);

   recursion= true;
   Trace::trace(".BUG", __LINE__, "Editor.cpp"); // (Trace error occurance)
   debug_backtrace();               // (Diagnostic backtrace, if OS supported)
   Config::debug(S.c_str());        // (Diagnostic dump)

   diagnostic= true;                // Enter diagnostic state
   Config::errorf("Diagnostic mode entered, alt-pause to exit\n");
   pub::Trace* trace= pub::Trace::table;
   if( trace )
     trace->flag[pub::Trace::X_HALT]= true;
   editor::put_message(S.c_str(), EdMess::T_MESS); // (Alert user)
   recursion= false;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::put_message
//
// Purpose-
//       editor::put_message with formatting
//
//----------------------------------------------------------------------------
void
   Editor::put_message(             // editor::put_message with formatting
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer
   va_start(argptr, fmt);           // Initialize va_ functions
   std::string S= pub::utility::to_stringv(fmt, argptr);
   va_end(argptr);                  // Close va_ functions

   editor::put_message(S.c_str());
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::allocate
//
// Purpose-
//       Allocate file/line text
//
//----------------------------------------------------------------------------
char*                               // The (immutable) text
   editor::allocate(                // Get (immutable) text
     size_t            length)      // Of this length (includes '\0' delimit)
{
   char* text= nullptr;             // Not allocated (yet)
   EdPool* pool= textPool.get_head(); // Get text pool
   if( pool == nullptr ) {          // If no pool yet
     pool= new EdPool(EdPool::MIN_SIZE); // Create one now
     textPool.lifo(pool);           // And use it
   }

   text= pool->allocate(length);
   if( text == nullptr ) {
     if( length > EdPool::MIN_SIZE ) { // If large allocation
       pool= new EdPool(length);    // All filePools are 100% allocated
       text= pool->allocate(length);
       filePool.lifo(pool);
     } else {                       // If small allocation
       for(pool= pool->get_next(); pool; pool= pool->get_next() ) {
         text= pool->allocate(length); // Try all textPools
         if( text )
           break;
       }

       if( text == nullptr ) {      // If a new EdPool is required
         if( opt_hcdm )
           debugh("Editor.allocate(%zd) New pool\n", length);
         pool= new EdPool(EdPool::MIN_SIZE);
         text= pool->allocate(length);
         textPool.lifo(pool);
       }
     }
   }

   if( opt_hcdm && opt_verbose > 1 )
     traceh("%p= editor::allocate(%zd)\n", text, length);
   return text;
}

const char*                         // The (immutable) text
   editor::allocate(                // Get (immutable) text
     const char*       source)      // Source (mutable) text
{
   size_t L= strlen(source);        // Source length
   while( L > 0 && source[L-1] == ' ' ) // This is unexpected, but correctable
     --L;
   if( L == 0 )                     // If empty source
     return "";                     // (Immutable) empty copy

   char* copy= allocate(L+1);       // Include trailing '\0' in string
   memcpy(copy, source, L);         // Duplicate the string
   copy[L]= '\0';                   // Including trailing '\0'
   return copy;
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::data_protected
//
// Purpose-
//       Check for protected file and data view
//
//----------------------------------------------------------------------------
int                          // Return code, TRUE if error message
   editor::data_protected( void ) // Error if protected file and data view
{
   if( file->protect && view == data ) {
     editor::put_message("Read/only");
     return true;
   }

   return false;
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_change
//
// Purpose-
//       Change next occurance of string.
//
// Implementation note-
//       REDO not required. The line is changed, but not committed.
//
//----------------------------------------------------------------------------
const char*                         // Return message, nullptr if OK
   editor::do_change( void )        // Change next occurance of string
{
   if( data_protected() )
     return nullptr;

   const char* error= do_locate(0); // First, locate the string
   if( error ) return error;        // (If cannot locate)

   // The string has been found and the line activated
   size_t column= data->col_zero + data->col; // The current column
   size_t length= locate_string.length();
   data->active.replace_text(column, length, change_string.c_str());
   term->draw();                    // (Only active line redraw required)
   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_find
//
// Purpose-
//       Locate next occurance of a string that begins a line.
//
// Implementation notes-
//       Case sensitive compare. (Don't want to implement memicmp.)
//
//----------------------------------------------------------------------------
const char*                         // Return message, nullptr if OK
   editor::do_find(                 // Find next col[0] occurance of
     const char*       S)           // This string
{
   data->commit();                  // Commit the active line

   size_t L= strlen(S);             // The string length

   //-------------------------------------------------------------------------
   // Search remainder of file
   EdLine* line= data->cursor;
   for(line= line->get_next(); line; line= line->get_next() ) {
     if( memcmp(line->text, S, L) == 0 ) {
       if( line->get_next() ) {     // If not "end of file" line
         term->activate(line);
         term->move_cursor_H(0);
         return nullptr;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Search wrap
   if( editor::locate_wrap ) {
     line= file->line_list.get_head(); // (The "top of file" line, skipped)
     for(line= line->get_next(); line; line= line->get_next()) {
       if( memcmp(line->text, S, L) == 0 && line->get_next() ) {
         term->activate(line);
         term->move_cursor_H(0);
         put_message("Wrapped");
         return nullptr;
       }
     }
   }

   return "Not found";
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_insert
//
// Purpose-
//       Insert a line after the cursor.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   editor::do_insert( void )        // Insert a new, empty line
{
   if( view != data )
     return "Cursor view";
   if( data_protected() )
     return nullptr;

   data->commit();
   EdLine* after= data->cursor;     // Insert after the current cursor line
   if( after->get_next() == nullptr ) // If it's the last line
     after= after->get_prev();      //  Use the prior line instead

   EdLine* head= file->new_line();  // Get new, empty insert line
   EdLine* tail= head;

   // Handle insert after no delimiter line
   // (The top_of_file line has a delimiter.)
   EdRedo* redo= new EdRedo();      // (First, create the REDO)
   if( after->delim[0] == '\0' && after->delim[1] == '\0' ) {
     head= file->new_line(after->text); // Get "after" line replacement

     pub::List<EdLine> list;        // Connect head and tail
     list.fifo(head);
     list.fifo(tail);

     // Remove the current "after" from the file, updating the REDO
     file->remove(after);           // (Does not modify edLine->get_prev())
     redo->head_remove= redo->tail_remove= after;
     after= after->get_prev();
   }

   // Insert the line(s) (with redo)
   data->col_zero= data->col= 0;
   file->insert(after, head, tail);
   redo->head_insert= head;
   redo->tail_insert= tail;
   file->redo_insert(redo);
   mark->handle_redo(file, redo);
   file->activate(tail);            // (Activate the newly inserted line)
   term->draw();                    // And redraw

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_join
//
// Purpose-
//       Join the current and next line.
//
//----------------------------------------------------------------------------
const char*                         // Return message, nullptr expected
   editor::do_join( void )          // Join the current and next line
{
   if( view != data )
     return "Cursor view";
   if( data_protected() )
     return nullptr;

   data->commit();                  // Commit the active line (removes blanks)

   // (The cursor line itself has already been verified non-protected)
   EdLine* head= data->cursor;      // Address the cursor line
   EdLine* tail= head->get_next();  // The join line
   if( tail->flags & EdLine::F_PROT )
     return "Protected";

   // Join the lines
   EdRedo* redo= new EdRedo();      // Create REDO
   file->remove(head, tail);        // Remove the lines
   redo->head_remove= head;
   redo->tail_remove= tail;
   if( head->text[0] == '\0' )      // If the head line is empty
     data->active.reset(tail->text); // The tail line becomes the join line
   else {
     data->active.reset(head->text); // The current line
     data->active.append_text(" ");
     const char* text= tail->text;
     while( *text == ' ' )
       text++;
     data->active.append_text(text);
   }
   EdLine* line= file->new_line( allocate(data->active.truncate()) );
   file->insert(head->get_prev(), line, line);
   redo->head_insert= redo->tail_insert= line;
   file->redo_insert(redo);
   mark->handle_redo(file, redo);
   data->active.reset(line->text);
   file->activate(line);
   term->draw();

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_locate
//
// Purpose-
//       Locate next occurance of string.
//
//----------------------------------------------------------------------------
const char*                         // Return message, nullptr if OK
   editor::do_locate(               // Locate next
     int               offset)      // Use offset 0 for locate_change
{
   data->commit();                  // Commit the active line

   if( locate_back )                // If reverse search active
     return prev_locate(offset);

   const char* S= locate_string.c_str();

   //-------------------------------------------------------------------------
   // Locate in the active line
   EdLine* line= data->cursor;
   size_t column= data->col_zero + data->col + offset;
   if( (line->flags & EdLine::F_PROT) == 0 ) { // If line is not protected
     const char* C= data->active.get_buffer(column); // Remaining characters
     const char* M= edit_strstr(C, S);
     if( M != nullptr ) {
       data->activate();
       column += M - C;
       term->move_cursor_H(column);
       term->draw_top();
       return nullptr;
     }
   }

   //-------------------------------------------------------------------------
   // Search remainder of file
   for(line= line->get_next(); line; line= line->get_next() ) {
     if( (line->flags & EdLine::F_PROT) == 0 ) {
       const char* M= edit_strstr(line->text, S);
       if( M != nullptr ) {
         data->activate();
         term->activate(line);
         term->move_cursor_H(M - line->text);
         return nullptr;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Search wrap
   if( editor::locate_wrap ) {
     line= file->line_list.get_head(); // (The "top of file" line, skipped)
     for(line= line->get_next(); line; line= line->get_next()) {
       if( (line->flags & EdLine::F_PROT) == 0 ) {
         const char* M= edit_strstr(line->text, S);
         if( M != nullptr ) {
           data->activate();
           term->activate(line);
           term->move_cursor_H(M - line->text);
           put_message("Wrapped");
           return nullptr;
         }
       }
     }
   }

   return "Not found";
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_quit
//
// Purpose-
//       (Safely) remove a file from the file list.
//
// Implementation notes-
//       Called after the active line has been committed, so
//       data->active.get_changed() always returns nullptr.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   editor::do_quit( void )          // Safely remove a file from the file list
{
   if( !file->damaged && file->is_changed() )
     return "File changed";

   remove_file();
   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_split
//
// Purpose-
//       Split the current line at the cursor.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expecte3d
   editor::do_split( void )         // Split the current line at the cursor
{
   if( view != data )
     return "Cursor view";
   if( data_protected() )
     return nullptr;

   data->commit();                  // Commit the active line

   // Create and initialize REDO object, updating the file
   EdLine* cursor= data->cursor;
   EdRedo* redo= new EdRedo();
   file->remove(cursor, cursor);
   redo->head_remove= redo->tail_remove= cursor;

   pub::List<EdLine> line_list;     // Replacement line list
   EdLine* head= file->new_line();
   EdLine* tail= file->new_line();
   line_list.fifo(head);
   line_list.fifo(tail);
   file->insert(cursor->get_prev(), head, tail);
   redo->head_insert= head;
   redo->tail_insert= tail;
   file->redo_insert(redo);
   mark->handle_redo(file, redo);

   ///  We use the active buffer as a modifiable work area. This corrupts it
   ///  so it needs to be reset (to the new head text) when we're done.
   ///  We build the head line in the Active buffer "H" and the tail line in
   ///  the active buffer "T".

   // Use the active buffer as a work area
   Active& A= data->active;         // (The active [split] line)
   A.index(data->get_column() + 1); // (Insure head and tail contain data)
   char* both= (char*)A.get_buffer(); // The modifiable head/tail work area

   // Create the tail line
   Active& T= *editor::actalt;
   T.reset();                       // (Now contains "")
   for(size_t lead= 0; ; lead++) {  // Insert leading blanks in tail
     if( both[lead] != ' ' ) {
       if( lead > 0 )
         T.fetch(lead - 1);
       break;
     }
   }
   size_t X= A.index(data->get_column()); // The split column offset
   while( both[X] == ' ' )          // Skip intervening blanks
     X++;
   T.append_text(both + X);         // (Append trailing text to leading blanks)
   tail->text= allocate(T.truncate()); // Set tail line text

   // Create the head line
   both[X]= '\0';                   // (Delimits the head line at tail origin)
   Active& H= *editor::active;
   H.reset(both);
   head->text= allocate(H.truncate()); // Set head line text

   A.reset(head->text);             // (Discards the working buffer content)
   term->activate(head);
   term->draw();

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_view
//
// Purpose-
//       Invert the view.
//
//----------------------------------------------------------------------------
void
   editor::do_view( void )          // Invert the view
{
   if( view == hist ) {
     data->activate();
     term->draw_cursor();
     term->flush();
   } else {
     term->undo_cursor();
     hist->activate();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::exit
//
// Purpose-
//       Unconditional editor (normal) exit.
//
//----------------------------------------------------------------------------
void
   editor::exit( void )             // Unconditional editor (normal) exit
{  device->operational= false; }

//----------------------------------------------------------------------------
//
// Method-
//       editor::file_command
//
// Purpose-
//       Load command input/output pseudo-file
//
//----------------------------------------------------------------------------
void
   editor::file_command(              // Load command input/output pseudo-file
     const char*       input,         // The command (required)
     const std::string&output)        // The command output
{  if( opt_hcdm )
     traceh("editor::file_command(%s)\n", input);

   EdFile* next= new EdFile(nullptr);
   next->command(input, output);

   file_list.insert(file, next, next);
   last= next;
   term->activate(last);
   hist->activate();
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::file_loader
//
// Purpose-
//       Load files, adding them to the file list
//
//----------------------------------------------------------------------------
#if defined(_OS_CYGWIN)
  #define wildstrcmp pub::utility::wildchar::strcasecmp
#else
  #define wildstrcmp pub::utility::wildchar::strcmp
#endif

void
   editor::file_loader(             // Load files, adding them to the file list
     const char*       name_,       // The file name (file wildcards allowed)
     int               protect)     // Protect file?
{  if( opt_hcdm )
     traceh("editor::file_loader(%s)\n", name_);

   if( name_ == nullptr )           // If missing parameter
     name_= "unnamed.txt";          // Use default name

   // Match existing file name(s)
   using namespace pub::fileman;    // Using fileman objects
   Name name(name_);
   std::string error= name.resolve(); // Remove link qualifiers
   if( error != "" ) {
     Editor::put_message("File(%s) %s", name_, error.c_str());
     return;
   }

   {{{{ // Search directory, handling all wildcard file name matches
     bool found= false;
     Path path(name.path_name);     // (Temporary)
     for(File* file= path.list.get_head(); file; file= file->get_next() ) {
       if( wildstrcmp(name.file_name.c_str(), file->name.c_str()) == 0 ) {
         std::string fqn= name.path_name + "/" + file->name;
         Name wild(fqn);            // The wildcard match name
         wild.resolve();            // (Resolve the name, which may be a link)

         found= true;
         bool is_dup= false;
         for(EdFile* dup= file_list.get_head(); dup; dup= dup->get_next()) {
           if( dup->name == wild.name ) { // If file already in file_list
             last= dup;
             is_dup= true;
             break;
           }
         }

         if( !is_dup ) {
           EdFile* next= new EdFile(wild.name.c_str());
           next->protect |= protect; // (May already be set)
           file_list.insert(last, next, next);
           last= next;
         }
       }
     }
     if( found )
       return;

     // If the file hasn't been written yet, it still might be a duplicate
     for(EdFile* dup= file_list.get_head(); dup; dup= dup->get_next()) {
       if( dup->name == name.name ) {
         last= dup;
         return;
       }
     }
   }}}}

   // Non-existent file (Never protected)
   EdFile* next= new EdFile(name.name.c_str());
   file_list.insert(last, next, next);
   last= next;
}
#undef wildstrcmp

//----------------------------------------------------------------------------
//
// Method-
//       editor::put_message
//
// Purpose-
//       Add message to file's message list
//
//----------------------------------------------------------------------------
void
   editor::put_message(             // Write message
     const char*       mess_,       // Message text
     int               type_)       // Message mode
{  if( file )
     file->put_message(mess_, type_);
   else
     fprintf(stderr, "ERROR: %s\n", mess_);
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::remove_file
//
// Purpose-
//       Unconditionally remove file from the file list, discarding changes
//
//----------------------------------------------------------------------------
void
   editor::remove_file( void )      // Remove the active file from the list
{
   EdFile* next= file->get_prev();
   if( next == nullptr ) {
     next= file->get_next();
     if( next == nullptr )          // If no other files are in the list
       device->operational= false;  // Terminate the Editor
   }

   if( next ) {                     // If another file remains in the list
     file_list.remove(file, file);  // Remove the active file from the list
     delete file;                   // (Delete it)
     editor::file= nullptr;         // (And don't reference it any more)

     term->activate(next);          // (Activate it)
     term->draw();                  // (And draw it)
   } // The last file removed remains on the file_list (It's still referenced)
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::un_changed
//
// Purpose-
//       If any (undamaged) file has changed, activate it
//
//----------------------------------------------------------------------------
bool                                // TRUE if editor in unchanged state
   editor::un_changed( void )       // If any file changed, activate it
{
   data->commit();                  // Commit the active line

   if( !file->damaged && file->is_changed() ) {
     put_message("File changed");
     return false;                  // (It's already active)
   }

   for(EdFile* file= file_list.get_head(); file; file= file->get_next()) {
     if( !file->damaged && file->is_changed() ) {
       term->activate(file);
       put_message("File changed");
       term->draw();
       return false;
     }
   }

   return true;
}

//----------------------------------------------------------------------------
// editor::Virtual Thread simulation methods
//----------------------------------------------------------------------------
void
   editor::join( void )
{  }

void
   editor::start( void )
{
   // Initialize the configuration
   device->configure();

   // Set initial file
   term->activate(file_list.get_head());

   // Start the Device
   device->draw();
   term->show();                    // (Set position fails unless visible)
   term->grab_mouse();              // (Also sets position)
   term->flush();
   device->run();
}
