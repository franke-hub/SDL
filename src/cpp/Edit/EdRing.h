//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2016 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdRing.h
//
// Purpose-
//       Editor: Ring functions.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#ifndef EDRING_H_INCLUDED
#define EDRING_H_INCLUDED

#ifndef EDITOR_H_INCLUDED
#include "Editor.h"
#endif

#ifndef EDLINE_H_INCLUDED
#include "EdLine.h"
#endif

#ifndef EDPOOL_H_INCLUDED
#include "EdPool.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       EdRing
//
// Purpose-
//       Editor ring.
//
//----------------------------------------------------------------------------
class EdRing : public List<EdRing>::Link { // Editor ring file descriptor
//----------------------------------------------------------------------------
// EdRing::Enumerations and typedefs
//----------------------------------------------------------------------------
public:
enum
{  LEN_FN= FILENAME_MAX
,  LEN_PN= FILENAME_MAX
,  MAX_UNDO= 4                      // Number of saved deletes
}; // enum

enum FileMode                       // File mode
{  FM_RESET                         // Reset (undefined)
,  FM_UNIX                          // Unix mode
,  FM_DOS                           // DOS mode
,  FM_MIXED                         // Mixed DOS/Unix mode
,  FM_BINARY                        // Binary mode
}; // enum RC

enum FileType                       // File type
{  FT_RESET                         // Reset (undefined)
,  FT_DATA                          // Normal data file
,  FT_PROTECTED                     // Internal file
,  FT_UNUSABLE                      // File is not usable
}; // enum RC

//----------------------------------------------------------------------------
// EdRing::Constructors
//----------------------------------------------------------------------------
public:
   ~EdRing( void );                 // Destructor
   EdRing( void );                  // Constructor
   EdRing(                          // Constructor
     const char*       fileName);   // Filename (for display only, not loaded)

//----------------------------------------------------------------------------
// EdRing::Methods
//----------------------------------------------------------------------------
public:
char*                               // -> Allocated text string
   allocateText(                    // Allocate text string
     unsigned          size);       // Of this size

const char*                         // Return message (NULL OK)
   append(                          // Append
     const char*       fileName,    // This file
     EdLine*           edLine);     // After this line

int                                 // Return code (TRUE || FALSE)
   contains(                        // Does this EdRing
     const char*       fileDesc) const; // Contain this file?

EdLine*                             // -> New, empty EdLine
   insertLine(                      // Allocate and insert new line
     EdLine*           edLine);     // After this line

const char*                         // Return message (NULL OK)
   read(                            // Read (load) the ring
     const char*       fileName);   // Using this fileName

void
   releaseText(                     // Release text string
     char*             addr);       // At this address

void
   removeLine(                      // Remove and delete (no undo)
     EdLine*           head,        // From this line
     EdLine*           tail);       // To this line

void
   removeLine(                      // Remove and delete (no undo)
     EdLine*           edLine);     // This line

void
   removeUndo(                      // Remove and delete with undo
     EdLine*           head,        // From this line
     EdLine*           tail);       // To this line

void
   removeUndo(                      // Remove and delete with undo
     EdLine*           edLine);     // This line

void
   reset( void );                   // Reset (empty) the ring

void
   resetCache( void );              // Reset the rownumber cache

void
   resetUndo( void );               // Reset the UNDO array

int                                 // Row number, (-1) if not present
   rowNumber(                       // Does this EdRing
     EdLine*           edLine);     // Contain this EdLine?

const char*                         // Return message
   undo(                            // Undo last removeUndo
     EdLine*&          head,        // First undo line
     EdLine*&          tail);       // Final undo line

const char*                         // Return message (NULL OK)
   write( void );                   // Write the ring

const char*                         // Return message (NULL OK)
   write(                           // Write the ring
     const char*       fileName);   // Using this fileName

//----------------------------------------------------------------------------
// EdRing::Debugging methods
//----------------------------------------------------------------------------
public:
void
   check( void ) const;             // Debugging check

void
   check(                           // Verify that Lines are contained
     const char*       file,        // Caller's file name
     int               line,        // Caller's line number
     const EdLine*     head,        // First Line of range
     const EdLine*     tail) const; // Final Line of range

void
   debug(                           // Debugging display
     const char*       message= "") const; // Display message

//----------------------------------------------------------------------------
// EdRing::Internal methods
//----------------------------------------------------------------------------
protected:
void
   deleteList(                      // Remove a list of EdLines
     EdLine*           edLine);     // List of lines, NULL terminated

//----------------------------------------------------------------------------
// EdRing::Attributes
//----------------------------------------------------------------------------
public:
   //-------------------------------------------------------------------------
   // File controls
   char                pathName[LEN_PN]; // Source path name qualifier
   char                fileName[LEN_FN]; // Source file name qualifier
   char                autoName[16];// Autosave file name, format
                                    // "AUTOSAVE.nnn" or "" if unassigned

   int                 mode;        // File mode (FileMode)
   int                 type;        // File type (FileType)
   unsigned char       changed;     // TRUE if file is changed
   unsigned char       damaged;     // TRUE if file is damaged

   //-------------------------------------------------------------------------
   // Allocation controls
   Pool                linePool;    // Storage pool for lines
   EdPool              textPool;    // Storage pool for text (line data)

   //-------------------------------------------------------------------------
   // Editor lines
   unsigned int        rows;        // Number of rows (includes predefineds)
   List<EdLine>        lineList;    // The EdLine list

   EdLine              topOfFile;   // The top of file line
   EdLine              botOfFile;   // The end of file line

   unsigned int        undoCount;   // Number of delete undo's
   EdLine*             undoArray[MAX_UNDO]; // Delete UNDO array

   unsigned int        cacheRow;    // Cached row number
   EdLine*             cacheLine;   // Cached line pointer

   //-------------------------------------------------------------------------
   // Viewing controls
   EdLine*             firstLine;   // First screen line
   unsigned int        firstCol;    // First screen column (0 origin)

   EdLine*             curLine;     // -> Current file line
   unsigned int        curCol;      // Current file column offset
   unsigned int        curRow;      // Current file row offset
}; // class EdRing

#endif // EDRING_H_INCLUDED
