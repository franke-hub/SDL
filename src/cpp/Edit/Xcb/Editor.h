//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2024 Frank Eskesen.
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
//       2024/04/02
//
//----------------------------------------------------------------------------
#ifndef EDITOR_H_INCLUDED
#define EDITOR_H_INCLUDED

#include <xcb/xproto.h>             // For xcb_keysym_t, xcb_rectangle_t, ...

#include <gui/Device.h>             // For gui::Device
#include <gui/Font.h>               // For gui::Font
#include <gui/Window.h>             // For gui::Window
#include <pub/List.h>               // For pub::List

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Active;                       // Editor Active line object
class EdFile;                       // Editor file descriptor
class EdHist;                       // Editor history view
class EdMark;                       // Editor mark controller
class EdPool;                       // Editor pool allocators
class EdTerm;                       // Editor terminal controller
class EdView;                       // Editor view

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
//----------------------------------------------------------------------------
// Editor::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum                                // TABS controls
{  TAB_DEFAULT= 8                   // TABS: default spacing
,  TAB_DIM= 128                     // TABS: table size (+1)
}; // enum

//----------------------------------------------------------------------------
// Editor::Constructor/Destructor
//----------------------------------------------------------------------------
   Editor(                          // Constructor
     int               argi,        // Argument index
     int               argc,        // Argument count
     char*             argv[]);     // Argument array

   ~Editor( void );                 // Destructor

//----------------------------------------------------------------------------
// Editor::debug (Debugging display)
//----------------------------------------------------------------------------
static void
   debug(                           // Debugging display
     const char*       info= nullptr); // Associated info

//----------------------------------------------------------------------------
//
// Method-
//       Editor::alertf
//
// Purpose-
//       Extend errorf to write screen alert.
//
// Implementation note-
//       Do not include trailing '\n' in string.
//       Side-effects: editor::diagnostic=true; internal trace stopped.
//
//----------------------------------------------------------------------------
ATTRIB_PRINTF(1, 2)
static void
   alertf(                          // Write to stderr, trace iff opt_hcdm
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF argruments

//----------------------------------------------------------------------------
//
// Method-
//       Editor::put_message
//
// Purpose-
//       Formatted put_message (with default mode)
//
//----------------------------------------------------------------------------
ATTRIB_PRINTF(1, 2)
static void
   put_message(                     // Formatted put_message
     const char*       fmt,         // The PRINTF format string
                       ...);        // PRINTF argruments
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
extern EdTerm*         term;        // The Terminal controller

extern pub::List<EdFile> file_list; // The list of EdFiles
extern EdFile*         file;        // The current File object
extern EdFile*         last;        // The last file inserted

extern Active*         actalt;      // Active object (for temporary use)
extern Active*         active;      // Active object (for temporary use)
extern EdView*         data;        // The data view
extern EdHist*         hist;        // The history view
extern EdMark*         mark;        // The Mark Handler
extern EdView*         view;        // The active view

extern std::string     locate_string; // The locate string
extern std::string     change_string; // The change string

extern pub::List<EdPool> filePool;  // File allocation EdPool
extern pub::List<EdPool> textPool;  // Text allocation EdPool

// Diagnostic controls -------------------------------------------------------
extern uint32_t        diagnostic;  // Diagnostic state (TRUE if halted)

// Search controls -----------------------------------------------------------
extern uint32_t        locate_back; // Reverse search (default= false)
extern uint32_t        locate_case; // Case sensitive search (default= false)
extern uint32_t        locate_wrap; // Autowrap (default= false)

// Margins (for format) ------------------------------------------------------
extern size_t          margins[2];  // [left][right] margins
extern size_t          tabs[Editor::TAB_DIM]; // Tabs array, tab[0] is count

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

const char*                         // The (immutable) text
   allocate(                        // Get (immutable) text
     const char*       source);     // Source (mutable) text

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
const char*                         // Error message, nullptr if none
   command(                         // Process a command
     char*             buffer);     // (MODIFIABLE) command buffer

//----------------------------------------------------------------------------
//
// Method-
//       editor::command_help
//
// Purpose-
//       Process the 'help' command.
//
// Implementation notes-
//       Implemented in EdBifs.cpp
//
//----------------------------------------------------------------------------
const char*                         // (Always nullptr)
   command_help(char* parm= nullptr); // Process the 'help' command

//----------------------------------------------------------------------------
//
// Method-
//       editor::data_protected
//
// Purpose-
//       Check for protected file and data view
//
//----------------------------------------------------------------------------
int                          // Return code, TRUE if error message
   data_protected( void );   // Error if protected file and data view

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_change
//
// Purpose-
//       Change next occurance of string.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   do_change( void );               // Change next occurance of string

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_find
//
// Purpose-
//       Change next occurance of string that begins a line.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   do_find(const char*);            // Find next col[0] occurance of string

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_insert
//
// Purpose-
//       Insert a line after the cursor.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   do_insert( void );               // Insert a new, empty line

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_join
//
// Purpose-
//       Join the current and next line.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   do_join( void );                 // Join the current and next line

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_locate
//
// Purpose-
//       Locate next occurance of string.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   do_locate(                       // Locate next
     int               offset= 1);  // Use offset 0 for locate_change

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_quit
//
// Purpose-
//       (Safely) remove the current file from the ring.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   do_quit( void );                 // Safely remove current file from the ring

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_split
//
// Purpose-
//       Split the current line at the cursor.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   do_split( void );                // Split the current line at the cursor

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_view
//
// Purpose-
//       Invert the view.
//
//----------------------------------------------------------------------------
void
   do_view( void );                 // Invert history view

