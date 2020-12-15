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
//       EdMark.cpp
//
// Purpose-
//       Editor: Implement EdMark.h
//
// Last change date-
//       2020/12/15
//
//----------------------------------------------------------------------------
#include <pub/Signals.h>            // For pub::signals::Connector

#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile, EdLine, EdRedo
#include "EdMark.h"                 // For EdMark (Implementation class)

using namespace editor::debug;
#define debugf editor::debug::debugf // Prevent ADL

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra brinbup diagnostics?
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
     if( event.file == file ) {
       file= nullptr;
       head= tail= touch_line= nullptr;
       touch_col= 0;
     }
   });
}

   EdMark::~EdMark( void )          // Destructor (Editor shutdown)
{
   for(;;) {
     EdLine* line= mark_list.remq();
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
     const char*       info)        // Associated text
{
   debugf("EdMark::debug(%s)\n", info ? info : "");

   debugf("..touch(%p) %zd [%zd,%zd]\n", touch_line, touch_col
         , lh_column, rh_column);
   debugf("..file(%p) head(%p) tail(%p)\n", file, head, tail);
   size_t row= 0;
   for(EdLine* line= head; line; line= line->get_next() ) {
     debugf("..[%2zd] ", row++); line->debug();
     if( line == tail )
       break;
   }
   debugf("..mark_list(%p,%p) %zd\n"
         , mark_list.get_head() , mark_list.get_tail(), mark_rows);
   row= 0;
   for(EdLine* line= mark_list.get_head(); line; line= line->get_next() ) {
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
   // Remove any current copy/cut
   for(;;) {
     EdLine* line= mark_list.remq();
     if( line == nullptr )
       break;

     delete line;
   }
   mark_rows= 0;

   // Create a new copy list
   if( file == nullptr )
     return "No mark";

   Copy copy= create_copy(head, tail);
   mark_list.insert(nullptr, copy.head, copy.tail);
   mark_rows= copy.rows;
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
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   EdMark::cut( void )              // Cut the marked area
{
   const char* error= copy();       // Copy the marked area
   if( error ) return error;        // Only "No mark" possible

   // Perform the cut
   file->line_list.remove(head, tail);
   file->rows -= mark_rows;
   file->activate(head->get_prev());

   // Create redo
   EdRedo* redo= new EdRedo();
   redo->head_remove= head;
   redo->tail_remove= tail;
   file->insert_undo(redo);

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
   if( edRedo->head_remove ) {      // If redo remove
     Mark mark= get_mark_state(edRedo->head_remove, edRedo->tail_remove);
     if( mark.mark_all ) {
       if( edRedo->head_remove == head && edRedo->tail_remove == tail ) {
         file= nullptr;
         head= tail= touch_line= nullptr;
       } else {
         clr_mark(edRedo->head_remove, edRedo->tail_remove);
         if( edRedo->head_remove == head ) {
           head= edRedo->tail_remove->get_next();
           if( touch_line != tail )
             touch_line= head;
         } else if( edRedo->tail_remove == tail ) {
           tail= edRedo->head_remove->get_prev();
           if( touch_line != head )
             touch_line= tail;
         }
       }
     } else if( mark.mark_any ) {
       clr_mark(edRedo->head_remove, edRedo->tail_remove);
       if( edRedo->head_remove->get_prev()->flags & EdLine::F_MARK ) {
         tail= edRedo->head_remove->get_prev();
         if( touch_line != head )
           touch_line= tail;
       } else if( edRedo->tail_remove->get_next()->flags & EdLine::F_MARK ) {
         head= edRedo->tail_remove->get_next();
         if( touch_line != tail )
           touch_line= head;
       } else {                     // If mark entirely within remove
         file= nullptr;
         head= tail= touch_line= nullptr;
       }
     }
   }
   if( edRedo->head_insert ) {      // If redo insert
     Mark mark= get_mark_state(edRedo->head_insert, edRedo->tail_insert);
     if( mark.mark_all /* || file == nullptr */ ) {
       redo(edFile, edRedo->head_insert, edRedo->tail_insert);
     } else if( mark.mark_any ) {   // Insert expected to be all or none
       errorf("%4d EdMark Unexpected\n", __LINE__); // TODO: REMOVE
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
   if( edUndo->head_insert ) {      // If undo insert
     Mark mark= get_mark_state(edUndo->head_insert, edUndo->tail_insert);
     if( mark.mark_all ) {
       if( edUndo->head_insert == head && edUndo->tail_insert == tail ) {
         file= nullptr;
         head= tail= touch_line= nullptr;
       } else {
         clr_mark(edUndo->head_insert, edUndo->tail_insert);
         if( edUndo->head_insert == head ) {
           head= edUndo->tail_insert->get_next();
           if( touch_line != tail )
             touch_line= head;
         } else if( edUndo->tail_insert == tail ) {
           tail= edUndo->head_insert->get_prev();
           if( touch_line != head )
             touch_line= tail;
         }
       }
     } else if( mark.mark_any ) {
       clr_mark(edUndo->head_insert, edUndo->tail_insert);
       if( edUndo->head_insert->get_prev()->flags & EdLine::F_MARK ) {
         tail= edUndo->head_insert->get_prev();
         if( touch_line != head )
           touch_line= tail;
       } else if( edUndo->tail_insert->get_next()->flags & EdLine::F_MARK ) {
         head= edUndo->tail_insert->get_next();
         if( touch_line != tail )
           touch_line= head;
       } else {                     // If mark entirely within insert
         file= nullptr;
         head= tail= touch_line= nullptr;
       }
     }
   }
   if( edUndo->head_remove ) {      // If undo remove
     Mark mark= get_mark_state(edUndo->head_remove, edUndo->tail_remove);
     if( mark.mark_all /* || file == nullptr */ ) {
       redo(edFile, edUndo->head_remove, edUndo->tail_remove);
     } else if( mark.mark_any ) {   // Insert expected to be all or none
       errorf("%4d EdMark Unexpected\n", __LINE__); // TODO: REMOVE
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
     ssize_t           _column)     // Start at this column (block copy)
{
   if( _column >= 0 )               // Only line marks are supported
     return "Not coded yet";

   if( edLine->flags & EdLine::F_PROT )
     return "Protected";

   if( file == nullptr ) {          // If no mark active
     file= edFile;
     head= tail= touch_line= edLine;
     edLine->flags |= EdLine::F_MARK;
     return nullptr;
   }

   if( file != edFile )
     return "Mark offscreen";

   if( edLine->flags & EdLine::F_MARK ) { // If mark contraction
     if( touch_line == head ) {     // Contract downward
       tail= edLine;
       EdLine* line= edLine->get_next();
       while( line && line->flags & EdLine::F_MARK ) {
         line->flags &= decltype(line->flags)(~EdLine::F_MARK);
         line= line->get_next();
       }
     } else {                       // Contract upward
       head= edLine;
       EdLine* line= edLine->get_prev();
       while( line && line->flags & EdLine::F_MARK) {
         line->flags &= decltype(line->flags)(~EdLine::F_MARK);
         line= line->get_prev();
       }
     }

     touch_line= edLine;
     return nullptr;
   }

   // Expand the mark (Consistency check: do not mark protected lines)
   EdLine* line= edLine;
   while( line && line != head ) {  // Locate downward
     if( line->flags & EdLine::F_PROT )
       line= nullptr;
     else
       line= line->get_next();
   }
   if( line ) {
     line= edLine;
     while( line != head ) {
       line->flags |= EdLine::F_MARK;
       line= line->get_next();
     }
     head= edLine;
   } else {                         // Not found downward, must be upward
     line= edLine;
     while( line != tail ) {
       if( line == nullptr || line->flags & EdLine::F_PROT ) {
         file->damaged= true;       // SHOULD NOT OCCUR
         return "EdMark internal error"; // ((Add repair code if occurs))
       }
       line->flags |= EdLine::F_MARK;
       line= line->get_prev();
     }
     tail= edLine;
   }

   touch_line= edLine;
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
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   EdMark::paste(                   // Paste the marked area
     EdFile*           edFile,      // Into this EdFile
     EdLine*           edLine,      // After this line
     ssize_t           column)      // Start at this column (block copy)
{
   if( mark_list.get_head() == nullptr )
     return "No copy/cut";

   if( column >= 0 )
     return "Not coded yet";

   undo();                          // Undo current mark

   // Copy and mark the pasted line's
   Copy copy= create_copy(mark_list.get_head(), mark_list.get_tail());
   unsigned char delim[2]= {'\n', 0}; // Default UNIX mode
   if( edFile->mode == EdFile::M_DOS )
     delim[1]= '\r';
   for(EdLine* line= copy.head; line; line= line->get_next() ) {
     line->delim[0]= delim[0];
     line->delim[1]= delim[1];
     line->flags |= EdLine::F_MARK;
   }

   // Insert the lines
   edFile->line_list.insert(edLine, copy.head, copy.tail);
   edFile->rows += copy.rows;
   edFile->activate(edLine);

   // Create REDO
   EdRedo* redo= new EdRedo();
   redo->head_insert= copy.head;
   redo->tail_insert= copy.tail;
   edFile->insert_undo(redo);

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
     EdFile*           _file,       // For this EdFile
     EdLine*           _head,       // From this EdLine
     EdLine*           _tail)       // *To* this EdLine
{
   undo();                          // Undo the current mark

   set_mark(_head, _tail);          // Redo the specified mark
   file= _file;
   head= touch_line= _head;
   tail= _tail;
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
   if( file ) {                     // If marked
     clr_mark(head, tail);          // Unmark the lines
     file= nullptr;
     head= tail= touch_line= nullptr;
   }
}
