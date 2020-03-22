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
//       Table.cpp
//
// Purpose-
//       Table implementation.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>
#include <stdio.h>

#include "Player.h"
#include "Rating.h"
#include "Strategy.h"
#include "Table.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       Table::~Table
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Table::~Table( void )            // Destructor
{
   #ifdef HCDM
     printf("Table(%p)::~Table()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Table::Table
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Table::Table( void )             // Default constructor
:  name("Featured")
,  game("NONE")
,  playerCount(0)
{
   #ifdef HCDM
     printf("Table(%p)::Table()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Table::addPlayer
//
// Purpose-
//       Add a Player to the Table.
//
//----------------------------------------------------------------------------
void
   Table::addPlayer(                // Add a Player to the Table
     Player*           player)      // The Player
{
   #ifdef HCDM
     printf("Table(%p)::addPlayer(%s) %s\n", this, player->getName(), name);
   #endif

   if( playerCount >= MAX_PLAYER )
     throw "Table::addPlayer.TooManyPlayers";

   this->player[playerCount++]= player;
   player->setTable(this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Table::debug
//
// Purpose-
//       Debug the Table.
//
//----------------------------------------------------------------------------
void
   Table::debug( void )             // Debug the Table
{
   display();
}

//----------------------------------------------------------------------------
//
// Method-
//       Table::display
//
// Purpose-
//       Display the Table.
//
//----------------------------------------------------------------------------
void
   Table::display( void )           // Display the Table
{
   int                 seat;        // Seat index

   printf("Table(%p)::display() %s, %s count(%d)\n",
          this, game, name, playerCount);
   for(seat= 0; seat<playerCount; seat++)
   {
     printf("[%2d] %s\n", seat, player[seat]->getName());
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Table::makeStrategy
//
// Purpose-
//       Create a Strategy
//
//----------------------------------------------------------------------------
Strategy*                           // -> new Strategy
   Table::makeStrategy(             // Make a Strategy
     Player*           player)      // The PokerPlayer
{
   return new Strategy(player, this);
}

//----------------------------------------------------------------------------
//
// Method-
//       Table::play
//
// Purpose-
//       Play one hand
//
//----------------------------------------------------------------------------
void
   Table::play( void )              // Play one Hand
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Table::remPlayer
//
// Purpose-
//       Remove a Player from the Table.
//
//----------------------------------------------------------------------------
void
   Table::remPlayer(                // Remove a Player from the Table
     Player*           player)      // The Player
{
   int                 seat;

   #ifdef HCDM
     printf("Table(%p)::remPlayer(%s) %s\n", this, player->getName(), name);
   #endif

   player->setTable(NULL);
   for(seat= 0; seat<playerCount; seat++)
   {
     if( this->player[seat] == player )
       break;
   }

   if( seat >= playerCount )
     throw "Table::remPlayer.NoSuchPlayer";

   while( seat<(playerCount-1) )
   {
     this->player[seat]= this->player[seat+1];
     seat++;
   }

   player->setTable(NULL);
   this->player[seat]= NULL;
   playerCount--;

   reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       Table::reset
//
// Purpose-
//       Reset the Table.
//
//----------------------------------------------------------------------------
void
   Table::reset( void )             // Reset the Table
{
   #ifdef HCDM
     printf("Table(%p)::reset()\n");
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::getPlayer
//
// Purpose-
//       Cast player to PokerPlayer*.
//
//----------------------------------------------------------------------------
PokerPlayer*                        // -> PokerPlayer
   PokerTable::getPlayer(           // Get PokerPlayer
     int               seat) const  // At this seat
{
   return check_cast(PokerPlayer*, player[seat]);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::~PokerTable
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   PokerTable::~PokerTable( void )  // Destructor
{
   #ifdef HCDM
     printf("PokerTable(%p)::~PokerTable()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::PokerTable
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   PokerTable::PokerTable( void )   // Default constructor
:  Table()
,  deck()
,  ante(0)
,  bBlind(10)
,  sBlind(5)
,  betLimit(BETLIMIT_TABLE)
,  button(0)
,  callCount(0)
,  history()
,  last2Act(0)
,  pot(0)
,  round(0)
,  roundCount(0)
{
   #ifdef HCDM
     printf("PokerTable(%p)::PokerTable()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::getMaxRaise
//
// Purpose-
//       Get maximum raise amount.
//
//----------------------------------------------------------------------------
int                                 // The maximum raise amount
   PokerTable::getMaxRaise( void ) const // Get maximum raise amount
{
   int                 result= 0;   // Resultant

   switch( betLimit )
   {
     case BETLIMIT_TABLE:
       result= 0x7fffffff;
       break;

     case BETLIMIT_POT:
       result= getMinRaise();
       if( pot > result )
         result= pot;
       break;

     case BETLIMIT_NDQ:
       if( history.getRaiseCount() < 3 )
         result= getMinRaise() * 4;
       break;

     default:
       throw "PokerTable::getMaxRaise.InvalidBetLimit";
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::getMinRaise
//
// Purpose-
//       Get the minimum raise.
//
//----------------------------------------------------------------------------
int                                 // The minimum raise
   PokerTable::getMinRaise( void ) const // Get minimum raise
{
   int                 result;

   result= ante;
   if( sBlind > result )
     result= sBlind;
   if( bBlind > result )
     result= bBlind;

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::anteUp
//
// Purpose-
//       Start a new Hand
//
//----------------------------------------------------------------------------
void
   PokerTable::anteUp( void )       // Start a new Hand
{
   #ifdef HCDM
     printf("PokerTable(%p)::anteUp() %s\n", this, name);
   #endif

   int                 seat;        // Seat index

   //-------------------------------------------------------------------------
   // Ante up
   //-------------------------------------------------------------------------
   button++;
   if( button >= playerCount )
     button= 0;
   seat= nextSeat(button);
   amount[seat] += getPlayer(seat)->ante(sBlind);
   seat= nextSeat(seat);
   amount[seat] += getPlayer(seat)->ante(bBlind);

   pot= 0;
   for(seat= 0; seat<playerCount; seat++)
     pot += amount[seat];
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::betAround
//
// Purpose-
//       Round of betting.
//
//----------------------------------------------------------------------------
void
   PokerTable::betAround(           // Round of betting
     int               last2Act)    // Last player to act
{
   int                 actors;      // Number of players who can still act
   int                 betMade;     // Amount of bet actually made
   int                 betReqd;     // Amount of bet required
   int                 maxBet;      // The highest bet
   int                 next2Act;    // The first player to act
   int                 prior;       // Prior seat index
   int                 raised;      // TRUE iff bet raised
   int                 seat;        // Seat index

   #ifdef HCDM
     printf("PokerTable(%p)::betAround() %s\n", this, name);
   #endif

   actors= 0;
   maxBet= 0;
   for(seat= 0; seat<playerCount; seat++)
   {
     if( amount[seat] > maxBet )
       maxBet= amount[seat];

     if( !folded[seat] && getPlayer(seat)->getStake() > 0 )
       actors++;
   }

   while( folded[last2Act] )
     last2Act= priorSeat(last2Act);

   history.create();
   prior= last2Act;
   next2Act= nextSeat(last2Act);

   raised= TRUE;
   if( actors <= 1 )
     raised= FALSE;
   while( raised == TRUE )
   {
     raised= FALSE;
     for(seat= next2Act; ; seat= nextSeat(seat))
     {
       if( !folded[seat] )
       {
         if( getPlayer(seat)->getStake() == 0 )
           printf("%s *ALL IN*\n", player[seat]->getName());

         else
         {
           // Make the bet
           this->last2Act= last2Act;
           betReqd= maxBet - amount[seat];
           betMade= getPlayer(seat)->bet(betReqd);

           // Account for the bet
           amount[seat] += betMade;
           pot += betMade;

           // Handle RAISE
           if( betMade > betReqd )
           {
             callCount= 0;
             printf("%s RAISE %4d%s\n",
                    player[seat]->getName(), betMade-betReqd,
                    getPlayer(seat)->getStake() == 0 ? ", ALL IN" : "");

             raised= TRUE;
             this->raised[seat] += betMade - betReqd;
             history.raise(betMade - betReqd);
             maxBet= amount[seat];
             last2Act= prior;
             next2Act= nextSeat(seat);
             prior= seat;
             break;
           }

           // Handle CALL, CHECK, and FOLD
           if( betMade > 0 )
           {
             callCount++;
             printf("%s CALLS%s\n",
                    player[seat]->getName(),
                    getPlayer(seat)->getStake() == 0 ? ", ALL IN" : "");

             history.call(betMade);
             if( betMade < betReqd && getPlayer(seat)->getStake() != 0 )
               throw "PokerTable::betAround.InvalidBetAmount";
           }

           else
           {
             if( betReqd == 0 )
             {
               callCount++;
               printf("%s CHECK\n", player[seat]->getName());
               history.check();
             }
             else
             {
               printf("%s FOLDS\n", player[seat]->getName());
               folded[seat]= TRUE;
               if( getActiveCount() <= 1 )
                 break;
             }
           }
         }

         if( seat == last2Act )
           break;

         if( !folded[seat] )
           prior= seat;
       }
       else if( FALSE )             // Player has folded
       {
         #ifdef __DEBUG__           // DEBUGGING
         {{{{
           if( amount[seat] > 0 )
             printf("%s (FOLDED)\n", player[seat]->getName());
         }}}}
         #endif
       }
     }
   }

   round++;
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::debug
//
// Purpose-
//       Debug the Table.
//
//----------------------------------------------------------------------------
void
   PokerTable::debug( void )        // Debug the PokerTable
{
   int                 seat;        // Seat index
   int                 total;       // Total pot

   printf("PokerTable(%p)::debug() %s, %s\n", this, game, name);
   printf("ante(%d) bBlind(%d) sBlind(%d) Button(%d)\n",
          ante, bBlind, sBlind, button);
   display();

   //-------------------------------------------------------------------------
   // Consistency check
   //-------------------------------------------------------------------------
   total= 0;
   for(seat= 0; seat<playerCount; seat++)
   {
     total += amount[seat];
   }

   if( total != pot )
   {
     printf("%6d ERROR: TOTAL != POT\n", total);
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::display
//
// Purpose-
//       Display the Table.
//
//----------------------------------------------------------------------------
void
   PokerTable::display( void )      // Display the PokerTable
{
   int                 seat;        // Seat index
   int                 stake;       // Total stake

   printf("PokerTable(%p)::display() %s, %s\n", this, game, name);

   stake= 0;
   for(seat= 0; seat<playerCount; seat++)
   {
     printf("%6d [%2d] %6d %s\n", amount[seat], seat,
            getPlayer(seat)->getStake(), player[seat]->getName());
     stake += getPlayer(seat)->getStake();
   }
   printf("%6d Pot  %6d Stake\n", pot, stake);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::getLeft2Act
//
// Purpose-
//       Count the number of players left to act.
//
//----------------------------------------------------------------------------
int                                 // The number of other players left to act
   PokerTable::getLeft2Act(         // Get number of other players left to act
     Player*           player) const// For this Player
{
   int                 result= 0;   // Resultant
   int                 seat;        // Seat index

   #ifdef HCDM
     printf("PokerTable(%p)::getLeft2Act(%d)\n", this, getSeat(player));
   #endif

   for(seat= nextSeat(getSeat(player)); ; seat= nextSeat(seat) )
   {
     if( !folded[seat] )
       result++;

     if( seat == last2Act )
       break;
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::makeStrategy
//
// Purpose-
//       Create a Strategy
//
//----------------------------------------------------------------------------
Strategy*                           // -> new Strategy
   PokerTable::makeStrategy(        // Make a Strategy
     Player*           player)      // The PokerPlayer
{
   return new PokerStrategy(check_cast(PokerPlayer*, player), this,
                            PokerStrategy::MODEL_DEFAULT);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::play
//
// Purpose-
//       Play one hand
//
// Development notes-
//       This routine performs all table initialization for a hand.
//       Play begin with round == 0 continuing through round == (roundCount-1).
//
//----------------------------------------------------------------------------
void
   PokerTable::play( void )         // Play one Hand
{
   #ifdef HCDM
     printf("PokerTable(%p)::play() %s\n", this, name);
   #endif

   int                 seat;

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   deck.shuffle();
   reset();

   anteUp();

   //-------------------------------------------------------------------------
   // Play the hand
   //-------------------------------------------------------------------------
   playHand();

   //-------------------------------------------------------------------------
   // ShowDown
   //-------------------------------------------------------------------------
   splitPot();
   showDown();

   //-------------------------------------------------------------------------
   // Remove Players that have no stake
   //-------------------------------------------------------------------------
   seat= 0;
   while( seat < playerCount )
   {
     if( getPlayer(seat)->getStake() == 0 )
     {
       remPlayer(player[seat]);
       seat= 0;
       continue;
     }

     seat++;
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::reset
//
// Purpose-
//       Reset the PokerTable.
//
//----------------------------------------------------------------------------
void
   PokerTable::reset( void )        // Reset the PokerTable
{
   #ifdef HCDM
     printf("PokerTable(%p)::reset()\n");
   #endif

   int                 seat;

   Table::reset();
   callCount= 0;
   pot= 0;
   round= 0;

   for(seat= 0; seat<playerCount; seat++)
   {
     amount[seat]= getPlayer(seat)->ante(ante);
     folded[seat]= FALSE;
     payout[seat]= 0;
     raised[seat]= 0;
   }

   history.reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::setWinners
//
// Purpose-
//       Set the winner array.
//
//----------------------------------------------------------------------------
void
   PokerTable::setWinners(          // Set the winner array
     int               isWinner[MAX_PLAYER], // TRUE iff player has winner
     const int         isFolded[MAX_PLAYER]) const // TRUE iff player folded
{
   #ifdef HCDM
     printf("PokerTable(%p)::setWinners() %s\n", this, name);
   #endif

   int                 cc;          // Compare code
   int                 winnerIndex; // First Winning seat

   int                 i;

   for(i= 0; i<playerCount; i++)
     isWinner[i]= FALSE;

   for(winnerIndex= 0; winnerIndex<playerCount; winnerIndex++)
   {
     if( !isFolded[winnerIndex] )
       break;
   }
   if( winnerIndex >= playerCount )
     throw "PokerTable::setWinners.NoWinner";

   isWinner[winnerIndex]= TRUE;
   for(i= winnerIndex+1; i<playerCount; i++)
   {
     if( isFolded[i] )
       continue;

     cc= getPlayer(winnerIndex)->compare(getPlayer(i));
     if( cc <= 0 )
     {
       isWinner[i]= TRUE;
       if( cc < 0 )
         winnerIndex= i;
     }
   }

   for(i= 0; i<winnerIndex; i++)
     isWinner[i]= FALSE;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::showDown
//
// Purpose-
//       Show the Hands.
//
//----------------------------------------------------------------------------
void
   PokerTable::showDown( void )     // Show the Hands
{
   int                 seat;

   for(seat= 0; seat<playerCount; seat++)
   {
     printf("%6d [%2d] %8s ", amount[seat], seat, player[seat]->getName());
     if( payout[seat] > 0 )
       printf("%6d\n", payout[seat]-amount[seat]);
     else if( folded[seat] )
       printf("FOLDED\n");
     else if( getPlayer(seat)->getStake() == 0 )
       printf("*LOST*\n");
     else
       printf("  LOST\n");
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerTable::splitPot
//
// Purpose-
//       Split the pot.
//
//----------------------------------------------------------------------------
void
   PokerTable::splitPot( void )     // Split the pot
{
   int                 amount[MAX_PLAYER]; // Duplicate this->amount
   int                 isFolded[MAX_PLAYER]; // Duplicate this->folded
   int                 isWinner[MAX_PLAYER]; // Winner array
   int                 maxBet;      // Maximum bet
   int                 minWin;      // Minimum wining bet
   int                 pot= this->pot; // The pot
   int                 seat;        // Seat index
   int                 share;       // Share of pot
   int                 split;       // Partial pot share
   int                 winnerCount; // Number of winners
   int                 winnerIndex; // First Winning seat

   #ifdef HCDM
     printf("PokerTable(%p)::splitPot() %s\n", this, name);
   #endif

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   for(seat= 0; seat<playerCount; seat++)
   {
     amount[seat]= this->amount[seat];
     isFolded[seat]= this->folded[seat];
     payout[seat]= 0;
   }

   //-------------------------------------------------------------------------
   // There may be multiple winners, each of which gets a portion of the pot
   //-------------------------------------------------------------------------
   while( pot > 0 )
   {
     //-----------------------------------------------------------------------
     // Find the winners
     //-----------------------------------------------------------------------
     setWinners(isWinner, isFolded);

     maxBet= 0;
     minWin= pot;
     winnerCount= 0;
     winnerIndex= (-1);
     for(seat= 0; seat<playerCount; seat++)
     {
       if( isWinner[seat] )
       {
         winnerCount++;
         if( winnerIndex < 0 )
           winnerIndex= seat;

         if( amount[seat] > maxBet )
           maxBet= amount[seat];

         if( amount[seat] < minWin )
           minWin= amount[seat];
       }
     }

     //-----------------------------------------------------------------------
     // Single winner
     //-----------------------------------------------------------------------
     if( winnerCount == 1 )
     {
       payout[winnerIndex] += pot;
       break;
     }

     //-----------------------------------------------------------------------
     // Split the pot
     //-----------------------------------------------------------------------
     split= 0;
     for(seat= 0; seat < playerCount; seat++)
     {
       if( amount[seat] > minWin )
       {
         amount[seat] -= minWin;
         split += minWin;
       }
       else
       {
         split += amount[seat];
         amount[seat]= 0;
         isFolded[seat]= TRUE;
       }
     }

     pot -= split;
     share= split/winnerCount;
     for(seat= 0; seat < playerCount; seat++)
     {
       if( isWinner[seat] )
       {
         payout[seat] += share;
         split -= share;
       }
     }

     for(seat= rand()%playerCount; split > 0; seat++)
     {
       if( seat >= playerCount )
         seat= 0;

       if( isWinner[seat] )
       {
         payout[seat]++;
         split--;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Distribute the winnings
   //-------------------------------------------------------------------------
   for(seat= 0; seat < playerCount; seat++)
   {
     if( payout[seat] > 0 )
       getPlayer(seat)->addStake(payout[seat]);
   }
}

