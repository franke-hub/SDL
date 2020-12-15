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
//       Editor.cpp
//
// Purpose-
//       Editor: Implement Editor.h
//
// Last change date-
//       2020/12/14
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::mutex, std::lock_guard

#include <assert.h>                 // For assert
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <unistd.h>                 // For close, ftruncate
#include <sys/stat.h>               // For stat
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include <pub/Fileman.h>            // For namespace pub::Fileman
#include <pub/Signals.h>            // For pub::signals
#include <pub/Thread.h>             // For pub::Thread::sleep
#include <pub/Trace.h>              // For pub::Trace

#include "Xcb/Device.h"             // For xcb::Device
#include <Xcb/Keysym.h>             // For xcb_keycode_t symbols
#include "Xcb/Layout.h"             // For xcb::Layout
#include "Xcb/Widget.h"             // For xcb::Widget, our base class
#include "Xcb/Window.h"             // For xcb::Window

#include "Editor.h"                 // For Editor (Implementation class)
#include "EdFile.h"                 // For EdFile, EdLine, EdPool
#include "EdMark.h"                 // For EdMark
#include "EdMisc.h"                 // For EdMisc TODO: REMOVE
#include "EdPool.h"                 // For EdPool
#include "EdText.h"                 // For EdText

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
static std::string     HOME;        // Home directory
static const mode_t    DIR_MODE= S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH;

//----------------------------------------------------------------------------
// Default .README.txt
//----------------------------------------------------------------------------
static const char*     README_text=
   "[Program]\n"
   "Http=http://eske-systems.com\n"
   "Exec=view\n"
   "Exec=edit\n"
   "Purpose=Graphic text editor\n"
   "Version=0.0\n"
   "\n"
   "[Options]\n"
   "## autosave_dir=~/.cache/uuid/e743e3ac-6816-4878-81a2-b47c9bbc2d37\n"
   "## ignore_case=true\n"
   ;

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
xcb::Device*           editor::device= nullptr; // The root Device
xcb::Window*           editor::window= nullptr; // The test Window

pub::List<EdFile>      editor::file_list; // The list of EdFiles
EdMark*                editor::mark= nullptr; // The Mark Handler
EdText*                editor::text= nullptr; // The Text Window

pub::List<EdPool>      editor::filePool; // File allocation EdPool
pub::List<EdPool>      editor::textPool; // Text allocation EdPool

std::string            editor::locate_string; // The locate string
std::string            editor::change_string; // The change string

// Debugging options
const char*            editor::debug::opt_test= nullptr;  // Bringup test?
int                    editor::debug::opt_hcdm= false; // Hard Core Debug Mode?
int                    editor::debug::opt_verbose= false; // Debug verbosity

// Operational controls
std::string            editor::autosave_dir;
int                    editor::autowrap= false;
int                    editor::ignore_case= true;
int                    editor::search_mode= 0;
//                     _unused=0;   // (Alignment)

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::mutex      mutex;       // Singleton mutex
static int             singleton= 0; // Singleton control

//----------------------------------------------------------------------------
//
// Subroutine-
//       use_debug
//
// Purpose-
//       Test whether or not debugging is active
//
//----------------------------------------------------------------------------
static inline bool use_debug( void ) // Is debugging active?
{ return HCDM || USE_BRINGUP || opt_hcdm; }

