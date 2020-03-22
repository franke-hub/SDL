//----------------------------------------------------------------------------
//
//       Copyright (C) 2017-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Strategy.cpp
//
// Purpose-
//       Strategy implementation.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#include <assert.h>

#include "Define.h"
#include "Player.h"
#include "Table.h"
#include "Utility.h"

#include "Strategy.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

#define UNTESTED 0                  // Logic is untested
#define _INTEST_ 0                  // Logic is undergoing test

#define ACCEPTED 1
#define REJECTED 0

#if 0                               // Used to compile test all options
#undef  UNTESTED
#undef  _INTEST_
#undef  ACCEPTED
#undef  REJECTED

#define UNTESTED 0
#define _INTEST_ 0
#define ACCEPTED 0
#define REJECTED 0
#endif

//----------------------------------------------------------------------------
// Macros
//----------------------------------------------------------------------------
#ifdef __DEBUG__
   #define ATLINE() printf("{%4d} ", __LINE__)
#else
   #define ATLINE() (void)0
#endif

//----------------------------------------------------------------------------
// Data areas
//----------------------------------------------------------------------------
const char*            PokerStrategy::stateName[]= // The state names
{  "RESET   "
,  "CONSERVE"
,  "NEUTRAL "
,  "AGGRESIV"
,  "BLUFF   "
,  "SLOWPLAY"
,  "FASTPLAY"
};

