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
//       EdFile.h
//
// Purpose-
//       Editor: File descriptor
//
// Last change date-
//       2020/12/04
//
//----------------------------------------------------------------------------
#ifndef EDFILE_H_INCLUDED
#define EDFILE_H_INCLUDED

#include <sys/stat.h>               // For struct stat
#include <pub/List.h>               // For pub::List
#include "Xcb/Types.h"              // For xcb::Line

#include "Editor.h"                 // For Editor

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class EdFile;

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
class EdLine : public ::xcb::Line { // Editor line descriptor
public:
//----------------------------------------------------------------------------
// EdLine::Attributes
//----------------------------------------------------------------------------
uint16_t               flags= 0;    // Control flags
enum FLAGS                          // Control flags
{  F_NONE= 0x0000                   // No flags
,  F_HIDE= 0x0001                   // Line is hidden
,  F_MARK= 0x0002                   // Line is marked
,  F_PROT= 0x0004                   // Line is read/only
};

unsigned char          delim[2] = {0, 0}; // Delimiter
//   For [0]= '\n', [1]= either '\r' or '\0' for DOS or Unix format.
//   For [0]= '\0', [1]= repetition count. {'\0',0}= NO delimiter

//----------------------------------------------------------------------------
// EdLine::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdLine(                          // Constructor
     const char*       text= nullptr) // Line text
:  ::xcb::Line(text)
{
   if( xcb::opt_hcdm && xcb::opt_verbose > 2 )
     xcb::debugh("EdLine(%p)::EdLine\n", this);
}

//----------------------------------------------------------------------------
virtual
   ~EdLine( void )                  // Destructor
{
   if( xcb::opt_hcdm && xcb::opt_verbose > 2 )
     xcb::debugh("EdLine(%p)::~EdLine...\n", this);
}

//----------------------------------------------------------------------------
// EdLine::Methods
//----------------------------------------------------------------------------
public:
inline EdLine*
   get_next( void ) const
{  return (EdLine*)::xcb::Line::get_next(); }

inline EdLine*
   get_prev( void ) const
{  return (EdLine*)::xcb::Line::get_prev(); }
}; // class EdLine

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
EdLine*                head= nullptr; // First hidden line
EdLine*                tail= nullptr; // Final hidden line

//----------------------------------------------------------------------------
// EdHide::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdHide(                          // Constructor
     EdLine*           _head= nullptr, // First hidden line
     EdLine*           _tail= nullptr) // Final hidden line
:  EdLine()
{
   if( xcb::opt_hcdm )
     xcb::debugh("EdHide(%p)::EdHide\n", this);

   flags= F_HIDE;
   if( _head ) {
     append(_head);
     if( _tail )
       append(_tail);
   }
}

//----------------------------------------------------------------------------
virtual
   ~EdHide( void )                  // Destructor
{
   if( xcb::opt_hcdm )
     xcb::debugh("EdHide(%p)::~EdHide...\n", this);

   EdLine* line= head;
   if( line ) {                     // If files remain
     // You should only get here from the EdFile destructor. Let's make sure.
     if( get_prev() ) {             // If the prior line wasn't removed
       fprintf(stderr, "~EdHide invalid state"); // (Not called from ~EdFile)
       return;
     }

     while( true ) {
       if( line == nullptr ) {
         fprintf(stderr, "~EdHide invalid chain"); // (Too late to debug now)
         return;
       }
       EdLine* next= line->get_next();
       delete line;
       if( line == tail ) break;
       line= next;
     }
   }
}

//----------------------------------------------------------------------------
// EdHide::Methods
//----------------------------------------------------------------------------
public:
void
   append(                          // Add to end of list
     EdLine*           line)        // Making this the new tail line
{
   if( tail )
     get_next()->set_prev(tail);
   else {
     line->get_prev()->set_next(this);
     set_prev(line->get_prev());
     head= line;
   }
   line->get_next()->set_prev(this);
   tail= line;

   update();
}

