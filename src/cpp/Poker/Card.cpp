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
//       Card.cpp
//
// Purpose-
//       Card implementation.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#include <string.h>

#include "Card.h"

//----------------------------------------------------------------------------
// Internal data areas
//----------------------------------------------------------------------------
const char*            Card::shortRankName[]=
{  "2"
,  "3"
,  "4"
,  "5"
,  "6"
,  "7"
,  "8"
,  "9"
,  "T"
,  "J"
,  "Q"
,  "K"
,  "A"
};

const char*            Card::shortSuitName[]=
{  "C"
,  "D"
,  "H"
,  "S"
};

const char*            Card::rankName[]=
{  "  Two"
,  "Three"
,  " Four"
,  " Five"
,  "  Six"
,  "Seven"
,  "Eight"
,  " Nine"
,  "  Ten"
,  " Jack"
,  "Queen"
,  " King"
,  "  Ace"
};

const char*            Card::suitName[]=
{  "   Clubs"
,  "Diamonds"
,  "  Hearts"
,  "  Spades"
};

//----------------------------------------------------------------------------
//
// Method-
//       BaseCard::~BaseCard
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   BaseCard::~BaseCard( void )      // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       BaseCard::BaseCard
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   BaseCard::BaseCard( void )        // Default constructor
:  visible(FALSE)
{
}

   BaseCard::BaseCard(              // Copy constructor
     const BaseCard&   source)      // Source Card
{
   this->visible= source.visible;
}

//----------------------------------------------------------------------------
//
// Method-
//       BaseCard::operator=
//
// Purpose-
//       Assignment operator.
//
//----------------------------------------------------------------------------
BaseCard&
   BaseCard::operator=(             // Assignement operator
     const BaseCard&   source)      // Source Card
{
   this->visible= source.visible;
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       BaseCard::toShortString
//       BaseCard::toString
//
// Purpose-
//       Prepare for display.
//
//----------------------------------------------------------------------------
const char*                         // Resultant
   BaseCard::toShortString(         // Get short name of the BaseCard
     char*             string) const// Work area
{
   string[0]= '\0';
   return string;
}

const char*                         // Resultant
   BaseCard::toString(              // Get name of the BaseCard
     char*             string) const// Work area
{
   string[0]= '\0';
   return string;
}

//----------------------------------------------------------------------------
//
// Method-
//       Card::~Card
//
// Purpose-
//       Destructor.
//
//----------------------------------------------------------------------------
   Card::~Card( void )              // Destructor
{
}

//----------------------------------------------------------------------------
//
// Method-
//       Card::Card
//
// Purpose-
//       Constructor.
//
//----------------------------------------------------------------------------
   Card::Card(                      // Constructor
     Rank              rank,        // Rank
     Suit              suit)        // Suit
:  BaseCard()
,  rank(rank)
,  suit(suit)
{
   assert( rank >= RANK_MIN && rank <= RANK_MAX );
   assert( suit >= SUIT_MIN && suit <= SUIT_MAX );
}

   Card::Card(                      // Copy constructor
     const Card&       source)      // Source Card
:  BaseCard(source)
{
   this->rank= source.rank;
   this->suit= source.suit;
}

//----------------------------------------------------------------------------
//
// Method-
//       Card::operator=
//
// Purpose-
//       Assignment operator.
//
//----------------------------------------------------------------------------
Card&                               // Resultant
   Card::operator=(                 // Assignment operator
     const Card&       source)      // Source Card
{
   BaseCard::operator=(source);

   this->rank= source.rank;
   this->suit= source.suit;
   return *this;
}

//----------------------------------------------------------------------------
//
// Method-
//       Card::toShortString
//       Card::toString
//
// Purpose-
//       Prepare for display.
//
//----------------------------------------------------------------------------
const char*                         // Resultant
   Card::toShortString(             // Get short name of the Card
     char*             string) const// Work area
{
   strcpy(string, shortRankName[rank]);
   strcat(string, shortSuitName[suit]);
   return string;
}

const char*                         // Resultant
   Card::toString(                  // Get name of the Card
     char*             string) const// Work area
{
   strcpy(string, rankName[rank]);
   strcat(string, " ");
   strcat(string, suitName[suit]);
   return string;
}

//----------------------------------------------------------------------------
//
// Method-
//       Card::sortByRank
//
// Purpose-
//       Sort by Rank, high to low (only).
//
//----------------------------------------------------------------------------
void
   Card::sortByRank(                // Sort by Rank
     int               count,       // The number of cards in the array
     Card**            array)       // An array of Card*
{
   int                 i, j;

   for(i= 0; i<count; i++)
   {
     for(j= i+1; j<count; j++)
     {
       if( array[i]->rank < array[j]->rank )
       {
         Card* card= array[i];
         array[i]= array[j];
         array[j]= card;
       }
     }
   }
}

//----------------------------------------------------------------------------
//
// Method-
//       Card::sortBySuit
//
// Purpose-
//       Sort by Suit high to low, then by Rank high to low.
//
//----------------------------------------------------------------------------
void
   Card::sortBySuit(                // Sort by Suit, then by Rank
     int               count,       // The number of cards in the array
     Card**            array)       // An array of Card*
{
   int                 i, j;

   for(i= 0; i<count; i++)
   {
     for(j= i+1; j<count; j++)
     {
       if( array[i]->suit < array[j]->suit )
       {
         Card* card= array[i];
         array[i]= array[j];
         array[j]= card;
       }

       else if( array[i]->suit == array[j]->suit )
       {
         if( array[i]->rank < array[j]->rank )
         {
           Card* card= array[i];
           array[i]= array[j];
           array[j]= card;
         }
       }
     }
   }
}

