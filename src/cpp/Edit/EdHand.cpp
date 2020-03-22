//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       EdHand.cpp
//
// Purpose-
//       EdHand object methods.
//
// Last change date-
//       2018/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <com/syslib.h>

#include "EdLine.h"
#include "EdRing.h"

#include "EdHand.h"

//----------------------------------------------------------------------------
//
// Method-
//       EdHand::~EdHand
//
// Purpose-
//       EdHand constructor
//
//----------------------------------------------------------------------------
   EdHand::~EdHand( void )          // Destructor
{
   #ifdef SCDM
     tracef("%4d EdHand(%p)::~EdHand()\n", __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdHand::EdHand
//
// Purpose-
//       EdHand constructor
//
//----------------------------------------------------------------------------
   EdHand::EdHand(                  // Constructor
     Terminal*         terminal)    // -> Terminal to be handled
:  Handler()
,  terminal(terminal)
{
   #ifdef SCDM
     tracef("%4d EdHand(%p)::EdHand()\n", __LINE__, this);
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       EdHand::handleAbort
//
// Purpose-
//       Handle 'abort' function.
//
//----------------------------------------------------------------------------
void
   EdHand::handleAbort( void )      // Handle terminal abort
{
   debugf("Edit: ABORT(%d) event\n", getIdent());
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       EdHand::handleError
//
// Purpose-
//       Handle 'error' function.
//
//----------------------------------------------------------------------------
void
   EdHand::handleError( void )      // Handle terminal error
{
   #ifdef HCDM
     tracef("Edit: ERROR(%d) event\n", getIdent());
   #endif
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       EdHand::handleEvent
//
// Purpose-
//       Handle 'event' function.
//
//----------------------------------------------------------------------------
void
   EdHand::handleEvent( void )      // Handle terminal event
{
   #ifdef HCDM
     tracef("Edit: EVENT(%d) event\n", getIdent());
   #endif

   if( getIdent() == Terminal::EventResize )
     terminal->handleResizeEvent();
}

