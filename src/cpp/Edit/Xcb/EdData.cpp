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
//       EdData.cpp
//
// Purpose-
//       Editor: Implement EdData.h
//
// Last change date-
//       2024/05/09
//
//----------------------------------------------------------------------------
#include <string>                   // For std::string
#include <sys/types.h>              // For system types

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Trace.h>              // For pub::Trace

#include "Active.h"                 // For Active
#include "Config.h"                 // For namespace config
#include "Editor.h"                 // For namespace editor
#include "EdFile.h"                 // For EdFile, EdLine
#include "EdMark.h"                 // For EdMark
#include "EdUnit.h"                 // For EdUnit
#include "EdData.h"                 // For EdData (Implementation class)

using namespace config;             // For opt_* variables
using namespace pub::debugging;     // For debugging
using pub::Trace;                   // For pub::Trace

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra bringup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
//
// Method-
//       EdData::EdData
//       EdData::~EdData
//
// Purpose-
//       Construtor.
//       Destrutor.
//
//----------------------------------------------------------------------------
   EdData::EdData( void )           // Constructor
:  EdView()
{  if( opt_hcdm ) traceh("EdData(%p)::EdData\n", this); }

   EdData::~EdData( void )          // Destructor
{  if( opt_hcdm ) traceh("EdData(%p)::~EdData\n", this); }

//----------------------------------------------------------------------------
//
// Method-
//       EdData::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   EdData::debug(                   // Debugging display
     const char*       info) const  // Associated info
{  traceh("EdData(%p)::debug(%s)\n", this, info ? info : "" );

   traceh("..gc_font(%u) gc_flip(%u) gc_mark(%u)\n"
         , gc_font, gc_flip, gc_mark);

   EdView::debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdData::draw_active
//
// Purpose-
//       Redraw the active (data) line
//
//----------------------------------------------------------------------------
void
   EdData::draw_active( void )      // Redraw the active line
{
   EdUnit* unit= editor::unit;
   active.index(col_zero+unit->col_size); // Blank fill
   EdLine line= *cursor;            // (Copy the cursor flags)
   line.flags |= EdLine::F_AUTO;    // (Do not trace ~EdLine)
   line.text= active.get_buffer();
   unit->draw_line(row, &line);
   if( editor::view == this )
     unit->show_cursor();
   unit->flush();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdData::get_gc
//
// Purpose-
//       Get the current graphic context
//
//----------------------------------------------------------------------------
GC_t                                 // The current graphic context
   EdData::get_gc( void )            // Get current graphic context
{
   GC_t gc= gc_font;
   if( cursor->flags & EdLine::F_MARK ) {
     ssize_t column= ssize_t(col_zero + col);
     EdMark& mark= *editor::mark;
     if( mark.mark_col < 0
         || ( column >= mark.mark_lh && column <= mark.mark_rh ) )
       gc= gc_mark;
   }

   return gc;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdData::activate
//
// Purpose-
//       Activate this EdData
//
//----------------------------------------------------------------------------
void
   EdData::activate( void )         // Activate this EdData
{
   editor::view= this;              // (EdHist or EdData)
   editor::unit->draw_top();        // (History or Status line)
}

//----------------------------------------------------------------------------
//
// Method-
//       EdData::commit
//
// Purpose-
//       Commit the Active data line
//
//----------------------------------------------------------------------------
void
   EdData::commit( void )           // Commit the Active line
{  if( opt_hcdm )
     traceh("EdData(%p)::commit buffer(%s)\n", this , active.get_changed());

   const char* buffer= active.get_changed();
   if( buffer                       // If actually changed
       && (cursor->flags & EdLine::F_PROT) == 0 ) { // and not protected
     // Create a new REDO line, duplicating the current line chain
     EdLine* line= new EdLine();
     *line= *cursor;                // (Duplicates links, flags, and delimiters)

     // Replace the text in the REDO line
     line->text= editor::allocate(buffer);
     active.reset(line->text);      // (Prevents duplicate commit)

     // The new line replaces the cursor line in the file_list
     editor::file->remove(cursor, cursor); // (Does not change links)
     editor::file->insert(cursor->get_prev(), line);

     // Create and initialize a REDO object
     EdRedo* redo= new EdRedo();
     redo->head_remove= redo->tail_remove= cursor;
     redo->head_insert= redo->tail_insert= line;
     editor::file->redo_insert(redo); // (Sets file->changed)
     editor::mark->handle_redo(editor::file, redo);

     Trace::trace(".CSR", "Vcmt", cursor, line); // (Old, new)
     cursor= line;                  // Replace the cursor
   } else if( USE_BRINGUP ) {
     Trace::trace(".CSR", "Vnop", cursor, cursor); // (Old, old)
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       EdData::enter_key
//
// Purpose-
//       Handle enter keypress
//
//----------------------------------------------------------------------------
void
   EdData::enter_key( void )        // Handle enter keypress
{  move_cursor_V(+1); }

//----------------------------------------------------------------------------
//
// Method-
//       EdData::move_cursor_V
//
// Purpose-
//       Move cursor vertically
//
//----------------------------------------------------------------------------
void
   EdData::move_cursor_V(           // Move cursor vertically
     int             n)             // The relative row (Down positive)
{
   EdUnit* unit= editor::unit;

   unit->hide_cursor();
   commit();                        // Commit the active line

   int rc= 1;                       // Default, no draw
   if( n > 0 ) {                    // Move down
     while( n-- ) {
       if( unit->row_used > row )
         row++;
       else {
         EdLine* line= (EdLine*)unit->head->get_next();
         if( line ) {
           unit->head= line;
           unit->row_used--;
           row_zero++;
           rc= 0;
         } else {
           break;
         }
         if( unit->tail->get_next() == nullptr )
           row--;
       }
     }
   } else if( n < 0 ) {             // Move up
     while( n++ ) {
       if( row > unit->USER_TOP )
         row--;
       else {
         EdLine* line= (EdLine*)unit->head->get_prev();
         if( line ) {
           unit->head= line;
           row_zero--;
           rc= 0;
         } else {
           break;
         }
       }
     }
   }

   unit->synch_active();
   if( rc == 0 )
     unit->draw();
   else
     unit->draw_status();
}

//----------------------------------------------------------------------------
//
// Method-
//       EdView::EdView
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   EdView::EdView( void )           // Constructor
:  active()
{  if( opt_hcdm ) traceh("EdView(%p)::EdView\n", this); }

//----------------------------------------------------------------------------
//
// Method-
//       EdView::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   EdView::debug(                   // Debugging display
     const char*       info) const  // Associated info
{  if( info ) traceh("EdView(%p)::debug(%s)\n", this, info);

   traceh("..cursor(%p) col_zero(%zd) col(%u) row_zero(%zd) row(%u)\n"
         , cursor, col_zero, col, row_zero, row);

   if( cursor )
     cursor->debug();
   active.debug(info);
}
