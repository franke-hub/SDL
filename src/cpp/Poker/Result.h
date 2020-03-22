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
//       Result.h
//
// Purpose-
//       Result enumeration.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef RESULT_H_INCLUDED
#define RESULT_H_INCLUDED

//----------------------------------------------------------------------------
//
// Enum-
//       PokerResult
//
// Purpose-
//       Enumerate the possible Poker play results.
//
//----------------------------------------------------------------------------
enum PokerResult                    // Result of (poker) play
{  RESULT_FOLD                      // Player folded
,  RESULT_LOST                      // Player lost
,  RESULT_TIES                      // Player ties
,  RESULT_WINS                      // Player wins
,  RESULT_TAKE                      // Player won without showing hand
,  RESULT_COUNT                     // Number of possible Results
}; // enum PokerResult

#endif // RESULT_H_INCLUDED