//----------------------------------------------------------------------------
//
// Method-
//       Strategy::~Strategy
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Strategy::~Strategy( void )      // Destructor
{
   #ifdef HCDM
     printf("Strategy(%p)::~Strategy()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Strategy::Strategy
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Strategy::Strategy(              // Constructor
     Player*           player,      // The Player
     Table*            table)       // The Table
:  player(player)
,  table(table)
{
   #ifdef HCDM
     printf("Strategy(%p)::Strategy()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Strategy::reset
//
// Purpose-
//       Reset the Strategy.
//
//----------------------------------------------------------------------------
void
   Strategy::reset( void )          // Reset the Strategy
{
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerStrategy::~PokerStrategy
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   PokerStrategy::~PokerStrategy( void ) // Destructor
{
   #ifdef HCDM
     printf("PokerStrategy(%p)::~PokerStrategy()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerStrategy::PokerStrategy
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   PokerStrategy::PokerStrategy(    // Constructor
     PokerPlayer*      player,      // The PokerPlayer
     PokerTable*       table,       // The PokerTable
     Model             model)       // The betting model
:  Strategy(player, table)
,  model(model)
,  state(STATE_RESET)
,  stateRound(-1)
,  rateActive(-1)
,  rateRound(-1)
,  rating()
{
   #ifdef HCDM
     printf("PokerStrategy(%p)::PokerStrategy(%d)\n", this, model);
   #endif

   rating.reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerStrategy::getPlayer
//
// Purpose-
//       Get the associated PokerPlayer
//
//----------------------------------------------------------------------------
PokerPlayer*                        // ->  associated PokerPlayer
   PokerStrategy::getPlayer( void ) const // Get associated PokerPlayer
{
   return check_cast(PokerPlayer*,player);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerStrategy::getRating
//
// Purpose-
//       Get the Player's (cached) Rating
//
//----------------------------------------------------------------------------
PokerRating&                        // The (cached) PokerRating
   PokerStrategy::getRating( void ) // Get (cached) PokerRating
{
   #ifdef HCDM
     printf("PokerStrategy(%p)::getRating() %s\n", this, name);
   #endif

   PokerTable*         table= getTable();

   if( rateRound != table->getRound()
       || rateActive != table->getActiveCount() )
   {
     rateRound= table->getRound();
     rateActive= table->getActiveCount();

     rate();
   }

   return rating;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerStrategy::getTable
//
// Purpose-
//       Get the associated PokerTable
//
//----------------------------------------------------------------------------
PokerTable*                         // ->  associated PokerTable
   PokerStrategy::getTable( void ) const // Get associated PokerTable
{
   return check_cast(PokerTable*,table);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerStrategy::rate
//
// Purpose-
//       Update the PokerRating
//
//----------------------------------------------------------------------------
void
   PokerStrategy::rate( void )      // Update the PokerRating
{
   rating.reset();
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerStrategy::setTable
//
// Purpose-
//       Set the associated PokerTable
//
//----------------------------------------------------------------------------
void
   PokerStrategy::setTable(         // Set the PokerTable
     PokerTable*       table)       // The PokerTable
{
   this->table= table;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerStrategy::bet
//
// Purpose-
//       Make a bet.
//
//----------------------------------------------------------------------------
int                                 // The actual bet
   PokerStrategy::bet(              // Make a bet
     int               amount)      // Minimum to call
{
   #ifdef HCDM
     printf("PokerStrategy(%p)::bet(%d)\n", this, amount);
   #endif

   enum                             // Action index
   {  ACT_CONSERVATIVE= 0           // Conservative
   ,  ACT_NEUTRAL=1                 // Neutral
   ,  ACT_AGGRESSIVE=2              // Aggressive
   };

   int                 result= 0;   // Actual bet

   PokerPlayer*        player= getPlayer();
   PokerTable*         table= getTable();
   int                 seat= table->getSeat(player);

   int                 betMin= 0;   // Minimum recommended bet
   int                 betMax= 0;   // Maximum recommended bet
   int                 minRaise= table->getMinRaise();
   int                 pot= table->getPot();
   int                 round= table->getRound();
   int                 stake= player->getStake() + table->getAmount(seat);
   int                 xAct;        // Adjusted state index
   double              xActAdj;     // Adjusted state index adjustment

   //-------------------------------------------------------------------------
   // Prepare to bet
   //-------------------------------------------------------------------------
   PokerStrategy::getRating();

   if( state == STATE_RESET )
   {
     stateRound= round;

     switch( model )
     {
       case MODEL_CONSERVATIVE:
         state= STATE_CONSERVATIVE;
         break;

       case MODEL_NEUTRAL:
         state= STATE_NEUTRAL;
         break;

       case MODEL_AGGRESSIVE:
         state= STATE_AGGRESSIVE;
         break;

       //   MODEL_DEFAULT:
       //   MODEL_RANDOM:
       default:
         state= STATE_AGGRESSIVE;
         if( randomly(0.333) )
           state= STATE_CONSERVATIVE;
         else if( randomly(0.500) )
           state= STATE_NEUTRAL;
         break;
     }
   }

   xAct= 0;
   switch( state )
   {
     case STATE_CONSERVATIVE:
     case STATE_NEUTRAL:
     case STATE_AGGRESSIVE:
       xAct= int(state) - 1;
       break;

     case STATE_BLUFFING:
       xAct= ACT_AGGRESSIVE;
       if( stateRound == round )
         xAct= ACT_CONSERVATIVE;
       break;

     case STATE_SLOWPLAY:
       xAct= ACT_CONSERVATIVE;
       if( stateRound != round )
       {
         if( rating.twoRate >= 0.900 )
           xAct= ACT_AGGRESSIVE;
       }
       break;

     case STATE_FASTPLAY:
       xAct= ACT_AGGRESSIVE;
       if( stateRound == round )
         xAct= ACT_CONSERVATIVE;
       break;

     default:
       break;
   }
   xActAdj= xAct * 0.025;

   //-------------------------------------------------------------------------
   // Debugging
   //-------------------------------------------------------------------------
   #ifdef __DEBUG__
   {{{{
     int               cardCount= player->getCardCount();
     Card*             card[Hand::MAX_HAND];
     char              string[Card::SIZE_NAME];

     int               i;

     player->store(card);
     printf("%6d [%2d] ", table->getAmount(seat), seat);
     printf("%6d ", pot);
     printf("%6.4f %6.4f ", rating.twoRate, rating.allRate);
     for(i= 0; i<cardCount; i++)
       printf("%s ", card[i]->toShortString(string));
   }}}}
   #endif

   //-------------------------------------------------------------------------
   // Handle special (should not occur) conditions
   //-------------------------------------------------------------------------
   if( stake == 0 )                 // If nothing left to bet with
     return 0;                      // ALL IN

   if( table->getActiveCount() <= 1 ) // If we are the only player
     return amount;                 // We call

#if( ACCEPTED )
   //-------------------------------------------------------------------------
   // Evaluate the hand using the hard-coded rating system
   //-------------------------------------------------------------------------
   if( round == 0 && table->getRoundCount() > 1 )
   {
     //-----------------------------------------------------------------------
     // Round[0] betting
     //-----------------------------------------------------------------------
     if( rating.twoRate > 0.80 )
     {
       betMin= minRaise + (rand()%6) * minRaise;
       betMax= betMin + minRaise + (rand()%6) * betMin;
     }

     else if( rating.twoRate > 0.75 )
     {
       betMin= minRaise + (rand()%5) * minRaise;
       betMax= betMin + minRaise + (rand()%5) * betMin;
     }

     else if( rating.twoRate > 0.70 )
     {
       betMin= minRaise + (rand()%4) * minRaise;
       betMax= betMin + minRaise + (rand()%4) * betMin;
     }

     else if( rating.twoRate > 0.65 )
     {
       betMin= minRaise + (rand()%3) * minRaise;
       betMax= betMin + (rand()%4) * minRaise;
     }

     else if( rating.twoRate > 0.60 )
     {
       betMin= minRaise + (rand()%2) * minRaise;
       betMax= betMin + (rand()%3) * minRaise;
     }

     else if( rating.twoRate > 0.55 )
     {
       betMin= 0;
       betMax= minRaise + (rand()%3) * minRaise;
     }

     else if( rating.twoRate > 0.50 )
     {
       betMin= 0;
       betMax= minRaise + (rand()%2) * minRaise;
     }

     else                           // If dealt two cards
     {
       betMin= 0;
       betMax= minRaise / 2;
     }
   }
   else
   {
     //-----------------------------------------------------------------------
     // Round[>0] betting
     //-----------------------------------------------------------------------
     if( rating.twoRate > 0.975 )
     {
       betMin= minRaise + (rand()%9) * minRaise;
       betMax= amount;
     }

     else if( rating.twoRate > 0.950 )
     {
       betMin= minRaise + (rand()%8) * minRaise;
       betMax= amount;
     }

     else if( rating.twoRate > 0.925 )
     {
       betMin= minRaise + (rand()%7) * minRaise;
       betMax= stake / 2;
     }

     else if( rating.twoRate > 0.900 )
     {
       betMin= minRaise + (rand()%6) * minRaise;
       betMax= stake / 4;
     }

     else if( rating.twoRate > 0.850 )
     {
       betMin= minRaise + (rand()%5) * minRaise;
       betMax= stake / 8;
     }

     else if( rating.twoRate > 0.800 )
     {
       betMin= minRaise + (rand()%4) * minRaise;
       betMax= stake / 16;
     }

     else if( rating.twoRate > 0.750 )
     {
       betMin= minRaise + (rand()%3) * minRaise;
       betMax= betMin + betMin;
     }

     else if( rating.twoRate > 0.700 )
     {
       betMin= minRaise + (rand()%2) * minRaise;
       betMax= betMin + betMin;
     }

     else if( rating.twoRate > 0.600 )
     {
       betMin= 0;
       betMax= minRaise + (rand()%2) * minRaise;
     }

     else if( rating.twoRate > 0.500 )
     {
       betMin= 0;
       betMax= minRaise;
     }

     else
     {
     }

     // Adjust bets for round
     betMin= (betMin * (round+1)) / table->getRoundCount();

     // The maximum can be less than the minimum for stake bets
     if( betMax < betMin )
       betMax= betMin + betMin;
   }
#endif

#if( UNTESTED )
   //-------------------------------------------------------------------------
   // Evaluate the hand using the pay-to-play rating system
   // So far, no accurate betting formula has been found
   //-------------------------------------------------------------------------
   double              useRate;     // Win probability

   useRate= rating.allRate;
   if( round == 0 && table->getRaiseCount() == 0 )
     useRate= rating.twoRate;

   if( useRate >= 1.0 )             // If we can't lose
   {
     betMin= amount + stake;        // Bet the farm
     betMax= betMin;
   }
   else
   {
     double ratio= 8.0 + randf() * 4.0; // range 8..12
printf("%6.4f %7.4f ", useRate, ratio);

     betMin= int((useRate * (pot+amount)) / ((1.0-useRate) * ratio));
     betMax= betMin;

     if( rating.twoRate > 0.975 )
       betMax= amount + stake;
     else if( rating.twoRate > 0.950 )
       betMax= betMin + betMin + betMin + betMin;
     else if( rating.twoRate > 0.925 )
       betMax= betMin + betMin + betMin + betMin/2;
     else if( rating.twoRate > 0.900 )
       betMax= betMin + betMin + betMin;
     else if( rating.twoRate > 0.800 )
       betMax= betMin + betMin + betMin/2;
     else if( rating.twoRate > 0.700 )
       betMax= betMin + betMin;
     else if( rating.twoRate > 0.600 )
       betMax= betMin + betMin/2;
     else if( rating.twoRate > 0.500 )
       betMax= betMin;
   }
     printf("(%3d,%4d) %c ", betMin, betMax, stateName[state][0]);
#endif

   //-------------------------------------------------------------------------
   // Determine the actual bet
   //-------------------------------------------------------------------------
// betMin += (minRaise/2);          // Round up the minimum
   betMin /= minRaise;              // Truncate the minimum
   betMin *= minRaise;

   betMax += (minRaise/2);          // Round up the maximum
   betMax /= minRaise;              // Truncate the maximum
   betMax *= minRaise;

   #ifdef __DEBUG__
     printf("(%3d,%4d) %c ", betMin, betMax, stateName[state][0]);
   #endif

   result= betMin;
   if( result <= table->getAmount(seat) )
     result= 0;
   else
     result= result - table->getAmount(seat);

   if( result < amount )
   {
     result= 0;
     if( amount <= (betMax - table->getAmount(seat)) )
       result= amount;
   }

   //-------------------------------------------------------------------------
   // Examine raise situations
   //-------------------------------------------------------------------------
   if( result > amount )
   {
     if( FALSE )
       ATLINE();

#if( UNTESTED )
     //-----------------------------------------------------------------------
     // Special case: undefined
     //-----------------------------------------------------------------------
#endif
   }

   //-------------------------------------------------------------------------
   // Examine call situations
   //-------------------------------------------------------------------------
   else if( result == amount && amount > 0 )
   {
     if( FALSE )
       ATLINE();

#if( UNTESTED )
     //-----------------------------------------------------------------------
     // Special case: (Semi-bluff) Call, excellent hand
     //-----------------------------------------------------------------------
     else if( betMax > amount && randomly(0.125) )
     {
       ATLINE();
       result += minRaise + (rand()%4) * minRaise;
     }
#endif

#if( UNTESTED )
     //-----------------------------------------------------------------------
     // Special case: Call, table idle, passable hand, not last round
     //-----------------------------------------------------------------------
     else if( table->getCallCount() >= table->getActiveCount()
         && rating.twoRate >= (0.600 - xActAdj)
         && (round+1) < table->getRoundCount() )
     {
       ATLINE();
       result= minRaise + (rand()%4) * minRaise;
     }
#endif
   }

   //-------------------------------------------------------------------------
   // Examine check situations
   //-------------------------------------------------------------------------
   else if( result == 0 && amount == 0 )
   {
     if( FALSE )
       ATLINE();

#if( UNTESTED )
     //-----------------------------------------------------------------------
     // Special case: Check, better to eliminate players, no big raises
     //-----------------------------------------------------------------------
     else if( rating.twoRate >= (0.600 - xActAdj)
              && (rating.twoRate - rating.allRate) >= 0.150
              && table->getAmount(seat) < (2*(minRaise + round*minRaise)) )
     {
       ATLINE();
       result= minRaise + (rand()%4) * minRaise;
     }
#endif

#if( ACCEPTED )
     //-----------------------------------------------------------------------
     // Special case: Check, table idle, passable hand, not last round
     //-----------------------------------------------------------------------
     else if( (table->getCallCount()+1) >= table->getActiveCount()
         && rating.twoRate >= (0.600 - xActAdj)
         && (round+1) < table->getRoundCount() )
     {
       ATLINE();
       result= minRaise + (rand()%4) * minRaise;
     }
#endif

#if( ACCEPTED )
     //-----------------------------------------------------------------------
     // Special case: Random bluff
     //-----------------------------------------------------------------------
     else if( (table->getCallCount()+1) >= table->getActiveCount()
              && randomly(0.150) )
     {
       ATLINE();
       state= STATE_BLUFFING;
       result= minRaise + (rand()%4) * minRaise;
     }
#endif
   }

   //-------------------------------------------------------------------------
   // Examine fold situations
   //-------------------------------------------------------------------------
   else if( result == 0 && amount > 0 )
   {
     if( FALSE )
       ATLINE();

#if( UNTESTED )
     //-----------------------------------------------------------------------
     // Special case: Fold, better to eliminate players, small raise
     //-----------------------------------------------------------------------
     else if( rating.twoRate >= (0.600 - xActAdj)
              && (rating.twoRate - rating.allRate) >= 0.150
              && amount < (rand()%4) * minRaise )
     {
       ATLINE();
       result= amount;
     }
#endif
   }

   //-------------------------------------------------------------------------
   // Examine other situations
   //-------------------------------------------------------------------------
   else
   {
     printf("%4d result(%d) amount(%d)\n", __LINE__, result, amount);
     throw "ShouldNotOccur";
   }

   //-------------------------------------------------------------------------
   // Make the bet
   //-------------------------------------------------------------------------
   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerStrategy::debug
//
// Purpose-
//       Debug the PokerStrategy.
//
//----------------------------------------------------------------------------
void
   PokerStrategy::debug( void )     // Debug the PokerStrategy
{
   #ifdef HCDM
     printf("PokerStrategy(%p)::debug()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerStrategy::display
//
// Purpose-
//       Display the PokerStrategy.
//
//----------------------------------------------------------------------------
void
   PokerStrategy::display( void )   // Display the PokerStrategy
{
   #ifdef HCDM
     printf("PokerStrategy(%p)::display()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerStrategy::reset
//
// Purpose-
//       Reset the PokerStrategy.
//
//----------------------------------------------------------------------------
void
   PokerStrategy::reset( void )     // Reset the PokerStrategy
{
   Strategy::reset();

   // Reset the state
   state= STATE_RESET;
   stateRound= (-1);

   // Reset the rating cache
   rateActive= (-1);
   rateRound= (-1);
   rating.reset();
}

