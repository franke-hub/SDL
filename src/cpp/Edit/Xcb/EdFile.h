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
//       EdFile.h
//
// Purpose-
//       Editor: File descriptor
//
// Last change date-
//       2024/03/31
//
//----------------------------------------------------------------------------
#ifndef EDFILE_H_INCLUDED
#define EDFILE_H_INCLUDED

#include <pub/List.h>               // For pub::List
#include <pub/Signals.h>            // For namespace pub::signals

#include "Editor.h"                 // For Editor
#include "EdLine.h"                 // For EdLine, EdHide
#include "EdMess.h"                 // For EdMess
#include "EdRedo.h"                 // For EdRedo

//----------------------------------------------------------------------------
//
// Class-
//       EdFile
//
// Purpose-
//       Editor File descriptor
//
//----------------------------------------------------------------------------
class EdFile : public pub::List<EdFile>::Link { // Editor file descriptor
//----------------------------------------------------------------------------
// EdFile::Attributes
public:
enum MODE { M_NONE, M_BIN, M_DOS, M_MIX, M_UNIX }; // The file mode

pub::List<EdMess>      mess_list;   // The List of warning messages
pub::List<EdLine>      line_list;   // The line list
pub::List<EdRedo>      redo_list;   // The redo list
pub::List<EdRedo>      undo_list;   // The undo list

std::string            name;        // The fully qualified file name
size_t                 rows= 0;     // The number of file rows

int                    mode= M_NONE; // The file mode
bool                   changed= false; // File is changed
bool                   chglock= false; // File is changed, undo not available
bool                   damaged= false; // File is damaged
bool                   protect= false; // File is protected

// Cursor position controls
EdLine*                top_line= nullptr; // The current top Line
EdLine*                csr_line= nullptr; // The current cursor (active) Line
size_t                 col_zero= 0; // The current top column[0]
size_t                 row_zero= 0; // The current top row[0]
unsigned               col= 0;      // The current cursor column (offset)
unsigned               row= 0;      // The current cursor row (offset)

// Signals -------------------------------------------------------------------
struct CloseEvent {                 // File close event
EdFile*                file;
};

static pub::signals::Signal<CloseEvent>
                       close_signal; // The CloseEvent Signal

//----------------------------------------------------------------------------
// EdFile::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdFile(                          // Constructor
     const char*       name);       // Fully qualified file name

   ~EdFile( void );                 // Destructor

//----------------------------------------------------------------------------
// EdFile::Accessor methods
//----------------------------------------------------------------------------
public:
char*
   allocate(                        // Allocate file text
     size_t            size) const; // Of this length

EdLine*                             // The EdLine*
   get_line(                        // Get EdLine*
     size_t            row) const;  // For this row number

std::string
   get_name( void ) const           // Get the file name (Named interface)
{  return name; }

size_t                              // The row number
   get_row(                         // Get row number
     const EdLine*     cursor) const; // For this line

bool                                // TRUE if file is changed or damaged
   is_changed( void ) const;        // Is file changed or damaged?

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= nullptr); // Associated info

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::activate
//
// Purpose-
//       Activate file line
//
//----------------------------------------------------------------------------
void
   activate(                        // Activate
     EdLine*           line);       // This line

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::command
//
// Purpose-
//       Load command output
//
//----------------------------------------------------------------------------
void
   command(                         // Load command output
     const char*       input,       // The command name
     const std::string&output);     // The command output

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::insert_file
//
// Purpose-
//       Load and insert file without redo/undo
//
//----------------------------------------------------------------------------
EdLine*                             // The last inserted line
   insert_file(                     // Insert file (Without redo/undo)
     const char*       name,        // The file name to insert
     EdLine*           line);       // Insert after this line

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::insert
//
// Purpose-
//       Insert file line(s) without redo/undo
//
//----------------------------------------------------------------------------
EdLine*                             // (Always tail)
   insert(                          // Insert without redo/undo
     EdLine*           after,       // After this line
     EdLine*           head,        // From this line
     EdLine*           tail);       // Upto this line

