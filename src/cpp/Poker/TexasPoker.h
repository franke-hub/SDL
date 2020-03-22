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
//       TexasPoker.h
//
// Purpose-
//       Texas hold'em declarations.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef TEXASPOKER_H_INCLUDED
#define TEXASPOKER_H_INCLUDED

#include "Strategy.h"
#include "Table.h"

//----------------------------------------------------------------------------
//
// Enum-
//       TexasRound
//
// Purpose-
//       Texas Hold'em Round enumeration.
//
//----------------------------------------------------------------------------
enum TexasRound                     // Betting round
{  TEXAS_DEAL                       // The DEAL
,  TEXAS_FLOP                       // The FLOP
,  TEXAS_TURN                       // The TURN
,  TEXAS_RIVER                      // The RIVER
,  TEXAS_COUNT                      // The number of betting rounds
,  TEXAS_LAST= TEXAS_RIVER          // The RIVER is the LAST round
}; // enum TexasRound

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class TexasTable;

//----------------------------------------------------------------------------
//
// Class-
//       TexasStrategy
//
// Purpose-
//       TexasStrategy object descriptor.
//
//----------------------------------------------------------------------------
class TexasStrategy : public PokerStrategy // TexasStrategy
{
//----------------------------------------------------------------------------
// TexasStrategy::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~TexasStrategy( void );          // Destructor
   TexasStrategy(                   // Constructor
     PokerPlayer*      player,      // The PokerPlayer
     PokerTable*       table,       // The PokerTable
     Model             model);      // The outerlying Model

private:                            // Bitwise copy is prohibited
   TexasStrategy(const TexasStrategy&); // Disallowed copy constructor
   TexasStrategy& operator=(const TexasStrategy&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// TexasStrategy::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Actual bet
   bet(                             // Bet
     int               amount);     // Minimum to call

virtual void
   rate( void );                    // Update the rating

virtual void
   reset( void );                   // Reset to default state

//----------------------------------------------------------------------------
// Static method (for external evaluators)
//----------------------------------------------------------------------------
static void
   getRating(                       // Get associated Rating
     PokerRating&      result,      // Resultant Rating
     int               playCount,   // Number of Players
     int               cardCount,   // Number of Cards
     Card**            card,        // The Player's Cards
     int               muckCount= 0,// Number of mucked Cards
     Card**            muck= NULL); // The mucked Cards

//----------------------------------------------------------------------------
// TexasStrategy::Attributes
//----------------------------------------------------------------------------
public:
   // None defined
}; // class TexasStrategy

//----------------------------------------------------------------------------
//
// Class-
//       TexasTable
//
// Purpose-
//       Texas Hold'em Table object descriptor.
//
//----------------------------------------------------------------------------
class TexasTable : public PokerTable  // TexasTable
{
//----------------------------------------------------------------------------
// TexasTable::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~TexasTable( void );             // Destructor
   TexasTable( void );              // Default constructor

private:                            // Bitwise copy is prohibited
   TexasTable(const TexasTable&);   // Disallowed copy constructor
   TexasTable& operator=(const TexasTable&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// TexasTable::Methods
//----------------------------------------------------------------------------
public:
virtual Strategy*                   // -> new Strategy
   makeStrategy(                    // Make a Strategy
     Player*           player);     // For this Player

//----------------------------------------------------------------------------
// TexasTable::Required methods
//----------------------------------------------------------------------------
protected:
virtual void
   playHand( void );                // Play a new hand

virtual void                        // Show the Hands
   showDown( void );                // Show the Hands

//----------------------------------------------------------------------------
// TexasTable::Attributes
//----------------------------------------------------------------------------
protected:
   Card*               downHi[MAX_PLAYER]; // The higher down card
   Card*               downLo[MAX_PLAYER]; // The lower down card

   int                 boardCount;  // Number of board Cards played
   Card*               board[5];    // The board
}; // class TexasTable

#endif // TEXASPOKER_H_INCLUDED

