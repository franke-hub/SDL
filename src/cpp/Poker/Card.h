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
//       Card.h
//
// Purpose-
//       Card description.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef CARD_H_INCLUDED
#define CARD_H_INCLUDED

#include "Define.h"

//----------------------------------------------------------------------------
//
// Class-
//       BaseCard
//
// Purpose-
//       Base Card object descriptor.
//
//----------------------------------------------------------------------------
class BaseCard                      // BaseCard
{
//----------------------------------------------------------------------------
// BaseCard::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
enum
{  SIZE_NAME= 32                    // The size of a standard name
};

//----------------------------------------------------------------------------
// BaseCard::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~BaseCard( void );               // Destructor
   BaseCard( void );                // Default constructor

public:                             // Bitwise copy
   BaseCard(const BaseCard&);       // Copy constructor
   BaseCard& operator=(const BaseCard&); // Assignment operator

//----------------------------------------------------------------------------
// BaseCard::Accessor methods
//----------------------------------------------------------------------------
public:
inline void
   display( void ) const;           // Display the short name of the card

inline int                          // The VISIBLE attribute
   getVisible( void ) const;        // Get VISIBLE attribute

inline void
   setVisible(                      // Set VISIBLE attribute
     int               visible);    // The VISIBLE attribute

virtual const char*                 // The short name of the Card
   toShortString(                   // Get short name of the Card
     char*             string) const; // Work area

virtual const char*                 // The name of the Card
   toString(                        // Get name of the Card
     char*             string) const; // Work area

//----------------------------------------------------------------------------
// BaseCard::Methods
//----------------------------------------------------------------------------
public:
inline void
   hide( void );                    // Set VISIBLE attribute FALSE

inline void
   show( void );                    // Set VISIBLE attribute TRUE

//----------------------------------------------------------------------------
// BaseCard::Attributes
//----------------------------------------------------------------------------
protected:
   int                 visible;     // The VISIBLE attribute
}; // class BaseCard

//----------------------------------------------------------------------------
//
// Class-
//       Card
//
// Purpose-
//       (Standard playing) Card object descriptor.
//
//----------------------------------------------------------------------------
class Card : public BaseCard        // Card
{
//----------------------------------------------------------------------------
// Card::Typedefs and enumerations
//----------------------------------------------------------------------------
public:
typedef int            Rank;        // Rank
typedef int            Suit;        // Suit

enum RANK
{  RANK_2
,  RANK_3
,  RANK_4
,  RANK_5
,  RANK_6
,  RANK_7
,  RANK_8
,  RANK_9
,  RANK_T
,  RANK_J
,  RANK_Q
,  RANK_K
,  RANK_A

,  RANK_MIN= RANK_2
,  RANK_MAX= RANK_A
}; // enum RANK

enum SUIT
{  SUIT_CLUBS
,  SUIT_DIAMONDS
,  SUIT_HEARTS
,  SUIT_SPADES

,  SUIT_C= SUIT_CLUBS
,  SUIT_D= SUIT_DIAMONDS
,  SUIT_H= SUIT_HEARTS
,  SUIT_S= SUIT_SPADES

,  SUIT_MIN= SUIT_CLUBS
,  SUIT_MAX= SUIT_SPADES
}; // enum SUIT

//----------------------------------------------------------------------------
// Card::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Card( void );                   // Destructor
   Card(                            // Constructor
     Rank              rank,        // Rank
     Suit              suit);       // Suit

public:                             // Bitwise copy
   Card(const Card&);               // Copy constructor
   Card& operator=(const Card&);    // Assignment operator

//----------------------------------------------------------------------------
// Card::Accessor methods
//----------------------------------------------------------------------------
public:
inline Rank                         // The Rank
   getRank( void ) const;           // Get Rank

inline Suit                         // The Suit
   getSuit( void ) const;           // Get Suit

virtual const char*                 // The short name of the Card
   toShortString(                   // Get short name of the Card
     char*             string) const; // Work area

virtual const char*                 // The name of the Card
   toString(                        // Get name of the Card
     char*             string) const; // Work area

//----------------------------------------------------------------------------
// Card::Methods
//----------------------------------------------------------------------------
public:
static void
   sortByRank(                      // Sort by Rank
     int               count,       // The number of cards in the array
     Card**            array);      // An array of Card*

static void
   sortBySuit(                      // Sort by Suit, then by Rank
     int               count,       // The number of cards in the array
     Card**            array);      // An array of Card*

//----------------------------------------------------------------------------
// Card::Static attributes
//----------------------------------------------------------------------------
public:
static const char*     shortRankName[]; // The short rank name array
static const char*     shortSuitName[]; // The short suit name array
static const char*     rankName[];  // The (long) rank name array
static const char*     suitName[];  // The (long) suit name array

//----------------------------------------------------------------------------
// Card::Attributes
//----------------------------------------------------------------------------
protected:
   Rank                rank;        // The Rank
   Suit                suit;        // The Suit
}; // class Card

#include "Card.i"

#endif // CARD_H_INCLUDED
