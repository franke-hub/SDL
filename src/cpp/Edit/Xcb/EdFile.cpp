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
//       EdFile.cpp
//
// Purpose-
//       Editor: Implement EdFile.h
//
// Last change date-
//       2020/12/14
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
#include <stdio.h>                  // For printf, ...
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp, ...
#include <unistd.h>                 // For close, ftruncate
#include <sys/stat.h>               // For stat
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Signals.h>            // For pub::signals::Signal
#include <pub/List.h>               // For pub::List
#include "Xcb/Types.h"              // For xcb::Line

#include "Editor.h"                 // For editor::debug
#include "EdFile.h"                 // For EdFile, EdLine, EdRedo
#include "EdMark.h"                 // For EdMark
#include "EdText.h"                 // For EdText

using namespace editor::debug;      // For opt_* controls
#define debugf editor::debug::debugf // Prevent ADL
#define debugh editor::debug::debugh // Prevent ADL

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
}; // Compilation controls

#define USE_BRINGUP true            // Use bringup diagnostics?

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
pub::signals::Signal<EdFile::CloseEvent>
                       EdFile::close_signal; // CloseEvent signal

//----------------------------------------------------------------------------
//
// Subroutine-
//       name_of
//
// Purpose-
//       Get filename part of file name
//
//----------------------------------------------------------------------------
static const char*                  // The file name (only)
   name_of(                         // Get file name
     const char*       full)        // File name, possibly with path qualifiers
{
   size_t L= strlen(full);          // The fully qualified name
   while( L > 0 ) {                 // Locate path qualifier
     if( full[L-1] == '/' )
       break;

     L--;
   }

   return full + L;
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
     const char*       _name)       // Fully qualified file name
