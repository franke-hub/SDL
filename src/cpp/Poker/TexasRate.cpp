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
//       TexasRate.cpp
//
// Purpose-
//       Texas Hold'em hand rater.
//
// Last change date-
//       2017/01/01
//
// Notes-
//       This program calculates a rating for a Texas poker hand using either
//       a full evaluation or, for the deal, a monte-carlo simulation.
//       The full evaluation is fast enough for real-time usage.
//
//       Cards are specified by a two character value/suit pair.
//       Values: 2, 3, 4, 5, 6, 7, 8, 9, T, J, Q, K, A.
//       Suits:  C, D, H, S.
//
//       Upper and lower case are permitted.
//       As example: "tc", "tC", "Tc", and "TC" all define the 10 of clubs.
//
// Usage-
//       testrate <2, 5, 6, or 7 card specifiers> {/ <muck card specifiers>}
//       Example: texasrate as ks
//       Example: texasrate as ks / ah ac
//
// Output-
//       A rating/ranking list for two through seven players:
//
//       #Play win+tie   p(win)  p(tie) p(loss)
//       [* 2]  rating { p(win)  p(tie) p(loss)} [TexasStrategy::getRating()]
//       Rate:  rating { p(win)  p(tie) p(loss)} [Rating]
//       Rate: win+tie {   wins    ties  losses} [Enumerated counts]
//       Rank: ranking { p(win)  p(tie) p(loss)} [Ranking]
//       Rank: win+tie {   wins    ties  losses} [Enumerated counts]
//       [* 7]  rating { p(win)  p(tie) p(loss)} [TexasStrategy::getRating()]
//
//       p(win)= probability(win)
//       rating= p(win) + p(tie), all evaluated hands
//       ranking= p(win) + p(tie), once per rank
//
//----------------------------------------------------------------------------
#include <time.h>
#include <string.h>

#include "Poker.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ITERATIONS 1000             // Monte-carlo iterations
#define PLAYERS 7                   // Number of players

#ifndef RANDOMIZE
  #if TRUE                          // Want true randomness?
    #define RANDOMIZE time(NULL)    // True randomness
  #else
    #define RANDOMIZE 12345         // Repeatable randomness
  #endif
#endif

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Deck            deck;        // The current Deck

//----------------------------------------------------------------------------
//
// Subroutine-
//       getCard
//
// Purpose-
//       Extract a card from the Deck.
//
//----------------------------------------------------------------------------
static Card*                        // -> Card
   getCard(                         // Extract a Card
     const char*       string)      // The card string
{
   Card*               result;      // Resultant

   int                 C;           // Working character
   Card**              card= deck.getDeck(); // The deck
   int                 count= deck.getCount(); // The number of cards
   Card::Rank          rank;        // The Rank of the Card
   Card::Suit          suit;        // The Suit of the Card

   int                 i;

   if( string[2] != '\0' )
   {
     printf("Invalid card '%s'\n", string);
     throw "InvalidCard";
   }

   C= string[0];                    // The rank
   switch( C )
   {
     case '2':
      rank= Card::RANK_2;
      break;

     case '3':
      rank= Card::RANK_3;
      break;

     case '4':
      rank= Card::RANK_4;
      break;

     case '5':
      rank= Card::RANK_5;
      break;

     case '6':
      rank= Card::RANK_6;
      break;

     case '7':
      rank= Card::RANK_7;
      break;

     case '8':
      rank= Card::RANK_8;
      break;

     case '9':
      rank= Card::RANK_9;
      break;

     case 't':
     case 'T':
      rank= Card::RANK_T;
      break;

     case 'j':
     case 'J':
      rank= Card::RANK_J;
      break;

     case 'q':
     case 'Q':
      rank= Card::RANK_Q;
      break;

     case 'k':
     case 'K':
      rank= Card::RANK_K;
      break;

     case 'a':
     case 'A':
      rank= Card::RANK_A;
      break;

     default:
      printf("Invalid RANK in %s\n", string);
      throw "InvalidCard";
      break;
   }

   C= string[1];                    // The suit
   switch( C )
   {
     case 'c':
     case 'C':
      suit= Card::SUIT_C;
      break;

     case 'd':
     case 'D':
      suit= Card::SUIT_D;
      break;

     case 'h':
     case 'H':
      suit= Card::SUIT_H;
      break;

     case 's':
     case 'S':
      suit= Card::SUIT_S;
      break;

     default:
      printf("Invalid SUIT in %s\n", string);
      throw "InvalidCard";
      break;
   }

   result= NULL;
   for(i= 0; i<count; i++)
   {
     if( rank == card[i]->getRank() && suit == card[i]->getSuit() )
     {
       result= card[i];
       break;
     }
   }
   if( result == NULL )
   {
     printf("Internal Error: No card(%s)\n", string);
     throw "InternalError";
   }

   return result;
}

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

   int                 i;

   srand(RANDOMIZE);                // RANDOMIZE

   count= deck.getCount();
   for(i= 0; i < count; i++)
     deck.shuffle();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       countHand
