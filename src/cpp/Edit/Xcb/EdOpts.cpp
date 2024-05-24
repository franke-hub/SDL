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
//       Editor: Xcb vs Term configuration options.
//
// Last change date-
//       2024/05/15
//
//----------------------------------------------------------------------------
#include "gui/Device.h"             // For gui::Device, allocated

#include "EdOpts.h"                 // For EdOpts, implemented
#include "EdOuts.h"                 // For EdOuts, allocated

//----------------------------------------------------------------------------
// EdOpts::Patch level
//----------------------------------------------------------------------------
const char*            EdOpts::PATCH= "0-100"; // Patch level

//----------------------------------------------------------------------------
// EdOpts::External data areas (control attributes)
//----------------------------------------------------------------------------
// Background is our preferred mode of operation, so we enable it.
int                    EdOpts::bg_enabled= true;
int                    EdOpts::utf8_enabled= true;

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

//----------------------------------------------------------------------------
// Static strings
//----------------------------------------------------------------------------
const char*            EdOpts::EDITOR= "editxcb";
const char*            EdOpts::DEFAULT_CONFIG=
   "[Program]\n"
   "URL=https://github.com/franke-hub/SDL/tree/trunk/src/cpp/Edit/Xcb\n"
   "Exec=Edit ; Edit in read-write mode\n"
   "Exec=View ; Edit in read-only mode\n"
   "Purpose=XCB based text editor\n"
   "Version=1.1.0\n"
   "\n"
   "[Options]\n"
   ";; (Defaulted) See sample: ~/src/cpp/Edit/Xcb/.SAMPLE/Edit.conf\n"
   ;
