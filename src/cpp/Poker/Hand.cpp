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
//       Hand.cpp
//
// Purpose-
//       Hand implementation.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include "Define.h"
#include "Card.h"
#include "Hand.h"

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#ifndef HCDM
#undef  HCDM                        // If defined, Hard Core Debug Mode
#endif

//----------------------------------------------------------------------------
// Constants for parameterization
//----------------------------------------------------------------------------
#define MAX_CARD 16                 // Maximum number of Cards in a Hand

//----------------------------------------------------------------------------
// External data areas
//----------------------------------------------------------------------------
const char*            PokerHand::rankName[]=
{  "UNRANKED"
,  "high card"
,  "one pair"
,  "two pairs"
,  "three of a kind"
,  "a straight"
,  "a flush"
,  "a full house"
,  "four of a kind"
,  "a straight flush"
,  "five of a kind"
};

//----------------------------------------------------------------------------
//
// Method-
//       Hand::~Hand
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Hand::~Hand( void )              // Destructor
{
   #ifdef HCDM
     printf("Hand(%p)::~Hand()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       Hand::Hand
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Hand::Hand( void )               // Constructor
:  count(0)
{
   #ifdef HCDM
     printf("Hand(%p)::Hand()\n", this);
   #endif
}

   Hand::Hand(                      // Copy constructor
     const Hand&       that)        // Source Hand
:  count(that.count)
{
   #ifdef HCDM
     printf("Hand(%p)::Hand(const Hand&(%p))\n", this, that);
   #endif

   for(int i= 0; i<count; i++)
     card[i]= that.card[i];
}

//----------------------------------------------------------------------------
//
// Method-
//       Hand::operator=
//
// Purpose-
//       Assignment operator
//
//----------------------------------------------------------------------------
Hand&                               // Resultant
   Hand::operator=(                 // Assignment operator
     const Hand&       that)        // Source Hand
{
   #ifdef HCDM
     printf("Hand(%p)::operator=(const Hand&(%p))\n", this, that);
   #endif

   count= that.count;
   for(int i= 0; i<count; i++)
     card[i]= that.card[i];

   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Hand::debug
//
// Purpose-
//       Debugging display
//
//----------------------------------------------------------------------------
void
   Hand::debug( void ) const        // Display the Hand
{
   char                string[Card::SIZE_NAME];

   int                 i;

   printf("Hand(%p)::debug()\n", this);
   for(i= 0; i<count; i++)
     printf("card[%d] %p %s\n", i, card[i], card[i]->toString(string));
}

//----------------------------------------------------------------------------
//
// Method-
//       Hand::display
//
// Purpose-
//       Display the Hand
//
//----------------------------------------------------------------------------
void
   Hand::display( void ) const      // Display the Hand
{
   char                string[Card::SIZE_NAME];

   int                 i;

   for(i= 0; i<count; i++)
   {
     printf("%s %s\n", card[i]->getVisible() ? "  UP" : "DOWN",
            card[i]->toString(string));
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Hand::load
//
// Purpose-
//       Load the Hand
//
//----------------------------------------------------------------------------
void
   Hand::load(                      // Load a hand
     int               count,       // Number of cards
     Card*             that[])      // The Card* array
{
   int                 i;

   if( count >= MAX_CARD )
     throw "Hand::load.InvalidCount";

   this->count= count;
   for(i= 0; i<count; i++)
     card[i]= that[i];
}

//----------------------------------------------------------------------------
//
// Method-
//       Hand::store
//
// Purpose-
//       Store the Hand
//
//----------------------------------------------------------------------------
void
   Hand::store(                     // Store a hand
     Card*             that[]) const// The Card* array
{
   for(int i= 0; i<count; i++)
     that[i]= card[i];
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHand::~PokerHand
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   PokerHand::~PokerHand( void )    // Destructor
{
   #ifdef HCDM
     printf("PokerHand(%p)::~PokerHand()\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHand::PokerHand
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   PokerHand::PokerHand( void )     // Default constructor
:  Hand()
,  ranking(UNRANKED)
{
   #ifdef HCDM
     printf("PokerHand(%p)::PokerHand()\n", this);
   #endif
}

   PokerHand::PokerHand(            // Copy constructor
     const Hand&       that)        // Source Hand
:  Hand()
,  ranking(UNRANKED)
{
   Card*               card[MAX_CARD]; // Working array

   #ifdef HCDM
     printf("PokerHand(%p)::PokerHand(const Hand&)\n", this);
   #endif

   that.store(card);
   load(that.getCount(), card);
}

   PokerHand::PokerHand(            // Constructor
     int               count,       // The number of available Cards
     Card*             that[])      // An array of available Cards
:  Hand()
,  ranking(UNRANKED)
{
   #ifdef HCDM
     printf("PokerHand(%p)::PokerHand(%d,Card*[])\n", this, count);
   #endif

   load(count, that);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHand::compare
//
// Purpose-
//       Compare Hands.
//
//----------------------------------------------------------------------------
int                                 // +1 (This better), =0, -1 (This worse)
   PokerHand::compare(              // Compare Hand
     const PokerHand&  that) const  // To this Hand
{
   int                 result= 0;   // Resultant;
   int                 count= this->count;

   int                 i;

   #ifdef HCDM
     printf("PokerHand(%p)::compare(%p)\n", this, &that);
   #endif

   if( ranking < that.ranking )     // We lose
     result= (-1);

   else if( ranking > that.ranking )// We win
     result= (+1);

   else
   {
     if( that.count < count )
       count= that.count;
     for(i= 0; i<count; i++)
     {
       if( this->card[i]->getRank() < that.card[i]->getRank() )
       {
         result= (-1);
         break;
       }

       else if( this->card[i]->getRank() > that.card[i]->getRank() )
       {
         result= (+1);
         break;
       }
     }
   }

   return result;
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHand::fill
//
// Purpose-
//       Fill remaining cards in the winning Hand
//
//----------------------------------------------------------------------------
void
   PokerHand::fill(                 // Fill the winning Hand
     Ranking           ranking,     // For this Ranking
     int               filled,      // Number of cards already filled
     int               source,      // Number of cards in that
     Card*             that[])      // The Card* array
{
   #ifdef HCDM
     printf("PokerHand(%p)::fill(%d)...\n", this, ranking);
   #endif

   this->ranking= ranking;
   count= filled;
   for(int i= count; i<5; i++)
   {
     card[i]= NULL;
     for(int j= 0; j<source; j++)
     {
       int inHand= FALSE;
       for(int k= 0; k<i; k++)
       {
         if( that[j] == card[k] )
         {
           inHand= TRUE;
           break;
         }
       }

       if( !inHand )
       {
         card[count++]= that[j];
         break;
       }
     }
   }

   #ifdef HCDM
     debug();
     printf("Hand(%p)::fill() COMPLETED\n", this);
   #endif
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHand::fourFlush
//
// Purpose-
//       Does the Hand contain a four flush?
//
//----------------------------------------------------------------------------
Card*                               // Highest resultant card
   PokerHand::fourFlush(            // Four cards to a flush?
     int               count,       // Number of cards
     Card*             load[])      // Card array
{
   Card*               card;        // Working resultant
   Card*               sort[Hand::MAX_HAND]; // The sorted load
   int                 H;           // Working hand index
   int                 X;           // Working card index
   Card::Suit          suit;        // Working suit

   assert(count < Hand::MAX_HAND);

   if( count >= 4 )
   {
     for(int i= 0; i<count; i++)
       sort[i]= load[i];
     Card::sortBySuit(count, sort);

     card= sort[0];
     suit= card->getSuit();
     H= 1;
     for(X= 1; X<count; X++)
     {
       if( sort[X]->getSuit() != suit )
       {
         if( (count-X) < 4 )
           break;

         card= sort[X];
         suit= card->getSuit();
         H= 1;
         continue;
       }

       H++;
       if( H == 4 )
         return card;
     }
   }

   return NULL;
}

Card*                               // Highest resultant card
   PokerHand::fourFlush(            // Four cards to a flush?
     const Hand&       that)        // In this Hand
{
   Card*               load[Hand::MAX_HAND];

   that.store(load);
   return fourFlush(that.getCount(), load);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHand::fourInside
//
// Purpose-
//       Does the Hand contain four cards to an inside straight?
//       (And does not contain either a straight or an outside straight)
//
//----------------------------------------------------------------------------
Card*                               // Highest resultant card
   PokerHand::fourInside(           // Four cards to an inside straight?
     int               count,       // Number of cards
     Card*             load[])      // Card array
{
   Card*               card;        // Working resultant
   Card*               sort[Hand::MAX_HAND]; // The sorted load
   int                 miss;        // Number of misses
   int                 H;           // Working hand index
   int                 X, Y;        // Working card index
   Card::Rank          rank;        // Working rank

   assert(count < Hand::MAX_HAND);

   for(int i= 0; i<count; i++)
     sort[i]= load[i];
   Card::sortByRank(count, sort);
   for(X= 0; X<count; X++)
   {
     card= sort[X];
     rank= card->getRank();
     H= 1;
     miss= FALSE;
     for(Y= X+1; Y<count; Y++)
     {
       if( sort[Y]->getRank() == rank )
         continue;

       if( sort[Y]->getRank() != (rank-1) )
       {
         if( miss == FALSE )
         {
           if( H == 4 && rank == Card::RANK_J )
             return card;

           if( H == 3 && rank == Card::RANK_2
               && sort[0]->getRank() == Card::RANK_A )
             return card;
         }

         if( sort[Y]->getRank() != (rank-2) || miss )
           break;

         miss= TRUE;
         H++;
       }

       H++;
       if( H == 5 )
       {
         if( miss == FALSE )
           break;

         return card;
       }
       rank= sort[Y]->getRank();
     }

     if( miss == FALSE )
     {
       if( H == 4 && rank == Card::RANK_J )
         return card;

       if( H == 3 && rank == Card::RANK_2
           && sort[0]->getRank() == Card::RANK_A )
         return card;
     }
   }

   return NULL;
}

Card*                               // Highest resultant card
   PokerHand::fourInside(           // Four cards to an inside straight?
     const Hand&       that)        // In this Hand
{
   Card*               load[Hand::MAX_HAND];

   that.store(load);
   return fourInside(that.getCount(), load);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHand::fourOutside
//
// Purpose-
//       Does the Hand contain four cards to an outside straight?
//
//----------------------------------------------------------------------------
Card*                               // Highest resultant card
   PokerHand::fourOutside(          // Four cards to an outside straight?
     int               count,       // Number of cards
     Card*             load[])      // Card array
{
   Card*               card;        // Working resultant
   Card*               sort[Hand::MAX_HAND]; // The sorted load
   int                 H;           // Working hand index
   int                 X;           // Working card index
   Card::Rank          rank;        // Working rank

   assert(count < Hand::MAX_HAND);

   if( count > 4 )
   {
     for(int i= 0; i<count; i++)
       sort[i]= load[i];
     Card::sortByRank(count, sort);

     card= sort[0];
     rank= card->getRank();
     H= 1;
     for(X= 1; X<count; X++)
     {
       if( sort[X]->getRank() == rank )
         continue;

       if( rank == Card::RANK_A || sort[X]->getRank() != (rank-1) )
       {
         if( (count-X) < 4 )
           break;

         card= sort[X];
         rank= card->getRank();
         H= 1;
         continue;
       }

       H++;
       if( H == 4 )
         return card;

       rank--;
     }
   }

   return NULL;
}

Card*                               // Highest resultant card
   PokerHand::fourOutside(          // Four cards to an outside straight?
     const Hand&       that)        // In this Hand
{
   Card*               load[Hand::MAX_HAND];

   that.store(load);
   return fourOutside(that.getCount(), load);
}

//----------------------------------------------------------------------------
//
// Method-
//       PokerHand::load
//
// Purpose-
//       Load the Hand.
//
//----------------------------------------------------------------------------
void
   PokerHand::load(                 // Load the Hand
     int               count,       // Number of cards
     Card*             that[])      // The Card* array
{
   Card*               byRank[MAX_HAND]; // Sorted by rank
   Card*               bySuit[MAX_HAND]; // Sorted by suit, then rank

   int                 H;           // Working hand index
   int                 X;           // Working card index
   int                 Y;           // Working card index
   Card*               aces[4];     // Aces {clubs, diamonds, hearts spades}
   Card::Rank          rank;        // Working rank
   Card::Suit          suit;        // Working suit

   // Debugging
   #ifdef HCDM
     printf("Hand(%p)::load()...\n", this);
     debug();
   #endif

   if( count >= MAX_HAND )
     throw "PokerHand::load.InvalidCount";

   // Initialize
   this->ranking= UNRANKED;
   this->count= 0;

   for(X= 0; X<count; X++)
   {
     byRank[X]= that[X];
     bySuit[X]= that[X];
   }

   Card::sortByRank(count, byRank);
   Card::sortBySuit(count, bySuit);

   // Look for Five of a kind
   if( ranking == UNRANKED )
   {
     if( count >= 5 )
     {
       card[0]= byRank[0];
       rank= card[0]->getRank();
       H= 1;
       for(X= 1; X<count; X++)
       {
         if( byRank[X]->getRank() != rank )
         {
           if( (count-X) < 5 )
             break;

           card[0]= byRank[X];
           rank= card[0]->getRank();
           H= 1;
           continue;
         }

         card[H++]= byRank[X];
         if( H == 5 )
         {
           fill(FiveOfAKind, 5, count, byRank);
           break;
         }
       }
     }
   }

   //-------------------------------------------------------------------------
   // Look for StraightFlush
   if( ranking == UNRANKED )
   {
     if( count >= 5 )
     {
       aces[0]= aces[1]= aces[2]= aces[3]= NULL;
       card[0]= bySuit[0];
       rank= card[0]->getRank();
       suit= card[0]->getSuit();
       H= 1;
       if( rank == Card::RANK_A )
         aces[suit]= card[0];
       for(X= 1; X<count; X++)
       {
         if( bySuit[X]->getSuit() != suit
             || bySuit[X]->getRank() != (rank-1) )
         {
           if( H == 4 && rank == Card::RANK_2 && aces[suit] != NULL )
           {
             card[4]= aces[suit];
             fill(StraightFlush, 5, count, bySuit);
             break;
           }

           if( (count-X) < 4 )
             break;

           card[0]= bySuit[X];
           rank= card[0]->getRank();
           suit= card[0]->getSuit();
           H= 1;
           if( rank == Card::RANK_A )
             aces[suit]= card[0];
           continue;
         }

         card[H++]= bySuit[X];
         if( H == 5 )
         {
           fill(StraightFlush, 5, count, bySuit);
           break;
         }
         rank--;
       }

       if( H == 4 && rank == Card::RANK_2 && aces[suit] != NULL )
       {
         card[4]= aces[suit];
         fill(StraightFlush, 5, count, bySuit);
       }
     }
   }

   //-------------------------------------------------------------------------
   // Look for FourOfAKind
   if( ranking == UNRANKED )
   {
     if( count >= 4 )
     {
       card[0]= byRank[0];
       rank= card[0]->getRank();
       H= 1;
       for(X= 1; X<count; X++)
       {
         if( byRank[X]->getRank() != rank )
         {
           if( (count-X) < 4 )
             break;

           card[0]= byRank[X];
           rank= card[0]->getRank();
           H= 1;
           continue;
         }

         card[H++]= byRank[X];
         if( H == 4 )
         {
           fill(FourOfAKind, 4, count, byRank);
           break;
         }
       }
     }
   }

   //-------------------------------------------------------------------------
   // Look for FullHouse
   if( ranking == UNRANKED )
   {
     if( count >= 5 )
     {
       Card* pair[2];
       pair[0]= pair[1]= NULL;

       card[0]= byRank[0];
       rank= card[0]->getRank();
       H= 1;
       for(X= 1; X<count; X++)
       {
         if( byRank[X]->getRank() != rank )
         {
           if( (count-X) < 3 || (pair[0] == NULL && (count-X) < 5) )
             break;

           card[0]= byRank[X];
           rank= card[0]->getRank();
           H= 1;
           continue;
         }

         card[H++]= byRank[X];
         if( H == 2 && pair[0] == NULL )
         {
           pair[0]= card[0];
           pair[1]= card[1];
         }

         if( H == 3 )
         {
           if( pair[0] != card[0] )
           {
             card[3]= pair[0];
             card[4]= pair[1];
             fill(FullHouse, 5, count, byRank);
             break;
           }

           for(Y= X+1; Y<count-1; Y++)
           {
             if( byRank[Y]->getRank() == byRank[Y+1]->getRank() )
             {
               card[3]= byRank[Y];
               card[4]= byRank[Y+1];
               fill(FullHouse, 5, count, byRank);
               break;
             }
           }
           break;
         }
       }
     }
   }

   //-------------------------------------------------------------------------
   // Look for Flush
   if( ranking == UNRANKED )
   {
     if( count >= 5 )
     {
       card[0]= bySuit[0];
       suit= card[0]->getSuit();
       H= 1;
       for(X= 1; X<count; X++)
       {
         if( bySuit[X]->getSuit() != suit )
         {
           if( (count-X) < 5 )
             break;

           card[0]= bySuit[X];
           suit= card[0]->getSuit();
           H= 1;
           continue;
         }

         card[H++]= bySuit[X];
         if( H == 5 )
         {
           fill(Flush, 5, count, bySuit);
           break;
         }
       }
     }
   }

   //-------------------------------------------------------------------------
   // Look for Straight
   if( ranking == UNRANKED )
   {
     if( count >= 5 )
     {
       card[0]= byRank[0];
       rank= card[0]->getRank();
       H= 1;
       for(X= 1; X<count; X++)
       {
         if( byRank[X]->getRank() == rank )
           continue;

         if( byRank[X]->getRank() != (rank-1) )
         {
           if( H == 4 && rank == Card::RANK_2
               && byRank[0]->getRank() == Card::RANK_A )
           {
             card[4]= byRank[0];
             fill(Straight, 5, count, byRank);
             break;
           }

           if( (count-X) < 4 )
             break;

           card[0]= byRank[X];
           rank= card[0]->getRank();
           H= 1;
           continue;
         }

         card[H++]= byRank[X];
         if( H == 5 )
         {
           fill(Straight, 5, count, byRank);
           break;
         }
         rank--;
       }

       if( H == 4 && rank == Card::RANK_2
           && byRank[0]->getRank() == Card::RANK_A )
       {
         card[4]= byRank[0];
         fill(Straight, 5, count, byRank);
       }
     }
   }

   //-------------------------------------------------------------------------
   // Look for ThreeOfAKind
   if( ranking == UNRANKED )
   {
     if( count >= 3 )
     {
       card[0]= byRank[0];
       rank= card[0]->getRank();
       H= 1;
       for(X= 1; X<count; X++)
       {
         if( byRank[X]->getRank() != rank )
         {
           if( (count-X) < 3 )
             break;

           card[0]= byRank[X];
           rank= card[0]->getRank();
           H= 1;
           continue;
         }

         card[H++]= byRank[X];
         if( H == 3 )
         {
           fill(ThreeOfAKind, 3, count, byRank);
           break;
         }
       }
     }
   }

   //-------------------------------------------------------------------------
   // Look for TwoPairs
   if( ranking == UNRANKED )
   {
     if( count >= 4 )
     {
       card[0]= byRank[0];
       rank= card[0]->getRank();
       for(X= 1; X<count; X++)
       {
         if( byRank[X]->getRank() != rank )
         {
           if( (count-X) < 4 )
             break;

           card[0]= byRank[X];
           rank= card[0]->getRank();
           continue;
         }

         card[1]= byRank[X];          // ONE pair complete
         for(Y= X+1; Y<count-1; Y++)
         {
           if( byRank[Y]->getRank() == byRank[Y+1]->getRank() )
           {
             card[2]= byRank[Y];
             card[3]= byRank[Y+1];
             fill(TwoPairs, 4, count, byRank);
             break;
           }
         }
         break;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Look for OnePair
   if( ranking == UNRANKED )
   {
     if( count >= 2 )
     {
       card[0]= byRank[0];
       for(X= 1; X<count; X++)
       {
         if( byRank[X]->getRank() != card[0]->getRank() )
         {
           card[0]= byRank[X];
           continue;
         }

         card[1]= byRank[X];
         fill(OnePair, 2, count, byRank);
         break;
       }
     }
   }

   //-------------------------------------------------------------------------
   // Look for HighCard
   if( ranking == UNRANKED )
   {
     fill(HighCard, 0, count, byRank);
   }

   #ifdef HCDM
     debug();
     printf("Hand(%p)::load() COMPLETED\n", this);
   #endif
}

