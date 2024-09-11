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
//       EdFile.cpp
//
// Purpose-
//       Editor: Implement EdFile.h
//
// Last change date-
//       2024/08/30
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf, fopen, fclose, ...
#include <stdlib.h>                 // For various
#include <unistd.h>                 // For unlink
#include <sys/stat.h>               // For stat

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For pub::Name
#include <pub/List.h>               // For pub::List
#include <pub/Signals.h>            // For pub::signals::Signal
#include <pub/Trace.h>              // For pub::Trace

#include "Config.h"                 // For Config::check, namespace config
#include "EdData.h"                 // For EdData
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile - implemented
#include "EdLine.h"                 // For EdLine
#include "EdMark.h"                 // For EdMark
#include "EdMess.h"                 // For EdMess
#include "EdOpts.h"                 // For EdOpts
#include "EdUnit.h"                 // For EdUnit
#include "EdRedo.h"                 // For EdRedo

using namespace config;             // For config::opt_*
using namespace pub::debugging;     // For debugging
using pub::Trace;                   // For pub::Trace

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  VERBOSE= 0                       // Verbosity, higher is more verbose
}; // Compilation controls

#define USE_REDO_DIAGNOSTICS true   // Use redo/undo diagnostics?

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
pub::signals::Signal<EdFile::CloseEvent>
                       EdFile::close_signal; // CloseEvent signal

//----------------------------------------------------------------------------
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
,  name(name_ ? name_ : "unnamed.txt")
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
     insert_file(name_, top);       // Insert the file
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

bool                                // TRUE if file is changed or damaged
   EdFile::is_changed( void ) const // Is file changed or damaged?
{  return changed||chglock||damaged || editor::data->active.get_changed(); }

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::debug
//
// Purpose-
//       Debugging display.
//
// Implementation notes-
//       If the file is active, unit->synch_file commits the active line and
//       updates the file state.
//
//----------------------------------------------------------------------------
static inline const char*           // "true" or "false"
   TF(bool B)
{  if( B ) return "true"; return "false"; }

