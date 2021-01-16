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
//       2021/01/16
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
     if( copy.head == nullptr )
       copy.head= edLine;
     copy.tail= edLine;

     list.fifo(edLine);             // (For list structure)
     copy.rows++;
     if( line == tail )
       break;
   }

   return copy;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       get_mark_state
//
// Purpose-
//       Determine sequence mark state
//
//----------------------------------------------------------------------------
struct Mark {                       // Resultant mark state
int                    mark_all= true;
int                    mark_any= false;
};

static Mark                         // The resultant Mark state
   get_mark_state(                  // Determine mark state
     EdLine*           head,        // First line to check
     EdLine*           tail)        // Last  line to check
{
   Mark mark;                       // resultant

   for(EdLine* line= head; line; line= line->get_next() ) {
     if( line->flags & EdLine::F_MARK )
       mark.mark_any= true;
     else
       mark.mark_all= false;
     if( line == tail )
       break;
   }

   return mark;
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
       mark_col= 0;
     }
     if( event.file == copy_file ) {
       copy_file= nullptr;
     }
   });
}

   EdMark::~EdMark( void )          // Destructor (Editor shutdown)
{
   for(;;) {
     EdLine* line= copy_list.remq();
     if( line == nullptr )
       break;

     delete line;
   }
}

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
   debugf("..mark_file(%p) [%p,%p] %p %zd [%zd,%zd]\n"
         , mark_file, mark_head, mark_tail, mark_line
         , mark_col, mark_lh, mark_rh);
   size_t row= 0;
   for(EdLine* line= mark_head; line; line= line->get_next() ) {
     debugf("..[%2zd] ", row++); line->debug();
     if( line == mark_tail )
       break;
   }
   debugf("..copy.name(%s)\n", copy_file ? copy_file->name.c_str() : "");
   debugf("..copy(%p) [%p,%p] %zd [%zd,%zd]\n", copy_file
         , copy_list.get_head(), copy_list.get_tail()
         , copy_rows, copy_lh, copy_rh);
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
   // Commit the current line
   editor::data->commit();

   // Remove any current copy/cut
   for(;;) {
     EdLine* line= copy_list.remq();
     if( line == nullptr )
       break;

     delete line;
   }
   copy_rows= 0;

   // Create a new copy list
   if( mark_file == nullptr )
     return "No mark";

   // Trace the copy
   Config::trace(".MRK", " C^C", mark_head, mark_tail);

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
//       Updates cursor if it's inside the cut. (TODO: Just check flag?)
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   EdMark::cut( void )              // Cut the marked area
{
   const char* error= copy();       // Copy the marked area
   if( error ) return error;        // Only "No mark" possible

   // Trace the cut
   Config::trace(".MRK", " C^X", mark_head, mark_tail);

   // If the file cursor is inside the cut, reset it
   if( copy_file ) {                // If the copy_file is still active
     editor::file->csr_line= editor::data->cursor; // (Avoids special case)
     for(EdLine* line= mark_head; line; line= line->get_next()) {
       if( line == copy_file->csr_line ) {
         copy_file->activate(mark_head->get_prev());
         break;
       }
       if( line == mark_tail )
         break;
     }
   }

   // Perform the cut
   EdRedo* redo= new EdRedo();      // Create cut REDO
   if( copy_col >= 0 ) {            // If block cut
     if( copy_col == copy_rh ) {
       redo->lh_col= copy_lh;
       redo->rh_col= copy_rh;
     } else {
       redo->rh_col= copy_lh;
       redo->lh_col= copy_rh;
     }
     xcb::Active::Ccount count= copy_rh - copy_lh + 1;
     xcb::Active& A= *config::active; // (Working Active line)
     Copy copy= create_copy(mark_head, mark_tail);
     for(EdLine* line= copy.head; line; line= line->get_next()) {
       A.reset(line->text);         // (Used to perform block cut)
       A.replace_text(copy_lh, count, "");
       const char* text= A.get_changed();
       if( text )
         line->text= editor::allocate(text);
       if( line == copy.tail )
         break;
     }
     redo->head_insert= copy.head;
     redo->tail_insert= copy.tail;
     mark_file->line_list.remove(mark_head, mark_tail);
     EdLine* after= mark_head->get_prev();
     mark_file->line_list.insert(after, copy.head, copy.tail);
   } else {                         // If line cut
     mark_file->line_list.remove(mark_head, mark_tail);
     mark_file->rows -= copy_rows;
   }

   redo->head_remove= mark_head;
   redo->tail_remove= mark_tail;
   mark_file->redo_insert(redo);

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
   EdMark::format( void )           // Cut the marked area
{  return "NOT CODED YET"; }

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::handle_redo
//
// Purpose-
//       Handle completed REDO operation
//
//----------------------------------------------------------------------------
void
   EdMark::handle_redo(             // Handle completed REDO operation
     EdFile*           edFile,      // For this file
     EdRedo*           edRedo)      // And this REDO
{
   Config::trace(".RDO", "mark", (void*)edRedo, edFile); // Trace REDO

   if( edRedo->head_remove ) {      // If redo remove
     Mark mark= get_mark_state(edRedo->head_remove, edRedo->tail_remove);
     if( mark.mark_all ) {
       if( edRedo->head_remove==mark_head && edRedo->tail_remove==mark_tail ) {
         mark_file= nullptr;
         mark_head= mark_tail= mark_line= nullptr;
       } else {
         clr_mark(edRedo->head_remove, edRedo->tail_remove);
         if( edRedo->head_remove == mark_head ) {
           mark_head= edRedo->tail_remove->get_next();
           if( mark_line != mark_tail )
             mark_line= mark_head;
         } else if( edRedo->tail_remove == mark_tail ) {
           mark_tail= edRedo->head_remove->get_prev();
           if( mark_line != mark_head )
             mark_line= mark_tail;
         }
       }
     } else if( mark.mark_any ) {
       clr_mark(edRedo->head_remove, edRedo->tail_remove);
       if( edRedo->head_remove->get_prev()->flags & EdLine::F_MARK ) {
         mark_tail= edRedo->head_remove->get_prev();
         if( mark_line != mark_head )
           mark_line= mark_tail;
       } else if( edRedo->tail_remove->get_next()->flags & EdLine::F_MARK ) {
         mark_head= edRedo->tail_remove->get_next();
         if( mark_line != mark_tail )
           mark_line= mark_head;
       } else {                     // If mark entirely within remove
         mark_file= nullptr;
         mark_head= mark_tail= mark_line= nullptr;
       }
     }
   }
   if( edRedo->head_insert ) {      // If redo insert
     Mark mark= get_mark_state(edRedo->head_insert, edRedo->tail_insert);
     if( mark.mark_all /* || mark_file == nullptr */ ) {
       redo(edFile, edRedo->head_insert, edRedo->tail_insert);
     } else if( mark.mark_any ) {   // Insert expected to be all or none
       Editor::alertf("%4d EdMark Unexpected\n", __LINE__); // TODO: REMOVE
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::handle_undo
//
// Purpose-
//       Handle completed UNDO operation
//
//----------------------------------------------------------------------------
void
   EdMark::handle_undo(             // Handle completed UNDO operation
     EdFile*           edFile,      // For this file
     EdRedo*           edUndo)      // And this UNDO
{
   Config::trace(".UDO", "mark", (void*)edUndo, edFile); // Trace UNDO

   if( edUndo->head_insert ) {      // If undo insert
     Mark mark= get_mark_state(edUndo->head_insert, edUndo->tail_insert);
     if( mark.mark_all ) {
       if( edUndo->head_insert==mark_head && edUndo->tail_insert==mark_tail ) {
         mark_file= nullptr;
         mark_head= mark_tail= mark_line= nullptr;
       } else {
         clr_mark(edUndo->head_insert, edUndo->tail_insert);
         if( edUndo->head_insert == mark_head ) {
           mark_head= edUndo->tail_insert->get_next();
           if( mark_line != mark_tail )
             mark_line= mark_head;
         } else if( edUndo->tail_insert == mark_tail ) {
           mark_tail= edUndo->head_insert->get_prev();
           if( mark_line != mark_head )
             mark_line= mark_tail;
         }
       }
     } else if( mark.mark_any ) {
       clr_mark(edUndo->head_insert, edUndo->tail_insert);
       if( edUndo->head_insert->get_prev()->flags & EdLine::F_MARK ) {
         mark_tail= edUndo->head_insert->get_prev();
         if( mark_line != mark_head )
           mark_line= mark_tail;
       } else if( edUndo->tail_insert->get_next()->flags & EdLine::F_MARK ) {
         mark_head= edUndo->tail_insert->get_next();
         if( mark_line != mark_tail )
           mark_line= mark_head;
       } else {                     // If mark entirely within insert
         mark_file= nullptr;
         mark_head= mark_tail= mark_line= nullptr;
       }
     }
   }
   if( edUndo->head_remove ) {      // If undo remove
     Mark mark= get_mark_state(edUndo->head_remove, edUndo->tail_remove);
     if( mark.mark_all /* || mark_file == nullptr */ ) {
       redo(edFile, edUndo->head_remove, edUndo->tail_remove);
     } else if( mark.mark_any ) {   // Insert expected to be all or none
       Editor::alertf("%4d EdMark Unexpected\n", __LINE__); // TODO: REMOVE
     }
   }
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

   if( mark_file != edFile )
     return "Mark offscreen";

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
   while( line && line != mark_head ) { // Locate downward TODO: marker <<<
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
         return "EdMark internal error"; // ((Add repair code if occurs))
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
//       Paste the marked area
//
// Implementation note-
//       If copy exists, unconditionally updates cursor.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   EdMark::paste(                   // Paste the marked area
     EdFile*           edFile,      // Into this EdFile
     EdLine*           edLine,      // After or into this line
     ssize_t           column)      // Start at this column (if block copy)
{
   if( copy_list.get_head() == nullptr )
     return "No copy/cut";
   if( edLine->get_next() == nullptr )
     return "Protected";
   if( copy_col >= 0 ) {            // Validate block copy (Must fit in file)
     EdLine* line= edLine;          // The first copy into line
     for(size_t i= 0; i<copy_rows; i++) {
       if( line == nullptr || line->flags & EdLine::F_PROT )
         return "Protected";
       line= line->get_next();
     }
   }

   undo();                          // Undo any current mark

   // Copy and mark the copy list
   Copy copy= create_copy(copy_list.get_head(), copy_list.get_tail());
   unsigned char delim[2]= {'\n', 0}; // Default UNIX mode
   if( edFile->mode == EdFile::M_DOS )
     delim[1]= '\r';
   for(EdLine* line= copy.head; line; line= line->get_next() ) {
     line->delim[0]= delim[0];
     line->delim[1]= delim[1];
     line->flags |= EdLine::F_MARK;
   }

   // Trace the paste
   Config::trace(".MRK", " C^V", copy.head, copy.tail);
   EdRedo* redo= new EdRedo();      // Create the REDO

   if( copy_col >= 0 ) {            // If block copy
     EdLine* head= edLine;          // The head copy into line
     EdLine* tail= head;            // The tail copy into line
     for(size_t i= 1; i<copy_rows; i++)
       tail= tail->get_next();

     edFile->line_list.remove(head, tail);
     redo->head_remove= head;
     redo->tail_remove= tail;

     EdLine* after= edLine->get_prev();
     edFile->line_list.insert(after, copy.head, copy.tail);
     redo->head_insert= copy.head;
     redo->tail_insert= copy.tail;

     if( copy_col == copy_rh ) {
       redo->lh_col= copy_lh;
       redo->rh_col= copy_rh;
     } else {
       redo->rh_col= copy_lh;
       redo->lh_col= copy_rh;
     }
     edFile->redo_insert(redo);

     mark_file= edFile;
     mark_head= copy.head;
     mark_tail= copy.tail;
     mark_line= mark_head;
     mark_col= column;
     mark_lh= mark_col;
     mark_rh= mark_col + (copy_rh - copy_lh);

     // Update the inserted text
     size_t cols= (copy_rh - copy_lh) + 1; // Number of copy columns
     xcb::Active  F;                // Copy from work area
     xcb::Active& I= *config::active; // Copy into work area
     EdLine* line= copy.head;       // The current copy from line
     for(;;) {
       F.reset(line->text);
       F.fetch(copy_rh);
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

     // Remove the edLine from the file, updating the REDO
     edFile->remove(edLine);        // (Does not modify edLine->get_prev())
     redo->head_remove= redo->tail_remove= edLine;
     edLine= edLine->get_prev();
   }

   // Insert the lines (with redo)
   edFile->line_list.insert(edLine, copy.head, copy.tail);
   edFile->rows += copy.rows;
   redo->head_insert= copy.head;
   redo->tail_insert= copy.tail;
   edFile->redo_insert(redo);
   edFile->activate(edLine);

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::redo
//
// Purpose-
//       Redo the mark
//
//----------------------------------------------------------------------------
void
   EdMark::redo(                    // Redo the mark
     EdFile*           file,        // For this EdFile
     EdLine*           head,        // From this EdLine
     EdLine*           tail)        // *To* this EdLine
{
   undo();                          // Undo the current mark

   set_mark(head, tail);            // Redo the specified mark
   mark_file= file;
   mark_head= mark_line= head;
   mark_tail= tail;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::undo
//
// Purpose-
//       Undo the mark
//
//----------------------------------------------------------------------------
void
   EdMark::undo( void )             // Undo the mark
{
   if( mark_file ) {                // If marked
     clr_mark(mark_head, mark_tail); // Unmark the lines
     mark_file= nullptr;
     mark_head= mark_tail= mark_line= nullptr;
     mark_lh= mark_rh= mark_col= -1;
   }
}
