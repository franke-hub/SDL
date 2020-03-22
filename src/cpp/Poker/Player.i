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
//       Player.i
//
// Purpose-
//       Player inline methods.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef PLAYER_I_INCLUDED
#define PLAYER_I_INCLUDED

//----------------------------------------------------------------------------
// Player::Accessor methods
//----------------------------------------------------------------------------
int                                 // The Player's Card Count
   Player::getCardCount( void ) const   // Get Player's Card count
{
   return cardCount;
}

const char*                         // The Player's name
   Player::getName( void ) const    // Get Player's name
{
   return name;
}

Table*                              // ->  associated Table
   Player::getTable( void ) const   // Get associated Table
{
   return table;
}

void
   Player::setTable(                // Set the Table
     Table*            table)       // The Table
{
   this->table= table;
}

//----------------------------------------------------------------------------
//
// Method-
//       Player::store
//
// Purpose-
//       Store the Card array
//
//----------------------------------------------------------------------------
void
   Player::store(                   // Store the Hand
     Card*             that[]) const// The Card* array
{
   for(int i= 0; i<cardCount; i++)
     that[i]= card[i];
}

//----------------------------------------------------------------------------
// PokerPlayer::Accessor methods
//----------------------------------------------------------------------------
int                                 // The PokerPlayer's stake
   PokerPlayer::getStake( void ) const  // Get PokerPlayer's stake
{
   return stake;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::addStake
//
// Purpose-
//       Add to the PokerPlayer's stake
//
//----------------------------------------------------------------------------
void
   PokerPlayer::addStake(           // Add to PokerPlayer's stake
     int               amount)      // This amount
{
   stake += amount;
}

#endif // PLAYER_I_INCLUDED
