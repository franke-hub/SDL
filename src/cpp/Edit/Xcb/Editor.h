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
//       2020/12/09
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

//----------------------------------------------------------------------------
//
// Method-
//       Editor::failure
//
// Purpose-
//       Write error message and exit
//
//----------------------------------------------------------------------------
static void
   failure(                         // Write error message and exit
     std::string       mess);       // (The error message)
}; // c;ass Editor

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

extern pub::List<EdFile> ring;      // The list of EdFiles
extern EdFind*         find;        // The Find Popup
extern EdFull*         full;        // The Full Window
extern EdMark*         mark;        // The Mark Handler
extern EdMenu*         menu;        // The Menu Layout
extern EdTabs*         tabs;        // The Tabs Layout
extern EdText*         text;        // The Text Window

extern pub::List<EdPool> filePool;  // File allocation EdPool
extern pub::List<EdPool> textPool;  // Text allocation EdPool

extern std::string     locate_string; // The locate string
extern std::string     change_string; // The change string

// Operational controls
extern std::string     autosave_dir; // "~/.cache/uuid/" + UUID
extern int             autowrap;   // Autowrap locate (= false)
extern int             ignore_case; // Ignore case when searching (= true)
extern int             search_mode; // (Positive= forward, else reverse) (= 0)

//----------------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------------
static constexpr const char* const AUTOSAVE= "*AUTOSAVE*.";
static constexpr const char* const NO_STRING= ""; // The empty string
static constexpr const char* const UUID= "e743e3ac-6816-4878-81a2-b47c9bbc2d37";

//----------------------------------------------------------------------------
// Immutable constants
//----------------------------------------------------------------------------
enum
{  CLR_UNUSED                       // (All other entries are comma delimited)
// Color definitions
,  CLR_Black=          0x00000000
,  CLR_DarkRed=        0x00900000   // A.K.A red4
,  CLR_FireBrick=      0x00B22222
,  CLR_LightBlue=      0x00C0F0FF
,  CLR_LightSkyBlue=   0x00B0E0FF
,  CLR_PaleMagenta=    0x00FFC0FF   // A.K.A plum1
,  CLR_PaleYellow=     0x00FFFFF0   // A.K.A ivory
,  CLR_PowderBlue=     0x00B0E0E0
,  CLR_White=          0x00FFFFFF
,  CLR_Yellow=         0x00FFFF00

// Color selectors
,  CHG_FG= CLR_DarkRed              // FG: Status line, file changed
,  CHG_BG= CLR_LightBlue            // BG: Status line, file changed

,  CMD_FG= CLR_Black                // FG: Command line
,  CMD_BG= CLR_PaleMagenta          // BG: Command line

,  MSG_FG= CLR_DarkRed              // FG: Message line
,  MSG_BG= CLR_Yellow               // BG: Message line

,  STS_FG= CLR_Black                // FG: Status line, file unchanged
,  STS_BG= CLR_LightBlue            // BG: Status line, file unchanged

,  TXT_FG= CLR_Black                // FG: Text
,  TXT_BG= CLR_PaleYellow           // BG: Text
}; // generic enum

//----------------------------------------------------------------------------
//
// Method-
//       editor::debug::debugf
//       editor::debug::debugh
//       editor::debug::errorf
//
// Purpose-
//       Debugging utilities, mostly identical to pub::debugging
//         (errorf writes to stderr and only writes to trace if opt_hcdm true.
//
//----------------------------------------------------------------------------
namespace debug {
// Debugging controls
extern int             opt_hcdm;    // Hard Core Debug Mode?
extern const char*     opt_test;    // Bringup test?
extern int             opt_verbose; // Debugging verbosity

// Debugging methods
void
   debugf(                          // Write to trace and stdout
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);

void
   debugh(                          // Write to trace and stdout with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);

void
   errorf(                          // Write to stderr, trace iff opt_hcdm
     const char*       fmt,         // The PRINTF format string
                       ...)         // PRINTF argruments
   _ATTRIBUTE_PRINTF(1, 2);
}  // namespace (editor::)debug

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
//       editor::do_done
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
//       editor::do_quit
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
//       editor::get_text
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
