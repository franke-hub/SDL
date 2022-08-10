//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2022 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdFile.cpp
//
// Purpose-
//       Editor: Implement EdFile.h
//
// Last change date-
//       2022/03/15
//
// Implements-
//       EdFile: Editor File descriptor
//       EdLine: Editor File Line descriptor
//       EdMess: Editor File Message descriptor
//       EdHide: (UNUSED, MOSTLY PLACEHOLDER)
//           REQUIRES LINE COUNT SCREEN MANAGEMENT UPDATES
//       EdRedo: EdFile redo/undo descriptor
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf, fopen, fclose, ...
#include <stdlib.h>                 // For various
#include <unistd.h>                 // For unlink
#include <sys/stat.h>               // For stat

#include <gui/Types.h>              // For gui::Line
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For pub::Name
#include <pub/Signals.h>            // For pub::signals::Signal
#include <pub/Trace.h>              // For pub::Trace
#include <pub/List.h>               // For pub::List

#include "Config.h"                 // For Config::check
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile, EdLine, EdRedo
#include "EdMark.h"                 // For EdMark
#include "EdText.h"                 // For EdText
#include "EdView.h"                 // For EdView

using namespace config;             // For config::opt_*
using namespace pub::debugging;     // For debugging
using pub::Trace;                   // For pub::Trace

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
}; // Compilation controls

#define USE_BRINGUP true            // Use bringup diagnostics? TODO: false

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
pub::signals::Signal<EdFile::CloseEvent>
                       EdFile::close_signal; // CloseEvent signal

//----------------------------------------------------------------------------
//
// Subroutine-
//       debug_redo
//       assert_line
//       assert_miss
//       assert_xxdo
//       assert_redo
//       assert_undo
//
// Purpose-
//       Bringup debugging
//
// Implementation notes-
//       For BRINGUP checking. NOT optimized.
//
//----------------------------------------------------------------------------
#if USE_BRINGUP
static void
   debug_redo(                      // Inconsistent REDO
     int               line,        // Line number
     EdRedo*           redo)        // For this REDO
{
   debugf("%4d EdFile redo(%p,%p,%p,%p)\n", line
         , redo->head_insert, redo->tail_insert
         , redo->head_remove, redo->tail_remove);

   redo->debug("Inconsistent");

   if( !editor::file->damaged ) {   // One report per file
     editor::file->damaged= true;
     Editor::alertf("REDO/UNDO inconsistent");
   } else
     debugf("\n");
}

static void
   assert_line(                     // Assert
     EdLine*           test,        // This line is active in
     EdFile*           file,        // This file for
     EdRedo*           redo)        // This redo
{
   for(EdLine* L= file->line_list.get_head(); L; L= L->get_next()) {
     if( L == test )
       return;
   }

   debugf("%4d EdFile(%p)->assert_line(%p) FAILED\n", __LINE__, file, test);
   debug_redo(__LINE__, redo);
}

static void
   assert_miss(                     // Assert
     EdLine*           test,        // This line is not active in
     EdFile*           file,        // This file for
     EdRedo*           redo)        // This redo
{
   for(EdLine* line= file->line_list.get_head(); line; line= line->get_next() ) {
     if( line == test ) {
       debugf("%4d EdFile(%p)->assert_miss(%p) FAILED\n", __LINE__, file, test);
       debug_redo(__LINE__, redo);
     }
   }
}

static void
   assert_base(                     // Assert
     EdRedo*           redo)        // This REDO/UNDO is self-consistent
{
   if( redo->head_insert ) {
     if( redo->tail_insert == nullptr )
       debug_redo(__LINE__, redo);
   } else if( redo->tail_insert )
     debug_redo(__LINE__, redo);

   if( redo->head_remove ) {
     if( redo->tail_remove == nullptr )
       debug_redo(__LINE__, redo);
   } else if( redo->tail_remove )
     debug_redo(__LINE__, redo);

   if( redo->head_insert && redo->head_remove ) {
     if( redo->head_insert->get_prev() != redo->head_remove->get_prev() )
       debug_redo(__LINE__, redo);
     if( redo->tail_insert->get_next() != redo->tail_remove->get_next() )
       debug_redo(__LINE__, redo);
   } else if( !(redo->head_insert || redo->head_remove) ) {
     debug_redo(__LINE__, redo);
   }
}

