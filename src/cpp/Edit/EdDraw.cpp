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
//       EdDraw.cpp
//
// Purpose-
//       EdDraw object methods.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#include "EdDraw.h"

//----------------------------------------------------------------------------
//
// Method-
//       EdDraw::~EdDraw
//
// Purpose-
//       EdDraw destructor
//
//----------------------------------------------------------------------------
   EdDraw::~EdDraw( void )          // Destructor
{
   #ifdef SCDM
     tracef("%4d EdDraw(%p)::~EdDraw()\n", __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdDraw::EdDraw
//
// Purpose-
//       EdDraw constructor
//
//----------------------------------------------------------------------------
   EdDraw::EdDraw(                  // Constructor
     Terminal*         terminal)    // Terminal
:  terminal(terminal)
{
   #ifdef SCDM
     tracef("%4d EdDraw(%p)::EdDraw(%p)\n", __LINE__, this, terminal);
   #endif
}

