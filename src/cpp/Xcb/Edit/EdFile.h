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
//       2020/10/16
//
//----------------------------------------------------------------------------
#ifndef EDFILE_H_INCLUDED
#define EDFILE_H_INCLUDED

#include "Bringup.h"                // TODO: REMOVE

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
// EdLine::Typedefs and enumerations
//----------------------------------------------------------------------------
enum FLAGS                          // Status flags
{  F_NONE= 0x0000                   // No flags
,  F_PROT= 0x0001                   // Content cannot be modified
};

//----------------------------------------------------------------------------
// EdLine::Attributes
//----------------------------------------------------------------------------
struct {                            // Status/control flags
  unsigned               _00    : 8; // Reserved for expansion
  unsigned               _01    : 5; // Reserved for expansion
  unsigned               rdonly : 1; // Line is read-only
  unsigned               hidden : 1; // Line is hidden
  unsigned               marked : 1; // Line is marked
}                      flags;       // Control flags

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
,  flags {0, 0, 0, 0, 0}
{
   if( opt_hcdm && opt_verbose > 2 )
     debugh("EdLine(%p)::EdLine\n", this);
}

//----------------------------------------------------------------------------
virtual
   ~EdLine( void )                  // Destructor
{
   if( opt_hcdm && opt_verbose > 2 )
     debugh("EdLine(%p)::~EdLine...\n", this);
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
std::string            mess;        // The message

//----------------------------------------------------------------------------
// EdMess::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdMess(                          // Constructor
     std::string       text)        // Message text
:  ::pub::List<EdMess>::Link(), mess(text)
{
   if( opt_hcdm )
     debugh("EdMess(%p)::EdMess(%s)\n", this, text.c_str());
}

//----------------------------------------------------------------------------
virtual
   ~EdMess( void )                  // Destructor
{
   if( opt_hcdm )
     debugh("EdMess(%p)::~EdMess...\n", this);
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
   if( opt_hcdm )
     debugh("EdUndo(%p)::EdUndo\n", this);
}

//----------------------------------------------------------------------------
virtual
   ~EdUndo( void )                  // Destructor
{
   if( opt_hcdm )
     debugh("EdUndo(%p)::~EdUndo...\n", this);
}

//----------------------------------------------------------------------------
// EdUndo::Methods
//----------------------------------------------------------------------------
public: // None defined
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

// ::pub::List<EdLine> redos;       // The redo list
// ::pub::List<EdLine> undos;       // The undo list

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
   if( opt_hcdm )
     debugh("EdFile(%p)::EdFile(%s)\n", this, get_name().c_str());

   EdLine* top= new EdLine("* * * * Top of file * * * *");
   EdLine* bot= new EdLine("* * * * End of file * * * *");
   lines.fifo(top);
   lines.fifo(bot);
   top->flags.rdonly= true;
   bot->flags.rdonly= true;

   top_line= top;
   csr_line= top;

   if( name )
     append(name, top);             // Insert the file
}

//----------------------------------------------------------------------------
virtual
   ~EdFile( void )                  // Destructor
{
   if( opt_hcdm )
     debugh("EdFile(%p)::~EdFile...\n", this);

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
public:
std::string
   get_name( void ) const           // Get the file name (Named interface)
{  return name; }

char*
   get_text(                        // Allocate file text
     size_t            size) const  // Of this length
{  return Editor::editor->get_text(size); }

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
     messages.fifo(new EdMess("File not found"));
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
     messages.fifo(new EdMess("Read failure"));
     size= L;
   }
   fclose(f);

   // Check for binary file
   char* last= strchr(text, '\0');
   if( size_t(last - text) < size ) { // If file contains '\0' delimiter
     messages.fifo(new EdMess("Binary file"));
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
         messages.fifo(new EdMess("Ending '\\n' missing"));
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
//       Insert one file line
//
//----------------------------------------------------------------------------
EdLine*                             // The last insertedline
   insert(                          // Insert after
     EdLine*           after,       // This line
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

EdLine*                             // The new line
   insert(                          // Insert after
     EdLine*           after,       // After this line
     EdLine*           line)        // This line
{  return insert(after, line, line); }

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
