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
//       EdNark.h
//
// Purpose-
//       Editor: Line/block mark descriptor
//
// Last change date-
//       2020/12/11
//
//----------------------------------------------------------------------------
#ifndef EDMARK_H_INCLUDED
#define EDMARK_H_INCLUDED

#include <pub/List.h>               // For List

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class EdFile;
class EdLine;
class EdRedo;

//----------------------------------------------------------------------------
//
// Class-
//       EdMark
//
// Purpose-
//       Line/block marker
//
//----------------------------------------------------------------------------
class EdMark {                      // Editor mark descriptor
//----------------------------------------------------------------------------
// EdMark::Attributes
//----------------------------------------------------------------------------
public:
// File touch information
EdFile*                file= nullptr; // The marked file
EdLine*                head= nullptr; // The first marked line
EdLine*                tail= nullptr; // The last  marked line

EdLine*                touch_line= nullptr; // The last touched line
ssize_t                touch_col= -1; // The last column marked (-1 for line)

// Copy/Cut information
pub::List<EdLine>      mark_list;   // The current copy/cut
size_t                 mark_rows= 0; // The number of copy/cut rows
size_t                 lh_column= -1; // Left-hand column
size_t                 rh_column= -1; // Right-hand column (+1)

//----------------------------------------------------------------------------
// EdMark::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdMark( void );                  // Constructor
   ~EdMark( void );                 // Destructor

//----------------------------------------------------------------------------
// EdMark::Methods
//----------------------------------------------------------------------------
public:
void
   debug(                           // Debugging display
     const char*       info= nullptr); // Associated text

const char*                         // Error message, nullptr expected
   copy( void );                    // Copy the marked area

const char*                         // Error message, nullptr expected
   cut( void );                     // Remove the marked area

const char*                         // Error message, nullptr expected
   format( void );                  // Format the mark

void
   handle_redo(                     // Handle completed redo operation
     EdFile*           file,        // For this file
     EdRedo*           redo);       // And this REDO

void
   handle_undo(                     // Handle completed undo operation
     EdFile*           file,        // For this file
     EdRedo*           undo);       // And this UNDO

const char*                         // Error message, nullptr expected
   mark(                            // Create/expand/contract the mark
     EdFile*           file,        // For this EdFile
     EdLine*           line,        // And this EdLine
     ssize_t           column= -1); // Start at this column (block copy)

const char*                         // Error message, nullptr expected
   paste(                           // Paste the marked area
     EdFile*           file,        // Into this EdFile
     EdLine*           line,        // After this line
     ssize_t           column= -1); // Start at this column (block copy)

void
   redo(                            // Redo the mark
     EdFile*           file,        // For this EdFile
     EdLine*           head,        // From this EdLine
     EdLine*           tail);       // *To* this EdLine

void
   undo( void );                    // Undo the mark
}; // class EdMark
#endif // EDMARK_H_INCLUDED
