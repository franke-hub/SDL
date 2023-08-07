//----------------------------------------------------------------------------
//
//       Copyright (C) 2017-2023 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       HumanPoker.cpp
//
// Purpose-
//       Human poker player implementations.
//
// Last change date-
//       2023/08/07
//
//----------------------------------------------------------------------------
#include <ctype.h>
#include <errno.h>

#include "Poker.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
//
// Method-
//       HumanPokerStrategy::~HumanPokerStrategy
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   HumanPokerStrategy::~HumanPokerStrategy( void ) // Destructor
{
   #ifdef HCDM
     printf("HumanPokerStrategy(%p)::~HumanPokerStrategy()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       HumanPokerStrategy::HumanPokerStrategy
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   HumanPokerStrategy::HumanPokerStrategy( // Constructor
     PokerPlayer*      player,      // The PokerPlayer
     PokerTable*       table)       // The PokerTable
:  PokerStrategy(player, table, MODEL_DEFAULT)
{
   #ifdef HCDM
     printf("HumanPokerStrategy(%p)::HumanPokerStrategy()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       HumanPokerStrategy::bet
//
// Purpose-
//       Make a bet.
//
//----------------------------------------------------------------------------
int                                 // The actual bet
   HumanPokerStrategy::bet(         // Make a bet
     int               amount)      // Minimum to call
{
   int                 result= 0;   // Actual bet

   PokerPlayer*        player= getPlayer();
   PokerTable*         table= getTable();
   int                 seat= table->getSeat(player);

   int                 activeCount= table->getActiveCount();
   int                 cardCount= player->getCardCount();
   Card*               card[Hand::MAX_HAND]; // The Player's Cards
   int                 inPot= table->getAmount(seat);
   int                 maxRaise= table->getMaxRaise();
   int                 minRaise= table->getMinRaise();
   int                 playerCount= table->getPlayerCount();
   int                 pot= table->getPot();
   int                 stake= player->getStake();
   char                string[256]; // Work area
   int                 temp;        // Temporary

   int                 i;

   //-------------------------------------------------------------------------
   // Initialize
   //-------------------------------------------------------------------------
   player->store(card);

   //-------------------------------------------------------------------------
   // Bet
   //-------------------------------------------------------------------------
   printf("Player(%s) %d to call\n", player->getName(), amount);
   printf("..Rating("); getRating().display(); printf(")\n");
   printf("..Pot(%d) Already bet(%d), Stake(%d), %d of %d Players remain\n",
          pot, inPot, stake, activeCount, playerCount);
   for(i= 0; i<cardCount; i++)
   {
     printf("%s", card[i]->getVisible() ? "  UP" : "DOWN");
     printf(" %s\n", card[i]->toString(string));
   }

   for(;;)
   {
     result= 0;
     printf("Fold, Call, Raise(*%d): ", minRaise);
     if( fgets(string, sizeof(string), stdin) == nullptr ) {
       if( ferror(stdin) )
         fprintf(stderr, ">>> Unable to read from stdin\n");
       printf("Leaving the table\n");
       exit(0);
     }
     if( toupper(string[0]) == 'F' )
       break;

     if( toupper(string[0]) == 'C' )
     {
       result= amount;
       break;
     }

     if( toupper(string[0]) == 'R' )
     {
       temp= (-1);
       errno= 0;
       sscanf(&string[1], "%d\n", &temp);
       if( temp <= 0 || errno != 0 )
       {
         printf("Invalid amount\n");
         continue;
       }

       result= amount + minRaise * temp;
       if( result < minRaise || result > maxRaise )
       {
         printf("Raise: Minimum/Maximum %d/%d (%d/%d)\n",
                minRaise, maxRaise, 1, maxRaise/minRaise);
         continue;
       }
       break;
     }

     if( toupper(string[0]) == 'A' )
     {
       result= stake;
       break;
     }

     printf("Eh\?(%c)\n", string[0]);
   }

   return result;
}

