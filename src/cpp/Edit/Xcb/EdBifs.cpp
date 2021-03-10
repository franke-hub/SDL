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
//       EdBifs.cpp
//
// Purpose-
//       Editor: Built in functions
//
// Last change date-
//       2021/03/10
//
//----------------------------------------------------------------------------
#include <sys/stat.h>               // For stat

#include <pub/Debug.h>              // For namespace pub::debugging
#include <pub/Tokenizer.h>          // For pub::Tokenizer

#include "Config.h"                 // For namespace config
#include "Editor.h"                 // For Editor Globals
#include "EdFile.h"                 // For EdFile, EdLine
#include "EdHist.h"                 // For EdHist
#include "EdMark.h"                 // For EdMark
#include "EdText.h"                 // For EdText

using namespace pub::debugging;     // For debugging
using namespace config;             // For opt_* controls

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
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
, {"mixed",            (int*)&editor::locate_case}
, {"wrap",             (int*)&editor::locate_wrap}
, {"locate.prior",     (int*)&editor::locate_back} // Official symbol names
, {"locate.mixed",     (int*)&editor::locate_case}
, {"locate.wrap",      (int*)&editor::locate_wrap}
, {"reverse",          (int*)&editor::locate_back} // Symbol name aliases
, {"mixed_case",       (int*)&editor::locate_case}
, {"autowrap",         (int*)&editor::locate_wrap}
, {"hidden",           (int*)&config::USE_MOUSE_HIDE} // Controls
, {"mouse_hide",       (int*)&config::USE_MOUSE_HIDE}
, {nullptr,            nullptr}     // Not found address
}; // true_addr[]

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
static const char*                  // Error message, nullptr expected
   command_locate(char*);           // Locate command

static const char*                  // Error message, nullptr expected
   command_quit(char*);             // (Unconditional) quit command

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
   using namespace editor;          // For editor::data, hist, text
   data->col_zero= data->col= 0;
   text->activate(file->line_list.get_tail());
   hist->activate();                // (Remain in command mode)
   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_change(                  // Change command
     char*             parm)        // (Mutable) parameter string
{
   // TODO: HANDLE UTF8
   if( parm == nullptr )
     return "Missing parameter";

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
   command_debug(                   // Change command
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
   else if( strcasecmp(parm, "text") == 0 )
     editor::text->debug("command");
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

   if( editor::file != editor::last ) {
     editor::text->activate(editor::last);
     editor::text->draw();
   }
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
   // THe no delimiter line CHANGES. It's part of the REDO/UNDO.
   return nullptr;                  // TODO: NOT CODED YET
}

static const char*                  // Error message, nullptr expected
   command_locate(                  // Locate command (current direction)
     char*             parm)        // (Mutable) parameter string
{
   // TODO: HANDLE UTF8
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
   editor::text->move_cursor_H(0);
   editor::text->activate(editor::file->get_line(number));

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
   command_sort(char*)              // Sort editor::file_list
{
   pub::List<EdFile> sort_list;     // The sorted list of EdFiles

   for(;;) {                        // Sort the file list
     EdFile* low= editor::file_list.get_head();
     if( low == nullptr )
       break;

     for(EdFile* file= low->get_next(); file; file= file->get_next()) {
       if( file->name < low->name )
         low= file;
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
   using namespace editor;          // For editor::data, hist, text
   data->col_zero= data->col= 0;
   text->activate(file->line_list.get_head());
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

   if( editor::file != editor::last ) {
     editor::text->activate(editor::last);
     editor::text->draw();
   }
   editor::hist->activate();

   return nullptr;
}

//----------------------------------------------------------------------------
//
// Data area-
//       command_list
//
// Purpose-
//       The list of build-in commands.
//
//----------------------------------------------------------------------------
typedef const char* Function(char*); // The command processor function

struct Command_desc {               // The Command descriptor item
const char*            name;        // The command name
Function*              func;        // THe command function
};                     // The Command descriptor

static const Command_desc
                       command_desc[]= // The Command descriptor list
{  {"BOT",      command_bot}        // Bottom
,  {"C",        command_change}     // Change
,  {"CMD",      command_cmd}        // Command
,  {"D",        command_debug}      // Debug
,  {"DEBUG",    command_debug}      // Debug
// {"DETAB",    command_detab}      // Detab
,  {"E",        command_edit}       // Edit
,  {"EDIT",     command_edit}       // Edit
,  {"EXIT",     command_exit}       // Exit
,  {"FILE",     command_file}       // File
,  {"GET",      command_get}        // Get
,  {"L",        command_locate}     // Locate (forward)
// {"MARGINS",  command_margins}    // Set margins
// {"MODE",     command_mode}       // Set mode
,  {"QUIT",     command_quit}       // Quit
,  {"SAVE",     command_save}       // Save
,  {"SET",      command_set}        // Set
,  {"SORT",     command_sort}       // Sort
// {"TABS",     command_tabs}       // Tabs
,  {"TOP",      command_top}        // Top
,  {"V",        command_view}       // View
,  {"VIEW",     command_view}       // View
,  {nullptr,    nullptr}            // End of list delimiter
};

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
