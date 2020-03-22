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
//       Deck.cpp
//
// Purpose-
//       Deck implementation.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include "Deck.h"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
static Card            card_2C(Card::RANK_2, Card::SUIT_C);
static Card            card_3C(Card::RANK_3, Card::SUIT_C);
static Card            card_4C(Card::RANK_4, Card::SUIT_C);
static Card            card_5C(Card::RANK_5, Card::SUIT_C);
static Card            card_6C(Card::RANK_6, Card::SUIT_C);
static Card            card_7C(Card::RANK_7, Card::SUIT_C);
static Card            card_8C(Card::RANK_8, Card::SUIT_C);
static Card            card_9C(Card::RANK_9, Card::SUIT_C);
static Card            card_TC(Card::RANK_T, Card::SUIT_C);
static Card            card_JC(Card::RANK_J, Card::SUIT_C);
static Card            card_QC(Card::RANK_Q, Card::SUIT_C);
static Card            card_KC(Card::RANK_K, Card::SUIT_C);
static Card            card_AC(Card::RANK_A, Card::SUIT_C);

static Card            card_2D(Card::RANK_2, Card::SUIT_D);
static Card            card_3D(Card::RANK_3, Card::SUIT_D);
static Card            card_4D(Card::RANK_4, Card::SUIT_D);
static Card            card_5D(Card::RANK_5, Card::SUIT_D);
static Card            card_6D(Card::RANK_6, Card::SUIT_D);
static Card            card_7D(Card::RANK_7, Card::SUIT_D);
static Card            card_8D(Card::RANK_8, Card::SUIT_D);
static Card            card_9D(Card::RANK_9, Card::SUIT_D);
static Card            card_TD(Card::RANK_T, Card::SUIT_D);
static Card            card_JD(Card::RANK_J, Card::SUIT_D);
static Card            card_QD(Card::RANK_Q, Card::SUIT_D);
static Card            card_KD(Card::RANK_K, Card::SUIT_D);
static Card            card_AD(Card::RANK_A, Card::SUIT_D);

static Card            card_2H(Card::RANK_2, Card::SUIT_H);
static Card            card_3H(Card::RANK_3, Card::SUIT_H);
static Card            card_4H(Card::RANK_4, Card::SUIT_H);
static Card            card_5H(Card::RANK_5, Card::SUIT_H);
static Card            card_6H(Card::RANK_6, Card::SUIT_H);
static Card            card_7H(Card::RANK_7, Card::SUIT_H);
static Card            card_8H(Card::RANK_8, Card::SUIT_H);
static Card            card_9H(Card::RANK_9, Card::SUIT_H);
static Card            card_TH(Card::RANK_T, Card::SUIT_H);
static Card            card_JH(Card::RANK_J, Card::SUIT_H);
static Card            card_QH(Card::RANK_Q, Card::SUIT_H);
static Card            card_KH(Card::RANK_K, Card::SUIT_H);
static Card            card_AH(Card::RANK_A, Card::SUIT_H);

static Card            card_2S(Card::RANK_2, Card::SUIT_S);
static Card            card_3S(Card::RANK_3, Card::SUIT_S);
static Card            card_4S(Card::RANK_4, Card::SUIT_S);
static Card            card_5S(Card::RANK_5, Card::SUIT_S);
static Card            card_6S(Card::RANK_6, Card::SUIT_S);
static Card            card_7S(Card::RANK_7, Card::SUIT_S);
static Card            card_8S(Card::RANK_8, Card::SUIT_S);
static Card            card_9S(Card::RANK_9, Card::SUIT_S);
static Card            card_TS(Card::RANK_T, Card::SUIT_S);
static Card            card_JS(Card::RANK_J, Card::SUIT_S);
static Card            card_QS(Card::RANK_Q, Card::SUIT_S);
static Card            card_KS(Card::RANK_K, Card::SUIT_S);
static Card            card_AS(Card::RANK_A, Card::SUIT_S);

BaseCard*              deckArray[]= // A standard Deck
{  &card_AS
,  &card_2S
,  &card_3S
,  &card_4S
,  &card_5S
,  &card_6S
,  &card_7S
,  &card_8S
,  &card_9S
,  &card_TS
,  &card_JS
,  &card_QS
,  &card_KS

,  &card_AH
,  &card_2H
,  &card_3H
,  &card_4H
,  &card_5H
,  &card_6H
,  &card_7H
,  &card_8H
,  &card_9H
,  &card_TH
,  &card_JH
,  &card_QH
,  &card_KH

,  &card_AD
,  &card_2D
,  &card_3D
,  &card_4D
,  &card_5D
,  &card_6D
,  &card_7D
,  &card_8D
,  &card_9D
,  &card_TD
,  &card_JD
,  &card_QD
,  &card_KD

,  &card_AC
,  &card_2C
,  &card_3C
,  &card_4C
,  &card_5C
,  &card_6C
,  &card_7C
,  &card_8C
,  &card_9C
,  &card_TC
,  &card_JC
,  &card_QC
,  &card_KC
};

//----------------------------------------------------------------------------
//
// Method-
//       BaseDeck::~BaseDeck
//
// Purpose-
//       BaseDestructor.
//
//----------------------------------------------------------------------------
   BaseDeck::~BaseDeck( void )      // Destructor
{
   free(deck);
}