EdLine*                             // (Always line)
   insert(                          // Insert without redo/undo
     EdLine*           after,       // After this line
     EdLine*           line)        // This line
{  return insert(after, line, line); }

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::new_line
//       EdFile::new_text
//
// Purpose-
//       Allocate a new line, also setting the delimiter (Immutable text)
//       Allocate a new line, also setting the delimiter (Duplicated text)
//
// Implementation note-
//       DOS files get DOS delimiters. All others get UNIX delimiters.
//
//----------------------------------------------------------------------------
EdLine*                             // The allocated line
   new_line(                        // Allocate a new line
     const char*       text= nullptr) const; // (Immutable) text

EdLine*                             // The allocated line
   new_text(                        // Allocate a new line with copied text
     const char*       text= nullptr) const // (Temporary) text
{
   if( text ) {
     char* copy= allocate(strlen(text) + 1);
     strcpy(copy, text);
     text= copy;
   }

   return new_line(text);
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::parse
//
// Purpose-
//       Parse text.
//
// Implementation note-
//       DOS files get DOS delimiters. All others get UNIX delimiters.
//
//----------------------------------------------------------------------------
EdLine*                             // The last inserted EdLine
   parse(                           // Parse text, inserting EdLines
     EdLine*           line,        // The line to insert after
     char*             text,        // The (allocated) text
     size_t            size);       // The text length

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::put_message
//       EdFile::rem_messgae
//
// Purpose-
//       Add message to list
//       Remove message from list
//
//----------------------------------------------------------------------------
void
   put_message(                     // Write message
     const char*       mess_,       // Message text
     int               type_= EdMess::T_INFO); // Message mode

int                                 // TRUE if message removed or remains
   rem_message( void );             // Remove current EdMess

int                                 // TRUE if message removed or remain
   rem_message_type(                // Remove current EdMess
     int                type_= EdMess::T_INFO);  // If at this level or lower

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::redo
//       EdFile::undo
//
// Purpose-
//       Perform redo action
//       Perform undo action
//
//----------------------------------------------------------------------------
void
   redo( void );                    // Perform redo action

void
   undo( void );                    // Perform undo action

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::redo_delete
//
// Purpose-
//       Delete the REDO list
//
//----------------------------------------------------------------------------
void
   redo_delete( void );             // Delete the REDO list

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::redo_insert
//
// Purpose-
//       Insert redo onto the UNDO list
//
//----------------------------------------------------------------------------
void
   redo_insert(                     // Insert
     EdRedo*           undo);       // This REDO onto the UNDO list

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::remove
//
// Purpose-
//       Remove file line(s)
//
//----------------------------------------------------------------------------
void
   remove(                          // Remove
     EdLine*           head,        // From this line
     EdLine*           tail);       // Upto this line

void
   remove(                          // Remove
     EdLine*           line)        // This line
{  return remove(line, line); }

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::reset
//
// Purpose-
//       Reset after file write
//
// Implementation notes-
//       This action cannot be undone.
//
//----------------------------------------------------------------------------
void
   reset( void );                   // Reset the undo/redo lists

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::set_mode
//
// Purpose-
//       Set the file mode (to M_DOS or M_UNIX)
//
//----------------------------------------------------------------------------
void
   set_mode(                        // Set the file mode
     int               mode);       // To this mode

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::undo_delete
//
// Purpose-
//       Delete the UNDO list
//
//----------------------------------------------------------------------------
void
   undo_delete( void );             // Delete the UNDO list

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::write
//
// Purpose-
//       Write file data
//
//----------------------------------------------------------------------------
int                                 // Return code, 0 OK
   write(                           // Write file
     const char*       name);       // The file name to write

int                                 // Return code, 0 OK
   write( void );                   // Write (replace) the file
}; // class EdFile
#endif // EDFILE_H_INCLUDED
