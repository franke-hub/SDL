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
//       TestDist.cpp
//
// Purpose-
//       Distribution tests.
//
// Last change date-
//       2017/01/01
//
// Notes-
//       This program runs for a long time.
//       It's used to calculate the deal distribution parameters. The output
//       is embedded in TexasPoker.cpp for use with the TexasStrategy object.
//
//       If RUN_DISTPOKERS is defined, it displays the number of occurances
//       for each of the poker hands found in a monte-carlo simulation.
//
// Usage-
//       testdist | tee testdist.out
//
//----------------------------------------------------------------------------
#include <time.h>

#include "Poker.h"
#include "TestDist.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ITERATIONS 10000000         // Number of iterations
#define RUN_DISTRIBUTE TRUE         // Run Texas Hold'em distribution?
#define RUN_DISTPOKERS TRUE         // Run Poker distributions?

#ifndef RANDOMIZE
  #if FALSE                         // Want true randomness?
    #define RANDOMIZE time(NULL)    // True randomness
  #else
    #define RANDOMIZE 12345         // Repeatable randomness
  #endif
#endif

#ifndef HCDM_DISTRIBUTE
#undef  HCDM_DISTRIBUTE             // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MAX_DIST 1024               // Maximum number of deal distributions
#define PLAYERS 7                   // The number of evaluation players

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Card*           deal[52];    // The cards in the Deck
static Deck            deck;        // The current Deck
static int             maxQ;        // Number of distribution qualifiers

static TexasDealCounter
                       texas[MAX_DIST]; // The deal distribution counters

