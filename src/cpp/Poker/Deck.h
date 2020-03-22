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
//       Deck.h
//
// Purpose-
//       Deck description.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef DECK_H_INCLUDED
#define DECK_H_INCLUDED

#include "Define.h"
#include "Card.h"

//----------------------------------------------------------------------------
//
// Class-
//       BaseDeck
//
// Purpose-
//       Deck object descriptor.
//
//----------------------------------------------------------------------------
class BaseDeck                      // BaseDeck
{
//----------------------------------------------------------------------------
// Deck::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~BaseDeck( void );               // Destructor
   BaseDeck(                        // Constructor
     int               count,       // The number of cards in the Deck
     BaseCard**        deck);       // The cards in the Deck

private:                            // Bitwise copy is prohibited
   BaseDeck(const BaseDeck&);       // Disallowed copy constructor
   BaseDeck& operator=(const BaseDeck&); // Disallowed assignment operator

//----------------------------------------------------------------------------
// BaseDeck::Accessor methods
//----------------------------------------------------------------------------
public:
inline int                          // The number of BaseCards in the Deck
   getCount( void ) const;          // Get number of BaseCards in the Deck

inline BaseCard**                   // The Deck
   getDeck( void );                 // Get the Deck

//----------------------------------------------------------------------------
// BaseDeck::Methods
//----------------------------------------------------------------------------
public:
BaseCard*                           // -> next BaseCard
   deal( void );                    // Deal one BaseCard

virtual void
   check( void ) const;             // Check the Deck

virtual void
   debug( void ) const;             // Debug the Deck

void
   shuffle( void );                 // Shuffle the deck

//----------------------------------------------------------------------------
// BaseDeck::Attributes
//----------------------------------------------------------------------------
protected:
   int                 index;       // The current card index
   int                 count;       // The number of cards in the deck
   BaseCard**          deck;        // The Card array
}; // class BaseDeck

//----------------------------------------------------------------------------
//
// Class-
//       Deck
//
// Purpose-
//       (Standard 52 playing card) Deck object descriptor.
//
//----------------------------------------------------------------------------
class Deck : public BaseDeck        // Deck
{
//----------------------------------------------------------------------------
// Deck::Constructors
//----------------------------------------------------------------------------
public:
virtual
   ~Deck( void );                   // Destructor
   Deck( void );                    // Default constructor

   Deck(                            // Constructor
     int               count,       // The number of cards in the Deck
     Card**            deck);       // The cards in the Deck

private:                            // Bitwise copy is prohibited
   Deck(const Deck&);               // Disallowed copy constructor
   Deck& operator=(const Deck&);    // Disallowed assignment operator

//----------------------------------------------------------------------------
// Deck::Accessor methods
//----------------------------------------------------------------------------
public:
inline Card**                       // The Deck
   getDeck( void );                 // Get the Deck

//----------------------------------------------------------------------------
// Deck::Methods
//----------------------------------------------------------------------------
public:
virtual void
   check( void ) const;             // Check the Deck

Card*                               // -> next Card
   deal( void );                    // Deal one Card

//----------------------------------------------------------------------------
// Deck::Attributes
//----------------------------------------------------------------------------
protected:
}; // class Deck

#include "Deck.i"

#endif // DECK_H_INCLUDED
