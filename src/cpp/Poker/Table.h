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
//       Table.h
//
// Purpose-
//       Table description.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include "Define.h"
#include "Deck.h"
#include "History.h"
#include "Strategy.h"

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Card;
class Player;
class PokerPlayer;
class PokerRating;

//----------------------------------------------------------------------------
//
// Class-
//       Table
//
// Purpose-
//       Table object descriptor.
//
//----------------------------------------------------------------------------
class Table                         // Table
{
//----------------------------------------------------------------------------
// Table::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  MAX_PLAYER= 8                    // The maximum number of Players
}; // enum

//----------------------------------------------------------------------------
// Table::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Table( void );                  // Destructor
   Table( void );                   // Default constructor

private:                            // Bitwise copy is prohibited
   Table(const Table&);             // Disallowed copy constructor
   Table& operator=(const Table&);  // Disallowed assignment operator

//----------------------------------------------------------------------------
// Table::Accessor methods
//----------------------------------------------------------------------------
public:
inline const char*                  // The table's game
   getGame( void ) const;           // Get table's game

inline const char*                  // The table's name
   getName( void ) const;           // Get table's name

inline Player*                      // -> Player
   getPlayer(                       // Get Player
     int               seat) const; // At this seat

inline int                          // The number of Players
   getPlayerCount( void ) const;    // Get number of Players

inline int                          // The seat number
   getSeat(                         // Get seat index
     Player*           player) const; // For this Player

inline int                          // The next seat
   nextSeat(                        // Get next seat
     int               seat) const; // From this seat

inline int                          // The prior seat
   priorSeat(                       // Get prior seat
     int               seat) const; // From this seat

//----------------------------------------------------------------------------
// Table::Methods
//----------------------------------------------------------------------------
public:
void
   addPlayer(                       // Add a Player
     Player*           player);     // The Player

virtual void
   debug( void );                   // Debug the Table

virtual void
   display( void );                 // Display the Table

virtual Strategy*                   // -> new Strategy
   makeStrategy(                    // Make a Strategy
     Player*           player);     // For this Player

virtual void
   play( void ) = 0;                // Play a new hand

virtual void
   remPlayer(                       // Remove a Player
     Player*           player);     // The Player

virtual void
   reset( void );                   // Reset the Table

//----------------------------------------------------------------------------
// Table::Attributes
//----------------------------------------------------------------------------
protected:
   const char*         name;        // The Table's name
   const char*         game;        // The Table's game

   int                 playerCount; // The number of Players
   Player*             player[MAX_PLAYER]; // The Player array
}; // class Table

//----------------------------------------------------------------------------
//
// Class-
//       PokerTable
//
// Purpose-
//       PokerTable object descriptor.
//
//----------------------------------------------------------------------------
class PokerTable : public Table     // PokerTable
{
//----------------------------------------------------------------------------
// PokerTable::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  MAX_ROUNDS = 16                  // The maximum number of Rounds
}; // enum

enum BetLimit                       // The betting limit
{  BETLIMIT_NDQ                     // Nickel, Dime, Quarter
,  BETLIMIT_POT                     // Pot limit
,  BETLIMIT_TABLE                   // Table stakes
}; // enum BetLimit

//----------------------------------------------------------------------------
// PokerTable::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~PokerTable( void );             // Destructor
   PokerTable( void );              // Default constructor

