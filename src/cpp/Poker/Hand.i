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
//       Hand.i
//
// Purpose-
//       Hand inline methods.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef HAND_I_INCLUDED
#define HAND_I_INCLUDED

//----------------------------------------------------------------------------
// Hand::Accessor methods
//----------------------------------------------------------------------------
int                                 // The number of cards in the Hand
   Hand::getCount( void ) const     // Get number of cards in the Hand
{
   return count;
}

Card*                               // The associated Card*
   Hand::getCard(                   // Get associated Card*
     int               index) const // The card index
{
   if( index < 0 || index >= count )
     throw "Hand::getCard.InvalidIndex";

   return card[index];
}

//----------------------------------------------------------------------------
// PokerHand::Operators
//----------------------------------------------------------------------------
int                                 // Resultant
   PokerHand::operator<(            // Comparison operator
     const PokerHand&  hand)        // Source Hand
{
   return compare(hand) < 0;
}

int                                 // Resultant
   PokerHand::operator>(            // Comparison operator
     const PokerHand&  hand)        // Source Hand
{
   return compare(hand) > 0;
}

int                                 // Resultant
   PokerHand::operator==(           // Comparison operator
     const PokerHand&  hand)        // Source Hand
{
   return compare(hand) == 0;
}

int                                 // Resultant
   PokerHand::operator!=(           // Comparison operator
     const PokerHand&  hand)        // Source Hand
{
   return compare(hand) != 0;
}

int                                 // Resultant
   PokerHand::operator<=(           // Comparison operator
     const PokerHand&  hand)        // Source Hand
{
   return compare(hand) <= 0;
}

int                                 // Resultant
   PokerHand::operator>=(           // Comparison operator
     const PokerHand&  hand)        // Source Hand
{
   return compare(hand) >= 0;
}

//----------------------------------------------------------------------------
// PokerHand::Accessors
//----------------------------------------------------------------------------
PokerHand::Ranking                  // The Ranking
   PokerHand::getRanking( void ) const // Get Ranking
{
   return ranking;
}

const char*                         // The Ranking name
   PokerHand::getRankName(          // Get Ranking name
     Ranking           rank)        // For this Ranking
{
   const char*         result= "UNDEFINED";
   if( rank >= 0 && rank < RANKING_COUNT )
     result= rankName[rank];

   return result;
}

const char*                         // The Ranking name
   PokerHand::getRankName( void ) const // Get Ranking name
{
   return getRankName(ranking);
}

#endif // HAND_I_INCLUDED
