//----------------------------------------------------------------------------
//
//       Copyright (c) 2007-2020 Frank Eskesen.
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
//       2020/10/03 (Version 2, Release 1) - Extra compiler warnings
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
   const void* v= &ctrl;
   const int*  i= (const int*)v;
   tracef("%4d EdLine(%p)::debug(%s) %.2x '%s'\n", __LINE__, this, message,
          *i, getText());
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
// *((unsigned*)&ctrl)= 0;          // (Disallowed easy initializer)
   ctrl._0= 0;                      // Field by field initializer
   ctrl._1= 0;
   ctrl.readonly= 0;
   ctrl.marked= 0;
   ctrl._2= 0;
   ctrl.hidden= 0;
   ctrl.delim= 0;
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

