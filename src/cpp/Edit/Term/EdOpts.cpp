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
//       TERM Editor: Configuration options.
//
// Last change date-
//       2024/06/14
//
//----------------------------------------------------------------------------
#ifndef  _GNU_SOURCE
#define  _GNU_SOURCE                // For strcasestr (Cygwin)
#endif
#include <string.h>                 // For strcasestr
#include <sys/stat.h>               // For struct stat

#include <ncurses.h>                // For ncurses (== curses.h)
#include "pub/Debug.h"              // For namespace pub::debugging
#include "pub/Fileman.h"            // For pub::Data, ...

#include "Config.h"                 // For Config, namespace config
#include "Editor.h"                 // For Editor::unit
#include "EdOpts.h"                 // For EdOpts, implemented
#include "EdInps.h"                 // For EdInps, window control
#include "EdOuts.h"                 // For EdOuts, allocated

using namespace config;             // For config::opt_*, ...
using namespace pub::debugging;     // For debugging
using namespace pub::fileman;       // For pub::fileman objects

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static bool            unicode_combining= true; // Default combining support
static bool            unicode_support= true; // Default unicode support

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
   atexit(EdOpts::at_exit);         // Set termination handler

   // Ubuntu and Cygwin implement UTF8 correctly, but (at least) Fedora
   // Fedora displays Unicode characters incorrectly as M-L~... with lines
   // spilling over into the next line.
   if( getenv("CYGWIN") == nullptr ) {
     // TODO: THIS IS A HACK, EXPLICITLY CHECKING FOR FEDORA.
     //       THERE HAS TO BE A BETTER WAY.
     struct stat info;              // File status information
     if( stat("/etc/os-release", &info) == 0 && S_ISREG(info.st_mode) ) {
       Data data("/etc", "os-release");
       for(Line* line= data.line().get_head(); line; line= line->get_next()) {
         if( strcasestr(line->text, "fedora") ) {
           unicode_combining= false;
           unicode_support= false;
           break;
         }
       }
     }
   }

   return new EdOuts();             // The associated EdUnit
}

void
   EdOpts::terminate(               // Terminate
     EdUnit*           unit)        // This EdUnit
{
   EdOpts::at_exit();               // Terminate ncurses

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
// EdOpts::Option control methods
//
// Implementation note:
//       Fedora linux ncurses does not natively support combining characters,
//       so we don't either.
//
//       We need to check this feature somehow, and update the controls
//       accordingly.
//
//----------------------------------------------------------------------------
bool                                // FALSE
   EdOpts::is_bg_enabled( void )    // Is opt_bg enabled?
{  return false; }

bool                                // TRUE (by default)
   EdOpts::has_unicode_combining( void ) // Unicode combining chars supported?
{  return unicode_combining; }

bool                                // TRUE (by default)
   EdOpts::has_unicode_support( void ) // Is Unicode display supported?
{  return unicode_support; }

//----------------------------------------------------------------------------
// EdOpts::Static strings
//----------------------------------------------------------------------------
const char*            EdOpts::DEFAULT_CONFIG=
   "[Program]\n"
   "URL=https://github.com/franke-hub/SDL/tree/trunk/src/cpp/Edit/Term\n"
   "Exec=Edit ; Edit in read-write mode\n"
   "Exec=View ; Edit in read-only mode\n"
   "Purpose=NCURSES based text editor\n"
   "Version=3.0.0-101\n"
   "\n"
   "[Options]\n"
   ";; (Defaulted) See sample: ~/src/cpp/Edit/Term/.Edit.conf\n"
   ;

const char*            EdOpts::EDITOR= "editerm";
const char*            EdOpts::PATCH= "0-101"; // Patch level