:  ::pub::List<EdFile>::Link()
,  name(_name)
{
   if( opt_hcdm )
     debugh("EdFile(%p)::EdFile(%s)\n", this, get_name().c_str());

   EdLine* top= new EdLine("* * * * Top of file * * * *");
   EdLine* bot= new EdLine("* * * * End of file * * * *");
   line_list.fifo(top);
   line_list.fifo(bot);
   top->flags= EdLine::F_PROT;
   bot->flags= EdLine::F_PROT;

   top_line= top;
   csr_line= top;

   if( _name )
     append(_name, top);            // Insert the file
}

   EdFile::~EdFile( void )          // Destructor
{
   if( opt_hcdm )
     debugh("EdFile(%p)::~EdFile\n", this);
// debug("Destructor");

   reset();                         // Delete REDO/UNDO lists

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
{  // TODO: VERIFY USAGE
   EdLine* line= line_list.get_head(); // Get top line
   while( row > 0 ) {               // Locate row
     line= (EdLine*)line->get_next();
     if( line == nullptr )          // SHOULD NOT OCCUR
       break;

     row--;
   }

   return line;
}

size_t                              // The row number
   EdFile::get_row(                 // Get row number
     const EdLine*    cursor) const // For this line
{  // TODO: VERIFY USAGE
   size_t row= 0;
   for(EdLine* line= line_list.get_head(); line; line= line->get_next() ) {
     if( line == cursor )
       break;

     row++;
   }

   return row;
}

size_t                              // The row count
   EdFile::get_rows(                // Get row count
     const EdLine*     head,        // From this line
     const EdLine*     tail)        // *To* this line
{
   size_t rows= 0;
   while( head ) {
     rows++;
     if( head == tail )
       break;

     head= head->get_next();
  }

  return rows;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
static inline const char*           // "true" or "false"
   TF(bool B)
{  if( B ) return "true"; return "false"; }

void
   EdFile::debug(                   // Debugging display
     const char*       text) const  // Associated text
{
   debugf("EdFile(%p)::debug(%s) '%s'\n", this
         , text ? text : "", get_name().c_str());

   debugf("..mode(%d) changed(%s) damaged(%s) protect(%s)\n"
         , mode, TF(changed), TF(damaged), TF(protect));
   debugf("..col_zero(%zd) col(%d) row_zero(%zd) row(%d) rows(%zd)\n"
         , col_zero, col, row_zero, row, rows);

   debugf("..mess_list:\n");
   for(EdMess* mess= mess_list.get_head(); mess; mess= mess->get_next()) {
     debugf("....(%p) %d '%s'\n", mess, mess->type, mess->mess.c_str());
   }

   debugf("..redo_list:\n");
   for(EdRedo* redo= redo_list.get_head(); redo; redo= redo->get_next()) {
     redo->debug("redo");
   }

   debugf("..undo_list:\n");
   for(EdRedo* redo= undo_list.get_head(); redo; redo= redo->get_next()) {
     redo->debug("undo");
   }

   if( strcasecmp(text, "lines") == 0 ) {
     debugf("..line_list:\n");
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
   EdText* text= editor::text;
   if( this == text->file ) {       // If the file is active
     text->activate(line);
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

   if( st.st_size == 0 )            // If empty file
     return nullptr;                // Nothing to append

   // Allocate the input data area Pool
   size_t size= st.st_size;         // The size of the file
   char* text= allocate(size + 1);  // Allocate space for entire file (+ '\0')
   memset(text, 0, size + 1);       // (In case read fails)

   // Load the file
   FILE* f= fopen(name, "rb");
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
     if( nend == nullptr ) {        // Missing '\\n' delimiter
       size_t L= strlen(from);      // String length
       if( (from + L) >= last ) {
         put_message("Ending '\\n' missing");
         break;
       }

       nend= from + L;              // '\0' delimiter found
       line->delim[1]= 1;
       while( ++nend < last ) {
         if( *nend ) break;         // If not a '\0' delimiter
         if( ++line->delim[1] == 0 ) { // If repetition count overflow
           line->delim[1]= 255;
           line= insert(line, new EdLine(nend));
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

   return tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::insert_undo
//
// Purpose-
//       Add REDO to the UNDO list
//
//----------------------------------------------------------------------------
void
   EdFile::insert_undo(             // Insert
     EdRedo*           edRedo)      // This REDO onto the UNDO list
{
   for(;;) {                        // Delete all REDO operations
     EdRedo* redo= redo_list.remq();
     if( redo == nullptr )
       break;

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

   editor::mark->handle_redo(this, edRedo);
   undo_list.lifo(edRedo);
   changed= true;
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
     const char*       _mess,       // Message text
     int               _type)       // Message mode
{
   if( _mess == nullptr )           // Ignore if no message
     return;

   EdMess* mess= mess_list.get_head();
   if( mess && _type <= mess->type )
     return;

   mess_list.fifo(new EdMess(_mess, _type));
   if( editor::text->file == this ) // (Only if this file is active)
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
     int                _type)      // If at this level or lower
{
   EdMess* mess= mess_list.get_head();
   if( mess && _type >= mess->type ) {
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
   Editor::failure("REDO/UNDO inconsistent");
}

static void
   assert_line(                     // Assert
     EdLine*           test,        // This line is active in
     EdFile*           file,        // This file for
     EdRedo*           redo)        // This redo
{
   for(EdLine* L= file->line_list.get_head(); L; L= L->get_next() ) {
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
   assert_redo(                     // Assert
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
     if( redo->tail_insert == nullptr || redo->tail_insert == nullptr )
       debug_redo(__LINE__, redo);
     if( redo->head_insert->get_prev() != redo->head_remove->get_prev() )
       debug_redo(__LINE__, redo);
     if( redo->tail_insert->get_next() != redo->tail_remove->get_next() )
       debug_redo(__LINE__, redo);
   } else if( !(redo->head_insert || redo->head_remove) )
     debug_redo(__LINE__, redo);
}
#endif

//============================================================================
// REDO ======================================================================
//============================================================================
void
   EdFile::redo( void )             // Perform redo action
{
// debugf("\n\n--------------------------------\n");
   if( opt_hcdm )
     debugh("EdFile(%p)::redo\n", this);

// debug("redo");

   EdRedo* redo= redo_list.remq();
   if( redo == nullptr ) {
     put_message("Cannot redo");
     return;
   }

#if USE_BRINGUP
   // TODO: REMOVE (BRINGUP CHECKING ** NOT OPTIMIZED **)
   assert_redo(redo);

   if( redo->head_remove ) {        // If redo remove
     assert_line(redo->head_remove->get_prev(), this, redo);
     assert_line(redo->tail_remove->get_next(), this, redo);

     for(EdLine* line= redo->head_remove; ; line= line->get_next() ) {
       if( line == nullptr ) {
         debugf("%4d EdFile nullptr redo(,,%p,%p)\n", __LINE__
               , redo->head_remove, redo->tail_remove);
         debug_redo(__LINE__, redo);
       }
       assert_line(line, this, redo);
       if( line == redo->tail_remove )
         break;
     }
   }
   if( redo->head_insert ) {        // If redo insert
     assert_line(redo->head_insert->get_prev(), this, redo);
     assert_line(redo->tail_insert->get_next(), this, redo);

     for(EdLine* line= redo->head_insert; ; line= line->get_next() ) {
       if( line == nullptr ) {
         debugf("%4d EdFile nullptr redo(%p,%p,,)\n", __LINE__
               , redo->head_insert, redo->tail_insert);
         debug_redo(__LINE__, redo);
       }
       assert_miss(line, this, redo);
       if( line == redo->tail_insert )
         break;
     }
   }
#endif

   // Perform redo action
   changed= true;                   // File changed

   EdLine* line= nullptr;           // Activation line
   if( redo->head_remove ) {        // If redo remove
     redo->head_remove->get_prev()->set_next(redo->tail_remove->get_next());
     redo->tail_remove->get_next()->set_prev(redo->head_remove->get_prev());

     line= redo->head_remove->get_prev();
   }
   if( redo->head_insert ) {        // If redo insert
     redo->head_insert->get_prev()->set_next(redo->head_insert);
     redo->tail_insert->get_next()->set_prev(redo->tail_insert);

     line= redo->head_insert->get_prev();
   }

   size_t old_rows= get_rows(redo->head_remove, redo->tail_remove);
   size_t new_rows= get_rows(redo->head_insert, redo->tail_insert);
   rows += ssize_t(new_rows - old_rows);

   editor::mark->handle_redo(this, redo);
   editor::text->activate(line);
   editor::text->draw();
   undo_list.lifo(redo);            // REDO => UNDO
// debug("redo-done");
}


//============================================================================
// UNDO ======================================================================
//============================================================================
void
   EdFile::undo( void )             // Perform undo action
{
// debugf("\n\n--------------------------------\n");
   if( opt_hcdm )
     debugh("EdFile(%p)::undo\n", this);

// debug("undo");

   EdRedo* undo= undo_list.remq();
   if( undo == nullptr ) {
     put_message("Cannot undo");
     return;
   }

#if USE_BRINGUP
   // TODO: REMOVE (BRINGUP CHECKING ** NOT OPTIMIZED **)
   assert_redo(undo);

   if( undo->head_insert ) {        // If undo insert
     assert_line(undo->head_insert->get_prev(), this, undo);
     assert_line(undo->tail_insert->get_next(), this, undo);

     for(EdLine* line= undo->head_insert; ; line= line->get_next() ) {
       if( line == nullptr ) {
         debugf("%4d EdFile nullptr undo(%p,%p,,)\n", __LINE__
               , undo->head_insert, undo->tail_insert);
         debug_redo(__LINE__, undo);
       }
       assert_line(line, this, undo);
       if( line == undo->tail_insert )
         break;
     }
   }
   if( undo->head_remove ) {        // If undo remove
     assert_line(undo->head_remove->get_prev(), this, undo);
     assert_line(undo->tail_remove->get_next(), this, undo);

     for(EdLine* line= undo->head_remove; ; line= line->get_next() ) {
       if( line == nullptr ) {
         debugf("%4d EdFile nullptr undo(,,%p,%p)\n", __LINE__
               , undo->head_remove, undo->tail_remove);
         return;
       }
       assert_miss(line, this, undo);
       if( line == undo->tail_remove )
         break;
     }
   }
#endif

   // Perform undo action
   if( undo_list.get_head() == nullptr ) // If nothing left to undo
     changed= false;                // File reverts to unchanged

   EdLine* line= nullptr;           // Activation line
   if( undo->head_insert ) {        // If undo insert
     undo->head_insert->get_prev()->set_next(undo->tail_insert->get_next());
     undo->tail_insert->get_next()->set_prev(undo->head_insert->get_prev());

     line= undo->head_insert->get_prev();
   }
   if( undo->head_remove ) {        // If undo remove
     undo->head_remove->get_prev()->set_next(undo->head_remove);
     undo->tail_remove->get_next()->set_prev(undo->tail_remove);

     line= undo->head_remove->get_prev();
   }

   size_t old_rows= get_rows(undo->head_insert, undo->tail_insert);
   size_t new_rows= get_rows(undo->head_remove, undo->tail_remove);
   rows += ssize_t(new_rows - old_rows);

   editor::mark->handle_undo(this, undo);
   editor::text->activate(line);
   editor::text->draw();
   redo_list.lifo(undo);            // UNDO => REDO
// debug("undo-done");
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::remove
//
// Purpose-
//       Remove file lines (or line)
//
//----------------------------------------------------------------------------
void
   EdFile::remove(                  // Remove
     EdLine*           head,        // From this line
     EdLine*           tail)        // Upto this line
{
   line_list.remove(head, tail);

   for(EdLine* line= head; line != tail; line= line->get_next()) {
     if( line == nullptr ) throw "Invalid remove chain";
     rows--;
   }
   rows--;                          // (Count the tail line)
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::reset
//
// Purpose-
//       Reset the undo/redo lists
//
// Implementation notes-
//       This action cannot be undone.
//
//----------------------------------------------------------------------------
void
   EdFile::reset( void )            // Reset the undo/redo lists
{
   for(;;) {                        // Delete all REDO operations
     EdRedo* redo= redo_list.remq();
     if( redo == nullptr )
       break;

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

   for(;;) {                        // Delete all UNDO operations
     EdRedo* undo= undo_list.remq();
     if( undo == nullptr )
       break;

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

   FILE* f= fopen(name, "wb");      // Open the file
   if( f ) {                        // If open succeeded
     for(EdLine* line= line_list.get_head(); line; line= line->get_next()) {
       if( (line->flags & EdLine::F_PROT) == 0 ) {
         rc= fprintf(f, "%s", line->text);
         if( rc >= 0 ) {
           if( line->delim[0] == '\n' ) { // If UNIX/DOS format
             if( line->delim[1] != '\0' ) { // If DOS format
               rc= fputc('\r', f);
               if( rc >= 0 )
                 rc= fputc('\n', f);
             } else                // If UNIX format
               rc= fputc('\n', f);
           } else if( line->delim[0] == '\0' ) { // If '\0' delimiter
             for(int i= 0; i<line->delim[1]; i++) {
               rc= fputc('\0', f);
               if( rc < 0 ) break;
             }
           } else if( line->delim[0] == '\r' ) { // If '\r' delimiter
             // TODO: REMOVE: NOT IMPLEMENTED IN READER
             for(int i= 0; i<line->delim[1]; i++) {
               rc= fputc('\r', f);
               if( rc < 0 ) break;
             }
           } else {                   // If INVALID format (should not occur)
             fprintf(stderr, "%4d EdFile INTERNAL ERROR\n", __LINE__);
             fprintf(stderr, "EdLine(%p) text(%p)[%2x,%2x] '%s'\n", line
                    , line->text, line->delim[0], line->delim[1], line->text);
             rc= -3;
           }
         }
         if( rc < 0 )
           break;
       }
     }

     if( rc < 0 )
       fclose(f);
     else
       rc= fclose(f);
   }

   return rc;
}

int                                 // Return code, 0 OK
   EdFile::write( void )            // Write (replace) the file
{
   std::string S= editor::autosave_dir; // The AUTOSAVE directory
   S += "/";                        // Add directory delimiter
   S += editor::AUTOSAVE;           // AUTOSAVE file name header
   S += name_of(name.c_str());      // Append file name

   int rc= write(S.c_str());        // Write AUTOSAVE file
   if( rc == 0 )
     rc= rename(S.c_str(), name.c_str()); // Rename the file

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
//----------------------------------------------------------------------------
   EdLine::EdLine(                  // Constructor
     const char*       text)        // Line text
:  ::xcb::Line(text)
{
   if( opt_hcdm && opt_verbose > 2 )
     debugh("EdLine(%p)::EdLine\n", this);
}

   EdLine::~EdLine( void )          // Destructor
{
   if( opt_hcdm && opt_verbose > 2 )
     debugh("EdLine(%p)::~EdLine\n", this);
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
   char buffer[16]; buffer[15]= '\0';
   strncpy(buffer, text, 15);
   debugf("%p F(%.4x) D(%.2x,%.2x) '%s'\n", this, flags
         , delim[0], delim[1], buffer);
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
     std::string       _mess,       // Message text
     int               _type)       // Message type
:  ::pub::List<EdMess>::Link(), mess(_mess), type(_type)
{
   if( opt_hcdm )
     debugh("EdMess(%p)::EdMess(%s,%d)\n", this, _mess.c_str(), _type);
}

   EdMess::~EdMess( void )          // Destructor
{
   if( opt_hcdm )
     debugh("EdMess(%p)::~EdMess\n", this);
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
     EdLine*           _head,       // First hidden line
     EdLine*           _tail)       // Final hidden line
:  EdLine()
{
   if( opt_hcdm )
     debugh("EdHide(%p)::EdHide\n", this);

   flags= F_HIDE;
   if( _head ) {
     append(_head);
     if( _tail )
       append(_tail);
   }
}

   EdHide::~EdHide( void )          // Destructor
{
   if( opt_hcdm )
     debugh("EdHide(%p)::~EdHide\n", this);

   EdLine* line= head;
   if( line ) {                     // If files remain
     // You should only get here from the EdFile destructor. Let's make sure.
     if( get_prev() ) {             // If the prior line wasn't removed
       fprintf(stderr, "~EdHide invalid state"); // (Not called from ~EdFile)
       return;
     }

     while( true ) {
       if( line == nullptr ) {
         fprintf(stderr, "~EdHide invalid chain"); // (Too late to debug now)
         return;
       }
       EdLine* next= line->get_next();
       delete line;
       if( line == tail ) break;
       line= next;
     }
   }
}

//----------------------------------------------------------------------------
// EdHide::Methods
//----------------------------------------------------------------------------
void
   EdHide::append(                  // Add to end of list
     EdLine*           line)        // Making this the new tail line
{
   if( tail )
     get_next()->set_prev(tail);
   else {
     line->get_prev()->set_next(this);
     set_prev(line->get_prev());
     head= line;
   }
   line->get_next()->set_prev(this);
   tail= line;

   update();
}

void
   EdHide::prepend(                 // Add to beginning of list
     EdLine*           line)        // Making this the new head line
{
   if( head ) {
     get_prev()->set_next(head);
   } else {
     line->get_next()->set_prev(this);
     set_next(line->get_next());
     tail= line;
   }
   line->get_prev()->set_next(this);
   head= line;

   update();
}

void
   EdHide::remove( void )           // Remove (and delete) this hidden line
{
   if( head )                       // If not inserted
     return;                        // Nothing to do

   get_prev()->set_next(head);
   get_next()->set_prev(tail);
   head= nullptr;
   tail= nullptr;

   delete this;
}

void
   EdHide::update( void )           // Update the count and the message
{
   count= 0;
   if( head ) {
     EdLine* line= head;
     count= 1;
     while( line != tail ) {
       count++;
       if( line == nullptr ) throw "Invalid EdHide chain";
       line= line->get_next();
     }
   }

   char buffer[128];                // Message work area
   memset(buffer, '-', sizeof(buffer));
   buffer[sizeof(buffer) - 1]= '\0';
   int L= sprintf(buffer, ">--- %zd lines hidden", count);
   buffer[L]= ' ';
   info= buffer;
   text= info.c_str();
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
{
   if( opt_hcdm )
     debugh("EdRedo(%p)::EdRedo\n", this);
}

   EdRedo::~EdRedo( void )          // Destructor
{
   if( opt_hcdm )
     debugh("EdRedo(%p)::~EdRedo\n", this);
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
     const char*       text) const  // Associated text
{
   debugf("EdRedo(%p)::debug(%s)\n", this, text ? text : "");

   debugf("  [");
   if( head_insert ) debugf("%p<-", head_insert->get_prev());
   debugf("%p,%p", head_insert, tail_insert);
   if( tail_insert ) debugf("->%p", tail_insert->get_next());
   debugf("],\n");

   for(EdLine* line= head_insert; line; line=line->get_next() ) {
     debugf("    "); line->debug();
     if( line == tail_insert )
       break;
   }

   debugf("  [");
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
