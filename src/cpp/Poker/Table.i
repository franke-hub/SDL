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
//       Table.i
//
// Purpose-
//       Table inline methods.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef TABLE_I_INCLUDED
#define TABLE_I_INCLUDED

//----------------------------------------------------------------------------
// Table::Accessor methods
//----------------------------------------------------------------------------
const char*                         // The Table's game
   Table::getGame( void ) const     // Get Table's game
{
   return game;
}

const char*                         // The Table's name
   Table::getName( void ) const     // Get Table's name
{
   return name;
}

Player*                             // -> Player
   Table::getPlayer(                // Get Player
     int               seat) const  // At this seat
{
   return player[seat];
}

int                                 // The number of Players
   Table::getPlayerCount( void ) const // Get number of Players
{
   return playerCount;
}

int                                 // The seat
   Table::getSeat(                  // Get seat
     Player*           player) const// For this Player
{
   int                 seat;        // The seat number

   for(seat= 0; seat<playerCount; seat++)
   {
     if( player == this->player[seat] )
       return seat;
   }

   throw "Table::getSeat.NoSuchPlayer";
}

//----------------------------------------------------------------------------
//
// Method-
//       Table::nextSeat
//
// Purpose-
//       Get the next seat index
//
//----------------------------------------------------------------------------
int                                 // The next seat
   Table::nextSeat(                 // Get next seat
     int               seat) const  // From this seat
{
   seat++;
   if( seat >= playerCount )
     seat= 0;

   return seat;
}

//----------------------------------------------------------------------------
//
// Method-
//       Table::priorSeat
//
// Purpose-
//       Get the prior seat index
//
//----------------------------------------------------------------------------
int                                 // The prior seat
   Table::priorSeat(                // Get prior seat
     int               seat) const  // From this seat
{
   if( seat == 0 )
     seat= playerCount;

   seat--;
   return seat;
}

//----------------------------------------------------------------------------
// PokerTable::Accessor methods
//----------------------------------------------------------------------------
int                                 // The number of active Players
   PokerTable::getActiveCount( void ) const // Get number of active Players
{
   int                 result= 0;
   int                 seat;

   for(seat= 0; seat<playerCount; seat++)
     result += (folded[seat] == FALSE);

   return result;
}

int                                 // The amount bet
   PokerTable::getAmount(           // Get amount bet
     int               seat) const  // For this seat
{
   return amount[seat];
}

PokerTable::BetLimit                // The betting limit
   PokerTable::getBetLimit( void ) const // Get betting limit
{
   return betLimit;
}

int                                 // The big blind ante
   PokerTable::getBigBlind( void ) const // Get big blind ante
{
   return bBlind;
}

int                                 // The number of bets without a raise
   PokerTable::getCallCount( void ) const // Get number of bets without raise
{
   return callCount;
}

int                                 // The FOLDED attribute
   PokerTable::getFolded(           // Get FOLDED attribute
     int               seat) const  // For this seat
{
   return folded[seat];
}

PokerHistory&                       // The PokerHistory
   PokerTable::getHistory( void )   // Get PokerHistory
{
   return history;
}

int                                 // The amount of money in the POT
   PokerTable::getPot( void ) const // Get amount of money in the POT
{
   return pot;
}

int                                 // The amount raised
   PokerTable::getRaised(           // Get amount raised
     int               seat) const  // For this seat
{
   return raised[seat];
}

int                                 // The number of different raisers
   PokerTable::getRaiseCount( void ) const // Get raise count
{
   int                 result= 0;

   int                 seat;

   for(seat= 0; seat < playerCount; seat++)
   {
     if( !folded[seat] && raised[seat] > 0 )
       result++;
   }

   return result;
}

int                                 // The current betting round
   PokerTable::getRound( void ) const // Get current betting round
{
   return round;
}

int                                 // The number of betting rounds
   PokerTable::getRoundCount( void ) const // Get number of betting rounds
{
   return roundCount;
}

int                                 // The small blind ante
   PokerTable::getSmallBlind( void ) const // Get small blind ante
{
   return sBlind;
}

void
   PokerTable::setAnte(             // Set common ante
     int               ante)        // The common ante
{
   this->ante= ante;
}

void
   PokerTable::setBetLimit(         // Set betting limit
     BetLimit          limit)       // The betting limit
{
   betLimit= limit;
}

void
   PokerTable::setBigBlind(         // Set big blind
     int               ante)        // The big blind
{
   bBlind= ante;
}

void
   PokerTable::setSmallBlind(       // Set small blind
     int               ante)        // The small blind
{
   sBlind= ante;
}

#endif // TABLE_I_INCLUDED
