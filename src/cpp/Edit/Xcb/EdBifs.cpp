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
//       EdBifs.cpp
//
// Purpose-
//       Editor: Built in functions
//
// Last change date-
//       2020/12/09
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <unistd.h>                 // For close, ftruncate
#include <sys/stat.h>               // For stat
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include "Editor.h"                 // For Editor Globals
#include "EdFile.h"                 // For EdFile, EdLine
#include "EdHist.h"                 // For EdHist
#include "EdText.h"                 // For EdText

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra brinbup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
static const char*                  // Error message, nullptr expected
   command_save(char*);             // Save command

static const char*                  // Error message, nullptr expected
   command_quit(char*);             // (Unconditional) quit command

//----------------------------------------------------------------------------
//
// Method-
//       Editor::commands
//
// Purpose-
//       Editor built-in commands
//
//----------------------------------------------------------------------------
static const char*                  // Error message, nullptr expected
   command_bot(char*)               // Bottom command
{
   using namespace editor;          // For editor::text
   text->data->col_zero= text->data->col= 0;
   text->activate(text->file->line_list.get_tail());
   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_change(                  // Change command
     char*             parm)        // (Mutable) parameter string
{
printf("command_change(%s)\n", parm); // JUST TO USE PARM
printf("locate '%s'\n", editor::locate_string.c_str());
printf("change '%s'\n", editor::change_string.c_str());

   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_debug(                   // Change command
     char*             parm)        // (Mutable) parameter string
{  (void)parm;
   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_file(                    // File command
     char*             parm)        // (Mutable) parameter string
{
   const char* mess= command_save(parm); // Save the file
   if( mess == nullptr )            // If saved
     mess= command_quit(parm);      // Quit the file

   return mess;
}

static const char*                  // Error message, nullptr expected
   command_locate(                  // Locate command
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
   return editor::text->do_locate();
}

static const char*                  // Error message, nullptr expected
   command_nop(                     // NOP (test) command
     char*             parm)        // (Mutable) parameter string
{  printf("command_nop(%p) '%s'\n", parm, parm ? parm : "");
   return nullptr;
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

   using namespace editor;          // For editor::text
   EdLine* line= text->file->line_list.get_head();
   while( number > 0 ) {
     EdLine* next= line->get_next();
     if( next == nullptr )
       break;

     line= next;
     number--;
   }

   text->view= text->data;
   text->move_cursor_H(0);
   text->activate(line);

   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_quit(char*)              // (Unconditional) quit command
{
   editor::do_quit(editor::text->file);
   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_save(                    // Save command
     char*             parm)        // (Mutable) parameter string
{
   EdFile* file= editor::text->file;

   if( file->protect )
     return "Read-only file";
   if( file->damaged )
     return "Damaged file";

   if( parm )                       // If filename specified
     return "Not coded yet";        // Need to check existence

   int rc= file->write();
   if( rc )
     return "Write failure";

   file->reset();

   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_top(char*)               // Top command
{
   using namespace editor;          // For editor::text
   text->data->col_zero= text->data->col= 0;
   text->activate(text->file->line_list.get_head());
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
typedef const char* Command(char*); // The command processor

struct Command_desc {               // The Command descriptor item
const char*            name;        // The command name
Command*               func;        // THe command function
};                     // The Command descriptor

static const Command_desc
                       command_desc[]= // The Command descriptor list
{  {"BOT",      command_bot}        // Bottom
,  {"C",        command_change}     // Change
,  {"D",        command_debug}      // Debug
// {"DETAB",    command_detab}      // Detab
// {"E",        command_edit}       // Edit
// {"EDIT",     command_edit}       // Edit
// {"EXIT",     command_exit}       // Exit
,  {"FILE",     command_file}       // File
// {"GET",      command_get}        // Get
,  {"L",        command_locate}     // Locate (forward)
// {"MARGINS",  command_margins}    // Set margins
// {"MODE",     command_mode}       // Set mode
,  {"NOP",      command_nop}        // (Test function)
,  {"QUIT",     command_quit}       // Quit
,  {"SAVE",     command_save}       // Save
// {"SET",      command_set}        // Set (separate list) Include margins? ... autowrap
// {"TABS",     command_tabs}       // Tabs
,  {"TOP",      command_top}        // Top
// {">",        command_forward}    // Locate forward
// {"<",        command_reverse}    // Locate reverse
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
void
   editor::command(                 // Process a command
     char*             buffer)      // (MODIFIABLE) command line buffer
{
   const char* error= "Invalid command";

   int C= *((const unsigned char*)buffer);
   if( C == '/' )                   // If locate command
     error= command_locate(buffer); // Handle it

   else if( C >= '0' && C <= '9' )       // If line number command
     error= command_number(buffer); // Handle it

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
     Command* func= nullptr;        // Default, not found
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

   if( error ) {                    // If error encountered
     text->file->put_message(error);
     text->hist->activate();
   } else {
     text->hist->hist_line= nullptr;
     text->view= text->data;
//   text->draw_cursor();
   }
   text->draw_info();
}
