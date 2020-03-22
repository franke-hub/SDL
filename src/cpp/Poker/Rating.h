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
//       Rating.h
//
// Purpose-
//       Rating description.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef RATING_H_INCLUDED
#define RATING_H_INCLUDED

#include <stdio.h>

//----------------------------------------------------------------------------
//
// Struct-
//       Rating
//
// Purpose-
//       The informaion used to evaluate a hand.
//
//----------------------------------------------------------------------------
struct Rating                       // Rating
{
}; // struct Rating

//----------------------------------------------------------------------------
//
// Struct-
//       PokerRating
//
// Purpose-
//       The informaion used to evaluate a poker hand.
//
//----------------------------------------------------------------------------
struct PokerRating : public Rating  // PokerRating
{
//----------------------------------------------------------------------------
// Rating::Attributes
//----------------------------------------------------------------------------
   // Rating with two players
   double              twoRate;     // twoWins + twoTies
   double              twoWins;     // Two player probability(win)
   double              twoTies;     // Two player probability(tie)
   double              twoLoss;     // Two player probability(loss)

   // Rating with all players
   double              allRate;     // allWins + allTies
   double              allWins;     // All player probability(win)
   double              allTies;     // All player probability(tie)
   double              allLoss;     // All player probability(loss)

//----------------------------------------------------------------------------
// Rating::Methods
//----------------------------------------------------------------------------
void
   display( void ) const            // Display the Rating
{
   printf("%7.4f {%7.4f %7.4f %7.4f} %7.4f {%7.4f %7.4f %7.4f}",
          twoRate, twoWins, twoTies, twoLoss,
          allRate, allWins, allTies, allLoss);
}

void
   reset( void )                    // Reset the Rating
{
   twoRate= 1.0;
   twoWins= 0.0;
   twoTies= 1.0;
   twoLoss= 0.0;

   allRate= 1.0;
   allWins= 0.0;
   allTies= 1.0;
   allLoss= 0.0;
}
}; // struct PokerRating

#endif // RATING_H_INCLUDED
