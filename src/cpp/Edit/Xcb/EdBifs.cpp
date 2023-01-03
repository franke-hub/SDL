//----------------------------------------------------------------------------
//
//       Copyright (C) 2020-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdBifs.cpp
//
// Purpose-
//       Editor: Built in functions
//
// Last change date-
//       2023/01/02
//
//----------------------------------------------------------------------------
#include <sys/stat.h>               // For stat

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Fileman.h>            // For pub::fileman::Name::get_file_name()
#include <pub/Tokenizer.h>          // For pub::Tokenizer

#include "Config.h"                 // For namespace config
#include "Editor.h"                 // For Editor Globals
#include "EdFile.h"                 // For EdFile, EdLine
#include "EdHist.h"                 // For EdHist
#include "EdMark.h"                 // For EdMark
#include "EdTerm.h"                 // For EdTerm

using namespace pub::debugging;     // For debugging
using namespace config;             // For opt_* controls

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  TAB= 8                           // TAB spacing (2**N)
,  USE_BRINGUP= false               // Extra bringup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
// Internal tables
//----------------------------------------------------------------------------
struct Name_addr {                  // Name address pair
const char*            name;        // The symbol name
int*                   addr;        // The symbol address
}; // struct Name_addr

struct Name_value {                 // Name value pair
const char*            name;        // The value name
int                    value;       // The associated value
}; // struct Name_value

static Name_value      bool_value[]= // Boolean value table
{ {"true",             1}
, {"false",            0}
, {"1",                1}
, {"0",                0}
, {"on",               1}
, {"off",              0}
, {"yes",              1}
, {"no",               0}
, {nullptr,            -1}          // Not found value
}; // bool_value[]

static Name_value      mode_value[]= // Mode value table
{ {"dos",              EdFile::M_DOS}
, {"unix",             EdFile::M_UNIX}
, {nullptr,            -1}          // Not found value
}; // mode_value[]

static Name_addr       true_addr[]= // Boolean symbols, default true
{ {"prior",            (int*)&editor::locate_back} // Short symbol names
, {"case",             (int*)&editor::locate_case}
, {"wrap",             (int*)&editor::locate_wrap}
, {"locate.prior",     (int*)&editor::locate_back} // Official symbol names
, {"locate.mixed",     (int*)&editor::locate_case}
, {"locate.wrap",      (int*)&editor::locate_wrap}
, {"reverse",          (int*)&editor::locate_back} // Symbol name aliases
, {"mixed",            (int*)&editor::locate_case}
, {"autowrap",         (int*)&editor::locate_wrap}
, {"hidden",           (int*)&config::USE_MOUSE_HIDE} // Controls
, {"mouse_hide",       (int*)&config::USE_MOUSE_HIDE}
, {"use_mouse_hide",   (int*)&config::USE_MOUSE_HIDE}
, {nullptr,            nullptr}     // Not found address
}; // true_addr[]

//----------------------------------------------------------------------------
//
// Data area-
//       command_desc
//
// Purpose-
//       The list of built-in commands.
//
//----------------------------------------------------------------------------
typedef const char* Function(char*); // The command processor function

struct Command_desc {               // The Command descriptor item
Function*              func;        // The command function
const char*            name;        // The command name
const char*            desc;        // The command description
};                     // The Command descriptor

static const char* command_bot(char*); // Forward references: commands
static const char* command_change(char*);
static const char* command_cmd(char*);
static const char* command_debug(char*);
static const char* command_detab(char*);
static const char* command_edit(char*);
static const char* command_exit(char*);
static const char* command_file(char*);
static const char* command_find(char*);
static const char* command_get(char*);
static const char* command_list(char*);
static const char* command_locate(char*);
// static const char* command_margins(char*);
static const char* command_quit(char*);
static const char* command_save(char*);
static const char* command_set(char*);
static const char* command_sort(char*);
// static const char* command_tabs(char*);
static const char* command_top(char*);
static const char* command_view(char*);

