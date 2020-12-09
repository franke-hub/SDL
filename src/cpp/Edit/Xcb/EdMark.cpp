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
//       2020/12/08
//
//----------------------------------------------------------------------------
#include <pub/Signals.h>            // For pub::signals::Connector

#include "Editor.h"                 // For Editor
#include "EdFile.h"                 // For EdFile, EdLine, EdRedo
#include "EdMark.h"                 // For EdMark (Implementation class)

using namespace editor::debug;

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
// Method-
//       EdMark::EdMark
//
// Purpose-
//       Constructor/Destructor/Initializer
//
//----------------------------------------------------------------------------
   EdMark::EdMark( void )           // Constructor
{
   initialize();                    // Initialize closeEvent_connector
}

   EdMark::~EdMark( void )          // Destructor (Editor shutdown)
{
   for(;;) {
     EdLine* line= mark_list.get_head();
     if( line == nullptr )
       break;

     delete line;
   }
}

void
   EdMark::initialize( void )       // Initialize closeEvent_connector
{
   using Event= EdFile::CloseEvent;
   closeEvent_connector= EdFile::close_signal.connect([this](Event& event) {
     if( opt_hcdm )
       debugf("EdMark CloseEvent.file(%s)\n", event.file->get_name().c_str());

     if( event.file == file ) {
       file= nullptr;
       line= nullptr;
       touch_line= nullptr;
       touch_col= 0;
     }
   });
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
{  /**** NOT CODED YET ****/ return nullptr; }

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
{  /**** NOT CODED YET ****/ return nullptr; }

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
//       EdMark::format
//
// Purpose-
//       Format the marked area
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   EdMark::mark(                    // Create/expand/contract the mark
     EdFile*           file,        // For this EdFile
     EdLine*           line,        // And this EdLine
     ssize_t           column)      // Start at this column (block copy)
{  (void)file; (void)line; (void)column; return nullptr; }

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
   EdMark::paste(                   // Paste the marked area
     EdFile*           file,        // Into this EdFile
     EdLine*           line,        // After this line
     ssize_t           column)      // Start at this column (block copy)
{  (void)file; (void)line; (void)column; return nullptr; }

//----------------------------------------------------------------------------
//
// Method-
//       EdMark::reset
//
// Purpose-
//       Reset (undo) the mark
//
//----------------------------------------------------------------------------
void
   EdMark::reset( void )            // Remove (undo) the mark
{  /**** NOT CODED YET ****/ }
