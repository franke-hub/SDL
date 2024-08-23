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
//       2024/08/23
//
//----------------------------------------------------------------------------
#include <new>                      // For placement operator new
#include <string>                   // For std::string

#include <ncurses.h>                // For ncurses (== curses.h)
#include "pub/Trace.h"              // For pub::Trace::trace

#include "Editor.h"                 // For Editor::unit
#include "EdOpts.h"                 // For EdOpts, implemented
#include "EdInps.h"                 // For EdInps, window control
#include "EdOuts.h"                 // For EdOuts, allocated
#include "EdUnit.h"                 // For EdUnit::draw

//----------------------------------------------------------------------------
// Local data area
//----------------------------------------------------------------------------
struct Local {
bool                   unicode_combining= true; // Default combining support
bool                   unicode_support= true; // Default unicode support

// Static methods
static inline Local*                // Our local data area
   get(                             // Get Local data area
     EdInps*           inps= static_cast<EdInps*>(editor::unit))
{
   EdOpts* opts= static_cast<EdOpts*>(&inps->opts);
   return (Local*)(&opts->local);
}

static inline EdInps*               // The EdInps*
   inps( void )                     // Get EdInps*
{  return static_cast<EdInps*>(editor::unit); }
}; // struct Local

//----------------------------------------------------------------------------
//
// (Static) methods-
//       EdOpts::initialize
//       EdOpts::terminate
//       EdOpts::at_exit
//       EdOpts::resume
//       EdOpts::suspend
//
// Purpose-
//       Initialize the EdUnit
//       Terminate  the EdUnit
//       Termination handler
//       Resume NCURSES operation
//       Suspend NCURSES operation
//
//----------------------------------------------------------------------------
EdUnit*                             // The EdUnit
   EdOpts::initialize( void )       // Initialize the EdUnit
{
   atexit(EdOpts::at_exit);         // Set termination handler

   EdOuts* unit= new EdOuts();      // Our new Unit*
   EdInps* inps= static_cast<EdInps*>(unit);
   Local* local= new(Local::get(inps)) Local();

   // Cygwin implements UTF8 correctly, but linux Fedora and Ubuntu do not.
   // Fedora displays Unicode characters incorrectly as M-L~... with lines
   // spilling over into the next line.
   if( getenv("CYGWIN") == nullptr ) {
     local->unicode_combining= false;
     local->unicode_support= false;
   }

   return unit;                     // The associated EdUnit
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
{
   pub::Trace::trace(".TXT", __LINE__, "EdOpts at_exit");

   EdInps* inps= Local::inps();
   if( inps && inps->win ) {        // If we have a WINDOW
     resetty();                     // Reset the keyboard
     endwin();                      // Terminate NCURSES
     inps->win= nullptr;            // (But only once)
   }
}

void
   EdOpts::resume( void )           // Resume NCURSES operation
{
   pub::Trace::trace(".TXT", __LINE__, "EdOpts RESUME");

   refresh();                       // Restart NCURSES
   editor::unit->draw();            // And redraw the screen
}

void
   EdOpts::suspend( void )          // Suspend NCURSES operation
{
   pub::Trace::trace(".TXT", __LINE__, "EdOpts SUSPEND");

   erase();                         // Clear the screen
   resetty();                       // Reset the keyboard
   endwin();                        // Suspend NCURSES operation
   errno= 0;                        // Clear errno
}

//----------------------------------------------------------------------------
// EdOpts::Option control methods
//----------------------------------------------------------------------------
bool                                // FALSE
   EdOpts::is_bg_enabled( void )    // Is opt_bg enabled?
{  return false; }

bool                                // TRUE (by default)
   EdOpts::has_unicode_combining( void ) // Unicode combining chars supported?
{  return Local::get()->unicode_combining; }

bool                                // TRUE (by default)
   EdOpts::has_unicode_support( void ) // Is Unicode display supported?
{  return Local::get()->unicode_support; }

//----------------------------------------------------------------------------
// EdOpts::Static strings
//----------------------------------------------------------------------------
std::string                         // The default configuration file
   EdOpts::DEFAULT_CONFIG()
{
   return
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
}

std::string                         // The Editor's name
   EdOpts::EDITOR()
{  return "xtmedit"; }

std::string                         // Version patch level
   EdOpts::PATCH()
{  return "1-101"; }
