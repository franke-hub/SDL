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
//       Player.cpp
//
// Purpose-
//       Player implementation.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#undef minor                        // Garbage clean up

#include "Define.h"
#include "Controls.h"
#include "Hand.h"
#include "Player.h"
#include "Table.h"
#include "Utility.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Player::~Player
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Player::~Player( void )          // Destructor
{
   #ifdef HCDM
     printf("Player(%p)::~Player()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Player::Player
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Player::Player( void )           // Default constructor
:  name("NOBODY")
,  table(NULL)
,  cardCount(0)
{
   #ifdef HCDM
     printf("Player(%p)::Player()\n", this);
   #endif
}

   Player::Player(                  // Constructor
     const char*       name)        // The Player's name
:  name(name)
,  table(NULL)
,  cardCount(0)
{
   #ifdef HCDM
     printf("Player(%p)::Player(%s)\n", this, name);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Player::addCard
//
// Purpose-
//       Add a Card to the Hand
//
//----------------------------------------------------------------------------
void
   Player::addCard(                 // Add a Card to the Hand
     Card*             card)        // Add this Card
{
   if( cardCount >= Hand::MAX_HAND )
     throw "Player::addCard.Overflow";

   this->card[cardCount++]= card;
}

//----------------------------------------------------------------------------
//
// Method-
//       Player::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Player::debug( void )            // Display the Player
{
   char                string[Card::SIZE_NAME]; // Working string

   printf("Player(%p)::debug() '%s' cardCount(%d)\n", this, name, cardCount);

   for(int i= 0; i<cardCount; i++)
     printf("[%2d] %s\n", i, card[i]->toString(string));
}

//----------------------------------------------------------------------------
//
// Method-
//       Player::display
//
// Purpose-
//       Display the Player
//
//----------------------------------------------------------------------------
void
   Player::display( void )          // Display the Player
{
   char                string[Card::SIZE_NAME]; // Working string

   printf("Player(%s)\n", name);

   for(int i= 0; i<cardCount; i++)
     printf("[%2d] %s\n", i, card[i]->toString(string));
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       Player::reset
//
// Purpose-
//       Reset the Player's Cards
//
//----------------------------------------------------------------------------
void
   Player::reset( void )            // Reset a Player
{
   #ifdef HCDM
     printf("Player(%p)::reset()\n", this);
   #endif

   cardCount= 0;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::~PokerPlayer
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   PokerPlayer::~PokerPlayer( void ) // Destructor
{
   #ifdef HCDM
     printf("PokerPlayer(%p)::~PokerPlayer()\n", this);
   #endif

   if( strategy != NULL )
   {
     delete strategy;
     strategy= NULL;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::PokerPlayer
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   PokerPlayer::PokerPlayer( void ) // Default constructor
:  Player("NOBODY")
,  hand()
,  stake(0)
,  strategy(NULL)
{
   #ifdef HCDM
     printf("PokerPlayer(%p)::PokerPlayer()\n", this);
   #endif
}

   PokerPlayer::PokerPlayer(        // Constructor
     const char*       name,        // The PokerPlayer's name
     int               stake)       // The PokerPlayer's initial stake
:  Player(name)
,  hand()
,  stake(stake)
,  strategy(NULL)
{
   #ifdef HCDM
     printf("PokerPlayer(%p)::PokerPlayer(%s,%d)\n", this, name, stake);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::setStrategy
//
// Purpose-
//       Set the Player's PokerStrategy
//
//----------------------------------------------------------------------------
void
   PokerPlayer::setStrategy(        // Set PokerPlayer's Strategy
     PokerStrategy*    strategy)    // The new Strategy (Owned by PokerPlayer)
{
   if( this->strategy != NULL )
     delete this->strategy;

   this->strategy= strategy;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::getTable
//
// Purpose-
//       Get the associated PokerTable
//
//----------------------------------------------------------------------------
PokerTable*                         // ->  associated PokerTable
   PokerPlayer::getTable( void ) const // Get associated PokerTable
{
   return check_cast(PokerTable*,table);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::setTable
//
// Purpose-
//       Set the associated PokerTable
//
//----------------------------------------------------------------------------
void
   PokerPlayer::setTable(           // Set the PokerTable
     PokerTable*       table)       // The PokerTable
{
   this->table= table;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::addCard
//
// Purpose-
//       Add a Card to the Hand
//
//----------------------------------------------------------------------------
void
   PokerPlayer::addCard(            // Add a Card to the Hand
     Card*             card)        // Add this Card
{
   Player::addCard(card);
   hand.load(cardCount, this->card);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::ante
//
// Purpose-
//       Ante up.
//
//----------------------------------------------------------------------------
int                                 // Actual ante
   PokerPlayer::ante(               // Ante up
     int               amount)      // This amount
{
   int                 result;

   #ifdef HCDM
     printf("PokerPlayer(%p)::ante(%d)\n", this, amount);
   #endif
   reset();

   if( amount >= stake )
   {
     result= stake;
     stake= 0;
   }
   else
   {
     result= amount;
     stake -= amount;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::bet
//
// Purpose-
//       Make a bet.
//
//----------------------------------------------------------------------------
int                                 // Actual bet
   PokerPlayer::bet(                // Make a bet
     int               amount)      // Minimum to call
{
   int                 result= 0;   // Actual bet

   PokerTable*         table= getTable();

   int                 maxRaise= table->getMaxRaise();

   //-------------------------------------------------------------------------
   // Handle special (should not occur) conditions
   //-------------------------------------------------------------------------
   if( stake == 0 )                 // If nothing left to bet with
     return 0;                      // ALL IN

   if( table->getActiveCount() <= 1 ) // If we are the only player
     return amount;                 // We call

   if( maxRaise > 0 && amount > maxRaise ) // If minimum > maximum
     amount= maxRaise;

   //-------------------------------------------------------------------------
   // BET
   //-------------------------------------------------------------------------
   result= strategy->bet(amount);

   if( result > stake )
     result= stake;

   else if( result < amount )
     result= 0;

   else if( maxRaise > 0 && result > maxRaise )
     result= maxRaise;

   stake -= result;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::compare
//
// Purpose-
//       Compare our Hand to another Player's.
//
//----------------------------------------------------------------------------
int                                 // +1 (This better), =0, -1 (This worse)
   PokerPlayer::compare(            // Compare Player
     PokerPlayer*      player)      // With this Player
{
   return hand.compare(player->hand);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   PokerPlayer::debug( void )       // Display the PokerPlayer
{
   printf("PokerPlayer(%p)::debug()\n", this);
   printf("name(%s) stake(%d)\n", name, stake);
   hand.debug();
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::display
//
// Purpose-
//       Display the PokerPlayer
//
//----------------------------------------------------------------------------
void
   PokerPlayer::display( void )     // Display the PokerPlayer
{
   printf("Player %s has %s\n", name, hand.getRankName());
   hand.display();
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerPlayer::reset
//
// Purpose-
//       Reset the PokerPlayer
//
//----------------------------------------------------------------------------
void
   PokerPlayer::reset( void )       // Reset the PokerPlayer
{
   #ifdef HCDM
     printf("PokerPlayer(%p)::reset()\n", this);
   #endif

   Player::reset();

   if( strategy == NULL )
     strategy= check_cast(PokerStrategy*,table->makeStrategy(this));
   strategy->reset();
}

