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
//       TestDist.h
//
// Purpose-
//       TestDist includes.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef TESTDIST_H_INCLUDED
#define TESTDIST_H_INCLUDED

#include "Result.h"

//----------------------------------------------------------------------------
//
// Struct-
//       Counter
//
// Purpose-
//       Result counter.
//
//----------------------------------------------------------------------------
struct Counter
{
   unsigned            hand;        // Number of Hands
   unsigned            ties;        // Number of TIES
   unsigned            wins;        // Number of WINS

//----------------------------------------------------------------------------
// Method: Counter::Counter
//----------------------------------------------------------------------------
inline
   Counter( void )
:  hand(0)
,  ties(0)
,  wins(0)
{
}

//----------------------------------------------------------------------------
// Method: Counter::countLoss
//----------------------------------------------------------------------------
inline void
   countLoss( void )                // Count a losing hand
{
   hand++;
}

//----------------------------------------------------------------------------
// Method: Counter::countTie
//----------------------------------------------------------------------------
inline void
   countTie( void )                 // Count a tied hand
{
   hand++;
   ties++;
}

//----------------------------------------------------------------------------
// Method: Counter::countWin
//----------------------------------------------------------------------------
inline void
   countWin( void )                 // Count a winning hand
{
   hand++;
   wins++;
}

//----------------------------------------------------------------------------
// Method: Counter::rate
//----------------------------------------------------------------------------
inline double                       // The rating
   rate( void )                     // Rate the Counter
{
   double              result= 1.0; // Resultant (default TIE)

   if( hand > 0 )
     result= double(wins+ties)/double(hand);

   return result;
}

//----------------------------------------------------------------------------
// Method: Counter::rateLoss
//----------------------------------------------------------------------------
inline double                       // The rating
   rateLoss( void )                 // Rate the Counter
{
   double              result= 0.0; // Resultant

   if( hand > 0 )
     result= double(hand-wins-ties)/double(hand);

   return result;
}

//----------------------------------------------------------------------------
// Method: Counter::rateTies
//----------------------------------------------------------------------------
inline double                       // The rating
   rateTies( void )                 // Rate the Counter
{
   double              result= 1.0; // Resultant

   if( hand > 0 )
     result= double(ties)/double(hand);

   return result;
}

//----------------------------------------------------------------------------
// Method: Counter::rateWins
//----------------------------------------------------------------------------
inline double                       // The rating
   rateWins( void )                 // Rate the Counter
{
   double              result= 0.0; // Resultant

   if( hand > 0 )
     result= double(wins)/double(hand);

   return result;
}

//----------------------------------------------------------------------------
// Method: Counter::display
//----------------------------------------------------------------------------
inline void
   display( void )                  // Display the heading
{
   printf("%7d (%7d/%7d) %7.4f", hand, wins, ties, rate());
}

//----------------------------------------------------------------------------
// Method: Counter::heading
//----------------------------------------------------------------------------
inline static void
   heading( void )                  // Display the heading
{
   printf("  Hands (    Won/   Tied)  Rating");
}

//----------------------------------------------------------------------------
// Method: Counter::reset
//----------------------------------------------------------------------------
inline void
   reset( void )                    // Reset the Counter
{
   hand= 0;
   ties= 0;
   wins= 0;
}
}; // struct Counter

//----------------------------------------------------------------------------
//
// Struct-
//       TexasDealCounter
//
// Purpose-
//       Texas Hold'em result distribution counters.
//
//----------------------------------------------------------------------------
struct TexasDealCounter
{
   Card::Rank          downHi;      // Higher down card rank
   Card::Rank          downLo;      // Lower down card rank
   int                 suited;      // TRUE iff suited

   Counter             player[8];   // For 0..7 players
}; // struct TexasDealCounter

#endif // TESTDIST_H_INCLUDED