//----------------------------------------------------------------------------
//
// Subroutine-
//       init
//
// Purpose-
//       Initialize
//
//----------------------------------------------------------------------------
static void
   init( void )                     // Initialize
{
   int                 count;       // The number of cards in the Deck
   Card::Rank          rankHi;      // The higher ranking card
   Card::Rank          rankLo;      // The lower ranking card

   int                 i;

   srand(RANDOMIZE);                // RANDOMIZE

   count= deck.getCount();          // Initialize getRating
   for(i= 0; i < count; i++)        // Initialize the available cards
     deal[i]= deck.deal();

   for(i= 0; i < count; i++)        // Shuffle the Deck
     deck.shuffle();

   // Initialize the distribution array
   maxQ= 0;
   for(rankHi= Card::RANK_MAX; rankHi>=Card::RANK_MIN; rankHi--)
   {
     texas[maxQ].downHi= rankHi;
     texas[maxQ].downLo= rankHi;
     texas[maxQ].suited= FALSE;
     maxQ++;
   }

   for(rankHi= Card::RANK_MAX; rankHi>=Card::RANK_MIN; rankHi--)
   {
     for(rankLo= rankHi-1; rankLo>=Card::RANK_MIN; rankLo--)
     {
       texas[maxQ].downHi= rankHi;
       texas[maxQ].downLo= rankLo;
       texas[maxQ].suited= TRUE;
       maxQ++;
     }
   }

   for(rankHi= Card::RANK_MAX; rankHi>=Card::RANK_MIN; rankHi--)
   {
     for(rankLo= rankHi-1; rankLo>=Card::RANK_MIN; rankLo--)
     {
       texas[maxQ].downHi= rankHi;
       texas[maxQ].downLo= rankLo;
       texas[maxQ].suited= FALSE;
       maxQ++;
     }
   }
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       getDealCounter
//
// Purpose-
//       Return the associated TexasDealCounter.
//
//----------------------------------------------------------------------------
static TexasDealCounter*            // -> TexasDealCounter
   getDealCounter(                  // Get TexasDealCounter
     Card*             downHi,      // The higher down card
     Card*             downLo)      // The lower down card
{
   TexasDealCounter*   result= NULL;// Resultant
   Card::Rank          rankHi;      // The higher rank
   Card::Rank          rankLo;      // The lower rank
   int                 suited;      // TRUE iff suited

   int                 i;

   rankHi= downHi->getRank();
   rankLo= downLo->getRank();
   suited= downHi->getSuit() == downLo->getSuit();

   for(i= 0; i<maxQ; i++)
   {
     if( rankHi == texas[i].downHi
         && rankLo == texas[i].downLo
         && suited == texas[i].suited )
     {
       result= &texas[i];
       break;
     }
   }

   assert( result != NULL );
   return result;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       texasDistribution
//
// Purpose-
//       Calculate Texas hold'em deal distribution ratings.
//
//----------------------------------------------------------------------------
static void
   texasDistribution( void )        // Get deal distribution ratings
{
   Card*               card;        // -> Current Card
   int                 cc;          // Condition code
   Card*               downHi[PLAYERS]; // -> Higher down card
   Card*               downLo[PLAYERS]; // -> Lower down card
   PokerHand           hand[PLAYERS]; // The Player's hand
   int                 iteration;   // Iteration number
   Card*               load[7];     // Hand loader
   TexasDealCounter*   counter;     // -> TexasDealCounter
   int                 winner[PLAYERS]; // The winner array

   #ifdef HCDM_DISTRIBUTE
     char              string[Card::SIZE_NAME]; // Working string
   #endif

   int                 i;
   int                 j;
   int                 m;

   m= ITERATIONS;
   #ifdef HCDM_DISTRIBUTE
     if( m > 1000 )
       m= 1000;
   #endif

   //-------------------------------------------------------------------------
   // Run the test
   //-------------------------------------------------------------------------
   for(iteration= 0; iteration<m; iteration++)
   {
     //-----------------------------------------------------------------------
     // Shuffle up
     //-----------------------------------------------------------------------
     deck.shuffle();

     //-----------------------------------------------------------------------
     // Deal DOWN cards to each Player
     //-----------------------------------------------------------------------
     for(i= 0; i<PLAYERS; i++)
       downHi[i]= deck.deal();

     for(i= 0; i<PLAYERS; i++)
       downLo[i]= deck.deal();

     for(i= 0; i<PLAYERS; i++)
     {
       if( downHi[i]->getRank() < downLo[i]->getRank() )
       {
         card= downHi[i];
         downHi[i]= downLo[i];
         downLo[i]= card;
       }
     }

     //-----------------------------------------------------------------------
     // Deal FLOP
     //-----------------------------------------------------------------------
     card= deck.deal();           // BURN card

     #ifdef HCDM_DISTRIBUTE
       printf("FLOP:");
       for(i= 0; i<3; i++)
       {
         card= deck.deal();
         card->setVisible(TRUE);
         load[i+2]= card;
         printf(" %s", card->toShortString(string));
       }
       printf("\n");
     #else
       for(i= 0; i<3; i++)
       {
         card= deck.deal();
         card->setVisible(TRUE);
         load[i+2]= card;
       }
     #endif

     //-----------------------------------------------------------------------
     // Deal TURN
     //-----------------------------------------------------------------------
     card= deck.deal();           // BURN card
     card= deck.deal();           // TURN
     card->setVisible(TRUE);
     load[5]= card;

     #ifdef HCDM_DISTRIBUTE
       printf("TURN: %s\n", card->toShortString(string));
     #endif

     //-----------------------------------------------------------------------
     // Deal RIVER
     //-----------------------------------------------------------------------
     card= deck.deal();           // BURN card
     card= deck.deal();           // TURN
     card->setVisible(TRUE);
     load[6]= card;

     #ifdef HCDM_DISTRIBUTE
       printf("RIVER: %s\n", card->toShortString(string));
     #endif

     //-----------------------------------------------------------------------
     // Check result
     //-----------------------------------------------------------------------
     for(i= 0; i<PLAYERS; i++)
     {
       load[0]= downHi[i];
       load[1]= downLo[i];
       hand[i].load(7, load);
     }

     winner[0]= 0;
     for(i= 1; i<PLAYERS; i++)
       winner[i]= hand[0].compare(hand[i]);

     counter= getDealCounter(downHi[0], downLo[0]);
     counter->player[0].countTie();

     cc= +1;
     for(i= 1; i<PLAYERS; i++)
     {
       if( cc > winner[i] )
         cc= winner[i];
       if( cc < 0 )
         counter->player[i].countLoss();
       else if( cc == 0 )
         counter->player[i].countTie();
       else
         counter->player[i].countWin();
     }

     #ifdef HCDM_DISTRIBUTE
       printf("\n");
       fflush(stdout);
     #endif
   }

   //-------------------------------------------------------------------------
   //-------------------------------------------------------------------------
   //-------------------------------------------------------------------------
   for(i= 0; i<maxQ; i++)
   {
     printf(",  {Card::RANK_%s, Card::RANK_%s, %s, {{0.0, 1.0}",
            Card::shortRankName[texas[i].downHi],
            Card::shortRankName[texas[i].downLo],
            texas[i].suited ? " TRUE" : "FALSE");

     for(j= 1; j<PLAYERS; j++)
     {
       printf(", {%6.4f, %6.4f}",
              texas[i].player[j].rateWins(), texas[i].player[j].rateTies());
     }
     printf("}}\n");
   }

   printf("\n");
   fflush(stdout);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       pokerDistribution
//
// Purpose-
//       Run poker distribution tests.
//
//----------------------------------------------------------------------------
static void
   pokerDistribution(               // Run poker distribution tests
     int               cards)       // Number of cards
{
   int                 ranking[PokerHand::RANKING_COUNT]; // Standard rankings
   Card*               card[PokerHand::MAX_HAND];

   int                 i;
   int                 j;

   for(i= 0; i<PokerHand::RANKING_COUNT; i++)
     ranking[i]= 0;

   for(i= 0; i < ITERATIONS; i++)
   {
     deck.shuffle();
     for(j= 0; j<cards; j++)
       card[j]= deck.deal();

     PokerHand hand(cards, card);
     PokerHand::Ranking rank= hand.getRanking();
     ranking[rank]++;
   }

   printf("%d Card ranking distribution\n", cards);
   for(i= 0; i<PokerHand::RANKING_COUNT; i++)
   {
     printf("[%2d] %8d %5.2f %s\n", i, ranking[i],
            double(ranking[i])*100.0/double(ITERATIONS),
            PokerHand::getRankName(PokerHand::Ranking(i)));
   }
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       main
//
// Purpose-
//       Mainline code
//
//----------------------------------------------------------------------------
int                                 // Return code
   main(int, char**)                // Mainline code
//   int               argc,        // Argument count (Unused)
//   char*             argv[])      // Argument array (Unused)
{

   try {
     init();

     if( RUN_DISTRIBUTE )
       texasDistribution();

     if( RUN_DISTPOKERS )
     {
       pokerDistribution(5);
       pokerDistribution(6);
       pokerDistribution(7);
       pokerDistribution(8);
       pokerDistribution(9);
     }
   } catch(const char* X) {
     printf("Exception(%s)\n", X);
   } catch(...) {
     printf("Exception(...)\n");
   }

   return 0;
}