private:                            // Bitwise copy is prohibited
   PokerTable(const PokerTable&);   // Disallowed copy constructor
   PokerTable& operator=(const PokerTable&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// PokerTable::Accessor methods
//----------------------------------------------------------------------------
public:
inline int                          // The number of active Players
   getActiveCount( void ) const;    // Get number of active Players

inline int                          // The amount bet
   getAmount(                       // Get amount bet
     int               seat) const; // For this seat

inline BetLimit                     // The betting limit
   getBetLimit( void ) const;       // Get betting limit

inline int                          // The big blind ante
   getBigBlind( void ) const;       // Get big blind ante

inline int                          // The number of bets without a raise
   getCallCount( void ) const;      // Get number of bets without a raise

inline int                          // The FOLDED attribute
   getFolded(                       // Get FOLDED attribute
     int               seat) const; // For this seat

inline PokerHistory&                // The PokerHistory
   getHistory( void );              // Get PokerHistory

int                                 // The number of players left to act
   getLeft2Act(                     // Get number of players left to act
     Player*           player) const; // For this player

int                                 // The maximum raise amount
   getMaxRaise( void ) const;       // Get maximum raise amount

int                                 // The minimum raise amount
   getMinRaise( void ) const;       // Get minimum raise amount

PokerPlayer*                        // --> associated PokerPlayer
   getPlayer(                       // Get PokerPlayer
     int               seat) const; // At this seat

inline int                          // The current POT
   getPot( void ) const;            // Get current POT

inline int                          // The amount raised
   getRaised(                       // Get amount raised
     int               seat) const; // For this seat

inline int                          // The number of different raisers
   getRaiseCount( void ) const;     // Get raise count

inline int                          // The current ROUND
   getRound( void ) const;          // Get current ROUND

inline int                          // The number of ROUNDs
   getRoundCount( void ) const;     // Get number of ROUNDs

inline int                          // The small blind ante
   getSmallBlind( void ) const;     // Get small blind ante

inline void
   setAnte(                         // Set common ante
     int               ante);       // The common ante

inline void
   setBetLimit(                     // Set betting limit
     BetLimit          limit);      // The betting limit

inline void
   setBigBlind(                     // Set big blind
     int               ante);       // The big blind

inline void
   setSmallBlind(                   // Set small blind
     int               ante);       // The small blind

//----------------------------------------------------------------------------
// PokerTable::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debug( void );                   // Debug the PokerTable

virtual void
   display( void );                 // Display the PokerTable

virtual Strategy*                   // -> new Strategy
   makeStrategy(                    // Make a Strategy
     Player*           player);     // For this Player

virtual void
   play( void );                    // Play a new poker hand

protected:
virtual void
   anteUp( void );                  // Start a new Hand

void
   betAround(                       // Round of betting
     int               last2Act);   // Last player to act

virtual void
   reset( void );                   // Reset the PokerTable

virtual void
   setWinners(                      // Set the winner array
     int               winner[MAX_PLAYER], // TRUE iff player has winner
     const int         folded[MAX_PLAYER]) const;// TRUE iff player folded

virtual void
   showDown( void );                // Show the result

void
   splitPot( void );                // Distribute the winnings

//----------------------------------------------------------------------------
// PokerTable::Required methods
//----------------------------------------------------------------------------
protected:
virtual void
   playHand( void ) = 0;            // Play a new hand

//----------------------------------------------------------------------------
// PokerTable::Attributes
//----------------------------------------------------------------------------
protected:
   Deck                deck;        // The Deck

   // The initial bets
   int                 ante;        // The common ante
   int                 bBlind;      // The big blind ante
   int                 sBlind;      // The small blind ante

   // Betting evaluation information
   BetLimit            betLimit;    // The Betting limit
   int                 button;      // The Player with the deal
   int                 callCount;   // The number of bets without a raise
   PokerHistory        history;     // This hand's betting history
   int                 last2Act;    // The last player to act
   int                 pot;         // The amount of money in the pot
   int                 round;       // The current betting round
   int                 roundCount;  // The number of betting rounds

   int                 amount[MAX_PLAYER]; // The Player hand bet amount
   int                 folded[MAX_PLAYER]; // TRUE iff player folded
   int                 payout[MAX_PLAYER]; // The amount won
   int                 raised[MAX_PLAYER]; // The Player hand raise amount
}; // class PokerTable

#include "Table.i"

#endif // TABLE_H_INCLUDED
