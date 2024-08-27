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
// Include-
//       EdRedo.hpp
//
// Purpose-
//       EdFile: Implement EdLine.h, EdMess.h, EdRedo.h
//
// Last change date-
//       2024/08/27
//
// Implementation notes-
//       (Only) included by EdFile.cpp
//
//----------------------------------------------------------------------------

#if USE_REDO_DIAGNOSTICS
//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  USE_OBJECT_COUNT= true           // Use object counting?
}; // Compilation controls

//----------------------------------------------------------------------------
//
// Subroutine-
//       checkstop
//       invalid_list
//       invalid_redo
//
// Purpose-
//       config_check (lambda) error handlers
//
//----------------------------------------------------------------------------
static bool                         // *ALWAYS* true
   checkstop(                       // Consistency check failure
     std::string       message)     // Error message
{
   editor::put_message(message.c_str(), EdMess::T_MESS);
   traceh("%s checkstop(%s)\n", __FILE__, message.c_str());

   if( pub::Trace::table )          // Turn off tracing
     pub::Trace::table->flag[pub::Trace::X_HALT]= true;

   return true;
}

static bool                         // TRUE if invalid head/tail pair
   invalid_list(                    // Check list consistency
     const EdRedo*     redo,        // For this redo item
     const EdLine*     head,        // And this head/tail pair
     const EdLine*     tail)
{
   if( bool(head) != bool(tail) ) {
     traceh("%4d %s redo(%p) head(%p) tail(%p)\n", __LINE__, __FILE__
           , redo, head, tail);
     return checkstop("invalid_list");
   } else if( head == nullptr ) return false;

   for(const EdLine* line= head; line; line= line->get_next() ) {
     if( line == tail ) return false;
   }

   traceh("%4d Ed::check redo(%p) head(%p) tail(%p)\n", __LINE__, redo, head, tail);
   return checkstop("missing tail");
}

static bool                         // TRUE if invalid redo
   invalid_redo(                    // Check redo consistency
     const EdRedo*     redo)        // For this redo
{
   return invalid_list(redo, redo->head_insert, redo->tail_insert);
}

static bool                         // TRUE if invalid undo
   invalid_undo(                    // Check undo consistency
     const EdRedo*     undo)        // For this undo
{
   return invalid_list(undo, undo->head_remove, undo->tail_remove);
}

//----------------------------------------------------------------------------
//
// (Lambda) subroutine-
//       config_check (config::check_signal listener)
//
// Purpose-
//       Redo/Undo list consistency check
//
//----------------------------------------------------------------------------
static pub::signals::Connector<const char*> config_check=
   config::check_signal()->connect([](const char*& info)
{
   // Verify undo/redo lists
   EdRedo* undo= editor::file->undo_list.get_tail();
   while( undo ) {
     if( invalid_undo(undo) ) {
       Config::debug(info);
       return;
     }
     undo= undo->get_prev();
   }

   EdRedo* redo= editor::file->redo_list.get_tail();
   while( redo ) {
     if( invalid_redo(redo) ) {
       Config::debug(info);
       return;
     }
     redo= redo->get_next();
   }
}); // config_check (pub::signals::Connector)
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
   editor::unit->draw_top();        // And redraw the top lines
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       trace_redo
//
// Purpose-
//       Trace utilities
//
//----------------------------------------------------------------------------
static void
   trace_redo(                      // Trace redo/undo operation
     const char*       ident,       // Trace identifier
     EdRedo*           redo,        // The REDO/UNDO
     EdFile*           file,        // The REDO/UNDO file
     EdLine*           line)        // The REDO/UNDO cursor line
{
   typedef pub::Trace::Record Record;
   Record* record= Trace::trace(sizeof(Record) + 32);
   if( record ) {
     memset(record, 0, sizeof(Record) + 32);
     struct unit {
       uint16_t lh_col;
       uint16_t rh_col;
     }* U= (unit*)(&record->unit);
     U->lh_col= htons((uint16_t)redo->lh_col);
     U->rh_col= htons((uint16_t)redo->rh_col);

     uintptr_t V0= uintptr_t(file);
     uintptr_t V1= uintptr_t(line);
     uintptr_t R0= uintptr_t(redo->head_insert);
     uintptr_t R1= uintptr_t(redo->tail_insert);
     uintptr_t R2= uintptr_t(redo->head_remove);
     uintptr_t R3= uintptr_t(redo->tail_remove);

#pragma GCC diagnostic push         // GCC regression START
#pragma GCC diagnostic ignored "-Wstringop-overflow"
     for(unsigned i= 8; i>0; i--) {
       record->value[ 0 + i - 1]= char(V0);
       record->value[ 8 + i - 1]= char(V1);
       ((char*)(record->value))[16 + i - 1]= char(R0);
       ((char*)(record->value))[24 + i - 1]= char(R1);
       ((char*)(record->value))[32 + i - 1]= char(R2);
       ((char*)(record->value))[40 + i - 1]= char(R3);

       V0 >>= 8;
       V1 >>= 8;
       R0 >>= 8;
       R1 >>= 8;
       R2 >>= 8;
       R3 >>= 8;
     }
     record->trace(ident);
   }
#pragma GCC diagnostic pop          // GCC regression END
}

