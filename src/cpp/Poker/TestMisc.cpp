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
//       TestMisc.cpp
//
// Purpose-
//       Miscellaneous tests.
//
// Last change date-
//       2017/01/01
//
// Notes-
//       This program performs miscellaneous tests.
//       Only errors are reported.
//
// Usage-
//       testmisc
//
//----------------------------------------------------------------------------
#include <time.h>

#include "Poker.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define ITERATIONS 1000000

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Deck            deck;        // The current Deck

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

// srand(time(NULL));               // RANDOMIZE

   count= deck.getCount();
   for(i= 0; i < count; i++)
     deck.shuffle();
}

//----------------------------------------------------------------------------
//
// Subroutine-
//       testDeck
//
// Purpose-
//       Run Deck tests.
//
//----------------------------------------------------------------------------
static void
   testDeck( void )                 // Test Deck
{
   Card*               card[52];    // The cards in the Deck
   int                 count;       // The number of cards in the Deck
   char                string[Card::SIZE_NAME];

   int                 i;
   int                 j;

   //-------------------------------------------------------------------------
   // Make sure that a Deck contains 52 unique cards
   //-------------------------------------------------------------------------
   count= deck.getCount();
   for(i= 0; i<count; i++)
     card[i]= deck.deal();

   if( count != 52 )
     throw "testDeck.52CardException";

   for(i= 0; i<count; i++)
   {
     Card::Rank rank= card[i]->getRank();
     Card::Suit suit= card[i]->getSuit();
     for(j= i+1; j<count; j++)
     {
       if( rank == card[j]->getRank()
          && suit == card[j]->getSuit() )
       {
         printf("Duplicate: [%2d] [%2d]: %s\n",
                i, j, card[i]->toShortString(string));
         throw "testDeck.DuplicateCardException";
       }
     }
   }

   if( 0 )
   {{{{
     //-----------------------------------------------------------------------
     // Make sure that dealing all the cards from a deck does not change it.
     //-----------------------------------------------------------------------
     for(i= 0; i<count; i++)
     {
       deck.shuffle();

       for(j= 0; j<count; j++)
         card[j]= deck.deal();

       for(j= 0; j<count; j++)
       {
         if( card[j] != deck.deal() )
           throw "testDeck.DeckDealException";
       }
     }
   }}}}
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
     testDeck();
   } catch(const char* X) {
     printf("Exception(%s)\n", X);
   } catch(...) {
     printf("Exception(...)\n");
   }

   return 0;
}