void
   prepend(                         // Add to beginning of list
     EdLine*           line)        // Making this the new head line
{
   if( head ) {
     get_prev()->set_next(head);
   } else {
     line->get_next()->set_prev(this);
     set_next(line->get_next());
     tail= line;
   }
   line->get_prev()->set_next(this);
   head= line;

   update();
}

void
   remove( void )                   // Remove (and delete) this hidden line
{
   if( head )                       // If not inserted
     return;                        // Nothing to do

   get_prev()->set_next(head);
   get_next()->set_prev(tail);
   head= nullptr;
   tail= nullptr;

   delete this;
}

void
   update( void )                   // Update the count and the message
{
   count= 0;
   if( head ) {
     EdLine* line= head;
     count= 1;
     while( line != tail ) {
       count++;
       if( line == nullptr ) throw "Invalid EdHide chain";
       line= line->get_next();
     }
   }

   char buffer[128];                // Message work area
   memset(buffer, '-', sizeof(buffer));
   buffer[sizeof(buffer) - 1]= '\0';
   int L= sprintf(buffer, ">--- %zd lines hidden", count);
   buffer[L]= ' ';
   info= buffer;
   text= info.c_str();
}
}; // class EdHide

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
int                    type= T_MESS; // The message type

//----------------------------------------------------------------------------
// EdMess::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdMess(                          // Constructor
     std::string       _mess,       // Message text
     int               _type= T_MESS)
:  ::pub::List<EdMess>::Link(), mess(_mess), type(_type)
{
   if( xcb::opt_hcdm )
     xcb::debugh("EdMess(%p)::EdMess(%s,%d)\n", this, _mess.c_str(), _type);
}

//----------------------------------------------------------------------------
virtual
   ~EdMess( void )                  // Destructor
{
   if( xcb::opt_hcdm )
     xcb::debugh("EdMess(%p)::~EdMess...\n", this);
}
}; // class EdMess

//----------------------------------------------------------------------------
//
// Class-
//       EdUndo
//
// Purpose-
//       Editor Undo/Redo action (placeholder)
//
//----------------------------------------------------------------------------
class EdUndo : public pub::List<EdUndo>::Link { // Editor Undo/Redo
//----------------------------------------------------------------------------
// EdUndo::Typedefs and enumerations
public:
enum // Operation type
{  OP_INSERT
,  OP_REMOVE
,  OP_CHANGE
};

//----------------------------------------------------------------------------
// EdUndo::Attributes
public:
unsigned               op;          // The operation type
EdLine*                head_insert; // First line inserted
EdLine*                tail_insert; // Last line  inserted
EdLine*                head_remove; // First line removed
EdLine*                tail_remove; // Last line  removed

//----------------------------------------------------------------------------
// EdUndo::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdUndo( void )                   // Constructor
:  ::pub::List<EdUndo>::Link()
{
   if( xcb::opt_hcdm )
     xcb::debugh("EdUndo(%p)::EdUndo\n", this);
}

//----------------------------------------------------------------------------
virtual
   ~EdUndo( void )                  // Destructor
{
   if( xcb::opt_hcdm )
     xcb::debugh("EdUndo(%p)::~EdUndo...\n", this);
}

//----------------------------------------------------------------------------
// EdUndo::Methods
//----------------------------------------------------------------------------
public:
void
   redo(                            // Redo this action
     EdFile*           file)        // For this File
{  (void)file; }                    // NOT CODED YET

void
   undo(                            // Undo this action
     EdFile*           file)        // For this File
{  (void)file; }                    // NOT CODED YET
}; // class EdUndo