static void
   assert_redo(                     // Assert
     EdRedo*           redo,        // This REDO is self-consistent
     EdFile*           file)        // For this file
{
   assert_base(redo);

   if( redo->head_remove ) {        // If redo remove
     assert_line(redo->head_remove->get_prev(), file, redo);
     assert_line(redo->tail_remove->get_next(), file, redo);

     for(EdLine* line= redo->head_remove; ; line= line->get_next() ) {
       if( line == nullptr ) {
         debugf("%4d EdFile nullptr redo(,,%p,%p)\n", __LINE__
               , redo->head_remove, redo->tail_remove);
         debug_redo(__LINE__, redo);
       }
       assert_line(line, file, redo);
       if( line == redo->tail_remove )
         break;
     }
   }
   if( redo->head_insert ) {        // If redo insert
     assert_line(redo->head_insert->get_prev(), file, redo);
     assert_line(redo->tail_insert->get_next(), file, redo);

     for(EdLine* line= redo->head_insert; ; line= line->get_next() ) {
       if( line == nullptr ) {
         debugf("%4d EdFile nullptr redo(%p,%p,,)\n", __LINE__
               , redo->head_insert, redo->tail_insert);
         debug_redo(__LINE__, redo);
       }
       assert_miss(line, file, redo);
       if( line == redo->tail_insert )
         break;
     }
   }
}

static void
   assert_undo(                     // Assert
     EdRedo*           undo,        // This UNDO is self-consistent
     EdFile*           file)        // For this file
{
   assert_base(undo);

   if( undo->head_insert ) {        // If undo insert
     assert_line(undo->head_insert->get_prev(), file, undo);
     assert_line(undo->tail_insert->get_next(), file, undo);

     for(EdLine* line= undo->head_insert; ; line= line->get_next() ) {
       if( line == nullptr ) {
         debugf("%4d EdFile nullptr undo(%p,%p,,)\n", __LINE__
               , undo->head_insert, undo->tail_insert);
         debug_redo(__LINE__, undo);
       }
       assert_line(line, file, undo);
       if( line == undo->tail_insert )
         break;
     }
   }
   if( undo->head_remove ) {        // If undo remove
     assert_line(undo->head_remove->get_prev(), file, undo);
     assert_line(undo->tail_remove->get_next(), file, undo);

     for(EdLine* line= undo->head_remove; ; line= line->get_next() ) {
       if( line == nullptr ) {
         debugf("%4d EdFile nullptr undo(,,%p,%p)\n", __LINE__
               , undo->head_remove, undo->tail_remove);
         return;
       }
       assert_miss(line, file, undo);
       if( line == undo->tail_remove )
         break;
     }
   }
}
#else // If USE_BRINGUP == false
static void assert_base(EdRedo*) {} // Assert self-consistent REDO insert
static void assert_redo(EdRedo*, EdFile*) {} // Assert self-consistent REDO
static void assert_undo(EdRedo*, EdFile*) {} // Assert self-consistent UNDO
#endif

//----------------------------------------------------------------------------
//
// Subroutine-
//       chg_mode
//
// Purpose-
//       Change the file mode after redo/undo.
//
//----------------------------------------------------------------------------
static void
   chg_mode(                        // Get file mode
     const EdLine*     head,        // Head changed line
     const EdLine*     tail)        // Tail changed line
{
   // The mode does not change unless all file lines changed, e.g. by set_mode
   if( head->get_prev()->get_prev() != nullptr )
     return;
   if( tail->get_next()->get_next() != nullptr )
     return;

   int delim= '\0';                 // Unix delimiter modifier
   int mode= EdFile::M_UNIX;
   if( head->delim[1] == '\r' ) {
     delim= '\r';
     mode= EdFile::M_DOS;
   }

   for(const EdLine* line= head; line; line= line->get_next()) {
     if( line->delim[0] != '\n' ) { // If neither DOS nor UNIX
       if( line->delim[1] != '\0' ) // (line->delim[0] == '\0' assumed)
         mode= EdFile::M_BIN;       // Must be binary file
       break;
     }
     if( line->delim[1] != delim )  // If inconsistent mode
       mode= EdFile::M_MIX;         // Mixed mode (Might still be binary)

     if( line == tail )             // If all lines checked
       break;
   }

   editor::file->mode= mode;        // Update the mode
   editor::text->draw_info();       // And redraw
}

//============================================================================
//
// Method-
//       EdFile::EdFile
//
// Purpose-
//       Constructor/Destructor
//
//----------------------------------------------------------------------------
   EdFile::EdFile(                  // Constructor
     const char*       name_)       // Fully qualified file name
