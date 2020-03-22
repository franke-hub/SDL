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
//       EdView.i
//
// Purpose-
//       Editor: EdView inline methods.
//
// Last change date-
//       2016/01/01 (Version 2, Release 1)
//
//----------------------------------------------------------------------------
#ifndef EDVIEW_I_INCLUDED
#define EDVIEW_I_INCLUDED

//----------------------------------------------------------------------------
//
// Method-
//       EdView::Accessor methods
//
// Purpose-
//       Get/set variables.
//
//----------------------------------------------------------------------------
Active*
   EdView::getActive( void ) const
{
   return active;
}

void
   EdView::setActive(
     Active*           active)
{
   this->active= active;
}

unsigned
   EdView::getColumn( void ) const
{
   return firstCol + curCol;
}

EdLine*
   EdView::getLine( void ) const
{
   return curLine;
}

EdRing*
   EdView::getRing( void ) const
{
   return curRing;
}

unsigned
   EdView::getRow( void ) const
{
   return firstRow + curRow;
}

int
   EdView::isDataView( void ) const
{
   return active != edit->histActive;
}

int
   EdView::isHistView( void ) const
{
   return active == edit->histActive;
}

#endif // EDVIEW_I_INCLUDED
