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
//       Player.h
//
// Purpose-
//       Player description.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef PLAYER_H_INCLUDED
#define PLAYER_H_INCLUDED

#include "Define.h"
#include "Card.h"
#include "Hand.h"
#include "Rating.h"
#include "Result.h"

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Table;
class PokerTable;
class PokerStrategy;

//----------------------------------------------------------------------------
//
// Class-
//       Player
//
// Purpose-
//       Player object descriptor.
//
//----------------------------------------------------------------------------
class Player                        // Player
{
//----------------------------------------------------------------------------
// Player::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Player( void );                 // Destructor
   Player( void );                  // Default constructor
   Player(                          // Constructor
     const char*       name);       // The Player's name

private:                            // Bitwise copy is prohibited
   Player(const Player&);           // Disallowed copy constructor
   Player& operator=(const Player&);// Disallowed assignment operator

//----------------------------------------------------------------------------
// Player::Accessor methods
//----------------------------------------------------------------------------
public:
inline int                          // The Player's Card count
   getCardCount( void ) const;      // Get Player's Card count

inline const char*                  // The Player's name
   getName( void ) const;           // Get Player's name

inline Table*                       // ->  associated Table
   getTable( void ) const;          // Get associated Table

inline void
   setTable(                        // Set the Table
     Table*            table);      // The Table

inline void
   store(                           // Store a hand
     Card*             that[]) const; // The Card* array

//----------------------------------------------------------------------------
// Player::Methods
//----------------------------------------------------------------------------
public:
virtual void
   addCard(                         // Add a Card to the Hand
     Card*             card);       // Add this Card

virtual void
   debug( void );                   // Debug the Player

virtual void
   display( void );                 // Display the Player

virtual void
   reset( void );                   // Reset the Player (New Hand)

//----------------------------------------------------------------------------
// Player::Attributes
//----------------------------------------------------------------------------
protected:
   const char*         name;        // The Player's name
   Table*              table;       // The Player's table

   int                 cardCount;   // Number of cards in the Hand
   Card*               card[Hand::MAX_HAND]; // The player's Cards
}; // class Player

//----------------------------------------------------------------------------
//
// Class-
//       PokerPlayer
//
// Purpose-
//       PokerPlayer object descriptor.
//
//----------------------------------------------------------------------------
class PokerPlayer : public Player   // Player
{
//----------------------------------------------------------------------------
// PokerPlayer::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~PokerPlayer( void );            // Destructor
   PokerPlayer( void );             // Default constructor
   PokerPlayer(                     // Constructor
     const char*       name,        // The Player's name
     int               stake);      // The Player's initial stake

private:                            // Bitwise copy is prohibited
   PokerPlayer(const PokerPlayer&); // Disallowed copy constructor
   PokerPlayer& operator=(const PokerPlayer&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// PokerPlayer::Accessor methods
//----------------------------------------------------------------------------
public:
inline int                          // The Player's stake
   getStake( void ) const;          // Get Player's stake

void
   setStrategy(                     // Set Player's Strategy
     PokerStrategy*    strategy);   // Strategy (Now owned by PokerPlayer)

PokerTable*                         // ->  associated PokerTable
   getTable( void ) const;          // Get associated PokerTable

void
   setTable(                        // Set the PokerTable
     PokerTable*       table);      // The PokerTable

//----------------------------------------------------------------------------
// PokerPlayer::Methods
//----------------------------------------------------------------------------
public:
virtual void
   addCard(                         // Add a Card to the Hand
     Card*             card);       // Add this Card

inline void
   addStake(                        // Add to Player's stake
     int               amount);     // This amount

int                                 // Actual ante
   ante(                            // Ante
     int               amount);     // This amount

int                                 // Actual bet
   bet(                             // Bet
     int               amount);     // Minimum to call

int                                 // +1 (This better), =0, -1 (This worse)
   compare(                         // Compare PokerPlayer's Hand
     PokerPlayer*      player);     // With this PokerPlayer's Hand

virtual void
   debug( void );                   // Debug the PokerPlayer

virtual void
   display( void );                 // Display the PokerPlayer

virtual void
   reset( void );                   // Reset the Player (for new Hand)

//----------------------------------------------------------------------------
// PokerPlayer::Attributes
//----------------------------------------------------------------------------
protected:
   PokerHand           hand;        // The Player's Hand
   int                 stake;       // The Player's stake
   PokerStrategy*      strategy;    // The Player's PokerStrategy
}; // class PokerPlayer

#include "Player.i"

#endif // PLAYER_H_INCLUDED
