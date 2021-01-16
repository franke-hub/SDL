//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2021 Frank Eskesen.
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
//       2021/01/11
//
//----------------------------------------------------------------------------
#include <mutex>                    // For std::mutex, std::lock_guard

#include <assert.h>                 // For assert
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <com/istring.h>            // For stristr
#include <unistd.h>                 // For close, ftruncate
#include <sys/stat.h>               // For stat
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include <pub/Debug.h>              // For Debug, namespace pub::debugging
#include <pub/Fileman.h>            // For namespace pub::fileman
#include <pub/Signals.h>            // For pub::signals
#include <pub/Thread.h>             // For pub::Thread::sleep
#include <pub/Trace.h>              // For pub::Trace

#include "Xcb/Device.h"             // For xcb::Device
#include "Xcb/Font.h"               // For xcb::Font
#include <Xcb/Keysym.h>             // For xcb_keycode_t symbols
#include "Xcb/Layout.h"             // For xcb::Layout
#include "Xcb/Widget.h"             // For xcb::Widget, our base class
#include "Xcb/Window.h"             // For xcb::Window

#include "Config.h"                 // For namespace config
#include "Editor.h"                 // For Editor (Implementation class)
#include "EdFile.h"                 // For EdFile, EdLine, EdPool, ...
#include "EdHist.h"                 // For EdHist
#include "EdMark.h"                 // For EdMark
#include "EdMisc.h"                 // For EdMisc TODO: REMOVE
#include "EdPool.h"                 // For EdPool
#include "EdText.h"                 // For EdText
#include "EdView.h"                 // For EdView

using namespace config;             // For namespace config (opt_*)
using namespace editor;             // For namespace editor
using namespace pub::debugging;     // For debugging

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= true                // Extra bringup diagnostics?
,  USE_HCDM_FILE_DEBUG= true        // (As opposed to name-only file info)
}; // Compilation controls

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
EdText*                editor::text= nullptr; // The Text Window

pub::List<EdFile>      editor::file_list; // The list of EdFiles
EdFile*                editor::file= nullptr; // The current File object

EdMark*                editor::mark= nullptr; // The Mark Handler
EdView*                editor::data= nullptr; // The data view
EdHist*                editor::hist= nullptr; // The history view
EdView*                editor::view= nullptr; // The active view

std::string            editor::locate_string; // The locate string
std::string            editor::change_string; // The change string

pub::List<EdPool>      editor::filePool; // File allocation EdPool
pub::List<EdPool>      editor::textPool; // Text allocation EdPool

// Screen controls -----------------------------------------------------------
xcb_rectangle_t        editor::geom= {1030, 0, 80, 50}; // The screen geometry

