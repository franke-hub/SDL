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
//       HumanPoker.h
//
// Purpose-
//       Human poker objects.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef HUMANPOKER_H_INCLUDED
#define HUMANPOKER_H_INCLUDED

#include "Strategy.h"

//----------------------------------------------------------------------------
//
// Class-
//       HumanPokerStrategy
//
// Purpose-
//       HumanPokerStrategy object descriptor.
//
//----------------------------------------------------------------------------
class HumanPokerStrategy : public PokerStrategy // Human PokerStrategy
{
//----------------------------------------------------------------------------
// HumanPokerStrategy::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~HumanPokerStrategy( void );     // Destructor
   HumanPokerStrategy(              // Constructor
     PokerPlayer*      player,      // The PokerPlayer
     PokerTable*       table);      // The PokerTable

private:                            // Bitwise copy is prohibited
   HumanPokerStrategy(const HumanPokerStrategy&); // Disallowed copy constructor
   HumanPokerStrategy& operator=(const HumanPokerStrategy&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// HumanPokerStrategy::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Actual bet
   bet(                             // Bet
     int               amount);     // Minimum to call
}; // class HumanPokerStrategy

#endif // HUMANPOKER_H_INCLUDED