:  ::pub::List<EdFile>::Link()
,  name(name_)
{  if( HCDM || opt_hcdm )
     traceh("EdFile(%p)::EdFile(%s)\n", this, get_name().c_str());

   Trace::trace(".NEW", "file", this);

   EdLine* top= new_line("* * * * Top of file * * * *");
   EdLine* bot= new_line("* * * * End of file * * * *");
   top->flags= bot->flags= EdLine::F_PROT; // (Protect these lines)
   line_list.fifo(top);
   line_list.fifo(bot);

   top_line= top;
   csr_line= top;

   if( name_ )
     append(name_, top);            // Insert the file
}

   EdFile::~EdFile( void )          // Destructor
{
   if( HCDM || opt_hcdm )
     traceh("EdFile(%p)::~EdFile(%s)\n", this, get_name().c_str());

   Trace::trace(".DEL", "file", this);

   if( HCDM && !line_list.is_coherent() )
     Editor::alertf("%4d incoherent\n", __LINE__);

   reset();                         // Delete REDO/UNDO lists

   if( HCDM && !line_list.is_coherent() )
     Editor::alertf("%4d incoherent\n", __LINE__);

   for(;;) {                        // Delete all lines
     EdLine* line= line_list.remq();
     if( line == nullptr )
       break;

     delete line;
   }

   // Raise CloseEvent signal
   CloseEvent close_event; close_event.file= this;
   close_signal.signal(close_event);
}

//----------------------------------------------------------------------------
// EdFile::Accessor methods
//----------------------------------------------------------------------------
char*
   EdFile::allocate(                // Allocate file text
     size_t            size) const  // Of this length
{  return editor::allocate(size); }

EdLine*                             // The EdLine*
   EdFile::get_line(                // Get EdLine*
     size_t            row) const   // For this row number
{
   EdLine* line= line_list.get_head(); // Get top line
   while( row > 0 ) {               // Locate row
     line= (EdLine*)line->get_next();
     if( line == nullptr )
       break;

     row--;
   }

   return line ? line : line_list.get_tail(); // (Never return nullptr)
}

