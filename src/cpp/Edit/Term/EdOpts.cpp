//----------------------------------------------------------------------------
//
//       Copyright (C) 2024 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdOpts.cpp (TERM version)
//
// Purpose-
//       Editor: Xcb vs Term configuration options.
//
// Last change date-
//       2024/05/13
//
//----------------------------------------------------------------------------
#include <ncurses.h>                // For ncurses (== curses.h)
#include "pub/Debug.h"              // For namespace pub::debugging

#include "Config.h"                 // For Config, namespace config
#include "Editor.h"                 // For Editor::unit
#include "EdOpts.h"                 // For EdOpts, implemented
#include "EdInps.h"                 // For EdInps, window control
#include "EdOuts.h"                 // For EdOuts, allocated

using namespace config;             // For config::opt_*, ...
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// EdOpts::External data areas (control attributes)
//----------------------------------------------------------------------------
// We don't run in background mode.
int                    EdOpts::bg_enabled= false;

// UTF-8 combining characters are NOT supported.
// The characters are combined by curses output, but there's a lot of other
// work to account for these characters properly.
int                    EdOpts::cc_enabled= true; // (This is unchecked)

//----------------------------------------------------------------------------
//
// (Static) methods-
//       EdOpts::initialize
//       EdOpts::terminate
//       EdOpts::at_exit
//
// Purpose-
//       Initialize the EdUnit
//       Terminate  the EdUnit
//       Termination handler
//
//----------------------------------------------------------------------------
static inline EdInps*               // The EdInps*
   inps( void )                     // Get EdInps*
{  return static_cast<EdInps*>(editor::unit); }

EdUnit*                             // The EdUnit
   EdOpts::initialize( void )       // Initialize the EdUnit
{
   return new EdOuts();             // The associated EdUnit
}

void
   EdOpts::terminate(               // Terminate
     EdUnit*           unit)        // This EdUnit
{
   delete unit;                     // Delete the EdUnit
}

void
   EdOpts::at_exit( void )          // (Idempotent) termination handler
{  if( opt_hcdm )
     traceh("EdOpts::at_exit(%s)\n", inps() && inps()->win ? "true" : "false");

   if( inps() && inps()->win ) {    // If we have a WINDOW
     resetty();                     // Reset the keyboard
     endwin();                      // Terminate the NCURSES window
     inps()->win= nullptr;          // (But only once)
   }
}

//----------------------------------------------------------------------------
// Static strings
//----------------------------------------------------------------------------
const char*            EdOpts::EDITOR= "editerm";
const char*            EdOpts::DEFAULT_CONFIG=
   "[Program]\n"
   "URL=https://github.com/franke-hub/SDL/tree/trunk/src/cpp/Edit/Term\n"
   "Exec=Edit ; Edit in read-write mode\n"
   "Exec=View ; Edit in read-only mode\n"
   "Purpose=NCURSES based text editor\n"
   "Version=1.1.0\n"
   "\n"
   "[Options]\n"
   ";; (Defaulted) See sample: ~/src/cpp/Edit/Term/.SAMPLE/Edit.conf\n"
   ;
