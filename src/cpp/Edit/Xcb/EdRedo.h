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
//       EdRedo.h
//
// Purpose-
//       Editor: Redo/Undo descriptor
//
// Last change date-
//       2024/03/30
//
// Implementation notes-
//       Implemented in EdFile.cpp
//
//----------------------------------------------------------------------------
#ifndef EDREDO_H_INCLUDED
#define EDREDO_H_INCLUDED

#include <pub/List.h>               // For pub::List

#include "Editor.h"                 // For Editor
#include "EdLine.h"                 // For EdLine

//----------------------------------------------------------------------------
//
// Class-
//       EdRedo
//
// Purpose-
//       Editor Redo/Undo descriptor
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
#endif // EDREDO_H_INCLUDED
