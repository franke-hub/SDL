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
//       Deck.i
//
// Purpose-
//       Deck inline methods.
//
// Last change date-
//       2017/01/01
//
//----------------------------------------------------------------------------
#ifndef DECK_I_INCLUDED
#define DECK_I_INCLUDED

//----------------------------------------------------------------------------
// BaseDeck:Accessor methods
//----------------------------------------------------------------------------
int                                 // The number of Cards in the Deck
   BaseDeck::getCount( void ) const // Get number of Cards in the Deck
{
   return count;
}

BaseCard**                          // The Deck
   BaseDeck::getDeck( void )        // Get the Deck
{
   return deck;
}

//----------------------------------------------------------------------------
// Deck:Accessor methods
//----------------------------------------------------------------------------
Card**                              // The Deck
   Deck::getDeck( void )            // Get the Deck
{
   return (Card**)deck;
}

#endif // DECK_I_INCLUDED