// Search controls -----------------------------------------------------------
int                    editor::autowrap= false;
int                    editor::case_sensitive= false;
int                    editor::direction= 0;

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static std::mutex      mutex;       // Singleton mutex
static int             singleton= 0; // Singleton control

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

   // Allocate initial textPool
   textPool.fifo(new EdPool(EdPool::MIN_SIZE));

   // Allocate editor namespace objects
   text= new EdText();              // Text (window) handler
   data= new EdView();              // Data view
   hist= new EdHist();              // History view
   mark= new EdMark();              // Mark handler
   view= hist;                      // (Initial view)

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
       xcb::RowLayout* row= new xcb::RowLayout(device, "Row");
       window= new EdMisc(nullptr, "Bottom", 64, 64);
       row->insert( text );
       row->insert( window );

     } else if( test == "top-only" ) { // (small horizontal window)
       // Result: No window visible
       //   Device->Row->(Top,EdText)
       xcb::RowLayout* row= new xcb::RowLayout(device, "Row");
       window= new EdMisc(nullptr, "Top", 64, 14);
       row->insert( window );
       row->insert( text );         // Not visible

     } else if( test == "left-only" ) { // (small vertial window)
       // Result: No window visible
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
   delete text;
   delete data;
   delete hist;
   delete mark;
   delete window;
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Editor::debug(                   // Debugging display
     const char*       info)        // Associated info
{
   debugf("Editor::debug(%s)\n", info ? info : "");
   debugf("..device(%p) window(%p) text(%p)\n", device, window, text);
   debugf("..file_list(%p,%p) file(%p)\n"
         , file_list.get_head(), file_list.get_tail(), file);
   for(EdFile* file= file_list.get_head(); file; file= file->get_next()) {
     if( USE_HCDM_FILE_DEBUG )      // If hard core file debugging
       file->debug(info);
     else                           // If name only file debugging
       debugf("..[%p] '%s'\n", file, file->name.c_str());
   }
   debugf("..mark(%p) data(%p) hist(%p) view(%p)\n", mark, data, hist, view);
   debugf("..locate[%s] change[%s]\n"
         , locate_string.c_str(), change_string.c_str());

   size_t size; size_t used;        // Total size, Total used
   size= used= 0;
   debugf("..filePool[%p,%p]\n", filePool.get_head(), filePool.get_tail());
   for(EdPool* pool= filePool.get_head(); pool; pool= pool->get_next()) {
     debugf("..[%p] used(%'8zu) size(%'8zu)\n", pool
          , pool->get_used(), pool->get_size());
     size += pool->get_size();
     used += pool->get_used();
   }
   debugf("..****TOTAL**** used(%'8zu) size(%'8zu)\n", used, size);

   size= used= 0;
   debugf("..textPool[%p,%p]\n", textPool.get_head(), textPool.get_tail());
   for(EdPool* pool= textPool.get_head(); pool; pool= pool->get_next()) {
     debugf("..[%p] used(%'8zu) size(%'8zu)\n", pool
           , pool->get_used(), pool->get_size());
     size += pool->get_size();
     used += pool->get_used();
   }
   debugf("..****TOTAL**** used(%'8zu) size(%'8zu)\n", used, size);
}

//----------------------------------------------------------------------------
//
// Method-
//       Editor::alertf
//
// Purpose-
//       Debugging alert
//
//----------------------------------------------------------------------------
void
   Editor::alertf(                  // Debugging alert
     const char*       fmt,         // The PRINTF format string
                       ...)         // The remaining arguments
{
   va_list             argptr;      // Argument list pointer

   va_start(argptr, fmt);           // Initialize va_ functions
   std::string S= pub::utility::to_stringv(fmt, argptr);
   va_end(argptr);                  // Close va_ functions

   debugf("%s\n", S.c_str());
   Config::debug(S.c_str());
   Config::trace(".BUG", __LINE__, "Config.cpp");
   Config::backtrace();

   editor::put_message(S.c_str(), EdMess::T_MESS);
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_change
//
// Purpose-
//       Change next occurance of string.
//
// Implementation note-
//       REDO not required. The line is changed, but not committed.
//
//----------------------------------------------------------------------------
const char*                         // Return message, nullptr if OK
   editor::do_change( void )        // Change next occurance of string
{
   const char* error= do_locate(0); // First, locate the string
   if( error ) return error;        // (If cannot locate)

   // The string has been found and the line activated
   size_t column= data->col_zero + data->col; // The current column
   size_t length= locate_string.length();
   data->active.replace_text(column, length, change_string.c_str());
   text->draw();                    // (Only active line redraw required)
   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_exit
//
// Purpose-
//       (Safely) remove a file from the file list.
//
//----------------------------------------------------------------------------
void
   editor::do_exit( void )          // Safely remove a file from the file list
{
   if( file->damaged )
     put_message("File damaged");
   else if( file->changed )
     put_message("File changed");
   else
     remove_file();
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_history
//
// Purpose-
//       Invert history view.
//
//----------------------------------------------------------------------------
void
   editor::do_history( void )       // Invert history view
{
   if( view == hist ) {
     data->activate();
     text->draw_cursor();
   } else {
     text->undo_cursor();
     hist->activate();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::do_locate
//
// Purpose-
//       Locate next occurance of string.
//
//----------------------------------------------------------------------------
const char*                         // Return message, nullptr if OK
   editor::do_locate(               // Locate next
     int               offset)      // Use offset 0 for locate_change
{
   data->commit();                  // Commit the active line

   const char* S= locate_string.c_str();

   //-------------------------------------------------------------------------
   // Locate in the active line
   EdLine* line= data->cursor;
   size_t column= data->col_zero + data->col + offset;
   if( (line->flags & EdLine::F_PROT) == 0 ) { // If line is not protected
     const char* C= data->active.get_buffer(column); // Remaining characters
     const char* M= stristr(C, S);
     if( M != nullptr ) {
       data->activate();
       column += M - C;
       text->move_cursor_H(column);
       text->draw_info();
       return nullptr;
     }
   }

   //-------------------------------------------------------------------------
   // Search remainder of file
   for(;;) {
     line= line->get_next();
     if( line == nullptr )
       return "Not found";

     if( (line->flags & EdLine::F_PROT) == 0 ) {
       const char* M= stristr(line->text, S);
       if( M != nullptr ) {
         text->activate(line);
         text->move_cursor_H(M - line->text);
         return nullptr;
       }
     }
   }
}

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
     Config::errorf("editor::do_test\n");
     if( window->state & xcb::Window::WS_VISIBLE )
       window->hide();
     else
       window->show();
     device->draw();
   } else {
     if( false ) {
       debugf("SEGFAULT TEST!!\n");
       printf("%p\n", window->get_parent()->get_parent()); // (SEGFAULT TEST)
     }
     Config::errorf("editor::do_test NOT CONFIGURED\n");
   }
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
     if( length > EdPool::MIN_SIZE ) { // If large allocation
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
     traceh("%p= editor::allocate(%zd)\n", text, length);
   return text;
}

const char*                         // The (immutable) text
   editor::allocate(                // Get (immutable) text
     const char*       source)      // Source (mutable) text
{
   if( *source == '\0' )            // If empty source
     return "";                     // (Immutable) empty copy

   size_t L= strlen(source);        // Source length
   if( source[L-1] == ' ' )         // (If UNEXPECTED trailing blanks)
     fprintf(stderr, "%4d Editor UNEXPECTED\n", __LINE__); // TODO: REMOVE
   char* copy= allocate(L+1);       // Include trailing '\0' in string
   strcpy(copy, source);            // Duplicate the string
   return copy;
}

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
   editor::exit( void )             // Unconditional editor (normal) exit
{  device->operational= false; }

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
     traceh("editor::insert_file(%s)\n", _name);

   using namespace pub::fileman;    // Using fileman objects

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
static char buffer[8];
static const char* F_KEY= "123456789ABCDEF";
   if( key >= 0x0020 && key <= 0x007f ) { // If text key
     buffer[0]= char(key);
     buffer[1]= '\0';
     return buffer;
   }

   if( key >= XK_F1 && key <= XK_F12 ) { // If function key
     buffer[0]= 'F';
     buffer[1]= F_KEY[key - XK_F1];
     buffer[2]= '\0';
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
//       editor::put_message
//
// Purpose-
//       Add message to file's message list
//
//----------------------------------------------------------------------------
void
   editor::put_message(             // Write message
     const char*       _mess,       // Message text
     int               _type)       // Message mode
{  if( file )
     file->put_message(_mess, _type);
   else
     debugh("Editor::put_message(%s)\n", _mess);
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
   editor::remove_file( void )      // Remove active file from the file list
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
     editor::file= nullptr;         // (Don't reference it any more)

     text->activate(next);
     text->draw();
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::set_option
//
// Purpose-
//       Set an option
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   editor::set_option(              // Set
     const char*       name,        // This option name to
     const char*       value)       // This option value
{
   if( strcmp(name, "geometry") == 0 ) {
     return 0;
   }

   if( strcmp(name, "autowrap") == 0 ) {
     return 0;
   }

   if( strcmp(name, "case") == 0 ) {
     return 0;
   }

   if( strcmp(name, "direction") == 0 ) {
     return 0;
   }

   (void)name; (void)value; return -1; // NOT CODED YET
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::un_changed
//
// Purpose-
//       If any file has changed, activate it
//
//----------------------------------------------------------------------------
bool                                // TRUE if editor in unchanged state
   editor::un_changed( void )       // If any file changed, activate it
{
   data->commit();                  // Commit the active line

   if( file->changed ) {            // If this file has changed
     put_message("File changed");
     return false;                  // (It's already active)
   }

   for(EdFile* file= file_list.get_head(); file; file= file->get_next()) {
     if( file->changed && !file->damaged ) { // If changed and changeable
       put_message("File changed");
       text->activate(file);
       text->draw();
       return false;
     }
   }

   return true;
}

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
   text->show();                    // (Set position fails unless visible)
   text->grab_mouse();
   text->flush();
   device->run();
}
