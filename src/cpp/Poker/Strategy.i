//----------------------------------------------------------------------------
//
//       Copyright (C) 2017 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Strategy.i
//
// Purpose-
//       Strategy inline methods; MUST BE INCLUDED SEPARATELY.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef STRATEGY_I_INCLUDED
#define STRATEGY_I_INCLUDED

//----------------------------------------------------------------------------
// Strategy::Accessor methods
//----------------------------------------------------------------------------
Player*                             // ->  associated Player
   Strategy::getPlayer( void ) const// Get associated Player
{
   return player;
}

Table*                              // ->  associated Table
   Strategy::getTable( void ) const // Get associated Table
{
   return table;
}

void
   Strategy::setTable(              // Set the Table
     Table*            table)       // The Table
{
   this->table= table;
}

//----------------------------------------------------------------------------
// PokerStrategy::Accessor methods
//----------------------------------------------------------------------------

#endif // STRATEGY_I_INCLUDED
