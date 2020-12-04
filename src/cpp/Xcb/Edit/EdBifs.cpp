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
//       2020/12/04
//
//----------------------------------------------------------------------------
#include <stdio.h>                  // For printf
#include <stdlib.h>                 // For various
#include <string.h>                 // For strcmp
#include <unistd.h>                 // For close, ftruncate
#include <sys/stat.h>               // For stat
#include <xcb/xcb.h>                // For XCB interfaces
#include <xcb/xproto.h>             // For XCB types

#include <pub/Debug.h>              // For pub::debugging

#include "Editor.h"                 // Editor Globals
#include "EdFile.h"                 // For EdFile, EdLine, EdPool
#include "EdHist.h"                 // For EdHist
#include "EdText.h"                 // For EdText

using namespace pub::debugging;

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
enum // Compilation controls
{  HCDM= false                      // Hard Core Debug Mode?
,  USE_BRINGUP= false               // Extra brinbup diagnostics?
}; // Compilation controls

//----------------------------------------------------------------------------
//
// Method-
//       Editor::commands
//
// Purpose-
//       scaffolding
//
//----------------------------------------------------------------------------
static const char*                  // Error message, nullptr expected
   command_bot(char*)               // Bottom command
{
   EdText* text= Editor::editor->text; // Address the Text window
   text->data->col_zero= text->data->col= 0;
   text->activate(text->file->lines.get_tail());
   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_change(                  // Change command
     char*             parm)        // (Mutable) command string
{
printf("command_change(%s)\n", parm); // JUST TO USE PARM
printf("locate '%s'\n", Editor::editor->locate_string.c_str());
printf("change '%s'\n", Editor::editor->change_string.c_str());

   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_debug(                   // Change command
     char*             parm)        // (Mutable) command string
{
   Editor* edit= Editor::editor;
   EdText* text= edit->text;
   char buffer[16];                 // Working buffer
   buffer[15]= '\0';                // (Always has '\0' delimiter)

   debugf("\ncommand_debug(%s)\n", parm);
   strncpy(buffer, text->line->text, 15);
   debugf("line_zero.text(%s)\n", buffer);
   strncpy(buffer, text->cursor->text, 15);
   debugf("cursor.text(%s)\n", buffer);

   text->debug(parm);
   text->data->debug("data");
   text->hist->debug("hist");
   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_locate(                  // Locate command
     char*             parm)        // (Mutable) command string
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

   Editor* edit= Editor::editor;
   edit->locate_string= std::string(parm, C - parm);
   edit->change_string= edit->locate_string;

   return edit->text->do_locate();
}

static const char*                  // Error message, nullptr expected
   command_nop(                     // NOP (test) command
     char*             parm)        // (Mutable) command string
{  printf("command_nop(%p) '%s'\n", parm, parm ? parm : "");
   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_number(                  // (Line) number command
     char*             parm)        // (Mutable) command string
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

   EdText* text= Editor::editor->text;
   EdLine* line= text->file->lines.get_head();
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
   Editor* edit= Editor::editor;
   edit->do_quit(edit->text->file);
   return nullptr;
}

static const char*                  // Error message, nullptr expected
   command_top(char*)               // Top command
{
   EdText* text= Editor::editor->text; // Address the Text window
   text->data->col_zero= text->data->col= 0;
   text->activate(text->file->lines.get_head());
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
,  {"L",        command_locate}     // Locate
,  {"NOP",      command_nop}        // (Test function)
,  {"QUIT",     command_quit}       // Quit
,  {"TOP",      command_top}        // Top
,  {nullptr,    nullptr}            // End of list delimiter
};

//----------------------------------------------------------------------------
//
// Method-
//       Editor::command
//
// Purpose-
//       Process a command.
//
//----------------------------------------------------------------------------
void
   Editor::command(                 // Process a command
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