//----------------------------------------------------------------------------
//
// Subroutine-
//       editor::debug::debugf
//       editor::debug::debugh
//       editor::debug::errorf
//
// Purpose-
//       Debugging interfaces, similar or identical to ::pub::debugging
//
//----------------------------------------------------------------------------
void
   editor::debug::debugf(           // Debug debug printf facility
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   ::pub::debugging::vdebugf(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

void
   editor::debug::debugh(           // Debug debug printf facility with heading
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   ::pub::debugging::vdebugh(fmt, argptr);
   va_end(argptr);                  // Close va_ functions
}

void
   editor::debug::errorf(           // Debug debug to stderr
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   vfprintf(stderr, fmt, argptr);   // Write to stderr
   va_end(argptr);                  // Close va_ functions

   if( opt_hcdm ) {                 // If Hard Core Debug Mode
     va_start(argptr, fmt);
     ::pub::debugging::vtraceh(fmt, argptr);
     va_end(argptr);
   }
}

//----------------------------------------------------------------------------
// Subroutine: make_dir, insure directory exists
//----------------------------------------------------------------------------
static void make_dir(std::string path) // Insure directory exists
{
   struct stat info;
   int rc= stat(path.c_str(), &info);
   if( rc != 0 ) {
     rc= mkdir(path.c_str(), DIR_MODE);
     if( rc )
       Editor::failure(std::string("Cannot create ") + path);
   }
}

//----------------------------------------------------------------------------
// Subroutine: make_file, insure file exists
//----------------------------------------------------------------------------
static void make_file(std::string name, const char* data) // Insure file exists
{
   struct stat info;
   int rc= stat(name.c_str(), &info);
   if( rc != 0 ) {
     FILE* f= fopen(name.c_str(), "wb"); // Open the file
     if( f == nullptr )             // If open failure
       Editor::failure(std::string("Cannot create ") + name);

     size_t L0= strlen(data);
     size_t L1= fwrite(data, 1, L0, f);
     rc= fclose(f);
     if( L0 != L1 || rc )
       Editor::failure(std::string("Write failure: ") + name);
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       configure
//
// Purpose-
//       Load the configuration.
//
//----------------------------------------------------------------------------
static void
   configure( void )                // Load configuaration
{  // TODO: Load from control file
   using namespace editor;

   const char* env= getenv("HOME"); // Get HOME directory
   if( env == nullptr )
     Editor::failure("No HOME directory");
   HOME= env;

   // If required, create "$HOME/.config/uuid/" + UUID + "/.README.txt"
   std::string S= HOME + "/.config";
   make_dir(S);
   S += "/uuid";
   make_dir(S);
   S += std::string("/") + UUID;
   make_dir(S);
   autosave_dir= S;

   S += std::string("/.README.txt");
   make_file(S, README_text);

   // Parse the configuration file
   // TODO: NOT CODED YET

   // Set AUTOSAVE subdirectory
   env= getenv("AUTOSAVE");         // Get AUTOSAVE directory
   if( env )
     autosave_dir= env;

   // Look for *AUTOSAVE* files in AUTOSAVE subdirectory
   pub::Fileman::Path path(autosave_dir);
   pub::Fileman::File* file= path.list.get_head();
   if( file == nullptr )
     Editor::failure(std::string("AUTOSAVE directory(") + autosave_dir
                    + ") empty");

   size_t L= strlen(AUTOSAVE);
   while( file ) {
     if( strcmp(AUTOSAVE, file->name.substr(0, L).c_str()) == 0 )
       Editor::failure(std::string("File exists: ") + autosave_dir
                      + "/" + file->name);

     file= file->get_next();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::Editor
//
// Purpose-
//       Constructor
//
//----------------------------------------------------------------------------
   Editor::Editor(                  // Constructor
     int               argi,        // Argument index
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   using namespace editor;

   if( opt_hcdm )
     debugh("Editor::Editor\n");

   // Initialize singleton
   {{ std::lock_guard<decltype(mutex)> lock(mutex);
      if( singleton ) throw "Multiple Editors"; // Enforce singleton
      singleton= true;
   }}

   //-------------------------------------------------------------------------
   // Initialize configuration options
   configure();

   // Allocate initial textPool
   textPool.fifo(new EdPool(EdPool::MIN_SIZE));

   // Allocate Device
   device= new xcb::Device();

   // Allocate sub-objects
   mark= new EdMark();              // Mark handler
   text= new EdText();              // Text (window) handler

   //-------------------------------------------------------------------------
   // Load the text files
   for(int i= argi; i<argc; i++) {
     insert_file(argv[i]);
   }
   if( file_list.get_head() == nullptr ) // Always have something
     insert_file(nullptr);          // Even if it's an empty file

   // Select-a-Config ========================================================
   if( opt_test ) {                 // If optional test selected
     std::string test= opt_test;    // The test name (For string == function)
     if( test == "miscwindow" ) { // EdText overlayed with EdMisc
       // Result: EdMisc over EdText window, as expected
       //   Device->EdText->EdMisc
       window= new EdMisc(text, "Misc00", 64, 64);
       device->insert(text);

     } else if( test == "textwindow" ) { // EdText (only)
       device->insert(text);

     } else if( test == "bot-only" ) { // EdText visible
       // Result: EdMisc only appears after screen enlarged
       //   Device->Row->(EdText,EdMisc)
       // PROBLEM: CLOSE Window gets SIGABRT, unable to diagnose
       //          (only occurs after screen enlarged)
       xcb::RowLayout* row= new xcb::RowLayout(device, "Row");
       window= new EdMisc(nullptr, "Bottom", 64, 64);
       row->insert( text );
       row->insert( window );

     } else if( test == "top-only" ) { // (small horizontal window)
       // Result: EdText only appears after screen enlarged
       //   Device->Row->(Top,EdText)
       //   (No problem with CLOSE Window)
       xcb::RowLayout* row= new xcb::RowLayout(device, "Row");
       window= new EdMisc(nullptr, "Top", 64, 14);
       row->insert( window );
       row->insert( text );         // Not visible

     } else if( test == "left-only" ) { // (small vertial window)
       // Result: EdText only appears after screen enlarged
       //   Device->Col->(EdMisc,EdText)
       //   (CLOSE Window checkstop: Bad Window when closing window)
       xcb::ColLayout* col= new xcb::ColLayout(device, "Col");
       window= new EdMisc(nullptr, "Left", 14, 64);
       col->insert(window);         // Apparently visible
       col->insert(text);           // Not visible

     } else if( test == "layout" ) { // (small horizontal window)
       // Result: EdText does not appear even after screen enlarged
       //   (No expose events. EdText parent "Left")
       //   Device->Row->(Top,Col->(Left,EdText),Bottom)
       //   (CLOSE Window OK, No EdText so no Ctrl-Q)
       xcb::RowLayout* row= new xcb::RowLayout(device, "Row");
       window= new EdMisc(nullptr, "Top", 64, 14);
       row->insert( window );

       xcb::ColLayout* col= new xcb::ColLayout(row, "Col"); // (Row->insert(col))
       if( false ) row->insert( col ); // (Tests duplicate insert)
       col->insert( new EdMisc(nullptr, "Left", 14, 64) );
       col->insert( text );

       row->insert( new EdMisc(nullptr, "Bottom", 64, 14));

     } else {
       errorf("Test(%s) not available\n", opt_test);
       exit(EXIT_FAILURE);
     }

     errorf("Test(%s) selected\n", opt_test);
   } else {                         // Default: textwindow ==================
     device->insert(text);
   }
   // Select-a-Config ========================================================
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::~Editor
//
// Purpose-
//       Destructor
//
//----------------------------------------------------------------------------
   Editor::~Editor( void )          // Destructor
{
   using namespace editor;

   // Remove and delete Files
   for(;;) {
     EdFile* file= file_list.remq();
     if( file == nullptr )
       break;

     delete file;
   }

   // Remove and delete storage pools
   for(;;) {
     EdPool* pool= textPool.remq();
     if( pool == nullptr )
       break;

     delete pool;
   }

   for(;;) {
     EdPool* pool= filePool.remq();
     if( pool == nullptr )
       break;

     delete pool;
   }

   // Delete allocated objects
   delete window;
   delete mark;
   delete text;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::debug
//
// Purpose-
//       Debugging display (modified as needed)
//
//----------------------------------------------------------------------------
void
   Editor::debug(                   // Debugging display
     const char*       info)        // Informational text
{
   debugf("Editor::debug(%s)\n", info ? info : "");

   // Debugging duplicate entries in file list
   debugf("..file_list(%p,%p)\n", editor::file_list.get_head()
         , editor::file_list.get_tail());
   unsigned count= 0;
   for(EdFile* file= editor::file_list.get_head(); file; file= file->get_next()) {
     debugf("[%2d] '%s'\n", count++, file->name.c_str());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::failure
//
// Purpose-
//       Write error message and exit
//
//----------------------------------------------------------------------------
void
   Editor::failure(                 // Write error message and exit
     std::string       mess)        // (The error message)
{  fprintf(stderr, "%s\n", mess.c_str()); exit(EXIT_FAILURE); }

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_done
//
// Purpose-
//       Handle DONE function: Safely exit all EdFiles
//
// Implementation note-
//       TODO: NOT (PROPERLY) CODED YET
//
//----------------------------------------------------------------------------
int                                 // Return code, 0OK
   editor::do_done( void )          // Handle DONE
{  device->operational= false; return 0; } // TODO: Safely exit

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_test
//
// Purpose-
//       Bringup test. Currently flips window visibility.
//
//----------------------------------------------------------------------------
void
   editor::do_test( void )          // Bringup test
{
   if( window && window->get_parent() ) { // If test window active
     fprintf(stderr, "editor::do_test\n");
     if( window->state & xcb::Window::WS_VISIBLE )
       window->hide();
     else
       window->show();
     device->draw();
   } else
     fprintf(stderr, "editor::do_test NOT CONFIGURED\n");
}

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
   editor::allocate(                // Get (immutable) text
     size_t            length)      // Of this length (includes '\0' delimit)
{
   char* text= nullptr;             // Not allocated (yet)
   EdPool* pool= textPool.get_head(); // Get text pool
   if( pool == nullptr ) {          // If no pool yet
     pool= new EdPool(EdPool::MIN_SIZE); // Create one now
     textPool.lifo(pool);           // And use it
   }

   text= pool->allocate(length);
   if( text == nullptr ) {
     if( length > (EdPool::MIN_SIZE / 8) ) { // If large allocation
       pool= new EdPool(length);    // All filePools are 100% allocated
       text= pool->allocate(length);
       filePool.lifo(pool);
     } else {                       // If small allocation
       for(pool= pool->get_next(); pool; pool= pool->get_next() ) {
         text= pool->allocate(length); // Try all textPools
         if( text )
           break;
       }

       if( text == nullptr ) {      // If a new EdPool is required
         if( opt_hcdm )
           debugh("Editor.allocate(%zd) New pool\n", length);
         pool= new EdPool(EdPool::MIN_SIZE);
         text= pool->allocate(length);
         textPool.lifo(pool);
       }
     }
   }

   if( opt_hcdm && opt_verbose > 1 )
     debugf("%p= editor::allocate(%zd)\n", text, length);
   return text;
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::insert_file
//
// Purpose-
//       Insert file(s) onto the file list
//
//----------------------------------------------------------------------------
#if defined(_OS_CYGWIN)
  #define wildstrcmp pub::utility::wildchar::strcasecmp
#else
  #define wildstrcmp pub::utility::wildchar::strcmp
#endif

EdFile*                             // The last file inserted
   editor::insert_file(             // Insert file(s) onto the file list
     const char*       _name)       // The file name (file wildcards allowed)
{  if( opt_hcdm )
     debugf("editor::insert_file(%s)\n", _name);

   using namespace pub::Fileman;    // Using Fileman objects

   EdFile* last= nullptr;           // The last EdFile inserted
   if( _name == nullptr )           // If missing parameter
     _name= "unnamed.txt";          // Use default name

   // Match existing file name(s)
   Name name(_name);
   std::string error= name.resolve(); // Remove link qualifiers
   if( error != "" ) {
     fprintf(stderr, "File(%s) %s\n", _name, error.c_str());
     return nullptr;
   }

   {{{{ // Search directory, handling all wildcard file name matches
     bool found= false;
     Path path(name.path_name);     // (Temporary)
     for(File* file= path.list.get_head(); file; file= file->get_next() ) {
       if( wildstrcmp(name.file_name.c_str(), file->name.c_str()) == 0 ) {
         std::string fqn= name.path_name + "/" + file->name;
         Name wild(fqn);            // The wildcard match name
         wild.resolve();            // (Resolve the name, which may be a link)

         found= true;
         bool is_dup= false;
         for(EdFile* dup= file_list.get_head(); dup; dup= dup->get_next()) {
           if( dup->name == wild.name ) { // If file already in file_list
             last= dup;
             is_dup= true;
             break;
           }
         }

         if( !is_dup ) {
           if( S_ISREG(wild.st.st_mode) ) {
             last= new EdFile(wild.name.c_str());
             file_list.fifo(last);
           } else if( S_ISDIR(wild.st.st_mode) ) { // TODO: NOT CODED YET
             fprintf(stderr, "File(%s) Directory: %s\n", _name, fqn.c_str());
           } else {
             fprintf(stderr, "File(%s) Unusable: %s\n", _name, fqn.c_str());
           }
         }
       }
     }
     if( found )
       return last;
   }}}}

   // Non-existent file
   last= new EdFile(name.name.c_str());
   file_list.fifo(last);
   return last;
}
#undef wildstrcmp

//----------------------------------------------------------------------------
//
// Method-
//       editor::key_to_name
//
// Purpose-
//       BRINGUP: Convert xcb_keysym_t to its name. (TODO: REMOVE)
//
//----------------------------------------------------------------------------
const char*
   editor::key_to_name(xcb_keysym_t key) // Convert xcb_keysym_t to name
{
   if( key >= 0x0020 && key <= 0x007f ) { // If text key
     static char buffer[2];
     buffer[0]= char(key);
     buffer[1]= '\0';
     return buffer;
   }

   switch( key ) {                  // Handle control key
     case XK_BackSpace:
       return "BackSpace";

     case XK_Tab:
       return "Tab";

     case XK_ISO_Left_Tab:
       return "Left_Tab";

     case XK_Return:
       return "Return";

     case XK_Scroll_Lock:
       return "Scroll_Lock";

     case XK_Escape:
       return "Escape";

     case XK_Delete:
       return "Delete";

     case XK_Insert:
       return "Insert";

     case XK_Home:
       return "Home";

     case XK_End:
       return "End";

     case XK_Menu:
       return "Menu";

     case XK_Left:
       return "Left arrow";

     case XK_Up:
       return "Up arrow";

     case XK_Right:
       return "Right arrow";

     case XK_Down:
       return "Down arrow";

     default:
       break;
   }

   return "???";
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::remove_file
//
// Purpose-
//       Unconditionally remove file from the file list, discarding changes
//
//----------------------------------------------------------------------------
void
   editor::remove_file(             // Remove file from the file list
     EdFile*           file)        // The file to remove
{
   EdFile* next= file->get_prev();
   if( next == nullptr ) {
     next= file->get_next();
     if( next == nullptr )          // If no more files
       device->operational= false;  // No need to stay around
   }

   if( next ) {                     // (If next == nullptr, leave file)
     file_list.remove(file, file);  // Remove the file from the file list
     delete file;                   // And delete it

     text->activate(next);
     text->draw();
   }
}

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
   editor::set_font(                // Set the font
     const char*       font)        // To this font name
{  return text->set_font(font); }   // (Defer to EdText)

//----------------------------------------------------------------------------
// editor::Virtual Thread simulation methods
//----------------------------------------------------------------------------
void
   editor::join( void )
{  }

void
   editor::start( void )
{
   // Initialize the configuration
   device->configure();

   // Set initial file
   text->activate(file_list.get_head());

   // Start the Device
   device->draw();
   device->run();
}
