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
//       TestHand.cpp
//
// Purpose-
//       Test Hand and Card objects.
//
// Last change date-
//       2017/01/01
//
// Notes-
//       This program tests the Hand and Card objects.
//       It self-tests, counting errors.
//
// Usage-
//       testhand | tee testhand.out
//
//----------------------------------------------------------------------------
#include <time.h>

#include "Poker.h"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Deck            deck;        // The current Deck
static Card*           card;        // The current Card
static int             count;       // The number of cards in the Deck

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
   int                 i;

   srand(time(NULL));               // RANDOMIZE

   count= deck.getCount();
   for(i= 0; i < count; i++)
     deck.shuffle();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       gotX
//
// Purpose-
//       Error display.
//
//----------------------------------------------------------------------------
static void
   gotX(                            // Error display
     Card*             actual,      // The actual card
     Card*             expect)      // The expected card
{
   char                stringA[Card::SIZE_NAME];
   char                stringE[Card::SIZE_NAME];

   printf(" got(%s) expected(%s)\n",
          actual->toString(stringA), expect->toString(stringE));
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testFour
//
// Purpose-
//       Test four card hands
//
//----------------------------------------------------------------------------
static int                          // Number of errors encountered
   testFour(                        // Test one Hand
     Card&             c1,          // Card 1
     Card&             c2,          // Card 2
     Card&             c3,          // Card 3
     Card&             c4,          // Card 4
     Card&             c5,          // Card 5
     Card&             c6,          // Card 6
     Card&             c7,          // Card 7
     Card*             fourFlush,   // The Four Flush Card, if any
     Card*             oStraight,   // The Outside Straight Card, if any
     Card*             iStraight)   // The Inside Straight Card, if any
{
   int                 errorCount= 0; // Number of errors encountered

   Card*               card;        // Working card
   Hand                hand;        // The Hand
   Card*               load[7];     // Working load

   load[0]= &c1;
   load[1]= &c2;
   load[2]= &c3;
   load[3]= &c4;
   load[4]= &c5;
   load[5]= &c6;
   load[6]= &c7;
   hand.load(7, load);

   card= PokerHand::fourFlush(hand);
   if( card != fourFlush )
   {
     errorCount++;
     printf("testFour: FOUR FLUSH:");
     gotX(card, fourFlush);
   }

   card= PokerHand::fourOutside(hand);
   if( card != oStraight)
   {
     errorCount++;
     printf("testFour: OUTSIDE STRAIGHT:");
     gotX(card, oStraight);
   }

   card= PokerHand::fourInside(hand);
   if( card != iStraight)
   {
     errorCount++;
     printf("testFour: INSIDE STRAIGHT:");
     gotX(card, iStraight);
   }
   if( errorCount > 0 )
   {
     hand.display();
     printf("\n");
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       showHand
//
// Purpose-
//       Display a Hand
//
//----------------------------------------------------------------------------
extern void
   showHand(                        // Display Hand
     Card&             c1,          // Card 1
     Card&             c2,          // Card 2
     Card&             c3,          // Card 3
     Card&             c4,          // Card 4
     Card&             c5,          // Card 5
     Card&             c6,          // Card 6
     Card&             c7)          // Card 7
{
   Card*               load[7];     // Working load
   PokerHand           hand;        // The Hand

   load[0]= &c1;
   load[1]= &c2;
   load[2]= &c3;
   load[3]= &c4;
   load[4]= &c5;
   load[5]= &c6;
   load[6]= &c7;
   hand.load(7, load);

   printf("Hand(%s)\n", hand.getRankName());
   hand.display();
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testHand
//
// Purpose-
//       Test one Hand
//
//----------------------------------------------------------------------------
static int                          // Number of errors encountered
   testHand(                        // Test one Hand
     Card&             c1,          // Card 1
     Card&             c2,          // Card 2
     Card&             c3,          // Card 3
     Card&             c4,          // Card 4
     Card&             c5,          // Card 5
     Card&             c6,          // Card 6
     Card&             c7,          // Card 7
     PokerHand::Ranking
                       rank)        // The expected rank
{
   int                 errorCount= 0; // Number of errors encountered

   PokerHand           hand;        // The Hand
   Card*               load[7];     // Working load

   load[0]= &c1;
   load[1]= &c2;
   load[2]= &c3;
   load[3]= &c4;
   load[4]= &c5;
   load[5]= &c6;
   load[6]= &c7;
   hand.load(7, load);
   if( hand.getRanking() != rank )  // If error

   {
     errorCount++;
     printf("testHand: expected(%s) got(%s)\n",
            hand.getRankName(rank), hand.getRankName());
     hand.display();
     printf("\n");
   }

   return errorCount;
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       test
//
// Purpose-
//       Run tests.
//
//----------------------------------------------------------------------------
static int                          // Number of errors encountered
   test( void )                     // Run tests
{
   int                 errorCount= 0; // Error counter

   Card                c2(Card::RANK_2, Card::SUIT_C);
   Card                c3(Card::RANK_3, Card::SUIT_C);
   Card                c4(Card::RANK_4, Card::SUIT_C);
   Card                c5(Card::RANK_5, Card::SUIT_C);
   Card                c6(Card::RANK_6, Card::SUIT_C);
   Card                c7(Card::RANK_7, Card::SUIT_C);
   Card                c8(Card::RANK_8, Card::SUIT_C);
   Card                c9(Card::RANK_9, Card::SUIT_C);
   Card                cT(Card::RANK_T, Card::SUIT_C);
   Card                cJ(Card::RANK_J, Card::SUIT_C);
   Card                cQ(Card::RANK_Q, Card::SUIT_C);
   Card                cK(Card::RANK_K, Card::SUIT_C);
   Card                cA(Card::RANK_A, Card::SUIT_C);

   Card                d2(Card::RANK_2, Card::SUIT_D);
   Card                d3(Card::RANK_3, Card::SUIT_D);
   Card                d4(Card::RANK_4, Card::SUIT_D);
   Card                d5(Card::RANK_5, Card::SUIT_D);
   Card                d6(Card::RANK_6, Card::SUIT_D);
   Card                d7(Card::RANK_7, Card::SUIT_D);
   Card                d8(Card::RANK_8, Card::SUIT_D);
   Card                d9(Card::RANK_9, Card::SUIT_D);
   Card                dT(Card::RANK_T, Card::SUIT_D);
   Card                dJ(Card::RANK_J, Card::SUIT_D);
   Card                dQ(Card::RANK_Q, Card::SUIT_D);
   Card                dK(Card::RANK_K, Card::SUIT_D);
   Card                dA(Card::RANK_A, Card::SUIT_D);

   Card                h2(Card::RANK_2, Card::SUIT_H);
   Card                h3(Card::RANK_3, Card::SUIT_H);
   Card                h4(Card::RANK_4, Card::SUIT_H);
   Card                h5(Card::RANK_5, Card::SUIT_H);
   Card                h6(Card::RANK_6, Card::SUIT_H);
   Card                h7(Card::RANK_7, Card::SUIT_H);
   Card                h8(Card::RANK_8, Card::SUIT_H);
   Card                h9(Card::RANK_9, Card::SUIT_H);
   Card                hT(Card::RANK_T, Card::SUIT_H);
   Card                hJ(Card::RANK_J, Card::SUIT_H);
   Card                hQ(Card::RANK_Q, Card::SUIT_H);
   Card                hK(Card::RANK_K, Card::SUIT_H);
   Card                hA(Card::RANK_A, Card::SUIT_H);

   Card                s2(Card::RANK_2, Card::SUIT_S);
   Card                s3(Card::RANK_3, Card::SUIT_S);
   Card                s4(Card::RANK_4, Card::SUIT_S);
   Card                s5(Card::RANK_5, Card::SUIT_S);
   Card                s6(Card::RANK_6, Card::SUIT_S);
   Card                s7(Card::RANK_7, Card::SUIT_S);
   Card                s8(Card::RANK_8, Card::SUIT_S);
   Card                s9(Card::RANK_9, Card::SUIT_S);
   Card                sT(Card::RANK_T, Card::SUIT_S);
   Card                sJ(Card::RANK_J, Card::SUIT_S);
   Card                sQ(Card::RANK_Q, Card::SUIT_S);
   Card                sK(Card::RANK_K, Card::SUIT_S);
   Card                sA(Card::RANK_A, Card::SUIT_S);

   // Extra cards
   Card                x2(Card::RANK_2, Card::SUIT_S);
   Card                x3(Card::RANK_3, Card::SUIT_S);
   Card                x4(Card::RANK_4, Card::SUIT_S);
   Card                x5(Card::RANK_5, Card::SUIT_S);
   Card                x6(Card::RANK_6, Card::SUIT_S);
   Card                x7(Card::RANK_7, Card::SUIT_S);
   Card                x8(Card::RANK_8, Card::SUIT_S);
   Card                x9(Card::RANK_9, Card::SUIT_S);
   Card                xT(Card::RANK_T, Card::SUIT_S);
   Card                xJ(Card::RANK_J, Card::SUIT_S);
   Card                xQ(Card::RANK_Q, Card::SUIT_S);
   Card                xK(Card::RANK_K, Card::SUIT_S);
   Card                xA(Card::RANK_A, Card::SUIT_S);

   errorCount += testHand(cA, dA, hA, sA, xA, c6, c5, PokerHand::FiveOfAKind);
   errorCount += testHand(cA, dA, d6, hA, sA, xA, c5, PokerHand::FiveOfAKind);
   errorCount += testHand(c5, cA, dA, hA, sA, xA, c6, PokerHand::FiveOfAKind);
   errorCount += testHand(cA, dA, c6, d6, h6, s6, x6, PokerHand::FiveOfAKind);

   errorCount += testHand(c5, c6, c3, c2, c4, dA, sA, PokerHand::StraightFlush);
   errorCount += testHand(cA, dA, d5, d9, d3, d2, d4, PokerHand::StraightFlush);
   errorCount += testHand(c5, c4, d5, d4, d3, d2, dA, PokerHand::StraightFlush);
   errorCount += testHand(cA, hA, sA, s5, s4, s3, s2, PokerHand::StraightFlush);
   errorCount += testHand(hA, dA, cA, c5, c4, c3, c2, PokerHand::StraightFlush);
   errorCount += testHand(dA, cQ, cJ, cT, c9, cK, hA, PokerHand::StraightFlush);
   errorCount += testHand(dA, hA, cA, cQ, cJ, cT, cK, PokerHand::StraightFlush);
   errorCount += testHand(dK, cQ, cJ, hK, cT, c9, cK, PokerHand::StraightFlush);

   errorCount += testHand(c5, cA, dA, c6, hA, sA, s9, PokerHand::FourOfAKind);
   errorCount += testHand(c6, hA, sA, c5, cA, dA, s9, PokerHand::FourOfAKind);
   errorCount += testHand(c6, cA, dA, c6, hA, sA, s9, PokerHand::FourOfAKind);
   errorCount += testHand(dA, cA, s6, c6, hA, sA, h6, PokerHand::FourOfAKind);

   errorCount += testHand(sA, cA, hA, h4, s4, s2, s3, PokerHand::FullHouse);
   errorCount += testHand(s3, c3, h3, hA, s2, sA, s5, PokerHand::FullHouse);
   errorCount += testHand(sA, cA, hA, h4, s4, s2, h2, PokerHand::FullHouse);
   errorCount += testHand(sA, cA, c4, h4, s4, s2, h2, PokerHand::FullHouse);

                 showHand(sA, cA, hA, h4, s4, s2, s3);
                 showHand(s3, c3, h3, hA, s2, sA, s5);
                 showHand(sA, cA, hA, h4, s4, s2, h2);
                 showHand(sA, cA, c5, h5, s2, d2, h2);

   errorCount += testHand(cA, h3, h6, s2, s3, h4, s5, PokerHand::Straight);
   errorCount += testHand(dA, cA, h5, s2, s3, h4, s7, PokerHand::Straight);
   errorCount += testHand(dA, cA, hQ, sJ, sT, h9, sK, PokerHand::Straight);

   errorCount += testHand(dA, cA, hA, h7, s2, s6, s5, PokerHand::ThreeOfAKind);

   errorCount += testHand(dA, cA, c7, h7, s2, s6, s5, PokerHand::TwoPairs);

   errorCount += testHand(dA, cA, c7, h8, s2, s6, s5, PokerHand::OnePair);

   errorCount += testHand(c9, c8, h7, s5, s4, s3, s2, PokerHand::HighCard);

   // Test four card hands                              4F,   OS,   IS
   errorCount += testFour(cA, s8, s7, s6, d3, h3, c2, NULL, NULL, NULL);
   errorCount += testFour(cA, s8, s7, s6, d3, h3, c2, NULL, NULL, NULL);
   errorCount += testFour(sA, s8, s7, h6, d3, h3, c2, NULL, NULL, NULL);
   errorCount += testFour(cA, s8, s7, s6, d4, h3, c2, NULL, NULL,  &s8);
   errorCount += testFour(cA, sK, cK, hQ, dJ, s3, s2, NULL, NULL,  &cA);
   errorCount += testFour(cQ, cQ, hQ, d4, s3, s2, sA, NULL, NULL,  &d4);
   errorCount += testFour(sK, d8, c7, s6, h6, h5, s2, NULL,  &d8, NULL);
   errorCount += testFour(c9, s8, s7, s5, d4, h3, c2, NULL,  &s5,  &c9);
   errorCount += testFour(dT, s8, s7, c6, s5, h3, c2, NULL,  &s8,  &dT);
   errorCount += testFour(s8, s3, s2, s6, h8, d6, c6,  &s8, NULL, NULL);
   errorCount += testFour(c8, c7, h6, s4, s3, s2, sA,  &sA, NULL,  &c8);
   errorCount += testFour(d9, s8, h7, s6, s3, s2, cA,  &s8,  &d9, NULL);
   errorCount += testFour(s9, s8, s7, c5, h4, d3, s2,  &s9,  &c5,  &s9);

   errorCount += testFour(cA, hK, sJ, hT, h2, d3, d2, NULL, NULL,  &cA);
   errorCount += testFour(hQ, h9, c8, s5, dT, d6, d2, NULL, NULL,  &hQ);

   if( 1 )                          // Miscellaneous Hands
   {{{{
     char  string[Card::SIZE_NAME];
     Card* load[7];
     int i;

     printf("DECK--------\n");
     for(i= 0; i < count; i++)
     {
       card= deck.deal();
       printf("%s ", card->toShortString(string));
       printf("%s\n", card->toString(string));
     }
     printf("\n");

     for(i= 0; i < 52; i++)
     {
       for(int j= 0; j<7; j++)
         load[j]= deck.deal();

       PokerHand hand(7, load);
       printf("HAND-------- %s\n", hand.getRankName());
       hand.display();
       printf("\n");
     }
   }}}}

   return errorCount;
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
   int                 errorCount= 0;

   try {
     init();
     errorCount += test();
   } catch(const char* X) {
     printf("Exception(%s)\n", X);
   } catch(...) {
     printf("Exception(...)\n");
   }

   if( errorCount == 0 )
     printf("NO Errors\n");
   else
   {
     printf("%d Errors\n", errorCount);
     errorCount= 1;
   }

   return errorCount;
}