#if USE_REDO_DIAGNOSTICS
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
#else // If USE_REDO_DIAGNOSTICS == false
static void assert_redo(EdRedo*, EdFile*) {} // Assert self-consistent REDO
static void assert_undo(EdRedo*, EdFile*) {} // Assert self-consistent UNDO
#endif

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
     traceh("\n\n--------------------------------\n");
     debug("redo");
   }

   EdRedo* redo= redo_list.remq();
   if( redo == nullptr ) {
     put_message("Cannot redo");
     return;
   }

   // Perform redo action
   trace_redo(".RDO", redo, this, editor::data->cursor);
   assert_redo(redo, this);         // (Only when USE_REDO_DIAGNOSTICS == true)

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
   editor::unit->activate(line);
   editor::unit->draw();
   undo_list.lifo(redo);            // Move REDO to UNDO list
   if( redo->head_insert )          // If lines inserted
     chg_mode(redo->head_insert, redo->tail_insert);

   if( USE_REDO_DIAGNOSTICS )
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
     traceh("\n\n--------------------------------\n");
     debug("undo");
   }

   EdRedo* undo= undo_list.remq();
   if( undo == nullptr ) {
     put_message("Cannot undo");
     return;
   }

   // Perform undo action
   trace_redo(".UDO", undo, this, editor::data->cursor);
   assert_undo(undo, this);         // (Only when USE_REDO_DIAGNOSTICS == true)

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
   editor::unit->activate(line);
   editor::unit->draw();
   redo_list.lifo(undo);            // Move UNDO to REDO list
   if( undo->head_remove )          // If lines removed
     chg_mode(undo->head_remove, undo->tail_remove);

   if( USE_REDO_DIAGNOSTICS )
     Config::check("undo");
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::redo_delete
//
// Purpose-
//       Delete the entire REDO list, also deleting associated insert lines
//
//----------------------------------------------------------------------------
void
   EdFile::redo_delete( void )      // Delete the REDO list
{
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
   trace_redo(".IDO", redo, this, editor::data->cursor);
   assert_undo(redo, this);         // (Only when USE_REDO_DIAGNOSTICS == true)
   redo_delete();                   // Delete the current REDO list

   undo_list.lifo(redo);            // Insert the REDO onto the UNDO list
   changed= true;

   if( USE_REDO_DIAGNOSTICS )
     Config::check("redo_insert");
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::undo_delete
//
// Purpose-
//       Delete the entire UNDO list, also deleting assocated remove lines
//
//----------------------------------------------------------------------------
void
   EdFile::undo_delete( void )      // Delete the UNDO list
{
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
//       Implicit copy constructor used only in EdView for a temporary EdLine.
//       (This copy constructor DOES NOT increment object_count.)
//
//----------------------------------------------------------------------------
   EdLine::EdLine(                  // Constructor
     const char*       text)        // (Immutable) text
:  ::pub::List<EdLine>::Link(), text(text ? text : "")
{  if( HCDM || (opt_hcdm && opt_verbose > 1) )
     traceh("EdLine(%p)::EdLine\n", this);

   Trace::trace(".NEW", "line", this);

   if( USE_OBJECT_COUNT )
     ++object_count;
}

   EdLine::~EdLine( void )          // Destructor
{  if( HCDM || (opt_hcdm && opt_verbose > 1) )
     traceh("EdLine(%p)::~EdLine\n", this);

   if( flags & F_AUTO )             // If this is a temporary line
     return;                        // Delete means nothing

   Trace::trace(".DEL", "line", this);

   if( USE_OBJECT_COUNT )
     --object_count;
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
   tracef("%p F(%.4x) D(%.2x,%.2x) '%s'\n", this, flags
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
{  if( HCDM || (opt_hcdm && opt_verbose > 1) )
     traceh("EdLine(%p)::is_within(%p,%p)\n", this, head, tail);

   for(const EdLine* line= head; line; line= line->get_next() ) {
     if( line == this )
       return true;
     if( line == tail )
       return false;
   }

   /// We get here because line == nullptr, which should not occur.
   /// The associated list segment is corrupt, and code needs fixing.
   if( head || tail )               // If the range is not empty
     traceh("%4d EdLine(%p).is_within(%p..%p) invalid range\n", __LINE__
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
//       EdRedo::EdRedo
//
// Purpose-
//       Constructor/Destructor
//
//----------------------------------------------------------------------------
   EdRedo::EdRedo( void )           // Constructor
:  ::pub::List<EdRedo>::Link()
{  if( HCDM || opt_hcdm )
     traceh("EdRedo(%p)::EdRedo\n", this);

   Trace::trace(".NEW", "redo", this);
}

   EdRedo::~EdRedo( void )          // Destructor
{  if( HCDM || opt_hcdm )
     traceh("EdRedo(%p)::~EdRedo\n", this);

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
{  traceh("EdRedo(%p)::debug(%s)\n", this, info ? info : "");

   traceh("  COL [%3zd:%3zd]\n", lh_col, rh_col);

   traceh("  INS [");
   if( head_insert ) tracef("%p<-", head_insert->get_prev());
   tracef("%p,%p", head_insert, tail_insert);
   if( tail_insert ) tracef("->%p", tail_insert->get_next());
   tracef("],\n");

   for(EdLine* line= head_insert; line; line=line->get_next() ) {
     traceh("    "); line->debug();
     if( line == tail_insert )
       break;
   }

   traceh("  REM [");
   if( head_remove ) tracef("%p<-", head_remove->get_prev());
   tracef("%p,%p", head_remove, tail_remove);
   if( tail_remove ) tracef("->%p", tail_remove->get_next());
   tracef("]\n");

   for(EdLine* line= head_remove; line; line=line->get_next() ) {
     traceh("    "); line->debug();
     if( line == tail_remove )
       break;
   }
}
