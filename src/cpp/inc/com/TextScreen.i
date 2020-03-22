//----------------------------------------------------------------------------
//
//       Copyright (c) 2007 Frank Eskesen.
//
//       This file is free content, distributed under the Lesser GNU
//       General Public License, version 3.0.
//       (See accompanying file LICENSE.LGPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/lgpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       TextScreen.i
//
// Purpose-
//       TextScreen inline methods.
//
// Last change date-
//       2007/01/01
//
//----------------------------------------------------------------------------
#ifndef TEXTSCREEN_I_INCLUDED
#define TEXTSCREEN_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::getCurrentCol
//
// Purpose-
//       Return the current screen column.
//
//----------------------------------------------------------------------------
unsigned int                        // The current screen column
   TextScreen::getCurrentCol( void )// Get the current screen column
{
   return currentCol;
}

//----------------------------------------------------------------------------
//
// Method-
//       TextScreen::getCurrentRow
//
// Purpose-
//       Return the current screen row.
//
//----------------------------------------------------------------------------
unsigned int                        // The current screen row
   TextScreen::getCurrentRow( void )// Get the current screen row
{
   return currentRow;
}

#endif // TEXTSCREEN_I_INCLUDED