//----------------------------------------------------------------------------
//
// Class-
//       EdFile
//
// Purpose-
//       Editor File
//
//----------------------------------------------------------------------------
class EdFile : public ::pub::List<EdFile>::Link { // Editor file descriptor
//----------------------------------------------------------------------------
// EdFile::Attributes
public:
enum MODE { M_NONE, M_BIN, M_DOS, M_MIX, M_UNIX }; // The file mode

::pub::List<EdMess>    messages;    // The List of warning messages
::pub::List<EdLine>    lines;       // The line list
::std::string          name;        // The file name
size_t                 rows= 0;     // The number of file rows

int                    mode= M_NONE; // The file mode
bool                   changed= false; // File is changed
bool                   damaged= false; // File is damaged

// ::pub::List<EdUndo> redos;       // The redo list
// ::pub::List<EdUndo> undos;       // The undo list

// Cursor position controls
EdLine*                top_line= nullptr; // The current top Line
EdLine*                csr_line= nullptr; // The current cursor (active) Line
size_t                 col_zero= 0; // The current top column[0]
size_t                 row_zero= 0; // The current top row[0]
unsigned               col= 0;      // The current cursor column (offset)
unsigned               row= 0;      // The current cursor row (offset)

//----------------------------------------------------------------------------
// EdFile::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdFile(                          // Constructor
     const char*       name= nullptr) // File name
:  ::pub::List<EdFile>::Link()
,  name(name ? name : "unnamed.txt")
{
   if( xcb::opt_hcdm )
     xcb::debugh("EdFile(%p)::EdFile(%s)\n", this, get_name().c_str());

   EdLine* top= new EdLine("* * * * Top of file * * * *");
   EdLine* bot= new EdLine("* * * * End of file * * * *");
   lines.fifo(top);
   lines.fifo(bot);
   top->flags= EdLine::F_PROT;
   bot->flags= EdLine::F_PROT;

   top_line= top;
   csr_line= top;

   if( name )
     append(name, top);             // Insert the file
}

//----------------------------------------------------------------------------
virtual
   ~EdFile( void )                  // Destructor
{
   if( xcb::opt_hcdm )
     xcb::debugh("EdFile(%p)::~EdFile\n", this);

   for(;;) {                        // Delete all lines
     EdLine* line= lines.remq();
     if( line == nullptr )
       break;

     delete line;
   }

   reset();
}

//----------------------------------------------------------------------------
// EdFile::Accessor methods
//----------------------------------------------------------------------------
// TODO: REFACTOR message operations in conjunction with EdText
public:
EdMess*                             // The current EdMess
   get_message( void ) const        // Get current EdMess
{  return messages.get_head(); }

// TODO: VERIFY USAGE
EdLine*                             // The EdLine*
   get_line(                        // Get EdLine*
     size_t            row) const   // For this row number
{
   EdLine* line= lines.get_head();  // Get top line
   while( row > 0 ) {               // Locate row
     line= (EdLine*)line->get_next();
     if( line == nullptr )          // SHOULD NOT OCCUR
       break;

     row--;
   }

   return line;
}

std::string
   get_name( void ) const           // Get the file name (Named interface)
{  return name; }

// TODO: VERIFY USAGE
size_t                              // The row number
   get_row(                         // Get row number
     xcb::Line*        cursor) const // For this line
{
   size_t row= 0;
   for(EdLine* line= lines.get_head(); line; line= (EdLine*)line->get_next() ) {
     if( line == cursor )
       return row;

     row++;
   }

   return row;                      // SHOULD NOT OCCUR
}

char*
   get_text(                        // Allocate file text
     size_t            size) const  // Of this length
{  return Editor::editor->get_text(size); }

void
   put_message(                     // Write message
     std::string       _mess,       // Message text
     int               _type= EdMess::T_INFO) // Message mode
{
   EdMess* mess= messages.get_head();
   if( mess && _type <= mess->type )
     return;

   messages.fifo(new EdMess(_mess, _type));
}

int                                 // TRUE if message removed or remain
   rem_message( void )              // Remove current EdMess
{
   EdMess* mess= messages.remq();
   delete mess;
   return bool(mess) || bool(messages.get_head());
}

