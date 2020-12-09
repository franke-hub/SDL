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
//       Editor: Line/block marker
//
// Last change date-
//       2020/12/08
//
//----------------------------------------------------------------------------
#ifndef EDMARK_H_INCLUDED
#define EDMARK_H_INCLUDED

#include <pub/List.h>               // For List

#include "EdFile.h"                 // For EdFile, EdLine

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
EdLine*                line= nullptr; // The first marked line

EdLine*                touch_line= nullptr; // The last line marked
ssize_t                touch_col= 0; // The last column marked (-1 for line)

// Copy/Cut information
pub::List<EdLine>      mark_list;   // The current copy/cut
size_t                 mark_rows= 0; // The number of copy/cut rows
size_t                 lh_column= 0; // Left-hand column
size_t                 rh_column= 0; // Right-hand column (+1)

//----------------------------------------------------------------------------
// EdMark::Constructor/Destructor
//----------------------------------------------------------------------------
public:
   EdMark( void );                  // Constructor
   ~EdMark( void );                 // Destructor

void
   initialize( void );              // Initialize EdFile::CloseEvent Connector

//----------------------------------------------------------------------------
// EdMark::Methods
//----------------------------------------------------------------------------
public:
const char*                         // Error message, nullptr expected
   copy( void );                    // Copy the marked area

const char*                         // Error message, nullptr expected
   cut( void );                     // Remove the marked area

const char*                         // Error message, nullptr expected
   format( void );                  // Format the mark

const char*                         // Error message, nullptr expected
   mark(                            // Create/expand/contract the mark
     EdFile*           file,        // For this EdFile
     EdLine*           line,        // And this EdLine
     ssize_t           column= 0);  // Start at this column (block copy)

const char*                         // Error message, nullptr expected
   paste(                           // Paste the marked area
     EdFile*           file,        // Into this EdFile
     EdLine*           line,        // After this line
     ssize_t           column= 0);  // Start at this column (block copy)

void
   reset( void );                   // Remove (undo) mark
}; // class EdMark
#endif // EDMARK_H_INCLUDED
