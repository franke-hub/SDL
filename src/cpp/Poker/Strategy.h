//----------------------------------------------------------------------------
//
//       Copyright (C) 2017-2018 Frank Eskesen.
//
//       This file is free content, distributed under the GNU General
//       Public License, version 3.0.
//       (See accompanying file LICENSE.GPL-3.0 or the original
//       contained within https://www.gnu.org/licenses/gpl-3.0.en.html)
//
//----------------------------------------------------------------------------
//
// Title-
//       Strategy.h
//
// Purpose-
//       Strategy description.
//
// Last change date-
//       2018/01/01
//
//----------------------------------------------------------------------------
#ifndef STRATEGY_H_INCLUDED
#define STRATEGY_H_INCLUDED

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Player;
class PokerPlayer;
class PokerRating;
class PokerTable;
class Table;

//----------------------------------------------------------------------------
//
// Class-
//       Strategy
//
// Purpose-
//       Strategy object descriptor.
//
//----------------------------------------------------------------------------
class Strategy                      // Strategy
{
//----------------------------------------------------------------------------
// Strategy::Typedefs and enumerations
//----------------------------------------------------------------------------
public:

//----------------------------------------------------------------------------
// Strategy::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Strategy( void );               // Destructor
   Strategy(                        // Constructor
     Player*           player,      // The Player
     Table*            table);      // The Table

private:                            // Bitwise copy is prohibited
   Strategy(const Strategy&);       // Disallowed copy constructor
   Strategy& operator=(const Strategy&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// Strategy::Accessor methods
//----------------------------------------------------------------------------
public:
inline Player*                      // ->  associated Player
   getPlayer( void ) const;         // Get associated Player

inline Table*                       // ->  associated Table
   getTable( void ) const;          // Get associated Table

inline void
   setTable(                        // Set the Table
     Table*            table);      // The Table

//----------------------------------------------------------------------------
// Strategy::Methods
//----------------------------------------------------------------------------
public:
virtual void
   reset( void );                   // Reset to default state

//----------------------------------------------------------------------------
// Strategy::Attributes
//----------------------------------------------------------------------------
protected:
   Player*             player;      // The player
   Table*              table;       // The table
}; // class Strategy

//----------------------------------------------------------------------------
//
// Class-
//       PokerStrategy
//
// Purpose-
//       PokerStrategy object descriptor.
//
//----------------------------------------------------------------------------
class PokerStrategy : public Strategy // PokerStrategy
{
//----------------------------------------------------------------------------
// PokerStrategy::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum Model                          // The betting model
{  MODEL_RANDOM                     // Random
,  MODEL_CONSERVATIVE               // Conservative
,  MODEL_NEUTRAL                    // Neutral
,  MODEL_AGGRESSIVE                 // Aggressive

,  MODEL_DEFAULT= MODEL_RANDOM      // DEFAULT
}; // enum Model

protected:
enum State                          // The betting state
{  STATE_RESET                      // Reset
,  STATE_CONSERVATIVE               // Conservative
,  STATE_NEUTRAL                    // Neutral
,  STATE_AGGRESSIVE                 // Aggressive
,  STATE_BLUFFING                   // Bluffing
,  STATE_SLOWPLAY                   // Slow play
,  STATE_FASTPLAY                   // Fast play
}; // enum State

//----------------------------------------------------------------------------
// PokerStrategy::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~PokerStrategy( void );          // Destructor

   PokerStrategy(                   // Constructor
     PokerPlayer*      player,      // The PokerPlayer
     PokerTable*       table,       // The PokerTable
     Model             model);      // The betting model

private:                            // Bitwise copy is prohibited
   PokerStrategy(const PokerStrategy&); // Disallowed copy constructor
   PokerStrategy& operator=(const PokerStrategy&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// PokerStrategy::Accessor methods
//----------------------------------------------------------------------------
public:
PokerPlayer*                        // ->  associated PokerPlayer
   getPlayer( void ) const;         // Get associated PokerPlayer

PokerTable*                         // ->  associated PokerTable
   getTable( void ) const;          // Get associated PokerTable

void
   setTable(                        // Set the PokerTable
     PokerTable*       table);      // The PokerTable

//----------------------------------------------------------------------------
// PokerStrategy::Methods
//----------------------------------------------------------------------------
public:
virtual int                         // Actual bet
   bet(                             // Bet
     int               amount);     // Minimum to call

virtual void
   debug( void );                   // Debug the PokerStrategy

virtual void
   display( void );                 // Display the PokerStrategy

PokerRating&                        // The (cached) PokerRating
   getRating( void );               // Get (cached) PokerRating

virtual void
   rate( void );                    // Update the rating

virtual void
   reset( void );                   // Reset to default state

//----------------------------------------------------------------------------
// PokerStrategy::Static attributes
//----------------------------------------------------------------------------
protected:
static const char*     stateName[]; // The state names

//----------------------------------------------------------------------------
// PokerStrategy::Attributes
//----------------------------------------------------------------------------
protected:
   Model               model;       // The betting model
   State               state;       // The betting state
   int                 stateRound;  // State last modified round

   // Rating cache
   int                 rateActive;  // Rating active player count
   int                 rateRound;   // Rating round
   PokerRating         rating;      // The associated PokerRating
}; // class PokerStrategy

#include "Strategy.i"

#endif // STRATEGY_H_INCLUDED
