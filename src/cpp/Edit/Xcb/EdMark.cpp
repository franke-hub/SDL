//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdMark.cpp
//
// Purpose-
//       Editor: Implement EdMark.h
//
// Last change date-
//       2021/04/10
//
//----------------------------------------------------------------------------
#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Signals.h>            // For pub::signals::Connector
#include <pub/Trace.h>              // For pub::Trace

#include "Config.h"                 // For namespace config
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile, EdLine, EdRedo
#include "EdMark.h"                 // For EdMark (Implementation class)
#include "EdText.h"                 // For EdText
#include "EdView.h"                 // For EdView

using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra bringup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static pub::signals::Connector<EdFile::CloseEvent>
                       closeEvent_connector;

//----------------------------------------------------------------------------
//
// Subroutine-
//       clr_mark
//
// Purpose-
//       Clear mark state in line sequence
//
//----------------------------------------------------------------------------
static void
   clr_mark(                        // Clear mark in line sequence
     EdLine*           head,        // First line to unmark
     EdLine*           tail)        // Last  line to unmark
{
   for(EdLine* line= head; line; line= line->get_next() ) {
     line->flags &= decltype(line->flags)(~EdLine::F_MARK);
     if( line == tail )
       break;
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       create_copy
//
// Purpose-
//       Copy EdLine sequence
//
//----------------------------------------------------------------------------
struct Copy {                       // Resultant copy
EdLine*                head= nullptr; // First line copy
EdLine*                tail= nullptr; // Last  line copy
size_t                 rows= 0;     // Number of rows copied
};

static Copy                         // The resultant Copy
   create_copy(                     // Copy line sequence
     EdLine*           head,        // First line to copy
     EdLine*           tail)        // Last  line to copy
{
   Copy copy;                       // Resultant
   pub::List<EdLine> list;

   for(EdLine* line= head; line; line= line->get_next() ) {
     EdLine* edLine= new EdLine(line->text);
     list.fifo(edLine);             // (For list structure)
     copy.rows++;
     if( line == tail )
       break;
   }
   copy.head= list.get_head();
   copy.tail= list.get_tail();

   return copy;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_mark
//
// Purpose-
//       Get MARK_TYPE for line sequence
//
//----------------------------------------------------------------------------
enum MARK_TYPE                      // The mark type
{  MT_NONE= 0                       // No lines marked
,  MT_PREV= 1                       // If head->get_prev() line marked
,  MT_HEAD= 2                       // HEAD line marked
,  MT_TAIL= 4                       // TAIL line marked
,  MT_NEXT= 8                       // If tail->get_next() line marked
,  MT_INNR= (MT_HEAD | MT_TAIL)     // If mark == {head .. tail} (Inner)
,  MT_OUTR= (MT_PREV | MT_NEXT)     // If mark == (PREV + NEXT)  (Outer)
,  MT_FULL= (MT_INNR | MT_OUTR)     // If fully marked
}; // enum MARK_TYPE

static int                          // The MARK_TYPE
   get_mark(                        // Get MARK_TYPE for line sequence
     EdLine*           head,        // First line in sequence
     EdLine*           tail)        // Last  line in sequence
{
   int type= MT_NONE;

   if( head->get_prev()->flags & EdLine::F_MARK )
     type |= MT_PREV;
   if( head->flags & EdLine::F_MARK )
     type |= MT_HEAD;
   if( tail->flags & EdLine::F_MARK )
     type |= MT_TAIL;
   if( tail->get_next()->flags & EdLine::F_MARK )
     type |= MT_NEXT;

   return type;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       set_mark
//
// Purpose-
//       Set mark state in line sequence
//
//----------------------------------------------------------------------------
static void
   set_mark(                        // Set mark in line sequence
     EdLine*           head,        // First line to mark
     EdLine*           tail)        // Last  line to mark
{
   for(EdLine* line= head; line; line= line->get_next() ) {
     line->flags |= EdLine::F_MARK;
     if( line == tail )
       break;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::EdMark
//
// Purpose-
//       Constructor/Destructor/Initializer
//
//----------------------------------------------------------------------------
   EdMark::EdMark( void )           // Constructor
{
   // Initialize EdFile::CloseEvent handler
   using Event= EdFile::CloseEvent;
   closeEvent_connector= EdFile::close_signal.connect([this](Event& event) {
     if( event.file == mark_file ) {
       mark_file= nullptr;
       mark_head= mark_tail= mark_line= nullptr;
       mark_lh= mark_rh= mark_col= -1;
     }
     if( event.file == copy_file ) {
       copy_file= nullptr;
     }
   });
}

   EdMark::~EdMark( void )          // Destructor (Editor shutdown)
{  reset(); }

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   EdMark::debug(                   // Debugging display
     const char*       info) const  // Associated info
{
   debugf("EdMark::debug(%s)\n", info ? info : "");

   debugf("..mark_file.name(%s)\n", mark_file ? mark_file->name.c_str() : "");
   debugf("..mark_file(%p) [%p,%p,%p] [%zd,%zd,%zd]\n", mark_file
         , mark_head, mark_line, mark_tail
         , mark_lh, mark_col, mark_rh);
   size_t row= 0;
   for(EdLine* line= mark_head; line; line= line->get_next() ) {
     debugf("..[%2zd] ", row++); line->debug();
     if( line == mark_tail )
       break;
   }
   debugf("..copy_file.name(%s)\n", copy_file ? copy_file->name.c_str() : "");
   debugf("..copy_file(%p) [%p,%p,%zd] [%zd,%zd,%zd]\n", copy_file
         , copy_list.get_head(), copy_list.get_tail(), copy_rows
         , copy_lh, copy_col, copy_rh);
   row= 0;
   for(EdLine* line= copy_list.get_head(); line; line= line->get_next() ) {
     debugf("..[%2zd] ", row++); line->debug(); // (Multi-statement)
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::copy
//
// Purpose-
//       Copy the marked area
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   EdMark::copy( void )             // Copy the marked area
{
   // Verify mark existence
   if( mark_file == nullptr )
     return "No mark";
   if( editor::view != editor::data )
     return "Cursor view";

   // Commit the current line
   editor::data->commit();

   // Create (and trace) the copy
   Config::trace(".MRK", " C^C", mark_head, mark_tail);

   // Remove any current copy/cut
   reset();

   Copy copy= create_copy(mark_head, mark_tail);
   copy_list.insert(nullptr, copy.head, copy.tail);
   copy_file= mark_file;
   copy_rows= copy.rows;
   copy_col= mark_col;
   copy_lh=  mark_lh;
   copy_rh=  mark_rh;

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::cut
//
// Purpose-
//       Cut the marked area
//
// Implementation note-
//       Updates cursor if it's inside the cut.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   EdMark::cut( void )              // Cut the marked area
{
   if( mark_file == nullptr )
     return "No mark";
   if( mark_file->protect )
     return "Read/only mark";
   const char* error= copy();
   if( error ) return error;

   // Trace the cut
   Config::trace(".MRK", " C^X", mark_head, mark_tail);

   // Perform the cut (with REDO)
   EdRedo* redo= new EdRedo();
   if( copy_col >= 0 ) {            // If block cut
     redo->lh_col= copy_rh;         // (Invert for cut)
     redo->rh_col= copy_lh;
     Active::Ccount count= copy_rh - copy_lh + 1;
     Active& A= *editor::active;    // (Working Active line)
     Copy copy= create_copy(mark_head, mark_tail);
     EdLine* repC= nullptr;         // Replacement cursor line
     EdLine* from= mark_head;       // (Working source line)
     for(EdLine* line= copy.head; line; line= line->get_next()) {
       line->delim[0]= '\n';
       if( mark_file->mode == EdFile::M_DOS )
         line->delim[1]= '\r';
       A.reset(line->text);         // (Used to perform block cut)
       A.replace_text(copy_lh, count, "");
       const char* text= A.get_changed();
       if( text )
         line->text= editor::allocate(text);
       if( from == editor::data->cursor) // If modifying the cursor line
         repC= line;                // (Replace it after insertion)
       if( from == editor::text->head ) // If modifying the head screen line
         editor::text->head= line;  // (Replace it now)
       if( line == copy.tail )
         break;
       from= from->get_next();
     }
     redo->head_insert= copy.head;
     redo->tail_insert= copy.tail;
     mark_file->line_list.remove(mark_head, mark_tail);
     EdLine* after= mark_head->get_prev();
     mark_file->line_list.insert(after, copy.head, copy.tail);
     if( repC)                      // If modifying the cursor line
       mark_file->activate(repC);   // Replace it now
   } else {                         // If line cut
     redo->rh_col= 0;               // (Indicate cut, not undo insert)
     // If the file cursor is inside the line cut, move it outside
     editor::file->csr_line= editor::data->cursor;
     if( mark_file->csr_line->flags & EdLine::F_MARK )
       mark_file->activate(mark_head->get_prev());
     else {
       EdLine* head= editor::text->head; // (The top screen data line)
       for(EdLine* line= mark_head; line; line= line->get_next()) {
         if( line == head ) {
           editor::text->head= mark_head->get_prev();
           break;
         }
         if( line == mark_tail )
           break;
       }
     }

     mark_file->line_list.remove(mark_head, mark_tail);
     mark_file->rows -= copy_rows;
   }

   redo->head_remove= mark_head;
   redo->tail_remove= mark_tail;
   mark_file->redo_insert(redo);
   undo();                          // (No mark remains after cut)

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::format
//
// Purpose-
//       Format the marked area
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   EdMark::format( void )           // Format the marked area
{  return "NOT CODED YET"; }

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::handle_redo
//
// Purpose-
//       Update mark for completed REDO operation
//
//----------------------------------------------------------------------------
void
   EdMark::handle_redo(             // Handle completed REDO operation
     EdFile*           file,        // For this file
     EdRedo*           redo)        // And this REDO
{
   int rem_type= MT_NONE;           // Default, no remove mark
   if( redo->head_remove ) {        // If remove specified
     rem_type= get_mark(redo->head_remove, redo->tail_remove);
     if( redo->lh_col >= 0 || redo->rh_col >= 0 ) // If cut/paste operation
       undo();                      // Undo the current mark
     else if( redo->head_insert == nullptr ) { // If UNDO insert
       if( rem_type & MT_INNR ) {   // If any removed lines were marked
         rem_type &= MT_OUTR;       // Consider previous and next lines
         if( rem_type == MT_PREV )  // If previous but not next marked
           mark_tail= redo->head_remove->get_prev(); // Reset the tail
         else if( rem_type == MT_NEXT ) // If next but not previous marked
           mark_head= redo->tail_remove->get_next(); // Reset the head
         else if( rem_type == MT_NONE ) // If neither next nor previous marked
           undo();                  // Undo the mark
       }
     }
   }

   if( redo->head_insert ) {        // If insert specified
     if( redo->lh_col >= 0 || redo->rh_col >= 0 ) { // If cut/paste operation
       undo();                      // Undo the current mark

       if( redo->lh_col <= redo->rh_col ) { // If paste redo
         mark_file= file;
         mark_head= mark_line= redo->head_insert;
         mark_tail= redo->tail_insert;
         set_mark(redo->head_insert, redo->tail_insert);
         if( redo->lh_col >= 0 ) {
           mark_lh= mark_col= redo->lh_col;
           mark_rh= redo->rh_col;
         }
       }
     } else {                       // Neither cut nor paste
       if( rem_type & MT_INNR )     // If any removed lines were marked
         set_mark(redo->head_insert, redo->tail_insert); // Set insert lines
       else                         // If no removed lines were marked
         clr_mark(redo->head_insert, redo->tail_insert); // Clear insert lines
       int ins_type= get_mark(redo->head_insert, redo->tail_insert);
       if( ins_type == (MT_PREV | MT_INNR) )
         mark_tail= redo->tail_insert;
       else if( ins_type == (MT_INNR | MT_NEXT) )
         mark_head= redo->head_insert;
       else if( ins_type == MT_OUTR )
         set_mark(redo->head_insert, redo->tail_insert);
       else if( ins_type == MT_INNR ) {
         mark_file= file;
         mark_head= mark_line= redo->head_insert;
         mark_tail= redo->tail_insert;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::handle_undo
//
// Purpose-
//       Update mark for completed UNDO operation
//
//----------------------------------------------------------------------------
void
   EdMark::handle_undo(             // Handle completed UNDO operation
     EdFile*           file,        // For this file
     EdRedo*           undo)        // And this UNDO
{
   EdRedo redo= *undo;              // Copy the UNDO

   redo.head_insert= undo->head_remove; // Convert UNDO into REDO
   redo.tail_insert= undo->tail_remove;
   redo.head_remove= undo->head_insert;
   redo.tail_remove= undo->tail_insert;
   redo.lh_col= undo->rh_col;       // (Invert columns)
   redo.rh_col= undo->lh_col;

   handle_redo(file, &redo);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::mark
//
// Purpose-
//       Create/expand/contract the mark
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   EdMark::mark(                    // Create/expand/contract the mark
     EdFile*           edFile,      // For this EdFile
     EdLine*           edLine,      // And this EdLine
     ssize_t           column)      // And this column (block copy)
{
   if( edLine->flags & EdLine::F_PROT )
     return "Protected";
   if( mark_file && mark_file != edFile )
     return "Mark offscreen";
   if( editor::view != editor::data )
     return "Cursor view";

   if( column >= 0 ) {               // If block mark
     if( mark_col < 0 ) {            // If no block mark yet
       mark_col= mark_lh= mark_rh= column;
     } else if( column > mark_col ) {
       mark_lh= mark_col;
       mark_rh= mark_col= column;
     } else if( column < mark_col ) {
       mark_rh= mark_col;
       mark_lh= mark_col= column;
     } else {
       mark_col= mark_lh= mark_rh= column;
     }
   } else {
     mark_lh= mark_rh= mark_col= -1;
   }

   if( mark_file == nullptr ) {     // If no mark active
     mark_file= edFile;
     mark_head= mark_tail= mark_line= edLine;
     edLine->flags |= EdLine::F_MARK;
     return nullptr;
   }

   if( edLine->flags & EdLine::F_MARK ) { // If mark contraction
     if( mark_line == mark_head ) { // Contract downward
       mark_tail= edLine;
       EdLine* line= edLine->get_next();
       while( line && line->flags & EdLine::F_MARK ) {
         line->flags &= decltype(line->flags)(~EdLine::F_MARK);
         line= line->get_next();
       }
     } else {                       // Contract upward
       mark_head= edLine;
       EdLine* line= edLine->get_prev();
       while( line && line->flags & EdLine::F_MARK) {
         line->flags &= decltype(line->flags)(~EdLine::F_MARK);
         line= line->get_prev();
       }
     }

     mark_line= edLine;
     return nullptr;
   }

   // Expand the mark (Consistency check: do not mark protected lines)
   EdLine* line= edLine;
   while( line && line != mark_head ) { // Locate downward
     if( line->flags & EdLine::F_PROT )
       line= nullptr;
     else
       line= line->get_next();
   }
   if( line ) {
     line= edLine;
     while( line != mark_head ) {
       line->flags |= EdLine::F_MARK;
       line= line->get_next();
     }
     mark_head= edLine;
   } else {                         // Not found downward, must be upward
     line= edLine;
     while( line != mark_tail ) {
       if( line == nullptr || line->flags & EdLine::F_PROT ) {
         mark_file->damaged= true;  // SHOULD NOT OCCUR
         return "EdMark internal error"; // ((Repair code if occurs))
       }
       line->flags |= EdLine::F_MARK;
       line= line->get_prev();
     }
     mark_tail= edLine;
   }

   mark_line= edLine;
   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::paste
//
// Purpose-
//       Paste the mark_copy area
//
// Implementation notes-
//       Updates cursor (if copy_list not empty.)
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   EdMark::paste(                   // Paste the marked area
     EdFile*           edFile,      // Into this EdFile
     EdLine*           edLine,      // After(line) or into(block) this line
     ssize_t           column)      // Start at this column (if block copy)
{
   if( copy_list.get_head() == nullptr )
     return "No copy/cut";
   if( editor::view != editor::data )
     return "Cursor view";
   if( editor::file->protect )
     return "Read/only";
   if( edLine->get_next() == nullptr )
     return "Protected";
   if( copy_col >= 0 ) {            // Validate block copy (Must fit in file)
     EdLine* line= edLine;          // The first copy into line
     for(size_t i= 0; i<copy_rows; i++) {
       if( line == nullptr || line->flags & EdLine::F_PROT )
         return "Protected paste";
       line= line->get_next();
     }
   }

   // Trace the paste, create the REDO
   Config::trace(".MRK", " C^V", edFile, edLine);
   EdRedo* redo= new EdRedo();      // Create the REDO

   // Duplicate the copy_list, marking the duplicated lines
   Copy copy= create_copy(copy_list.get_head(), copy_list.get_tail());
   unsigned char delim[2]= {'\n', 0}; // Default UNIX mode
   if( edFile->mode == EdFile::M_DOS )
     delim[1]= '\r';
   for(EdLine* line= copy.head; line; line= line->get_next()) {
     line->delim[0]= delim[0];
     line->delim[1]= delim[1];
     line->flags |= EdLine::F_MARK;
   }

   // Replace the mark with the paste
   undo();                          // Undo any current mark
   mark_file= edFile;
   mark_head= mark_line= copy.head;
   mark_tail= copy.tail;

   if( copy_col >= 0 ) {            // If block copy
     mark_lh= mark_col= column;     // Replace mark columns
     mark_rh= column + (copy_rh - copy_lh);
     redo->lh_col= mark_lh;
     redo->rh_col= mark_rh;

     EdLine* head= edLine;          // The head original line
     EdLine* tail= head;            // The tail original line
     for(size_t i= 1; i<copy_rows; i++)
       tail= tail->get_next();

     edFile->line_list.remove(head, tail);
     redo->head_remove= head;
     redo->tail_remove= tail;

     EdLine* after= edLine->get_prev();
     edFile->line_list.insert(after, copy.head, copy.tail);
     redo->head_insert= copy.head;
     redo->tail_insert= copy.tail;

     edFile->redo_insert(redo);

     // Update the replacement text
     size_t cols= (copy_rh - copy_lh) + 1; // Number of copy columns
     Active& F= *editor::actalt;    // Copy from work area
     Active& I= *editor::active;    // Copy into work area
     EdLine* line= copy.head;       // The current copy from line
     for(;;) {
       F.reset(line->text);
       F.fetch(copy_rh + cols);
       I.reset(head->text);
       I.replace_text(column, 0, F.get_buffer(copy_lh), cols);
       I.truncate();
       line->text= editor::allocate(I.get_buffer());
       if( head == tail )
         break;

       line= line->get_next();
       head= head->get_next();
     }
     edFile->activate(copy.head);   // (The original line was removed)

     return nullptr;
   }

   // Handle paste after no delimiter line
   if( edLine->delim[0] == '\0' &&  edLine->delim[1] == '\0' ) {
     EdLine* line= edFile->new_line(edLine->text); // Get edLine replacement
     line->flags |= EdLine::F_MARK; // (Join the mark)

     pub::List<EdLine> list;        // Connect line to the copy list
     list.fifo(line);
     list.insert(line, copy.head, copy.tail);
     copy.head= line;
     mark_head= mark_line= copy.head;

     // Remove the edLine from the file, updating the REDO
     edFile->remove(edLine);        // (Does not modify edLine->get_prev())
     redo->head_remove= redo->tail_remove= edLine;
     edLine= edLine->get_prev();
   }

   // Insert the lines (with redo)
   edFile->line_list.insert(edLine, copy.head, copy.tail);
   edFile->rows += copy.rows;
   redo->rh_col= 0;                 // (Indicates paste, not insert)
   redo->head_insert= copy.head;
   redo->tail_insert= copy.tail;
   edFile->redo_insert(redo);
   edFile->activate(edLine);

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::reset
//
// Purpose-
//       Reset the mark, removing the copy
//
//----------------------------------------------------------------------------
void
   EdMark::reset( void )            // Reset the mark, removing the copy
{
   for(EdLine* line= copy_list.remq(); line; line= copy_list.remq())
     delete line;

   copy_file= nullptr;
   copy_rows= 0;
   copy_lh= copy_rh= copy_col= -1;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::undo
//
// Purpose-
//       Undo the mark
//
// Implementation note-
//       This may be called when the mark is in an inconsistent state,
//       e.g. after a redo remove but before the redo insert.
//       If the mark_head line is removed, its chain leads to the mark tail.
//       If the mark_tail line is removed (but not mark_head), clr_mark will
//       clear marks on unmarked lines but won't complain.
//
//----------------------------------------------------------------------------
void
   EdMark::undo( void )             // Undo the mark
{
   clr_mark(mark_head, mark_tail);  // Unmark the lines
   mark_file= nullptr;
   mark_head= mark_tail= mark_line= nullptr;
   mark_lh= mark_rh= mark_col= -1;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::verify_copy
//
// Purpose-
//       Verify copy + paste operation parameters
//
// Implementation notes-
//       UNDO eliminates need for block column checks.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   EdMark::verify_copy(             // Verify copy operation parameters
     EdLine*           edLine)      // After(line) or into(block) this line
{
   if( mark_file == nullptr )
     return "No mark";
   if( editor::file->protect )
     return "Read/only";
   if( editor::view != editor::data )
     return "Cursor view";

   if( mark_col < 0 ) {             // Verify line copy
     if( edLine->get_next() == nullptr ) // Disallow copy past end
       return "Protected";
   } else {                         // Verify block copy
     // Verify there is room in target for paste
     EdLine* line= edLine;          // The first copy into line
     for(EdLine* from= mark_head; from; from= from->get_next()) {
       if( line == nullptr || line->flags & EdLine::F_PROT )
         return "Protected paste";
       if( from == mark_tail )
         break;
       line= line->get_next();
     }
   }

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::verify_move
//
// Purpose-
//       Verify cut + paste operation parameters
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   EdMark::verify_move(             // Verify move operation parameters
     EdLine*           edLine)      // After(line) or into(block) this line
{
   const char* error= verify_copy(edLine);
   if( error == nullptr ) {
     if( mark_file->protect )
       return "Read/only mark";

     // If moving columns within a mark and to the right of the mark,
     // subtract the number of columns moved from the current column.
     EdView& data= *editor::data;   // The data view
     if( (data.cursor->flags & EdLine::F_MARK) // If moving into marked line
         && mark_lh >= 0            // If column move
         && data.get_column() > size_t(mark_rh) ) { // If move left past mark
       size_t cols= mark_rh - mark_lh + 1; // Column move count
       if( cols <= data.col )       // If column adjustment sufficient
         data.col -= (decltype(data.col))cols;
       else {
         cols -= data.col;
         data.col= 0;
         data.col_zero -= (decltype(data.col_zero))cols;
       }
     }
   }

   return error;
}
