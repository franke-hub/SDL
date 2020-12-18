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
//       2020/12/16
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
class EdFull;                       // Editor full Window (experimental)
class EdMark;                       // Editor mark controller
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
//       Constructor/destructor (for namespace editor)
//
//----------------------------------------------------------------------------
class Editor {                      // Editor constuctor/destructor
public:
   Editor(                          // Constructor
     int               argi,        // Argument index
     int               argc,        // Argument count
     char*             argv[]);     // Argument array

   ~Editor( void );                 // Destructor
}; // class Editor

//----------------------------------------------------------------------------
//
// Namespace-
//       editor
//
// Purpose-
//       Editor: Global data areas
//
//----------------------------------------------------------------------------
namespace editor {                  // The Editor namespace
//----------------------------------------------------------------------------
// editor::Global attributes
//----------------------------------------------------------------------------
extern xcb::Device*    device;      // The root Device
extern xcb::Window*    window;      // The test Window

extern pub::List<EdFile> file_list; // The list of EdFiles
extern EdMark*         mark;        // The Mark Handler
extern EdText*         text;        // The Text Window

extern pub::List<EdPool> filePool;  // File allocation EdPool
extern pub::List<EdPool> textPool;  // Text allocation EdPool

extern std::string     locate_string; // The locate string
extern std::string     change_string; // The change string

//----------------------------------------------------------------------------
//
// Method-
//       editor::command
//
// Purpose-
//       Process a command.
//
// Implementation notes-
//       Implemented in EdBifs.cpp
//
//----------------------------------------------------------------------------
void
   command(                         // Process a command
     char*             buffer);     // (MODIFIABLE) command buffer

//----------------------------------------------------------------------------
//
// Method-
//       editor::allocate
//
// Purpose-
//       Allocate file/line text
//
//----------------------------------------------------------------------------
char*                               // The (immutable) text
   allocate(                        // Get (immutable) text
     size_t            length);     // Of this length (includes '\0' delimit)

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_done
//
// Purpose-
//       (Safely) remove all files from the file list. (Error if any changed.)
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   do_done( void );                 // (Safely) terminate, error if changed.

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_test
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
//       editor::insert_file
//
// Purpose-
//       Insert file onto the file list
//
//----------------------------------------------------------------------------
EdFile*                             // The last file inserted
   insert_file(                     // Insert file(s) onto the file list
     const char*       name= nullptr); // The file name (wildards allowed)

//----------------------------------------------------------------------------
//
// Method-
//       editor::key_to_name
//
// Purpose-
//       BRINGUP: Convert xcb_keysym_t to its name. (TODO: REMOVE)
//
//----------------------------------------------------------------------------
const char*                         // The symbol name, "???" if unknown
   key_to_name(xcb_keysym_t key);   // Convert xcb_keysym_t to name

//----------------------------------------------------------------------------
//
// Method-
//       editor::remove_file
//
// Purpose-
//       Remove file from the file list
//
//----------------------------------------------------------------------------
void
   remove_file(                     // Remove file from the file list
     EdFile*           file);       // The file to remove (and delete)

//----------------------------------------------------------------------------
//
// Method-
//       editor::set_font
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
//       editor::join
//       editor::start
//
// Purpose-
//       Virtual thread implementation
//
//----------------------------------------------------------------------------
void
   join( void );                    // Wait for "Thread"

void
   start( void );                   // Start "Thread"
}  // namespace editor
#endif // EDITOR_H_INCLUDED
