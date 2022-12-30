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
//       Config.hpp
//
// Purpose-
//       Editor: Config.cpp extension: debugging listeners
//
// Last change date-
//       2022/12/29
//
// Implementation note-
//       Only included from Config.cpp
//
//----------------------------------------------------------------------------
#include "EdFile.h"                 // For EdRedo, EdLine
#include "EdHist.h"                 // For EdHist
#include "EdMark.h"                 // For EdMark
#include "EdTerm.h"                 // For EdTerm
#include "EdView.h"                 // For EdView

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
   debugf("Config::check checkstop(%s)\n", message.c_str());

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
     debugf("%4d Ed::check redo(%p) head(%p) tail(%p)\n", __LINE__, redo, head, tail);
     return checkstop("invalid_list");
   } else if( head == nullptr ) return false;

   for(const EdLine* line= head; line; line= line->get_next() ) {
     if( line == tail ) return false;
   }

   debugf("%4d Ed::check redo(%p) head(%p) tail(%p)\n", __LINE__, redo, head, tail);
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
//       config_check (config::checkSignal listener)
//
// Purpose-
//       Bringup error checking (Verify undo/redo list consistency)
//
//----------------------------------------------------------------------------
static pub::signals::Connector<const char*> config_check=
                       checkSignal.connect([](const char*& info)
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
}); // config_check (Connector)

//----------------------------------------------------------------------------
//
// (Lambda) subroutine-
//       config_debug (config::debugSignal listener)
//
// Purpose-
//       Bringup debugging (Write diagnostic data to console and debug.out)
//
//----------------------------------------------------------------------------
static pub::signals::Connector<const char*> config_debug=
                       debugSignal.connect([](const char*& info)
{
   debugf("\n============================================================\n");
   debugf("Config::debugSignal(%s)\n", info ? info : "");

   Editor::debug(info);
   debugf("\n");
   editor::mark->debug(info);
   debugf("\n");
   editor::file->debug("lines");
   debugf("\n");
   editor::term->debug(info);
   debugf("\n");
   config::font->debug(info);
   debugf("\n");
   editor::data->debug(info);
   debugf("\n");
   editor::hist->debug(info);
   debugf("============================================================\n\n");
}); // config_debug (Connector)