//----------------------------------------------------------------------------
//
// Method-
//       editor::exit
//
// Purpose-
//       Unconditional editor (normal) exit.
//
//----------------------------------------------------------------------------
void
   exit( void );                    // Unconditional editor (normal) exit

//----------------------------------------------------------------------------
//
// Method-
//       editor::file_command
//
// Purpose-
//       Load command input/output pseudo-file
//
//----------------------------------------------------------------------------
void
   file_command(                      // Load command input/output pseudo-file
     const char*       input,         // The command (required)
     const std::string&output);       // The command output

//----------------------------------------------------------------------------
//
// Method-
//       editor::file_loader
//
// Purpose-
//       Load files, adding them to the file list
//
//----------------------------------------------------------------------------
void
   file_loader(                     // Load files, adding them to the file list
     const char*       name= nullptr, // The file name (wildards allowed)
     int               protect= false); // Protect file?

//----------------------------------------------------------------------------
//
// Method-
//       editor::put_message
//
// Purpose-
//       Add a message to file's message list
//
//----------------------------------------------------------------------------
void
   put_message(                     // Write message
     const char*       mess_,       // Message text
     int               type_= 0);   // Message mode (default EdMess::T_INFO)

//----------------------------------------------------------------------------
//
// Method-
//       editor::remove_file
//
// Purpose-
//       Remove the active file from the file list
//
//----------------------------------------------------------------------------
void
   remove_file( void );             // Remove active file from the file list

//----------------------------------------------------------------------------
//
// Method-
//       editor::tab_forward
//
// Purpose-
//       Get next tab column.
//
//----------------------------------------------------------------------------
size_t                              // The next tab column
   tab_forward(                     // Get the tab column
     size_t            column);     // After this column

//----------------------------------------------------------------------------
//
// Method-
//       editor::tab_reverse
//
// Purpose-
//       Get prior tab column.
//
//----------------------------------------------------------------------------
size_t                              // The prior tab column
   tab_reverse(                     // Get the tab column
     size_t            column);     // Before this column

//----------------------------------------------------------------------------
//
// Method-
//       editor::un_changed
//
// Purpose-
//       If any file has changed, activate it.
//
//----------------------------------------------------------------------------
bool                                // TRUE iff editor is in unchanged state
   un_changed( void );              // Activate a changed file

//----------------------------------------------------------------------------
//
// Method-
//       editor::write_file
//
// Purpose-
//       Write a/the file
//
// Implementation notes-
//       Implemented in EdBifs.cpp
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr if none
   write_file(                      // Write a file (nullptr for current file)
     char*             buffer);     // (MODIFIABLE) command buffer

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