int                                 // TRUE if message removed or remain
   rem_message_type(                // Remove current EdMess
     int                _type= 0)   // If at this level or lower
{
   EdMess* mess= messages.get_head();
   if( mess && _type >= mess->type ) {
     messages.remove(mess, mess);
     delete mess;
     return true;
   }

   return bool(messages.get_head());
}

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
     EdLine*           line)        // Insert after this line
{
   struct stat st;                  // File stats
   int rc= stat(name, &st);         // Get file information
   if( rc != 0 ) {                  // If failure
     put_message("File not found");
     return line;
   }

   // Allocate the input data area Pool
   size_t size= st.st_size;         // The size of the file
   char* text= get_text(size + 1);  // Allocate space for entire file (+ '\0')
   memset(text, 0, size + 1);       // (In case read fails)

   // Load the file
   FILE* f= fopen(name, "rb");
   size_t L= fread(text, 1, size, f);
   if( L != size )
   {
     damaged= true;
     put_message("Read failure");
     size= L;
   }
   fclose(f);

   // Check for binary file
   char* last= strchr(text, '\0');
   if( size_t(last - text) < size ) { // If file contains '\0' delimiter
     put_message("Binary file");
     mode= M_BIN;
   }

   // Parse the text into lines (Performance critical path)
   char* used= text;
   while( used < last )
   {
     char* from= used;              // Starting character
     line= insert(line, new EdLine(from));

     char* nend= strchr(used, '\n'); // Get next line delimiter
     if( nend == nullptr ) {        // Missing '\\n' delimiter
       size_t L= strlen(from);      // String length
       if( from + L >= last ) {
         put_message("Ending '\\n' missing");
         break;
       }

       nend= from + L;              // '\0' delimiter found
       line->delim[1]= 1;
       while( ++nend < last ) {
         if( *nend ) break;         // If not a '\0' delimiter
         if( ++line->delim[1] == 0 ) { // If repetition count overflow
           line->delim[1]= 255;
           line= insert(line, new EdLine(nend));
           line->delim[1]= 1;
         }
       }
       used= nend;
       continue;
     }

     // '\n' delimiter found
     *nend= '\0';                   // Replace with string delimiter
     used= nend + 1;                // Next line origin
     if( nend == from || *(nend-1) != '\r' ) { // If UNIX delimiter
       line->delim[0]= '\n';
       if( mode == M_UNIX || mode == M_MIX || mode == M_BIN ) continue;
       if( mode == M_NONE ) {
         mode= M_UNIX;
       } else {                     // Mode M_DOS ==> M_MIX
         mode= M_MIX;
       }
     } else {                       // IF DOS delimiter
       line->delim[1]= '\r';
       *(nend - 1)= '\0';
       if( mode == M_DOS || mode == M_MIX || mode == M_BIN ) continue;
       if( mode == M_NONE ) {
         mode= M_DOS;
       } else {                     // Mode M_UNIX ==> M_MIX
         mode= M_MIX;
       }
     }
   }

   return line;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::insert
//
// Purpose-
//       Insert file lines (or line)
//
//----------------------------------------------------------------------------
EdLine*                             // (Always tail)
   insert(                          // Insert
     EdLine*           after,       // After this line
     EdLine*           head,        // From this line
     EdLine*           tail)        // Upto this line
{
   lines.insert(after, head, tail);

   for(EdLine* line= head; line != tail; line= line->get_next()) {
     if( line == nullptr ) throw "Invalid insert chain";
     rows++;
   }
   rows++;                          // (Count the tail line)

   return tail;
}

EdLine*                             // (Always line)
   insert(                          // Insert
     EdLine*           after,       // After this line
     EdLine*           line)        // This line
{  return insert(after, line, line); }

//----------------------------------------------------------------------------
//
// Method-
//       EdFile::remove
//
// Purpose-
//       Remove file lines (or line)
//
//----------------------------------------------------------------------------
void
   remove(                          // Remove
     EdLine*           head,        // From this line
     EdLine*           tail)        // Upto this line
{
   lines.remove(head, tail);

   for(EdLine* line= head; line != tail; line= line->get_next()) {
     if( line == nullptr ) throw "Invalid remove chain";
     rows--;
   }
   rows--;                          // (Count the tail line)
}

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
//       Reset the undo/redo lists
//
//----------------------------------------------------------------------------
void                                // (Action cannot be undone)
   reset( void )                    // Reset the undo/redo lists
{  } // TODO: NOT CODED YET
}; // class EdFile
#endif // EDFILE_H_INCLUDED