static const Command_desc
                       command_desc[]= // The Command descriptor list
{  {command_bot,    "BOT",     "Bottom of file"}
,  {command_change, "C",       "Change"}
,  {command_cmd,    "CMD",     "(System) command"}
,  {command_debug,  "D",       "Alias for DEBUG"}
,  {command_debug,  "DEBUG",   "Debugging display"}
,  {command_detab,  "DETAB",   "Convert tabs to spaces"}
,  {command_edit,   "E",       "Alias for EDIT"}
,  {command_edit,   "EDIT",    "Edit file(s)"}
,  {command_exit,   "EXIT",    "(Safe) Exit" }
,  {command_find,   "FI",      "Find (column 1)"} // (Alias)
,  {command_file,   "FILE",    "Write and close file"}
,  {command_find,   "FIND",    "Find (column 1)"}
,  {command_get,    "GET",     "Append file"}
,  {command_locate, "L",       "Locate (forward)"}
,  {command_list,   "LIST",    "List commands"}
// {command_margins,"MARGINS", "Set margins"}
,  {command_quit,   "QUIT",    "(Unconditionally) close file"}
,  {command_save,   "SAVE",    "Write file"}
,  {command_set,    "SET",     "Set option= value"}
,  {command_sort,   "SORT",    "{-f} Sort file list"
                               " (using fully-qualified name)"}
// {command_tabs,   "TABS",    "Set tabs"}
,  {command_top,    "TOP",     "Top of File"}
,  {command_view,   "V",       "Alias for VIEW"}
,  {command_view,   "VIEW",    "Edit file(s) in read/only mode"}

// Spelling errors/typos
,  {command_list,   "",     nullptr} // Misspelled commands follow
,  {command_save,  "SAE",   nullptr} // (SAVE)
,  {command_save,  "SAVAE", nullptr} // (SAVE)
,  {command_save,  "SAVCE", nullptr} // (SAVE)
,  {command_save,  "SAVVE", nullptr} // (SAVE)
,  {command_save,  "SVAE",  nullptr} // (SAVE)
,  {nullptr     ,  nullptr, nullptr} // End of list delimiter
};

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
//----------------------------------------------------------------------------
//
// Subroutine-
//       find_addr
//
// Purpose-
//       Locate a symbol address
//
//----------------------------------------------------------------------------
static int*                         // The symbol address
   find_addr(                       // Find symbol address
     const char*       name,        // The symbol name
     Name_addr*        addr)        // The address table
{
   while( addr->name ) {            // Search valid names
     if( strcasecmp(name, addr->name) == 0 )
       break;
     addr++;
   }

   return addr->addr;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       find_value
//
// Purpose-
//       Locate a symbol's value
//
//----------------------------------------------------------------------------
static int                          // The symbol's value
   find_value(                      // Find symbol value
     const char*       name,        // The symbol name
     Name_value*       addr)        // The value table
{
   while( addr->name ) {            // Search valid names
     if( strcasecmp(name, addr->name) == 0 )
       break;
     addr++;
   }

   return addr->value;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       write_file
//
// Purpose-
//       Write file (with error checking)
//
//----------------------------------------------------------------------------
static const char*                  // Error message, nullptr expected
   write_file(                      // Write file
     char*             parm)        // (Mutable) parameter string
{
   EdFile* file= editor::file;

   if( file->protect )
     return "Read-only";
   if( file->damaged )
     return "Damaged file";

   if( parm ) {                     // If filename specified
     struct stat info;
     int rc= stat(parm, &info);     // Get file information
     if( rc == 0 )                  // If file exists
       return "File exists";

     rc= file->write(parm);         // Write the file
     if( rc )
       return "Write failure";
     return nullptr;                // (File remains changed)
   }

   // Replace the file (even if unchanged)
   int rc= file->write();
   if( rc )
     return "Write failure";

   file->reset();
   return nullptr;
}

//----------------------------------------------------------------------------
//
// Subroutines-
//       Editor::commands
//
// Purpose-
//       Editor built-in commands
//
//----------------------------------------------------------------------------
static const char*                  // Error message, nullptr expected
   command_bot(char*)               // Bottom command
{
   using namespace editor;          // For editor::(data, hist, term)
   data->col_zero= data->col= 0;
   term->activate(file->line_list.get_tail());
   hist->activate();                // (Remain in command mode)
   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_change(                  // Change command
     char*             parm)        // (Mutable) parameter string
{
   if( parm == nullptr )
     return "Missing parameter";

   if( editor::file->protect )      // Do not modify protected files
     return "Read/only";

   int D= (unsigned char)parm[0];   // The string delimiter
   parm++;
   char* locate= parm;              // The locate string origin
   char* C= strchr(parm, D);
   if( C == nullptr || C == parm )
     return "Invalid parameter";

   *C= '\0';
   parm= C + 1;
   char* change= parm;              // The change string origin
   C= strchr(parm, D);
   if( C == nullptr )
     C= strchr(parm, '\0');

   if( *C != '\0' && *(C+1) != '\0' ) // If delimiter is not final character
       return "Invalid parameter";
   *C= '\0';

   editor::locate_string= locate;
   editor::change_string= change;
   return editor::do_change();
}

static const char*                  // Error message, nullptr expected
   command_cmd(                     // Command command
     char*             parm)        // (Mutable) parameter string
{
   int rc= system(parm);
   if( rc )
     return "Command failed";

   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_comment( void )          // Comment command (no parameter)
{
   editor::hist->activate();        // Handle it
   return nullptr;                  // (No error)
}

static const char*                  // Error message, nullptr expected
   command_debug(                   // Debug command
     char*             parm)        // (Mutable) parameter string
{
   if( parm == nullptr || strcasecmp(parm, "all") == 0 )
     Config::debug("command");
   else if( strcasecmp(parm, "edit") == 0 )
     Editor::debug("command");
   else if( strcasecmp(parm, "file") == 0 )
     editor::file->debug("command");
   else if( strcasecmp(parm, "font") == 0 )
     config::font->debug("command");
   else if( strcasecmp(parm, "lines") == 0 )
     editor::file->debug("lines");
   else if( strcasecmp(parm, "mark") == 0 )
     editor::mark->debug("command");
   else if( strcasecmp(parm, "term") == 0 )
     editor::term->debug("command");
   else if( strcasecmp(parm, "view") == 0 ) {
     editor::data->debug("command");
     editor::hist->debug("command");
   } else
     return "Invalid command";
   debug_flush();

   editor::hist->activate();
   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_detab(char*)             // De-tab command
{
   EdView* data= editor::data;
   EdFile* file= editor::file;
   EdLine* top= editor::term->head; // Save the head line
   EdLine* cur= data->cursor;       // Save the cursor line

   if( file->protect )              // Do not modify protected files
     return "Read/only";

   for(EdLine* line= file->line_list.get_head(); line; line= line->get_next()) {
     Active* active= nullptr;
     const char* text= line->text;  // Using the text line
     const char* tabs= strchr(text, '\t'); // Locate the first tab
     while( tabs ) {                // Remove tabs from the line
       if( active == nullptr ) {
         active= &data->active;
         active->reset("");
         data->cursor= line;
       }

       size_t L= tabs - text;       // Length of text
       active->append_text(text, L); // Append the text
       L= active->get_used();
       L += TAB;
       L &= ~(TAB - 1);
       active->fetch(L-1);
       if( L > active->get_used() )
         active->append_text(" ");

       text= tabs + 1;            // Skip past this tab
       tabs= strchr(text, '\t');  // Locate the next tab
     }

     if( active ) {               // If tabs found
       active->append_text(text); // Append trailing text
       data->commit();
       if( line == cur )
         cur= data->cursor;
       if( line == top )
         editor::term->head= data->cursor;
     }
   }

   // Reset the active line and redraw (whether or not needed)
   data->cursor= cur;
   data->active.reset(cur->text);
   editor::term->draw();

   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_edit(                    // Edit command
     char*             parm)        // (Mutable) parameter string
{
   if( parm == nullptr )
     return "Missing parameter";

   typedef pub::Tokenizer Tokenizer;
   typedef Tokenizer::Iterator Iterator;

   editor::last= editor::file;      // Insert after current file
   Tokenizer t(parm);
   for(Iterator i= t.begin(); i != t.end(); i.next() ) {
     editor::insert_file( i().c_str() );
   }

   if( editor::file != editor::last )
     editor::term->activate(editor::last);
   editor::hist->activate();

   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_exit(char*)              // Exit command
{
   if( editor::un_changed() )       // If unchanged, safe to exit
     editor::exit();

   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_file(                    // File command
     char*             parm)        // (Mutable) parameter string
{
   const char* mess= write_file(parm); // Save the file
   if( mess == nullptr )            // If saved
     mess= command_quit(parm);      // Quit the file

   return mess;
}

static const char*                  // Error message, nullptr expected
   command_find(                    // Find command
     char*             parm)        // (Mutable) parameter string
{
   // Leading blanks have been removed, so we need special handling.
   // We use the special character '.' and ignore it if it's first.
   // To find an actual leading '.', use "..".
   if( *parm == '.' )
     parm++;

   return editor::do_find(parm);
}

static const char*                  // Error message, nullptr expected
   command_forward(                 // Forward locate command
     char*             parm)        // (Mutable) parameter string
{
   editor::locate_back= false;      // Forward search

   while( *parm == ' ' )
     parm++;

   return command_locate(parm);
}

static const char*                  // Error message, nullptr expected
   command_get(char*)               // Get command
{
   // SPECIAL CASE: Get after ending line with no delimiter.
   // The no delimiter line CHANGES. It's part of the REDO/UNDO.
   return nullptr;                  // TODO: NOT CODED YET
}

static const char*                  // Error message, nullptr expected
   command_list(char*)              // Display the command list
{
   printf("Command list: (Command names are not case sensitive)\n");
   for(int i= 0; command_desc[i].name[0] != '\0'; ++i) {
     char buffer[32];
     sprintf(buffer, "%s:", command_desc[i].name);
     printf("%-8s %s\n", buffer, command_desc[i].desc);
   }

   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_locate(                  // Locate command (current direction)
     char*             parm)        // (Mutable) parameter string
{
   if( parm == nullptr )
     return "Missing parameter";

   int D= (unsigned char)parm[0];   // The string delimiter
   parm++;
   char* C= strchr(parm, D);
   if( C == nullptr )
     C= strchr(parm, '\0');
   if( C == parm )
     return "Invalid parameter";

   if( *C != '\0' && *(C+1) != '\0' ) // If delimiter is not final character
       return "Invalid parameter";

   editor::locate_string= std::string(parm, C - parm);
   editor::change_string= editor::locate_string;
   return editor::do_locate();
}

static const char*                  // Error message, nullptr expected
   command_number(                  // (Line) number command
     char*             parm)        // (Mutable) parameter string
{
   ssize_t number= 0;

   while( *parm != '\0' ) {
     if( *parm < '0' || *parm > '9' )
       return "Invalid number";

     number *= 10;
     number += *parm - '0';
     if( number < 0 )
       return "Invalid number";

     parm++;
   }

   editor::data->activate();
   editor::term->move_cursor_H(0);
   editor::term->activate(editor::file->get_line(number));

   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_quit(char*)              // (Unconditional) quit command
{
   editor::remove_file();
   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_reverse(                 // Reverse locate command
     char*             parm)        // (Mutable) parameter string
{
   editor::locate_back= true;       // Reverse search

   while( *parm == ' ' )
     parm++;

   return command_locate(parm);
}

static const char*                  // Error message, nullptr expected
   command_save(                    // Save command
     char*             parm)        // (Mutable) parameter string
{  return write_file(parm); }       // Write the file

static const char*                  // Error message, nullptr expected
   command_set(                     // Set command
     char*             parm)        // (Mutable) parameter string
{
   if( parm == nullptr )
     return "Missing parameter";

   typedef pub::Tokenizer Tokenizer;
   typedef Tokenizer::Iterator Iterator;

   Tokenizer t(parm);
   Iterator i= t.begin();
   const char* name= i().c_str();
   const char* value= i.next().remainder();

   if( strcasecmp(name, "mode") == 0 ) {
     int mode= find_value(value, mode_value);
     if( mode >= 0 ) {
       editor::file->set_mode(mode);
       return nullptr;
     }

     return "Invalid mode";
   }

   int* addr= find_addr(name, true_addr);
   if( addr ) {
     int mode= true;
     if( value[0] != '\0' )
       mode= find_value(value, bool_value);
     if( mode >= 0 ) {
       *addr= mode;
       return nullptr;
     }

     return "Invalid value";
   }

   return "Unknown option";
}

static const char*                  // Error message, nullptr expected
   command_sort(char* parm)         // Sort editor::file_list
{
   using pub::fileman::Name;        // File name extractor
   pub::List<EdFile> sort_list;     // The sorted list of EdFiles

   bool opt_full= (parm && strcmp(parm, "-f") == 0);
   for(;;) {                        // Sort the file list
     EdFile* low= editor::file_list.get_head();
     if( low == nullptr )
       break;

     std::string low_name= Name::get_file_name(low->name);
     for(EdFile* file= low->get_next(); file; file= file->get_next()) {
       if( opt_full ) {             // Sort using fully qualified name
         if( file->name < low->name )
           low= file;
       } else {                     // Sort using file name (w/ extension)
         std::string file_name= Name::get_file_name(file->name);
         if( file_name < low_name ) {
           low= file;
           low_name= Name::get_file_name(file_name);
         }
       }
     }

     editor::file_list.remove(low, low);
     sort_list.fifo(low);
   }

   editor::file_list.insert(nullptr, sort_list.get_head(), sort_list.get_tail());

   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_top(char*)               // Top command
{
   using namespace editor;          // For editor::(data, hist, term)
   data->col_zero= data->col= 0;
   term->activate(file->line_list.get_head());
   hist->activate();                // (Remain in command mode)
   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_view(                    // View command
     char*             parm)        // (Mutable) parameter string
{
   if( parm == nullptr )
     return "Missing parameter";

   typedef pub::Tokenizer Tokenizer;
   typedef Tokenizer::Iterator Iterator;

   editor::last= editor::file;      // Insert after current file
   Tokenizer t(parm);
   for(Iterator i= t.begin(); i != t.end(); i.next() ) {
     editor::insert_file(i().c_str(), true);
   }

   if( editor::file != editor::last )
     editor::term->activate(editor::last);
   editor::hist->activate();

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::command
//
// Purpose-
//       Process a command.
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr if none
   editor::command(                 // Process a command
     char*             buffer)      // (MODIFIABLE) command line buffer
{  if( HCDM || opt_hcdm )
     debugf("editor::command(%s)\n", buffer);

   data->commit();                  // All commands commit the active line

   const char* error= "Invalid command"; // Default, invalid command
   int C= *((const unsigned char*)buffer);
   if( C == '/' || C == '\'' || C == '\"' ) // If locate command
     error= command_locate(buffer); // Handle it

   else if( C == '>' )              // If forward locate
     error= command_forward(buffer+1); // Handle it

   else if( C == '<' )              // If reverse locate
     error= command_reverse(buffer+1); // Handle it

   else if( C >= '0' && C <= '9' )  // If line number command
     error= command_number(buffer); // Handle it

   else if( C == '#' )              // If comment
     error= command_comment();      // Handle it

   else {
     // Extract the command name (!!! MODIFIES histActive buffer !!!)
     char* parm= nullptr;           // Default, no parameter
     char* text= strchr(buffer, ' '); // Find blank delimiter
     if( text ) {                   // If found
       *text= '\0';                 // Delimit command name
       parm= text + 1;              // Address parameter(s)

       while( *parm == ' ' )        // Ignore leading blanks
         parm++;
     }

     // Process builtin commands
     Function* func= nullptr;       // Default, not found
     for(int i= 0; command_desc[i].name; i++) { // Find command
       if( strcasecmp(buffer, command_desc[i].name) == 0 ) {
         func= command_desc[i].func;
         break;
       }
     }
     if( func )
       error= func(parm);
     else
       error= "Invalid command";
   }

   return error;
}