//----------------------------------------------------------------------------
//
// Method-
//       BaseDeck::BaseDeck
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   BaseDeck::BaseDeck(              // Constructor
     int               count,       // The number of cards in the Deck
     BaseCard**        load)        // The cards in the Deck
:  index(0)
,  count(count)
,  deck(NULL)
{
   int                 i;

   deck= (BaseCard**)malloc(count * sizeof(BaseCard*));
   if( deck == NULL )
     throw "NoStorageException";

   for(i= 0; i<count; i++)
     deck[i]= load[i];
}

//----------------------------------------------------------------------------
//
// Method-
//       BaseDeck::check
//
// Purpose-
//       Check the Deck
//
//----------------------------------------------------------------------------
void
   BaseDeck::check( void ) const    // Check the Deck
{
   for(int i= 0; i<count; i++)
   {
     for(int j= i+1; j<count; j++)
     {
       if( deck[i] == deck[j] )
         throw "BaseDeck::check.Exception";
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       BaseDeck::debug
//
// Purpose-
//       Debug the BaseDeck
//
//----------------------------------------------------------------------------
void
   BaseDeck::debug( void ) const    // Debug the BaseDeck
{
   char                string[Card::SIZE_NAME];

   printf("BaseDeck(%p)::debug() count(%d) index(%d)\n", this, count, index);
   for(int i= 0; i<count; i++)
   {
     deck[i]->toString(string);
     printf("[%3d] %s: %s\n",
            i, deck[i]->getVisible() ? "*UP*" : "DOWN", string);
   }
   printf("\n");
}

//----------------------------------------------------------------------------
//
// Method-
//       BaseDeck::deal
//
// Purpose-
//       Deal the next Card
//
//----------------------------------------------------------------------------
BaseCard*                           // -> next BaseCard
   BaseDeck::deal( void )           // Deal the next BaseCard
{
   #if defined(__CHECK__) && FALSE
     if( index >= count )
       throw "BaseDeck::deal.Exception";
   #else
     if( index >= count )
       index= 0;
   #endif

   return deck[index++];            // The next card
}

//----------------------------------------------------------------------------
//
// Method-
//       BaseDeck::shuffle
//
// Purpose-
//       Shuffle the BaseDeck
//
//----------------------------------------------------------------------------
void
   BaseDeck::shuffle( void )        // Shuffle the Deck
{
   BaseCard*           card[1024];  // Working deck

   int                 i;

   this->index= 0;
   for(i= 0; i<count; i++)
     deck[i]->setVisible(FALSE);

   #if( TRUE )
     //-----------------------------------------------------------------------
     // Mix the cards
     //-----------------------------------------------------------------------
     for(i= 0; i<count; i++)        // Move the deck to the working array
     {
       card[i]= deck[i];
       deck[i]= NULL;
     }

     for(int iteration= 0; iteration<count; iteration++)
     {
       i= rand()%count;
       while( card[i] == NULL )
       {
         i++;
         if( i >= count )
           i= 0;
       }

       int j= rand()%count;
       while( deck[j] != NULL )
       {
         j++;
         if( j >= count )
           j= 0;
       }

       deck[j]= card[i];
       card[i]= NULL;
     }

   #else
     //-----------------------------------------------------------------------
     // True shuffle emulation
     //-----------------------------------------------------------------------
     for(int iteration= 0; iteration<8; iteration++)
     {
       int split= count/2;          // Split index
       int left= 0;                 // Left shuffle index
       int right= split;            // Right shuffle index

       int index= 0;                // Resultant card placement
       while( left < split && right < count )
       {
         int chunk= rand()%4;       // Number of cards in chunk
         if( (left+chunk) > split )
           chunk= split - left;
         for(i= 0; i<chunk; i++)
           card[index++]= deck[left++];

         chunk= rand()%4;
         if( (right+chunk) > count )
           chunk= count - right;
         for(i= 0; i<chunk; i++)
           card[index++]= deck[right++];
       }

       for(i= left; i<split; i++)
         card[index++]= deck[i];

       for(i= right; i<count; i++)
         card[index++]= deck[i];

       for(i= 0; i<count; i++)
         deck[i]= card[i];
     }
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Deck::~Deck
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Deck::~Deck( void )              // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Deck::Deck
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Deck::Deck( void )               // Constructor
:  BaseDeck(52, deckArray)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Deck::Deck
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Deck::Deck(                      // Constructor
     int               count,       // The number of cards in the Deck
     Card**            deck)        // The cards in the Deck
:  BaseDeck(count, (BaseCard**)deck)
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Deck::check
//
// Purpose-
//       Check the Deck
//
//----------------------------------------------------------------------------
void
   Deck::check( void ) const        // Check the Deck
{
   BaseDeck::check();

   for(int i= 0; i<count; i++)
   {
     for(int j= i+1; j<count; j++)
     {
       Card* cardI= check_cast(Card*, deck[i]);
       Card* cardJ= check_cast(Card*, deck[j]);

       if( cardI->getRank() == cardJ->getRank()
           && cardI->getSuit() == cardJ->getSuit() )
         throw "Deck::check.Exception";
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Deck::deal
//
// Purpose-
//       Deal one Card
//
//----------------------------------------------------------------------------
Card*                               // -> Card
   Deck::deal( void )               // Deal one Card
{
   return check_cast(Card*, BaseDeck::deal());
}

