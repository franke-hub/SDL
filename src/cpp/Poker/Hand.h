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
//       Hand.h
//
// Purpose-
//       Hand description.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef HAND_H_INCLUDED
#define HAND_H_INCLUDED

#include "Define.h"

//----------------------------------------------------------------------------
// Forward references
//----------------------------------------------------------------------------
class Card;

//----------------------------------------------------------------------------
//
// Class-
//       Hand
//
// Purpose-
//       Hand object descriptor.
//
//----------------------------------------------------------------------------
class Hand                          // Hand
{
//----------------------------------------------------------------------------
// Hand::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum                                // Generic enum
{  MAX_HAND= 128                    // Largest number of Cards in a Hand
}; // enum

//----------------------------------------------------------------------------
// Hand::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Hand( void );                   // Destructor
   Hand( void );                    // Default constructor
   Hand(                            // Copy constructor
     const Hand&       that);       // Source Hand

//----------------------------------------------------------------------------
// Hand::Operators
//----------------------------------------------------------------------------
public:
Hand&
   operator=(                       // Assignment operator
     const Hand&       that);       // Source Hand

//----------------------------------------------------------------------------
// Hand::Accessor methods
//----------------------------------------------------------------------------
public:
inline int                          // The number of cards in the Hand
   getCount( void ) const;          // Get number of cards in the Hand

inline Card*                        // The associated Card
   getCard(                         // Get associated Card
     int               index) const;// The card index

//----------------------------------------------------------------------------
// Hand::Methods
//----------------------------------------------------------------------------
public:
virtual void
   debug( void ) const;             // Debug the Hand

virtual void
   display( void ) const;           // Display the Hand

virtual void
   load(                            // Load a hand
     int               count,       // Number of cards
     Card*             that[]);     // The Card* array

virtual void
   store(                           // Store a hand
     Card*             that[]) const; // The Card* array

//----------------------------------------------------------------------------
// Hand::Attributes
//----------------------------------------------------------------------------
protected:
   int                 count;       // Number of cards in Hand
   Card*               card[MAX_HAND]; // The Cards in the Hand
}; // class Hand

//----------------------------------------------------------------------------
//
// Class-
//       PokerHand
//
// Purpose-
//       PokerHand object descriptor.
//
//----------------------------------------------------------------------------
class PokerHand : public Hand       // PokerHand
{
//----------------------------------------------------------------------------
// PokerHand::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum Ranking                        // Hand ranking
{  UNRANKED                         // Error, invalid
,  HighCard
,  OnePair
,  TwoPairs
,  ThreeOfAKind
,  Straight
,  Flush
,  FullHouse
,  FourOfAKind
,  StraightFlush
,  FiveOfAKind
,  RANKING_COUNT
}; // enum Ranking

//----------------------------------------------------------------------------
// PokerHand::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~PokerHand( void );              // Destructor
   PokerHand( void );               // Default constructor
   PokerHand(                       // Copy constructor
     const Hand&       that);       // Source Hand
   PokerHand(                       // Constructor
     int               count,       // The number of available Cards
     Card*             that[]);     // An array of available Cards

//----------------------------------------------------------------------------
// PokerHand::Operators
//----------------------------------------------------------------------------
PokerHand&                          // Resultant (*this)
   operator=(                       // Assignment operator
     const Hand&       that);       // Source Hand

inline int                          // Resultant
   operator<(                       // Comparison operator
     const PokerHand&  that);       // Source Hand

inline int                          // Resultant
   operator>(                       // Comparison operator
     const PokerHand&  that);       // Source Hand

inline int                          // Resultant
   operator==(                      // Comparison operator
     const PokerHand&  that);       // Source Hand

inline int                          // Resultant
   operator!=(                      // Comparison operator
     const PokerHand&  that);       // Source Hand

inline int                          // Resultant
   operator<=(                      // Comparison operator
     const PokerHand&  that);       // Source Hand

inline int                          // Resultant
   operator>=(                      // Comparison operator
     const PokerHand&  that);       // Source Hand

//----------------------------------------------------------------------------
// PokerHand::Accessor methods
//----------------------------------------------------------------------------
public:
inline Ranking                      // The associated Ranking
   getRanking( void ) const;        // Get associated Ranking

static inline const char*           // The associated Rank Name
   getRankName(                     // Get associated Rank Name
     Ranking           rank);       // For this Ranking

const inline char*                  // The associated Rank Name
   getRankName( void ) const;       // Get associated Rank Name

//----------------------------------------------------------------------------
// PokerHand::Methods
//----------------------------------------------------------------------------
public:
int                                 // +1 (This better), =0, -1 (This worse)
   compare(                         // Compare Hand
     const PokerHand&  that) const; // With this Hand

static Card*                        // The highest card (NULL if none)
   fourFlush(                       // Four cards to a flush?
     const Hand&       that);       // In this Hand

static Card*                        // The highest card (NULL if none)
   fourFlush(                       // Four cards to a flush?
     int               count,       // Number of cards
     Card*             load[]);     // Card array

static Card*                        // The highest card (NULL if none)
   fourInside(                      // Four cards to an inside straight?
     const Hand&       that);       // In this Hand

static Card*                        // The highest card (NULL if none)
   fourInside(                      // Four cards to an inside straight?
     int               count,       // Number of cards
     Card*             load[]);     // Card array

static Card*                        // The highest card (NULL if none)
   fourOutside(                     // Four cards to an outside straight?
     const Hand&       that);       // In this Hand

static Card*                        // The highest card (NULL if none)
   fourOutside(                     // Four cards to an outside straight?
     int               count,       // Number of cards
     Card*             load[]);     // Card array

virtual void
   load(                            // Load a hand
     int               count,       // Number of cards
     Card*             array[]);    // The Card* array

//----------------------------------------------------------------------------
// PokerHand::Protected Methods
//----------------------------------------------------------------------------
protected:
void
   fill(                            // Fill the winning Hand
     Ranking           ranking,     // For this Ranking
     int               filled,      // Number of cards already filled
     int               source,      // Number of cards in that
     Card*             that[]);     // The Card* array

//----------------------------------------------------------------------------
// PokerHand::Static attributes
//----------------------------------------------------------------------------
protected:
static const char*     rankName[];  // The ranking names

//----------------------------------------------------------------------------
// PokerHand::Attributes
//----------------------------------------------------------------------------
protected:
   Ranking             ranking;     // Current Ranking
}; // class Hand

#include "Hand.i"

#endif // HAND_H_INCLUDED
