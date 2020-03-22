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
//       ExtendedTexasStrategy.h
//
// Purpose-
//       ExtendedTexasStrategy description.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef EXTENDEDTEXASSTRATEGY_H_INCLUDED
#define EXTENDEDTEXASSTRATEGY_H_INCLUDED

#ifndef TEXASPOKER_H_INCLUDED
#include "TexasPoker.h"
#endif

//----------------------------------------------------------------------------
//
// Class-
//       ExtendedTexasStrategy
//
// Purpose-
//       ExtendedTexasStrategy object descriptor.
//
//----------------------------------------------------------------------------
class ExtendedTexasStrategy : public TexasStrategy // ExtendedTexasStrategy
{
//----------------------------------------------------------------------------
// ExtendedTexasStrategy::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~ExtendedTexasStrategy( void );  // Destructor

   ExtendedTexasStrategy(           // Constructor
     PokerPlayer*      player,      // The PokerPlayer
     PokerTable*       table);      // The PokerTable

private:                            // Bitwise copy is prohibited
   ExtendedTexasStrategy(const ExtendedTexasStrategy&); // Disallowed copy constructor
   ExtendedTexasStrategy& operator=(const ExtendedTexasStrategy&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// ExtendedTexasStrategy::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Actual bet
   bet(                             // Bet
     int               amount);     // Minimum to call

virtual void                        // Resultant
   rate( void );                    // Rate a Hand

virtual void
   reset( void );                   // Reset to default state

//----------------------------------------------------------------------------
// ExtendedTexasStrategy::Attributes
//----------------------------------------------------------------------------
protected:
   // None defined
}; // class ExtendedTexasStrategy

#endif // EXTENDEDTEXASSTRATEGY_H_INCLUDED
