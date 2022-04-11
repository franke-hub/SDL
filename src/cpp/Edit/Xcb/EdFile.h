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
//       EdFile.h
//
// Purpose-
//       Editor: File descriptor
//
// Last change date-
//       2021/06/17
//
// Implementation objects-
//       EdLine: Editor EdFile line descriptor
//       EdMess: Editor EdFile message descriptor
//       EdHide: Editor EdFile hidden line group (NOT IMPLEMENTED)
//       EdRedo: Editor EdFile Redo/Undo control
//       EdFile: Editor File descriptor
//
//----------------------------------------------------------------------------
#ifndef EDFILE_H_INCLUDED
#define EDFILE_H_INCLUDED

#include <sys/stat.h>               // For struct stat

#include <gui/Types.h>              // For gui::Line
#include <pub/List.h>               // For pub::List
#include <pub/Signals.h>            // For namespace pub::signals

#include "Editor.h"                 // For Editor

//----------------------------------------------------------------------------
//
// Class-
//       EdLine
//
// Purpose-
//       Editor Line
//
// Implementation note-
//       Lines are allocated and deleted, but text is never deleted
//
//----------------------------------------------------------------------------
class EdLine : public pub::List<EdLine>::Link { // Editor Line descriptor
//----------------------------------------------------------------------------
// EdLine::Attributes
public:
const char*            text;        // Text, never nullptr

uint16_t               flags= 0;    // Control flags
enum FLAGS                          // Control flags
{  F_NONE= 0x0000                   // No flags
,  F_MARK= 0x0001                   // Line is marked (selected)
,  F_PROT= 0x0002                   // Line is read/only
,  F_HIDE= 0x0004                   // Line is hidden
};

unsigned char          delim[2]= {'\0', 0}; // Delimiter (NONE default)
//   For [0]= '\n', [1]= either '\r' or '\0' for DOS or Unix format.
//   For [0]= '\0', [1]= repetition count. {'\0',0}= NO delimiter

//----------------------------------------------------------------------------
// EdLine::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdLine(                          // Constructor
     const char*       text= nullptr); // Line text

   ~EdLine( void );                 // Destructor

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::debug
//
// Purpose-
//       (Minimal) debugging display
//
//----------------------------------------------------------------------------
void
   debug( void ) const;             // Debugging display

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::is_within
//
// Purpose-
//       Is this line within range head..tail (inclusive)?
//
//----------------------------------------------------------------------------
bool
   is_within(                       // Is this line within range head..tail?
     const EdLine*     head,        // First line in range
     const EdLine*     tail) const; // Final line in range
}; // class EdLine

//----------------------------------------------------------------------------
//
// Class-
//       EdMess
//
// Purpose-
//       Editor Message
//
//----------------------------------------------------------------------------
class EdMess : public pub::List<EdMess>::Link { // Editor message descriptor
//----------------------------------------------------------------------------
// EdMess::Attributes
public:
enum                                // Message types
{  T_INFO                           // T_INFO: Informational, any key removes
,  T_MESS                           // T_MESS: Action, button click required
,  T_BUSY                           // T_BUSY: Limited function until complete
};

std::string            mess;        // The message
int                    type= T_INFO; // The message type

//----------------------------------------------------------------------------
// EdMess::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdMess(                          // Constructor
     std::string       mess_,       // Message text
     int               type_= T_INFO);

   ~EdMess( void );                 // Destructor
}; // class EdMess

//----------------------------------------------------------------------------
//
// Class-
//       EdHide
//
// Purpose-
//       Editor hidden line group
//
// Implementation note-
//       Caller *ALWAYS* verifies that head/tail not protected
//
//----------------------------------------------------------------------------
class EdHide : public EdLine {      // Editor hidden line group
//----------------------------------------------------------------------------
// EdHide::Attributes
//----------------------------------------------------------------------------
public:
std::string            info;        // The text line string
size_t                 count= 0;    // The number of hidden lines
pub::List<EdLine>      list;        // The hidden line list

//----------------------------------------------------------------------------
// EdHide::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdHide(                          // Constructor
     EdLine*           head_= nullptr, // First hidden line
     EdLine*           tail_= nullptr); // Final hidden line

   ~EdHide( void );                 // Destructor

//----------------------------------------------------------------------------
// EdHide::Methods
//----------------------------------------------------------------------------
public:
void
   append(                          // Add to end of list
     EdLine*           line);       // Making this the new tail line

void
   prepend(                         // Add to beginning of list
     EdLine*           line);       // Making this the new head line

void
   remove( void );                  // Remove (and delete) this hidden line

void
   update( void );                  // Update the count and the message
}; // class EdHide

//----------------------------------------------------------------------------
//
// Class-
//       EdRedo
//
// Purpose-
//       Editor Redo/Undo action
//
//----------------------------------------------------------------------------
class EdRedo : public pub::List<EdRedo>::Link { // Editor Undo/Redo
//----------------------------------------------------------------------------
// EdRedo::Attributes
public:
EdLine*                head_insert= nullptr; // First line inserted
EdLine*                tail_insert= nullptr; // Last line  inserted
EdLine*                head_remove= nullptr; // First line removed
EdLine*                tail_remove= nullptr; // Last line  removed

// Block copy/move columns
ssize_t                lh_col= -1;  // Left  hand column
ssize_t                rh_col= -1;  // Right hand column

//----------------------------------------------------------------------------
// EdRedo::Destructor/Constructor
//----------------------------------------------------------------------------
public:
   ~EdRedo( void );                 // Destructor
   EdRedo( void );                  // Constructor

//----------------------------------------------------------------------------
//
// Method-
//       EdRedo::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   debug(                           // Debugging display
     const char*       info= nullptr) const; // Associated info
}; // class EdRedo

//----------------------------------------------------------------------------
//
// Class-
//       EdFile
//
// Purpose-
//       Editor File
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

static size_t                       // The row count
   get_rows(                        // Get row count
     const EdLine*     head,        // From this line
     const EdLine*     tail);       // *To* this line

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
//       EdFile::append
//
// Purpose-
//       Load file data
//
//----------------------------------------------------------------------------
EdLine*                             // The last inserted line
   append(                          // Append file
     const char*       name,        // The file name to insert
     EdLine*           line);       // Insert after this line

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::insert
//
// Purpose-
//       Insert file line(s)
//
//----------------------------------------------------------------------------
EdLine*                             // (Always tail)
   insert(                          // Insert
     EdLine*           after,       // After this line
     EdLine*           head,        // From this line
     EdLine*           tail);       // Upto this line

EdLine*                             // (Always line)
   insert(                          // Insert
     EdLine*           after,       // After this line
     EdLine*           line)        // This line
{  return insert(after, line, line); }

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::new_line
//
// Purpose-
//       Allocate a new line, also setting the delimiter
//
// Implementation note-
//       DOS files get DOS delimiters. All others get UNIX delimiters.
//
//----------------------------------------------------------------------------
EdLine*                             // The allocated line
   new_line(                        // Allocate a new line
     const char*       text= nullptr) const; // Line text

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

int                                 // TRUE if message removed or remain
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
//       EdFile::redo_delete        // (Only used by EdFile.cpp)
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
