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
//       EdOpts.cpp (XCB version)
//
// Purpose-
//       XCB Editor: Configuration options.
//
// Last change date-
//       2024/07/27
//
//----------------------------------------------------------------------------
#include "gui/Device.h"             // For gui::Device, allocated

#include "EdOpts.h"                 // For EdOpts, implemented
#include "EdOuts.h"                 // For EdOuts, allocated

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
//       Resume NCURSES operation (Does not apply)
//       Suspend NCURSES operation (Does not apply)
//
//----------------------------------------------------------------------------
EdUnit*                             // The EdUnit
   EdOpts::initialize( void )       // Initialize an EdUnit
{
   gui::Device* device= new gui::Device(); // The associated gui::Device
   return new EdOuts(device, "EdUnit"); // The associated EdUnit
}

void
   EdOpts::terminate(               // Terminate
     EdUnit*           unit)        // This EdUnit
{
   EdOuts*      outs=   static_cast<EdOuts*>(unit);
   gui::Device* device= static_cast<gui::Device*>(outs->get_parent());
   delete unit;                     // Delete the EdUnit
   delete device;                   // Then, delete the Device
}

void
   EdOpts::at_exit( void )          // (Idempotent) termination handler
{  } // NOT NEEDED

void
   EdOpts::resume( void )           // Resume NCURSES operaion
{  } // DOES NOT APPLY

void
   EdOpts::suspend( void )          // Suspend NCURSES operaion
{  } // DOES NOT APPLY

//----------------------------------------------------------------------------
// EdOpts::Option control methods
//----------------------------------------------------------------------------
bool                                // TRUE
   EdOpts::is_bg_enabled( void )    // Is opt_bg enabled?
{  return true; }

bool                                // FALSE (At least for now)
   EdOpts::has_unicode_combining( void ) // Unicode combining chars supported?
{  return false; }

bool                                // TRUE
   EdOpts::has_unicode_support( void ) // Is Unicode display supported?
{  return true; }

//----------------------------------------------------------------------------
// EdOpts::Static strings
//----------------------------------------------------------------------------
std::string                         // The default configuration file
   EdOpts::DEFAULT_CONFIG()
{
   return
     "[Program]\n"
     "URL=https://github.com/franke-hub/SDL/tree/trunk/src/cpp/Edit/Xcb\n"
     "Exec=Edit ; Edit in read-write mode\n"
     "Exec=View ; Edit in read-only mode\n"
     "Purpose=XCB based text editor\n"
     "Version=3.0.0-101\n"
     "\n"
     "[Options]\n"
     ";; (Defaulted) See sample: ~/src/cpp/Edit/Xcb/.Edit.conf\n"
     ;
}

std::string                         // The Editor's name
   EdOpts::EDITOR()
{  return "editxcb"; }

std::string                         // Version patch level
   EdOpts::PATCH()
{  return "1-101"; }
