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
//       EdLine.cpp
//
// Purpose-
//       EdLine object methods.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <com/syslib.h>

#include "EdLine.h"
#include "EdRing.h"

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::check
//
// Purpose-
//       Debugging consistency check.
//
//----------------------------------------------------------------------------
void
   EdLine::check( void ) const      // Debugging check
{
#ifdef HCDM
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::debug
//
// Purpose-
//       Debugging display.
//
//----------------------------------------------------------------------------
void
   EdLine::debug(                   // Debugging display
     const char*       message) const // Display message
{
#ifdef HCDM
   tracef("%4d EdLine(%p)::debug(%s) %.2x '%s'\n", __LINE__, this, message,
          *((int*)&ctrl), getText());
#endif
}

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::~EdLine
//
// Purpose-
//       EdLine destructor
//
//----------------------------------------------------------------------------
   EdLine::~EdLine( void )          // Destructor
{
   assert( ctrl.readonly || text == NULL );
}

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::EdLine
//
// Purpose-
//       EdLine constructor
//
//----------------------------------------------------------------------------
   EdLine::EdLine( void )           // Constructor
:  List<EdLine>::Link()
,  text(NULL)
{
   *((unsigned*)&ctrl)= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       EdLine::between
//
// Purpose-
//       Determine whether a line is within a range of lines.
//
//----------------------------------------------------------------------------
int                                 // TRUE if within range
   EdLine::between(                 // Is line within range?
     const EdLine*     head,        // First range line
     const EdLine*     tail) const  // Final range line
{
   const EdLine*       line;        // Working Line

   for(line= head; line != NULL; line= (EdLine*)line->getNext())
   {
     if( line == this )
       return TRUE;

     if( line == tail )
       break;
   }

   return FALSE;
}