//
// Purpose-
//       Determine the results of a Hand.
//
//----------------------------------------------------------------------------
inline void
   countHand(                       // Determine resultant
     Card*             load[9],     // The hand loader
     int&              playHand,    // The number of hands played
     int&              playTies,    // The number of hands tied
     int&              playWins)    // The number of hands won
{
   int                 cc;          // Comparison code
   PokerHand           handPlay;    // The Player's hand
   PokerHand           handThey;    // The Opponent's hand

   // Compute winner
   handPlay.load(7, &load[0]);
   handThey.load(7, &load[2]);

   cc= handPlay.compare(handThey);
   if( cc >= 0 )
   {
     if( cc > 0 )
       playWins++;
     else
       playTies++;
   }
   playHand++;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testHand
//
// Purpose-
//       Evaluate a hand.
//
//----------------------------------------------------------------------------
static void
   testHand(                        // Test a hand
     int               cardCount,   // The number of cards (first two down)
     Card*             card[],      // The cards already played
     int               muckCount,   // The number of mucked cards
     Card*             muck[])      // The mucked cards
{
   Card*               deal;        // The dealt card
   Deck                deck;        // The deck of cards
   int                 iteration;   // The iteration number
   int                 iterPrint;   // The next print iteration number
   Card*               load[9];     // The hand loader
   int                 playHand;    // The number of hands played
   int                 playTies;    // The number of hands tied
   int                 playWins;    // The number of hands won
   int                 rankHand;    // The number of hands ranked
   int                 rankTies;    // The number of ranked ties
   int                 rankWins;    // The number of ranked wins
   int                 rateHand;    // The number of hands rated
   int                 rateTies;    // The number of rated ties
   int                 rateWins;    // The number of rated wins

   int                 packCount;   // The number of cards left in the pack
   Deck                packDeck;    // The source cards for the pack
   Card*               pack[52];    // The cards left in the pack

   int                 playCount;   // The count - 2
   Card*               play[52];    // The "in play" cards

   int                 i;
   int                 j;

   rankHand= 0;                     // Initialize ranking
   rankTies= 0;
   rankWins= 0;

   rateHand= 0;                     // Initialize rating
   rateTies= 0;
   rateWins= 0;

   for(i= 0; i<cardCount; i++)
     load[i]= card[i];

   iterPrint= 1;
   if( 0 )
   {
     printf("\n");
     printf("[iterations] Players:     1        2"
            "        3        4        6        7\n");
   }

   // Initialize the pack
   packCount= 0;
   for(i= 0; i<52; i++)
   {
     deal= packDeck.deal();
     for(j= 0; j<cardCount; j++)
     {
       if( deal->getRank() == load[j]->getRank()
           && deal->getSuit() == load[j]->getSuit() )
       {
         deal= NULL;
         break;
       }
     }

     if( deal != NULL )
     {
       for(j= 0; j<muckCount; j++)
       {
         if( deal->getRank() == muck[j]->getRank()
             && deal->getSuit() == muck[j]->getSuit() )
         {
           deal= NULL;
           break;
         }
       }
       if( deal != NULL )
         pack[packCount++]= deal;
     }
   }
   if( cardCount + packCount < 9 )
   {
     printf("Not enough cards left in the deck!\n");
     throw "InvalidHand";
   }

   // Rank each hand separately
   for(int x0= 0; x0 < packCount; x0++)
   {
     for(int x1= x0+1; x1<packCount; x1++)
     {
       load[7]= pack[x0];           // The opponent's hand
       load[8]= pack[x1];

       playHand= 0;                 // Initialize summary for hand
       playTies= 0;
       playWins= 0;

       playCount= 0;
       for(i= 0; i<packCount; i++)
       {
         deal= pack[i];
         for(j= 7; j<9; j++)
         {
           if( deal->getRank() == load[j]->getRank()
               && deal->getSuit() == load[j]->getSuit() )
           {
             deal= NULL;
             break;
           }
         }
         if( deal != NULL )
           play[playCount++]= deal;
       }
       assert( playCount+2 == packCount );

       switch( cardCount )
       {
         case 2:
           {{{{
           Deck playDeck(playCount, play); // The "in play" pack
           for(iteration= 0; iteration<ITERATIONS; iteration++)
           {
             // Deal the cards
             playDeck.shuffle();
             for(i= cardCount; i<7; i++)
               load[i]= playDeck.deal();

             // Get the resultant
             countHand(load, playHand, playTies, playWins);

             // Display the results
             if( 0 )
             {
               if( iteration == iterPrint )
               {
                 iterPrint *= 10;
                 printf("[%10d]", iteration+1);

                 if( 0 )
                 {
                   printf(" Wins: %8.5f", double(playWins)/double(playHand));
                   printf(" Ties: %8.5f", double(playTies)/double(playHand));
                 }

                 if( 0 )
                 {
                   printf(" Wins %8d", playWins);
                   printf(" Ties %8d", playTies);
                   printf(" Hand %8d", playHand);
                 }
                 printf("\n");
               }
             }
           }
           }}}}
           break;

         case 5:
           for(int x5= 0; x5<playCount; x5++)
           {
             load[5]= play[x5];
             for(int x6= x5+1; x6<playCount; x6++)
             {
               load[6]= play[x6];
               countHand(load, playHand, playTies, playWins);
             }
           }
           break;

         case 6:
           for(int x6= 0; x6<playCount; x6++)
           {
             load[6]= play[x6];
             countHand(load, playHand, playTies, playWins);
           }
           break;

         case 7:
           countHand(load, playHand, playTies, playWins);
           break;

         default:
           printf("Invalid cardCount(%d)\n", cardCount);
           throw "InvalidHand";
       }

       // Update rating
       rateHand += playHand;
       rateWins += playWins;
       rateTies += playTies;

       // Update ranking
       int playLoss= playHand-playWins-playTies;

       if( playWins > playLoss + playLoss/8
           && playWins > playLoss + playTies/8 )
         rankWins++;
       else if( playWins + playWins/8 >= playLoss
           || playWins + playTies/8 >= playLoss )
         rankTies++;

       rankHand++;

       if( 0 )
       {
         char s7[32], s8[32];
         printf("%s %s {%6d, %6d, %6d} %6d ",
                load[7]->toShortString(s7),
                load[8]->toShortString(s8),
                playWins, playTies, playHand-playWins-playTies, playHand);

         if( playWins > playLoss + playLoss/8
             && playWins > playLoss + playTies/8 )
           printf("WIN\n");
         else if( playWins + playWins/8 >= playLoss
             || playWins + playTies/8 >= playLoss )
           printf("TIE\n");
         else
           printf("LOSS\n");
       }
     }
   }

   // Display results
   assert(rateHand > 0 && rankHand > 0);

   printf("Rate: %7.4f {%7.4f %7.4f %7.4f}\n"
          "Rate: %7d {%7d %7d %7d} %8d\n",
          double(rateWins+rateTies)/double(rateHand),
          double(rateWins)/double(rateHand),
          double(rateTies)/double(rateHand),
          double(rateHand-rateWins-rateTies)/double(rateHand),
          rateWins+rateTies, rateWins, rateTies, rateHand-rateWins-rateTies,
          rateHand);

   printf("Rank: %7.4f {%7.4f %7.4f %7.4f}\n"
          "Rank: %7d {%7d %7d %7d} %8d\n",
          double(rankWins+rankTies)/double(rankHand),
          double(rankWins)/double(rankHand),
          double(rankTies)/double(rankHand),
          double(rankHand-rankWins-rankTies)/double(rankHand),
          rankWins+rankTies, rankWins, rankTies, rankHand-rankWins-rankTies,
          rankHand);
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testParm
//
// Purpose-
//       Run using parameters.
//
//----------------------------------------------------------------------------
static void
   testParm(                        // Test a hand
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   int                 argx;        // Argument index
   int                 cardCount;   // The number of cards in cardArray
   Card*               cardArray[7];// The Player's Cards
   Card*               card;        // Working card
   int                 mucking;     // TRUE iff mucking cards
   int                 muckCount;   // The number of cards in muckArray
   Card*               muckArray[52]; // The mucked Cards
   int                 playCount;   // The number of players
   PokerRating         rating;      // The Rating
   char                string[Card::SIZE_NAME]; // Working string

   int                 i;
   int                 j;

   if( argc < 2 )                   // If no parameters
     return;

   if( argc < 3 )                   // Need at least two down cards
   {
     printf("Need at least two down cards\n");
     throw "InvalidHand";
   }

   mucking= FALSE;
   cardCount= 0;
   muckCount= 0;
   playCount= PLAYERS;
   cardArray[cardCount++]= getCard(argv[1]);
   cardArray[cardCount++]= getCard(argv[2]);
   for(argx= 3; argx<argc; argx++)
   {
     if( mucking )
     {
       if( muckCount >= 52 )
       {
         printf("Too many muck cards\n");
         throw "InvalidHand";
       }

       muckArray[muckCount++]= getCard(argv[argx]);
     }

     else
     {
       if( strcmp(argv[argx], "/") == 0 )
       {
         mucking= TRUE;
         continue;
       }

       if( cardCount >= 7 )
       {
         printf("Too many board cards\n");
         throw "InvalidHand";
       }

       cardArray[cardCount++]= getCard(argv[argx]);
     }
   }

   switch( cardCount )
   {
     case 2:
     case 5:
     case 6:
     case 7:
       break;

     case 3:
     case 4:
       printf("Need a flop of at least three cards\n");
       throw "InvalidHand";

     default:
       printf("Too many cards\n");
       throw "InvalidHand";
   }

   for(i= 0; i<cardCount; i++)
     printf(" %s", cardArray[i]->toShortString(string));
   if( muckCount > 0 )
   {
     printf(" / ");
     for(i= 0; i<muckCount; i++)
       printf(" %s", muckArray[i]->toShortString(string));
   }
   printf(":\n");

   for(i= 0; i<cardCount; i++)
   {
     card= cardArray[i];
     for(j= i+1; j<cardCount; j++)
     {
       if( cardArray[j]->getRank() == card->getRank()
           && cardArray[j]->getSuit() == card->getSuit() )
       {
         printf("%s Appears multiple times in hand\n",
                card->toShortString(string));
         throw "InvalidHand";
       }
     }
   }

   for(i= 0; i<muckCount; i++)
   {
     card= muckArray[i];
     for(j= 0; j<cardCount; j++)
     {
       if( cardArray[j]->getRank() == card->getRank()
           && cardArray[j]->getSuit() == card->getSuit() )
       {
         printf("%s Mucked, but appears in hand\n",
                card->toShortString(string));
         throw "InvalidHand";
       }
     }

     for(j= i+1; j<muckCount; j++)
     {
       if( muckArray[j]->getRank() == card->getRank()
           && muckArray[j]->getSuit() == card->getSuit() )
       {
         printf("%s Mucked multiple times\n",
                card->toShortString(string));
         throw "InvalidHand";
       }
     }
   }

   TexasStrategy::getRating(rating, playCount, cardCount, cardArray,
                            muckCount, muckArray);

   printf("#Play win+tie { p(win)  p(tie) p(loss)}\n");
   printf("[*%2d] %7.4f {%7.4f %7.4f %7.4f}\n", 2,
          rating.twoRate, rating.twoWins, rating.twoTies, rating.twoLoss);
   testHand(cardCount, cardArray, muckCount, muckArray);
   printf("[*%2d] %7.4f {%7.4f %7.4f %7.4f}\n", playCount,
          rating.allRate, rating.allWins, rating.allTies, rating.allLoss);
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
   main(                            // Mainline code
     int               argc,        // Argument count
     char*             argv[])      // Argument array
{
   try {
     init();
     testParm(argc, argv);
   } catch(const char* X) {
     printf("Exception(%s)\n", X);
   } catch(...) {
     printf("Exception(...)\n");
   }

   return 0;
}

