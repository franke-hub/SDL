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
//       Editor.h
//
// Purpose-
//       Editor: Global data areas
//
// Last change date-
//       2020/10/17
//
// Implementation notes-
//       TODO: REMOVE: Debugging controls
//
//----------------------------------------------------------------------------
#ifndef EDITOR_H_INCLUDED
#define EDITOR_H_INCLUDED

#include <xcb/xproto.h>             // For xcb_keysym_t
#include <pub/List.h>               // For pub::List

#include "Xcb/Active.h"             // For xcb::Active
#include "Xcb/Device.h"             // For xcb::Device
#include "Xcb/Widget.h"             // For xcb::Widget, our base class
#include "Xcb/Window.h"             // For xcb::Window

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class EdFile;                       // Editor file descriptor
class EdFind;                       // Editor find Popup
class EdMain;                       // Editor main Window
class EdMenu;                       // Editor menu Layout
class EdPool;                       // Editor pool allocators
class EdTabs;                       // Editor tabs Layout
class EdText;                       // Editor text Window

//----------------------------------------------------------------------------
//
// Class-
//       Editor
//
// Purpose-
//       Editor: Global data areas
//
// Implementation note-
//       The Editor object is a singleton. Only one Editor exists per process.
//
//----------------------------------------------------------------------------
class Editor : public xcb::Widget { // The Editor control object
//----------------------------------------------------------------------------
// Editor::Attributes
//----------------------------------------------------------------------------
public:
static Editor*         editor;      // The Editor singleton

xcb::Device*           device= nullptr; // The root Device
xcb::Window*           window= nullptr; // The test Window

pub::List<EdFile>      ring;        // The list of EdFiles
EdFind*                find= nullptr; // The Find Popup
EdMain*                main= nullptr; // The Main Window
EdMenu*                menu= nullptr; // The Menu Layout
EdTabs*                tabs= nullptr; // The Tabs Layout
EdText*                text= nullptr; // The Text Window

pub::List<EdPool>      filePool;    // File allocation EdPool
pub::List<EdPool>      textPool;    // Text allocation EdPool

xcb::Active            active;      // The current active text
xcb::Connector<xcb::DeviceEvent>
                       device_listener; // Our DeviceListener Connector

std::string            locate_string; // The locate string
std::string            change_string; // The change string

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
public:
static constexpr const char* const NO_STRING= ""; // The empty string

//----------------------------------------------------------------------------
// Editor::Constructor/Destructor
//----------------------------------------------------------------------------
public:
virtual
   ~Editor( void );                 // Destructor

   Editor(                          // Constructor
     int               argi,        // Argument index
     int               argc,        // Argument count
     char*             argv[]);     // Argument array

//----------------------------------------------------------------------------
//
// Method-
//       Editor::do_done
//
// Purpose-
//       (Safely) remove all files from the ring. (Error if any changed.)
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   do_done( void );                 // (Safely) terminate, error if changed.

//----------------------------------------------------------------------------
//
// Method-
//       Editor::do_quit
//
// Purpose-
//       Remove a file from the ring, discarding changes.
//
//----------------------------------------------------------------------------
void
   do_quit(                         // (Unconditionally) remove
     EdFile*           file);       // This file from the ring

//----------------------------------------------------------------------------
//
// Method-
//       Editor::do_test
//
// Purpose-
//       Bringup test.
//
//----------------------------------------------------------------------------
void
   do_test( void );                 // Bringup test

//----------------------------------------------------------------------------
//
// Method-
//       Editor::get_text
//
// Purpose-
//       Allocate file/line text
//
//----------------------------------------------------------------------------
char*                               // The (immutable) text
   get_text(                        // Get (immutable) text
     size_t            length);     // Of this length (includes '\0' delimit)

//----------------------------------------------------------------------------
//
// Method-
//       Editor::key_to_name
//
// Purpose-
//       BRINGUP: Convert xcb_keysym_t to its name. (TODO: REMOVE)
//
//----------------------------------------------------------------------------
public:
static const char*                  // The symbol name, "???" if unknown
   key_to_name(xcb_keysym_t key);   // Convert xcb_keysym_t to name

//----------------------------------------------------------------------------
//
// Method-
//       Editor::set_font
//
// Purpose-
//       Set the font
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   set_font(                        // Set the font
     const char*       font= nullptr); // To this font name

//----------------------------------------------------------------------------
//
// Method-
//       Editor::join
//       Editor::start
//
// Purpose-
//       Virtual thread implementation
//
//----------------------------------------------------------------------------
virtual void
   join( void );                    // Wait for "Thread"

virtual void
   start( void );                   // Start "Thread"
}; // class Editor
#endif // EDITOR_H_INCLUDED