size_t                              // The row number
   EdFile::get_row(                 // Get row number
     const EdLine*    cursor) const // For this line
{
   size_t row= 0;
   for(EdLine* line= line_list.get_head(); line; line= line->get_next() ) {
     if( line == cursor )
       break;

     row++;
   }

   return row;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::debug
//
// Purpose-
//       Debugging display.
//
// Implementation notes-
//       If the file is active, text->synch_file updates the state and commits
//       the active line
//
//----------------------------------------------------------------------------
static inline const char*           // "true" or "false"
   TF(bool B)
{  if( B ) return "true"; return "false"; }

void
   EdFile::debug(                   // Debugging display
     const char*       info)        // Associated info
{
   debugf("EdFile(%p)::debug(%s) '%s'\n", this
         , info ? info : "", get_name().c_str());

   if( this == editor::file )       // If this is the active file
     editor::text->synch_file(this); // Get current text state
   debugf("..mode(%d) changed(%s) damaged(%s) protect(%s)\n"
         , mode, TF(changed), TF(damaged), TF(protect));
   debugf("..top_line(%p) csr_line(%p)\n", top_line, csr_line);
   debugf("..col_zero(%zd) col(%d) row_zero(%zd) row(%d) rows(%zd)\n"
         , col_zero, col, row_zero, row, rows);

   debugf("..mess_list[%p,%p]:\n", mess_list.get_head(), mess_list.get_tail());
   for(EdMess* mess= mess_list.get_head(); mess; mess= mess->get_next()) {
     debugf("....(%p) %d '%s'\n", mess, mess->type, mess->mess.c_str());
   }

   debugf("..redo_list[%p,%p]:\n", redo_list.get_head(), redo_list.get_tail());
   for(EdRedo* redo= redo_list.get_head(); redo; redo= redo->get_next()) {
     redo->debug("redo");
   }

   debugf("..undo_list[%p,%p]:\n", undo_list.get_head(), undo_list.get_tail());
   for(EdRedo* redo= undo_list.get_head(); redo; redo= redo->get_next()) {
     redo->debug("undo");
   }

   debugf("..line_list[%p,%p]:\n", line_list.get_head(), line_list.get_tail());
   if( strcasecmp(info, "lines") == 0 ) {
     size_t N= 0;
     for(EdLine* line= line_list.get_head(); line; line= line->get_next()) {
       debugf("[%4zd] ", N++);
       line->debug();
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::activate
//
// Purpose-
//       Activate file line
//
//----------------------------------------------------------------------------
void
   EdFile::activate(                // Activate
     EdLine*           line)        // This line
{
   if( this == editor::file ) {     // If the file is active
     editor::text->activate(line);
   } else {                         // If the file is off-screen
     top_line= csr_line= line;
     col_zero= col= row= 0;
     row_zero= get_row(line);
  }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::append
//
// Purpose-
//       Load file data
//
//----------------------------------------------------------------------------
EdLine*                             // The last inserted line
   EdFile::append(                  // Append file
     const char*       name,        // The file name to insert
     EdLine*           line)        // Insert after this line
{
   struct stat st;                  // File stats
   int rc= stat(name, &st);         // Get file information
   if( rc != 0 ) {                  // If failure
     put_message("File not found");
     return nullptr;
   }

   if( !S_ISREG(st.st_mode) ) {
     damaged= true;
     protect= true;
     if( S_ISDIR(st.st_mode) )
       put_message("Directory");
     else
       put_message("Unusable");
     return nullptr;
   }

   if( st.st_size == 0 )            // If empty file
     return nullptr;                // Nothing to append

   // Allocate the input data area Pool
   size_t size= st.st_size;         // The size of the file
   char* text= allocate(size + 1);  // Allocate space for entire file (+ '\0')
   memset(text, 0, size + 1);       // (In case read fails)

   // Load the file
   FILE* f= fopen(name, "rb");
   if( f == nullptr ) {
     damaged= true;
     put_message("Open failure");
     return nullptr;
   }

   size_t L= fread(text, 1, size, f);
   fclose(f);
   if( L != size ) {
     damaged= true;
     put_message("Read failure");
     size= L;
   }

   // Check for binary file
   char* last= strchr(text, '\0');
   if( last != (text + size) ) {    // If file contains '\0' delimiter
     put_message("Binary file");
     last= text + size;
     mode= M_BIN;
   }

   // Parse the text into lines (Performance critical path)
   char* used= text;
   while( used < last )
   {
     char* from= used;              // Starting character
     line= insert(line, new EdLine(from));

     char* nend= strchr(used, '\n'); // Get next line delimiter
     if( nend == nullptr ) {        // Missing '\n' delimiter
       size_t L= strlen(from);      // String length
       if( (from + L) >= last ) {
         line->delim[0]= line->delim[1]= '\0';
         put_message("Ending '\\n' missing");
         break;
       }

       nend= from + L;              // '\0' delimiter found
       line->delim[0]= 0;
       line->delim[1]= 1;
       while( ++nend < last ) {
         if( *nend ) break;         // If not a '\0' delimiter
         if( ++line->delim[1] == 0 ) { // If repetition count overflow
           line->delim[1]= 255;
           line= insert(line, new EdLine(nend));
           line->delim[0]= 0;
           line->delim[1]= 1;
         }
       }
       used= nend;
       continue;
     }

     // '\r' delimiter UNUSED
     // '\n' delimiter found
     *nend= '\0';                   // Replace with string delimiter
     used= nend + 1;                // Next line origin
     line->delim[0]= '\n';
     if( nend == from || *(nend-1) != '\r' ) { // If UNIX delimiter
       if( mode == M_UNIX || mode == M_MIX || mode == M_BIN ) continue;
       if( mode == M_NONE ) {
         mode= M_UNIX;
       } else {                     // Mode M_DOS ==> M_MIX
         mode= M_MIX;
       }
     } else {                       // IF DOS delimiter
       line->delim[1]= '\r';
       *(nend - 1)= '\0';
       if( mode == M_DOS || mode == M_MIX || mode == M_BIN ) continue;
       if( mode == M_NONE ) {
         mode= M_DOS;
       } else {                     // Mode M_UNIX ==> M_MIX
         mode= M_MIX;
       }
     }
   }

   return line;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::insert
//
// Purpose-
//       Insert file lines (or line)
//
//----------------------------------------------------------------------------
EdLine*                             // (Always tail)
   EdFile::insert(                  // Insert
     EdLine*           after,       // After this line
     EdLine*           head,        // From this line
     EdLine*           tail)        // Upto this line
{
   line_list.insert(after, head, tail);

   for(EdLine* line= head; line != tail; line= line->get_next()) {
     if( line == nullptr ) throw "Invalid insert chain";
     rows++;
   }
   rows++;                          // (Count the tail line)
   row_zero= get_row(top_line);     // Correct row_zero

   return tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::new_line
//
// Purpose-
//       Allocate a new line, also setting the delimiter
//
// Implementation note-
//       DOS files get DOS delimiters. All others get UNIX delimiters.
//
//----------------------------------------------------------------------------
EdLine*                             // The allocated line
   EdFile::new_line(                // Allocate a new line
     const char*       text) const  // Line text
{
   EdLine* line= new EdLine(text);
   line->delim[0]= '\n';            // Default, UNIX delimiter
   if( mode == M_DOS )              // For DOS mode files
     line->delim[1]= '\r';          // Use DOS delimiter

   return line;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::put_message
//       EdFile::rem_messgae
//
// Purpose-
//       Add message to list
//       Remove message from list
//
//----------------------------------------------------------------------------
void
   EdFile::put_message(             // Write message
     const char*       mess_,       // Message text
     int               type_)       // Message mode
{
   if( mess_ == nullptr )           // Ignore if no message
     return;

   EdMess* mess= mess_list.get_head();
   if( mess && type_ < mess->type ) // Ignore less important message
     return;

   std::string S(mess_);            // Message string
   if( type_ == EdMess::T_MESS )    // If action message
     S += ": Click here to continue";
   mess_list.fifo(new EdMess(S, type_));
   if( editor::file == this )       // (Only if this file is active)
     editor::text->draw_info();     // (Otherwise, message is deferred)
}

int                                 // TRUE if message removed or remain
   EdFile::rem_message( void )      // Remove current EdMess
{
   EdMess* mess= mess_list.remq();
   delete mess;
   return bool(mess) || bool(mess_list.get_head());
}

int                                 // TRUE if message removed or remain
   EdFile::rem_message_type(        // Remove current EdMess
     int                type_)      // If at this level or lower
{
   EdMess* mess= mess_list.get_head();
   if( mess && type_ >= mess->type ) {
     mess_list.remq();
     delete mess;
     return true;
   }

   return bool(mess);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::redo
//       EdFile::undo
//
// Purpose-
//       Perform redo action
//       Perform undo action
//
// Visual-
//       OLD: ...<-A<->D->...
//       NEW: ...<-A<->B<->C<->D->...
//       UNDO: hi(A<-B) ti(C->D) hr(0) tr(0)
//
//       OLD: ...<-A<->B<->C<->D->...
//       NEW: ...<-A<->G<->H<->I<->D->...
//       UNDO: hi(A<-G) ti(I->D) hr(A<-B) tr(C->D)
//
//       OLD: ...<-A<->G<->H<->I<->D->...
//       NEW: ...<-A<->B<->C<->D->...
//       UNDO: hi(A<-B) ti(C->D) hr(A<-G) tr(I->D)
//
//       OLD: ...<-A<->G<->H<->I<->D->...
//       NEW: ...<-A<->D->...
//       UNDO: hi(0) ti(0) hr(A<-G) tr(I->D)
//
//----------------------------------------------------------------------------
//============================================================================
// REDO ======================================================================
//============================================================================
void
   EdFile::redo( void )             // Perform redo action
{  if( HCDM || opt_hcdm )
     traceh("EdFile(%p)::redo\n", this);

   if( HCDM ) {
     debugf("\n\n--------------------------------\n");
     debug("redo");
   }

   EdRedo* redo= redo_list.remq();
   if( redo == nullptr ) {
     put_message("Cannot redo");
     return;
   }

   // Perform redo action
   Config::trace(".RDO", redo, this, editor::data->cursor);
   assert_redo(redo, this);         // (Only active when USE_BRINGUP == true)

   EdLine* line= nullptr;           // Activation line
   if( redo->head_remove ) {        // If redo remove
     remove(redo->head_remove, redo->tail_remove);

     line= redo->head_remove->get_prev();
   }
   if( redo->head_insert ) {        // If redo insert
     EdLine* after= redo->head_insert->get_prev();
     insert(after, redo->head_insert, redo->tail_insert);

     line= redo->head_insert->get_prev();
   }

   changed= true;                   // File changed
   editor::mark->handle_redo(this, redo);
   editor::text->activate(line);
   editor::text->draw();
   undo_list.lifo(redo);            // Move REDO to UNDO list
   if( redo->head_insert )          // If lines inserted
     chg_mode(redo->head_insert, redo->tail_insert);

   if( USE_BRINGUP )
     Config::check("redo");
}

//============================================================================
// UNDO ======================================================================
//============================================================================
void
   EdFile::undo( void )             // Perform undo action
{  if( HCDM || opt_hcdm )
     traceh("EdFile(%p)::undo\n", this);

   if( HCDM ) {
     debugf("\n\n--------------------------------\n");
     debug("undo");
   }

   EdRedo* undo= undo_list.remq();
   if( undo == nullptr ) {
     put_message("Cannot undo");
     return;
   }

   // Perform undo action
   Config::trace(".UDO", undo, this, editor::data->cursor);
   assert_undo(undo, this);         // (Only active when USE_BRINGUP == true)

   if( undo_list.get_head() == nullptr ) // If nothing left to undo
     changed= false;                // File reverts to unchanged

   EdLine* line= nullptr;           // Activation line
   if( undo->head_insert ) {        // If undo insert
     line= undo->head_insert->get_prev();
     remove(undo->head_insert, undo->tail_insert);
   }
   if( undo->head_remove ) {        // If undo remove
     line= undo->head_remove->get_prev();
     insert(line, undo->head_remove, undo->tail_remove);
   }

   editor::mark->handle_undo(this, undo);
   editor::text->activate(line);
   editor::text->draw();
   redo_list.lifo(undo);            // Move UNDO to REDO list
   if( undo->head_remove )          // If lines removed
     chg_mode(undo->head_remove, undo->tail_remove);

   if( USE_BRINGUP )
     Config::check("undo");
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::redo_delete        // (Only used by EdFile.cpp)
//
// Purpose-
//       Delete the REDO list
//
//----------------------------------------------------------------------------
void
   EdFile::redo_delete( void )      // Delete the REDO list
{
   // Delete the entire REDO list, also deleting all insert lines
   for(EdRedo* redo= redo_list.remq(); redo; redo= redo_list.remq() ) {
     EdLine* line= redo->head_insert;
     while( line ) {
       EdLine* next= line->get_next();
       delete line;

       if( line == redo->tail_insert )
         break;
       line= next;
     }

     delete redo;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::redo_insert
//
// Purpose-
//       Add REDO to the UNDO list
//
// Implementation notes-
//       DOES NOT update the cursor.
//
//----------------------------------------------------------------------------
void
   EdFile::redo_insert(             // Insert
     EdRedo*           redo)        // This REDO onto the UNDO list
{
   Config::trace(".IDO", redo, this, editor::data->cursor);
   assert_undo(redo, this);         // (Only active when USE_BRINGUP == true)
   redo_delete();                   // Delete the current REDO list

   undo_list.lifo(redo);            // Insert the REDO onto the UNDO list
   changed= true;

   if( USE_BRINGUP )
     Config::check("redo_insert");
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::remove
//
// Purpose-
//       Remove file line(s)
//
//----------------------------------------------------------------------------
void
   EdFile::remove(                  // Remove
     EdLine*           head,        // From this line
     EdLine*           tail)        // Upto this line
{
   line_list.remove(head, tail);

   for(EdLine* line= head; line != tail; line= line->get_next()) {
     if( line == top_line )         // If removing top line
       top_line= head->get_prev()->get_next(); // Insure top_line is valid

     if( line == nullptr ) throw "Invalid remove chain";
     rows--;
   }
   rows--;                          // (Count the tail line)
   row_zero= get_row(top_line);     // Correct row_zero
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::reset
//
// Purpose-
//       Reset the undo/redo lists
//
//----------------------------------------------------------------------------
void
   EdFile::reset( void )            // Reset the undo/redo lists
{
   // Delete the entire REDO list, also deleting all insert lines
   redo_delete();

   // Delete the entire UNDO list, also deleting all remove lines
   for(EdRedo* undo= undo_list.remq(); undo; undo= undo_list.remq() ) {
     EdLine* line= undo->head_remove;
     while( line ) {
       EdLine* next= line->get_next();
       delete line;

       if( line == undo->tail_remove )
         break;
       line= next;
     }

     delete undo;
   }

   changed= false;
   damaged= false;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::set_mode
//
// Purpose-
//       Set the file mode (to M_DOS or M_UNIX)
//
// Implemenation note-
//       Commit not needed since method is only called from EdBifs.cpp.
//
//       REDO/UNDO used for logical consistency when a change occurs.
//
//----------------------------------------------------------------------------
void
   EdFile::set_mode(                // Set the file mode
     int               mode_)       // To this mode
{
   if( mode_ != M_DOS )             // If not DOS mode
     mode_= M_UNIX;                 // (Only M_DOS/M_UNIX allowed)
   if( mode == mode_ )              // If unchanged
     return;                        // Do nothing

   mode= mode_;                     // Update the mode
   if( rows == 0 ) {                // If Empty file
     editor::put_message("Empty file");
     return;
   }
   changed= true;

   // Clear any existing mark
   editor::mark->undo();

   // Create the REDO
   EdRedo* redo= new EdRedo();
   pub::List<EdLine> list;          // The replacement list
   EdLine* const head= line_list.get_head(); // (Multi-use)
   EdLine* const next= head->get_next(); // (Multi-use)
   redo->head_remove= next;
   redo->tail_remove= line_list.get_tail()->get_prev();

   for(EdLine* from= next; ; from= from->get_next()) {
     if( from == nullptr ) {
       // Since this shouldn't happen, memory leak is the least of our worries
       Editor::alertf("%4d EdFile should not occur", __LINE__);
       return;
     }

     list.fifo( new_line(from->text) ); // Replacement line
     if( from == redo->tail_remove )
       break;
   }

   redo->head_insert= list.get_head();
   redo->tail_insert= list.get_tail();

   // Replace the lines
   line_list.remove(redo->head_remove, redo->tail_remove);
   line_list.insert(head, redo->head_insert, redo->tail_insert);

   redo_insert(redo);               // Insert the redo
   activate(head);                  // (The old cursor's gone)
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::write
//
// Purpose-
//       Write the file
//
// Implementation notes-
//       Caller responsible for damaged/protected checking
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   EdFile::write(                   // Write the file
     const char*       name)        // The file name to write
{
   int                 rc= -2;      // Default, open failure

   FILE* F= fopen(name, "wb");      // Open the file
   if( F ) {                        // If open successful
     rc= -1;                        // Default, write failure
     for(EdLine* line= line_list.get_head(); ; line= line->get_next()) {
       if( line == nullptr ) {      // If all lines written
         rc= 0;                     // No error
         break;
       }
       if( (line->flags & EdLine::F_PROT) == 0 ) {
         // Write line data
         if( line->text[0] != '\0' ) {
           int wc= fprintf(F, "%s", line->text);
           if( wc < 0 ) break;      // If write failure
         }

         // Write line delimiter
         if( line->delim[0] == '\n' ) { // If UNIX or DOS format
           if( line->delim[1] != '\0' ) { // If DOS format
             int cc= fputc('\r', F);
             if( cc < 0 ) break;    // If write failure
           }
           int cc= fputc('\n', F);
           if( cc < 0 ) break;      // If write failure

         } else if( line->delim[0] == '\0' ) { // If '\0' delimiter
           size_t L= 1;             // Default, OK
           for(int i= 0; i<line->delim[1]; i++) {
             int cc= fputc('\0', F);
             if( cc < 0 ) break;    // If write failure
           }
           if( L != 1 ) break;      // If write failure

#if 0    // REMOVED: '\r' not implemented in reader
         } else if( line->delim[0] == '\r' ) { // If '\r' delimiter
           int cc= 1;               // Default, OK
           for(int i= 0; i<line->delim[1]; i++) {
             cc= fputc('\r', F);
             if( cc < 0 ) break;    // If write failure
           }
           if( cc < 0 ) break;      // If write failure
#endif   // REMOVED: '\r' not implemented in reader

         } else {                   // If INVALID delimiter (should not occur)
           rc= -3;
           Config::errorf("%4d EdFile INTERNAL ERROR\n", __LINE__);
           Config::errorf("EdLine(%p) text(%p)[%2x,%2x] '%s'\n", line
                  , line->text, line->delim[0], line->delim[1], line->text);
           break;
         }
       }
     }

     if( rc )
       fclose(F);
     else
       rc= fclose(F);
   }

   return rc;
}

int                                 // Return code, 0 OK
   EdFile::write( void )            // Write (replace) the file
{
   const char* const file_name= name.c_str();
   using namespace pub::fileman;    // For pub::fileman::Name
   std::string S= Name::get_path_name(name);
   S += "/";                        // Add directory delimiter
   S += config::AUTOFILE;           // AUTOSAVE file name header
   S += Name::get_file_name(name);  // Append file name

   int rc= write(S.c_str());        // Write AUTOSAVE file
   if( rc == 0 ) {
     struct stat st;                // File stats
     rc= stat(file_name, &st);      // Get file information
     if( rc != 0 )                  // If failure (writing new file)
       st.st_mode= (S_IRUSR | S_IWUSR); // Default, user read/write
     rc= rename(S.c_str(), name.c_str()); // Rename the file
     if( rc == 0 )                  // If renamed
       rc= chmod(file_name, st.st_mode); // Restore the file mode
   }

   if( rc )                         // If write failure
     unlink(S.c_str());             // Remove AUTOSAVE file

   return rc;
}

//============================================================================
//
// Method-
//       EdLine
//
// Purpose-
//       Constructor/Destructor
//
// Implementation notes-
//       Implicit copy operator only used in EdView for a temporary EdLine
//
//----------------------------------------------------------------------------
   EdLine::EdLine(                  // Constructor
     const char*       text)        // Line text
:  ::pub::List<EdLine>::Link(), text(text ? text : "")
{  if( HCDM || (opt_hcdm && opt_verbose > 2) )
     traceh("EdLine(%p)::EdLine\n", this);

   Trace::trace(".NEW", "line", this);
}

   EdLine::~EdLine( void )          // Destructor
{  if( HCDM || (opt_hcdm && opt_verbose > 2) )
     traceh("EdLine(%p)::~EdLine\n", this);

   if( (flags & F_AUTO) == 0 )      // Do not trace temporary lines
     Trace::trace(".DEL", "line", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::debug
//
// Purpose-
//       (Minimal) debugging display
//
//----------------------------------------------------------------------------
void
   EdLine::debug( void ) const      // Minimal debugging display
{
   char buffer[42]; buffer[41]= '\0';
   strncpy(buffer, text, 41);
   debugf("%p F(%.4x) D(%.2x,%.2x) '%s'\n", this, flags
         , delim[0], delim[1], buffer);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::is_within
//
// Purpose-
//       Is this line within range head..tail (inclusive)?
//
//----------------------------------------------------------------------------
bool
   EdLine::is_within(               // Is this line within range head..tail?
     const EdLine*     head,        // First line in range
     const EdLine*     tail) const  // Final line in range
{  if( HCDM || (opt_hcdm && opt_verbose > 2) )
     debugh("EdLine(%p)::is_within(%p,%p)\n", this, head, tail);

   for(const EdLine* line= head; line; line= line->get_next() ) {
     if( line == this )
       return true;
     if( line == tail )
       return false;
   }

   /// We get here because line == nullptr, which should not occur.
   /// The associated list segment is corrupt, and code needs fixing.
   if( head || tail )               // If the range is not empty
     debugf("%4d EdLine(%p).is_within(%p..%p) invalid range\n", __LINE__
           , this, head, tail);
   return false;
}

//============================================================================
//
// Method-
//       EdMess::EdMess
//
// Purpose-
//       Constructor/Destructor
//
//----------------------------------------------------------------------------
   EdMess::EdMess(                  // Constructor
     std::string       mess_,       // Message text
     int               type_)       // Message type
:  ::pub::List<EdMess>::Link(), mess(mess_), type(type_)
{  if( HCDM || opt_hcdm )
     traceh("EdMess(%p)::EdMess(%s,%d)\n", this, mess_.c_str(), type_);
}

   EdMess::~EdMess( void )          // Destructor
{  if( HCDM || opt_hcdm )
     traceh("EdMess(%p)::~EdMess\n", this);
}

//============================================================================
//
// Method-
//       EdHide::EdHide
//
// Purpose-
//       Constructor/Destructor
//
//----------------------------------------------------------------------------
   EdHide::EdHide(                  // Constructor
     EdLine*           head_,       // First hidden line
     EdLine*           tail_)       // Final hidden line
:  EdLine()
{  if( HCDM || opt_hcdm )
     traceh("EdHide(%p)::EdHide\n", this);

   flags= F_HIDE;
   list.insert(nullptr, head_, tail_);
}

   EdHide::~EdHide( void )          // Destructor
{  if( HCDM || opt_hcdm )
     debugh("EdHide(%p)::~EdHide\n", this);

   for(;;) {
     EdLine* line= list.remq();
     if( line == nullptr )
       break;

     delete line;
   }
}

//----------------------------------------------------------------------------
// EdHide::Methods
//----------------------------------------------------------------------------
void
   EdHide::append(                  // Add to end of list
     EdLine*           line)        // Making this the new tail line
{
   list.fifo(line);
   update();
}

void
   EdHide::prepend(                 // Add to beginning of list
     EdLine*           line)        // Making this the new head line
{
   list.lifo(line);
   update();
}

void
   EdHide::remove( void )           // Remove (and delete) this hidden line
{
}

void
   EdHide::update( void )           // Update the count and the message
{
}

//============================================================================
//
// Method-
//       EdRedo::EdRedo
//
// Purpose-
//       Constructor/Destructor
//
//----------------------------------------------------------------------------
   EdRedo::EdRedo( void )           // Constructor
:  ::pub::List<EdRedo>::Link()
{  if( HCDM || opt_hcdm )
     debugh("EdRedo(%p)::EdRedo\n", this);

   Trace::trace(".NEW", "redo", this);
}

   EdRedo::~EdRedo( void )          // Destructor
{  if( HCDM || opt_hcdm )
     debugh("EdRedo(%p)::~EdRedo\n", this);

   Trace::trace(".DEL", "redo", this);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdRedo::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   EdRedo::debug(                   // Debugging display
     const char*       info) const  // Associated info
{  debugf("EdRedo(%p)::debug(%s)\n", this, info ? info : "");

   debugf("  INS [");
   if( head_insert ) debugf("%p<-", head_insert->get_prev());
   debugf("%p,%p", head_insert, tail_insert);
   if( tail_insert ) debugf("->%p", tail_insert->get_next());
   debugf("],\n");

   for(EdLine* line= head_insert; line; line=line->get_next() ) {
     debugf("    "); line->debug();
     if( line == tail_insert )
       break;
   }

   debugf("  COL [%3zd:%3zd]\n", lh_col, rh_col);

   debugf("  REM [");
   if( head_remove ) debugf("%p<-", head_remove->get_prev());
   debugf("%p,%p", head_remove, tail_remove);
   if( tail_remove ) debugf("->%p", tail_remove->get_next());
   debugf("]\n");

   for(EdLine* line= head_remove; line; line=line->get_next() ) {
     debugf("    "); line->debug();
     if( line == tail_remove )
       break;
   }
}