void
   EdFile::debug(                   // Debugging display
     const char*       info)        // Associated info
{
   traceh("EdFile(%p)::debug(%s) '%s'\n", this
         , info ? info : "", get_name().c_str());

   if( this == editor::file )       // If this is the active file
     editor::unit->synch_file();    // Synchronize current I/O state
   traceh("..mode(%d) changed(%s) chglock(%s) damaged(%s)\n"
         , mode, TF(changed), TF(chglock), TF(damaged));
   traceh("..contains_UTF8(%s) protect(%s)\n", TF(contains_UTF8), TF(protect));
   traceh("..top_line(%p) csr_line(%p)\n", top_line, csr_line);
   traceh("..col_zero(%zd) col(%d) row_zero(%zd) row(%d) rows(%zd)\n"
         , col_zero, col, row_zero, row, rows);

   traceh("..mess_list[%p,%p]:\n", mess_list.get_head(), mess_list.get_tail());
   for(EdMess* mess= mess_list.get_head(); mess; mess= mess->get_next()) {
     traceh("....(%p) %d '%s'\n", mess, mess->type, mess->mess.c_str());
   }

   traceh("..redo_list[%p,%p]:\n", redo_list.get_head(), redo_list.get_tail());
   for(EdRedo* redo= redo_list.get_head(); redo; redo= redo->get_next()) {
     redo->debug("redo");
   }

   traceh("..undo_list[%p,%p]:\n", undo_list.get_head(), undo_list.get_tail());
   for(EdRedo* redo= undo_list.get_head(); redo; redo= redo->get_next()) {
     redo->debug("undo");
   }

   traceh("..line_list[%p,%p]:\n", line_list.get_head(), line_list.get_tail());
   if( strcasecmp(info, "lines") == 0 ) {
     size_t N= 0;
     for(EdLine* line= line_list.get_head(); line; line= line->get_next()) {
       traceh("[%4zd] ", N++);
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
     editor::unit->activate(line);
   } else {                         // If the file is off-screen
     top_line= csr_line= line;
     col_zero= col= row= 0;
     row_zero= get_row(line);
  }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::command
//
// Purpose-
//       Load command output (into empty file)
//
//----------------------------------------------------------------------------
void
   EdFile::command(                 // Load command output
     const char*       input,       // The command name
     const std::string&output)      // The command output
{
   name= input;                     // The file name is the command name
   protect= true;                   // The file is protected

   size_t size= output.size();      // The size of the file
   char* text= allocate(size + 2);  // Allocate space for file (+ "\n\0")
   memcpy(text, output.c_str(), size); // Copy text into allocated buffer
   if( text[size-1] != '\n' )       // If missing trailing '\n'
     text[size++]= '\n';            // Add it in
   text[size]= '\0';                // Add trailing '\0' delimiter

   parse(line_list.get_head(), text, size); // Parse text into lines
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::insert_file
//
// Purpose-
//       Load file data
//
//----------------------------------------------------------------------------
EdLine*                             // The last inserted line
   EdFile::insert_file(             // Insert file
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

   if( st.st_size == 0 ) {          // If empty file
     put_message("Empty file");
     return nullptr;
   }

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

   // Parse the text into lines
   return parse(line, text, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::insert
//
// Purpose-
//       Insert file line(s) without redo/undo
//
//----------------------------------------------------------------------------
EdLine*                             // (Always tail)
   EdFile::insert(                  // Insert file lines without redo/undo
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
     const char*       text) const  // (Immutable) text
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
//       EdFile::parse
//
// Purpose-
//       Parse (allocated) text
//
//----------------------------------------------------------------------------
EdLine*                             // The last inserted line
   EdFile::parse(                   // Parse (allocated) text
     EdLine*           line,        // The line to insert after
     char*             text,        // The (allocated) text
     size_t            size)        // The text length
{
   // Check for binary or unicode character file
   char* last= strchr(text, '\0');
   if( last != (text + size) ) {    // If file contains '\0' delimiter
     put_message("Binary file");
     last= text + size;
     mode= M_BIN;
   } else {                         // Not binary, check for Unicode encodings
     for(size_t i= 0; i<size; ++i) {
       if( text[i] & 0x80 ) {       // If character isn't in ASCII range
         contains_UTF8= true;       // It's either UTF-8 or garbage
         if( !EdOpts::has_unicode_support() ) { // If UTF-8 not supported
           put_message("UTF-8 not supported, file not writable");
           damaged= true;
         }
         break;
       }
     }
   }

   // Parse the text into lines (Performance critical path)
   char* used= text;
   while( used < last ) {
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

   std::string S(mess_);            // Message string
   EdMess* mess= mess_list.get_head();
   if( mess ) {                     // If a message is already present
     if( type_ < mess->type )       // Ignore less important message
       return;
     if( type_ == mess->type && S == mess->mess ) // Ignore duplicate message
       return;
   }

   if( type_ == EdMess::T_MESS )    // If action message
     S += ": Click here to continue";
   mess_list.fifo(new EdMess(S, type_));
   if( editor::file == this )       // (Only if this file is active)
     editor::unit->draw_top();      // (Otherwise, message is deferred)
}

int                                 // TRUE if a message removed or remains
   EdFile::rem_message( void )      // Remove current EdMess
{
   EdMess* mess= mess_list.remq();
   delete mess;
   return bool(mess) || bool(mess_list.get_head());
}

int                                 // TRUE if a message removed or remains
   EdFile::rem_message_type(        // Remove current EdMess
     int                type_)      // If at this level or lower
{
   EdMess* mess= mess_list.get_head();
   if( mess && type_ >= mess->type )
     return rem_message();

   return bool(mess);
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
//       Reset the file state and changed/damaged flags.
//
//----------------------------------------------------------------------------
void
   EdFile::reset( void )            // Reset the undo/redo lists
{
   // Delete the entire REDO list, also deleting all insert lines
   redo_delete();

   // Delete the entire UNDO list, also deleting all remove lines
   undo_delete();

   changed= false;
   chglock= false;
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
   int cc;                          // Completion code
   int rc= -2;                      // Return code: default: open failure

   FILE* F= fopen(name, "wb");      // Open the file
   if( F ) {                        // If open successful
     rc= -2;                        // Default, write failure
     for(EdLine* line= line_list.get_head(); ; line= line->get_next()) {
       if( line == nullptr ) {      // If all lines written
         rc= 0;                     // No error
         break;
       }
       if( (line->flags & EdLine::F_PROT) == 0 ) {
         // Write line data
         if( line->text[0] != '\0' ) {
           cc= fprintf(F, "%s", line->text);
           if( cc < 0 ) break;      // If write failure
         }

         // Write line delimiter
         if( line->delim[0] == '\n' ) { // If UNIX or DOS format
           if( line->delim[1] != '\0' ) { // If DOS format
             cc= fputc('\r', F);
             if( cc < 0 ) break;    // If write failure
           }
           cc= fputc('\n', F);
           if( cc < 0 ) break;      // If write failure
         } else if( line->delim[0] == '\0' ) { // If '\0' delimiter
           cc= -2;
           unsigned L= line->delim[1];
           for(unsigned i= 0; i<L; i++) {
             cc= fputc('\0', F);
             if( cc < 0 ) break;    // If write failure
           }
           if( cc < 0 )             // If write failure or invalid delimiter
             break;
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

//----------------------------------------------------------------------------
//
// Include-
//       EdFile.hpp
//
// Purpose-
//       Implement file redo and undo
//
//----------------------------------------------------------------------------
#include "EdFile.hpp"
