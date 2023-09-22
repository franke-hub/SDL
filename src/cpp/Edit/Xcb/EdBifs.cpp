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
//       2023/09/19
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

,  TABS= 8                          // TAB spacing (2**N)
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

// Command: set mode= dos | unix
static Name_value      mode_value[]= // Mode value table
{ {"dos",              EdFile::M_DOS}
, {"unix",             EdFile::M_UNIX}
, {nullptr,            -1}          // Not found value
}; // mode_value[]

// Command: set <symbol>= <bool_value>
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
, {nullptr,            nullptr}     // Symbol not found
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
// static const char* command_get(char*); // NOT CODED YET
//         editor::command_help(char*);
static const char* command_locate(char*);
// static const char* command_margins(char*);
static const char* command_quit(char*);
// static const char* command_redo(char*);
static const char* command_save(char*);
static const char* command_set(char*);
static const char* command_sort(char*);
// static const char* command_tabs(char*);
static const char* command_top(char*);
// static const char* command_undo(char*);
static const char* command_view(char*);
static const char* command_0042(char*);

static const Command_desc
                       command_desc[]= // The Command descriptor list
{  {command_bot,    "BOT",     "Bottom of file"}
,  {command_change, "C",       "Change"}
,  {command_debug,  "DEBUG",   nullptr}
,  {command_detab,  "DETAB",   "Convert tabs to spaces"}
,  {command_edit,   "E",       "Alias for EDIT"}
,  {command_edit,   "EDIT",    "Edit file(s)"}
,  {command_exit,   "EXIT",    "(Safe) Exit" }
,  {command_find,   "FI",      "Find (starting in column 1)"} // (Alias)
,  {command_file,   "FILE",    "(Unconditionally) save and close file"}
,  {command_find,   "FIND",    "Find (starting in column 1)"}
// {nullptr,        "GET",     "Append file"} // NOT CODED YET
,  {editor::command_help,
                    "HELP",    "Help command"}
,  {command_locate, "L",       "Locate"}
// {command_margins,"MARGINS", "Set margins"}
,  {command_quit,   "QUIT",    "(Unconditionally) close file"}
// {command_redo,   "REDO",    "REDO an UNDO"}
,  {command_save,   "SAVE",    "Write file"}
,  {command_set,    "SET",     "Set option value"}
,  {command_sort,   "SORT",    "Sort file list using file name"}
,  {nullptr,        "SORT -f", "Sort using fully-qualified name"}
// {command_tabs,   "TABS",    "Set tabs"}
,  {command_top,    "TOP",     "Top of File"}
// {command_undo,   "UNDO",    "UNDO a change"}
,  {command_view,   "V",       "Alias for VIEW"}
,  {command_view,   "VIEW",    "Edit file(s) in read/only mode"}
,  {nullptr,        "<",       "Locate (reverse search)"}
,  {nullptr,        ">",       "Locate (forward search)"}
,  {nullptr,        "#",       "(Comment)"}
,  {command_0042,   "number",  "Set current line to 'number'"}

// Spelling errors/typos
,  {nullptr,        "",        nullptr} // Misspelled commands follow
,  {command_save,  "SAE",      nullptr} // (SAVE)
,  {command_save,  "SAVAE",    nullptr} // (SAVE)
,  {command_save,  "SAVCE",    nullptr} // (SAVE)
,  {command_save,  "SAVVE",    nullptr} // (SAVE)
,  {command_save,  "SVAE",     nullptr} // (SAVE)
,  {command_top,   "TIO",      nullptr} // (TOP)
,  {nullptr,       nullptr,    nullptr} // End of list delimiter
};

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
   char buffer[256];                // Input buffer
   std::string input(parm);         // The input command
   std::string output;              // The output buffer
   input += " 2>&1";                // (Route stderr onto stdout)

   auto pipe= popen(input.c_str(), "r"); // The output pipe
   int rc= EXIT_FAILURE;

   try {
     if( !pipe ) {
       sprintf(buffer, "popen error: %d:%s", errno, strerror(errno));
       throw std::runtime_error(buffer);
     }

     while( !feof(pipe) ) {
       if( fgets(buffer, sizeof(buffer), pipe) )
         output += buffer;
     }

     rc= pclose(pipe);
     if( rc != 0 ) {
       output += "\n\n";
       output += "Command failed";
     }
   } catch( std::exception& X ) {
     output += "\n\n";
     output += "Exception: ";
     output += X.what();
     if( pipe )
       pclose(pipe);
   }

   // Create result pseudo-file
   editor::insert_command(parm, output);
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

   if( file->changed ) {            // If the file is changed
     // Convert the file from changed to chdetab state.
     file->redo_delete();
     file->undo_delete();
     file->changed= false;
     file->chdetab= true;
   }

   for(EdLine* line= file->line_list.get_head(); line; line= line->get_next()) {
     if( line->flags & EdLine::F_PROT ) // If the line is protected
       continue;                    // (Skip line; don't remove tabs)
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
       L += TABS;
       L &= ~(TABS - 1);
       active->fetch(L-1);
       if( L > active->get_used() )
         active->append_text(" ");

       text= tabs + 1;            // Skip past this tab
       tabs= strchr(text, '\t');  // Locate the next tab
     }

     if( active ) {               // If tabs found
       active->append_text(text); // Append trailing text
       active->append_text(" ");  // (Indicate changed)
       data->commit_only();       // (Commit without REDO/UNDO)
       file->chdetab= true;       // (File changed by DETAB command)
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
   const char* mess= editor::write_file(parm); // Save the file
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

#if 0
static const char*                  // Error message, nullptr expected
   command_get(char*)               // Get command
{
   // SPECIAL CASE: Get after ending line with no delimiter.
   // The no delimiter line CHANGES. It's part of the REDO/UNDO.
   return "Not coded yet";          // TODO: NOT CODED YET
}
#endif

const char*                         // (Always nullptr)
   editor::command_help(char*)      // The 'help' command
{
   std::string text= "Function keys:\n"
       " F1:     This help message\n"
       " F2:     NOP\n"
       " F3:     Quit file (if unchanged)\n"
       " F4:     Query: Any files changed?\n"
       " F5:     Locate (next)\n"
       " F6:     Change (current or next)\n"
       " F7:     Switch to previous file\n"
       " F8:     Switch to next file\n"
       " F9:     Copy cursor line to command line\n"
       "F10:     Move cursor line to top of screen\n"
       "F11:     Undo\n"
       "F12:     Redo\n"
       ;

   text += "\nCommand list: (Command names are not case sensitive)\n";
   for(int i= 0; command_desc[i].name[0] != '\0'; ++i) {
     if( command_desc[i].desc ) {
       char name[16];
       char line[128];
       sprintf(name, "%s:", command_desc[i].name);
       sprintf(line, "%-8s %s\n", name, command_desc[i].desc);
       text += line;
     }
   }
   editor::insert_command("**Editor help**", text);
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

#if 0 // Works, but not needed with F11 key fix
static const char*                  // Error message, nullptr expected
   command_redo(char*)              // REDO an UNDO command
{
   editor::data->commit();
   editor::file->redo();
   return nullptr;
}
#endif

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
{  return editor::write_file(parm); } // Write the file

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

   if( strcasecmp(name, "help") == 0 ) {
     std::string text=
         "set mode {dos | unix}\n"
         " mode dos  (Use DOS file mode.)\n"
         " mode unix (Use UNIX file mode.)\n"
         "\n"
         "set <option> {ON | off}, options:\n"
         " hidden    (Hide idle mouse cursor?)\n"
         " mixed     (Use case sensitive locate?)\n"
         " reverse   (Use locate toward top of file?)\n"
         " wrap      (Use locate wrap-around?)\n"
         ;
     editor::insert_command("**SET command help**", text);
     return nullptr;
   }

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

#if 0 // Works, but not needed with F11 key fix
static const char*                  // Error message, nullptr expected
   command_undo(char*)              // UNDO last file change
{
   if( editor::data->active.undo() ) {
     editor::data->draw_active();
     editor::term->draw_top();
   } else
     editor::file->undo();
   return nullptr;
}
#endif

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

static const char*                  // Error message, nullptr expected
   command_0042(char*)              // NUMBER command
{  return "'number' isn't a command. Try using a numeric value instead."; }

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
     for(int i= 0; command_desc[i].name; i++) { // Find command
       if( strcasecmp(buffer, command_desc[i].name) == 0 ) {
         if( command_desc[i].func )
           return command_desc[i].func(parm);
         return "OOPS";
       }
     }

     // Process system command
     if( parm )
       *text= ' ';

     error= command_cmd(buffer);
   }

   return error;
}

//----------------------------------------------------------------------------
//
// Method-
//       editor::write_file
//
// Purpose-
//       Write file (with error checking)
//
//----------------------------------------------------------------------------
const char*                         // Error message, nullptr expected
   editor::write_file(              // Write file
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
